#include "stdafxf.h"
#include "FileCache.h"
#include <inttypes.h>
#include "log/Log.h"
#include "utils/AutoPtrHandle.h"
#include "../XRThreads/SystemClock.h"

#define READ_CACHE_CHUNK_SIZE (64*1024)
#define CACHE_MEM_BUFFER_SIZE 20971520

using namespace AUTOPTR;

class WriteRate
{
public:
	WriteRate()
	{
		m_stamp = XR::SystemClockMillis();
		m_pos = 0;
		m_pause = 0;
	}

	void Reset(int64_t pos)
	{
		m_stamp = XR::SystemClockMillis();
		m_pos = pos;
	}

	unsigned Rate(int64_t pos, unsigned int time_bias = 0)
	{
		const unsigned ts = XR::SystemClockMillis() + time_bias;
		if (ts == m_stamp)
			return 0;
		return (unsigned)(1000 * (pos - m_pos) / (ts - m_stamp));
	}

	void Pause()
	{
		m_pause = XR::SystemClockMillis();
	}

	void Resume()
	{
		m_stamp += XR::SystemClockMillis() - m_pause;
		m_pause = 0;
	}

private:
	unsigned m_stamp;
	int64_t  m_pos;
	unsigned m_pause;
};

FileCache::FileCache(bool useDoubleCache) : CThread("FileCache")
{
	m_bDeleteCache = true;
	m_nSeekResult = 0;
	m_seekPos = 0;
	m_readPos = 0;
	m_writePos = 0;
	if (CACHE_MEM_BUFFER_SIZE == 0)
		m_pCache = new SimpleFileCache();
	else
	{
		size_t front = CACHE_MEM_BUFFER_SIZE;
		size_t back = std::max<size_t>(CACHE_MEM_BUFFER_SIZE / 4, 1024 * 1024);
		if (useDoubleCache)
		{
			front = front / 2;
			back = back / 2;
		}
		m_pCache = new CircularCache(front, back);
	}
	if (useDoubleCache)
	{
		m_pCache = new SimpleDoubleCache(m_pCache);
	}
	m_seekPossible = 0;
	m_cacheFull = false;
}

FileCache::FileCache(CacheStrategy *pCache, bool bDeleteCache) : CThread("FileCacheStrategy")
{
	m_pCache = pCache;
	m_bDeleteCache = bDeleteCache;
	m_seekPos = 0;
	m_readPos = 0;
	m_writePos = 0;
	m_nSeekResult = 0;
	m_chunkSize = 0;
}


FileCache::~FileCache()
{
	Close();

	if (m_bDeleteCache && m_pCache)
		delete m_pCache;

	m_pCache = NULL;
}

void FileCache::SetCacheStrategy(CacheStrategy *pCache, bool bDeleteCache)
{
	if (m_bDeleteCache && m_pCache)
		delete m_pCache;

	m_pCache = pCache;
	m_bDeleteCache = bDeleteCache;
}

IFile *FileCache::GetFileImp()
{
	return m_source.GetImplemenation();
}

bool FileCache::Open(const CUrl& url)
{
	Close();

	XR::CSingleLock lock(m_sync);

	LOGDEBUG("CFileCache::Open - opening <%s> using cache", url.GetRedacted().c_str());

	if (!m_pCache)
	{
		LOGERR("CFileCache::Open - no cache strategy defined");
		return false;
	}

	m_sourcePath = url.Get();

	// open cache strategy
	if (m_pCache->Open() != CACHE_RC_OK)
	{
		LOGERR("CFileCache::Open - failed to open cache");
		Close();
		return false;
	}

	// opening the source file.
	if (!m_source.Open(m_sourcePath, READ_NO_CACHE | READ_TRUNCATED | READ_CHUNKED))
	{
		LOGERR("Failed to open source <%s>", url.GetRedacted().c_str());
		Close();
		return false;
	}

	m_source.IoControl(IOCTRL_SET_CACHE, this);

	// check if source can seek
	m_seekPossible = m_source.IoControl(IOCTRL_SEEK_POSSIBLE, NULL);
	m_chunkSize = File::GetChunkSize(m_source.GetChunkSize(), READ_CACHE_CHUNK_SIZE);

	m_readPos = 0;
	m_writePos = 0;
	m_writeRate = 1024 * 1024;
	m_writeRateActual = 0;
	m_cacheFull = false;
	m_seekEvent.Reset();
	m_seekEnded.Reset();

	CThread::Create(false);

	return true;
}

void FileCache::Process()
{
	if (!m_pCache)
	{
		LOGERR("CFileCache::Process - sanity failed. no cache strategy");
		return;
	}

	// create our read buffer
	auto_aptr<char> buffer(new char[m_chunkSize]);
	if (buffer.get() == NULL)
	{
		LOGERR("%s - failed to allocate read buffer", __FUNCTION__);
		return;
	}

	WriteRate limiter;
	WriteRate average;
	bool cacheReachEOF = false;

	while (!m_bStop)
	{
		// check for seek events
		if (m_seekEvent.WaitMSec(0))
		{
			m_seekEvent.Reset();
			int64_t cacheMaxPos = m_pCache->CachedDataEndPosIfSeekTo(m_seekPos);
			cacheReachEOF = cacheMaxPos == m_source.GetLength();
			bool sourceSeekFailed = false;
			if (!cacheReachEOF)
			{
				m_nSeekResult = m_source.Seek(cacheMaxPos, SEEK_SET);
				if (m_nSeekResult != cacheMaxPos)
				{
					LOGERR("CFileCache::Process - Error %d seeking. Seek returned %" PRId64, (int)GetLastError(), m_nSeekResult);
					m_seekPossible = m_source.IoControl(IOCTRL_SEEK_POSSIBLE, NULL);
					sourceSeekFailed = true;
				}
			}
			if (!sourceSeekFailed)
			{
				m_pCache->Reset(m_seekPos, false);
				m_readPos = m_seekPos;
				m_writePos = m_pCache->CachedDataEndPos();
				assert(m_writePos == cacheMaxPos);
				average.Reset(m_writePos);
				limiter.Reset(m_writePos);
				m_cacheFull = false;
				m_nSeekResult = m_seekPos;
			}

			m_seekEnded.Set();
		}

		while (m_writeRate)
		{
			if (m_writePos - m_readPos < m_writeRate)
			{
				limiter.Reset(m_writePos);
				break;
			}

			if (limiter.Rate(m_writePos) < m_writeRate)
				break;

			if (m_seekEvent.WaitMSec(100))
			{
				m_seekEvent.Set();
				break;
			}
		}

		ssize_t iRead = 0;
		if (!cacheReachEOF)
			iRead = m_source.Read(buffer.get(), m_chunkSize);
		if (iRead == 0)
		{
			LOGINFO("CFileCache::Process - Hit eof.");
			m_pCache->EndOfInput();

			// The thread event will now also cause the wait of an event to return a false.
			if (AbortableWait(m_seekEvent) == WAIT_SIGNALED)
			{
				m_pCache->ClearEndOfInput();
				m_seekEvent.Set(); // hack so that later we realize seek is needed
			}
			else
				break;
		}
		else if (iRead < 0)
			m_bStop = true;

		int iTotalWrite = 0;
		while (!m_bStop && (iTotalWrite < iRead))
		{
			int iWrite = 0;
			iWrite = m_pCache->WriteToCache(buffer.get() + iTotalWrite, iRead - iTotalWrite);

			// write should always work. all handling of buffering and errors should be
			// done inside the cache strategy. only if unrecoverable error happened, WriteToCache would return error and we break.
			if (iWrite < 0)
			{
				LOGERR("CFileCache::Process - error writing to cache");
				m_bStop = true;
				break;
			}
			else if (iWrite == 0)
			{
				m_cacheFull = true;
				average.Pause();
				m_pCache->m_space.WaitMSec(5);
				average.Resume();
			}
			else
				m_cacheFull = false;

			iTotalWrite += iWrite;

			// check if seek was asked. otherwise if cache is full we'll freeze.
			if (m_seekEvent.WaitMSec(0))
			{
				m_seekEvent.Set(); // make sure we get the seek event later.
				break;
			}
		}

		m_writePos += iTotalWrite;

		// under estimate write rate by a second, to
		// avoid uncertainty at start of caching
		m_writeRateActual = average.Rate(m_writePos, 1000);
	}
}

void FileCache::OnExit()
{
	m_bStop = true;

	// make sure cache is set to mark end of file (read may be waiting).
	if (m_pCache)
		m_pCache->EndOfInput();

	// just in case someone's waiting...
	m_seekEnded.Set();
}

bool FileCache::Exists(const CUrl& url)
{
	return File::Exists(url.Get());
}

int FileCache::Stat(const CUrl& url, struct __stat64* buffer)
{
	return File::Stat(url.Get(), buffer);
}

ssize_t FileCache::Read(void* lpBuf, int64_t uiBufSize)
{
	XR::CSingleLock lock(m_sync);
	if (!m_pCache)
	{
		LOGERR("Sanity failed. no cache strategy!");
		return -1;
	}
	int64_t iRc;

	if (uiBufSize > SSIZE_MAX)
		uiBufSize = SSIZE_MAX;

retry:
	// attempt to read
	iRc = m_pCache->ReadFromCache((char *)lpBuf, (size_t)uiBufSize);
	if (iRc > 0)
	{
		m_readPos += iRc;
		return (int)iRc;
	}

	if (iRc == CACHE_RC_WOULD_BLOCK)
	{
		// just wait for some data to show up
		iRc = m_pCache->WaitForData(1, 10000);
		if (iRc > 0)
			goto retry;
	}

	if (iRc == CACHE_RC_TIMEOUT)
	{
		LOGWARN("Timeout waiting for data");
		return -1;
	}

	if (iRc == 0)
		return 0;

	// unknown error code
	LOGERR("Cache strategy returned unknown error code %d", (int)iRc);
	return -1;
}

int64_t FileCache::Seek(int64_t iFilePosition, int iWhence)
{
	XR::CSingleLock lock(m_sync);

	if (!m_pCache)
	{
		LOGERR("Sanity failed. no cache strategy!");
		return -1;
	}

	int64_t iCurPos = m_readPos;
	int64_t iTarget = iFilePosition;
	if (iWhence == SEEK_END)
		iTarget = GetLength() + iTarget;
	else if (iWhence == SEEK_CUR)
		iTarget = iCurPos + iTarget;
	else if (iWhence != SEEK_SET)
		return -1;

	if (iTarget == m_readPos)
		return m_readPos;

	if ((m_nSeekResult = m_pCache->Seek(iTarget)) != iTarget)
	{
		if (m_seekPossible == 0)
			return m_nSeekResult;

		/* never request closer to end than 2k, speeds up tag reading */
		m_seekPos = std::min(iTarget, std::max((int64_t)0, m_source.GetLength() - m_chunkSize));

		m_seekEvent.Set();
		if (!m_seekEnded.Wait())
		{
			LOGWARN("Seek to %" PRId64" failed.", m_seekPos);
			return -1;
		}

		/* wait for any remainin data */
		if (m_seekPos < iTarget)
		{
			LOGDEBUG("Waiting for position %" PRId64".", iTarget);
			if (m_pCache->WaitForData((unsigned)(iTarget - m_seekPos), 10000) < iTarget - m_seekPos)
			{
				LOGWARN("Failed to get remaining data", __FUNCTION__);
				return -1;
			}
			m_pCache->Seek(iTarget);
		}
		m_readPos = iTarget;
		m_seekEvent.Reset();
	}
	else
		m_readPos = iTarget;

	return m_nSeekResult;
}

void FileCache::Close()
{
	StopThread();

	XR::CSingleLock lock(m_sync);
	if (m_pCache)
		m_pCache->Close();

	m_source.Close();
}

int64_t FileCache::GetPosition()
{
	return m_readPos;
}

int64_t FileCache::GetLength()
{
	return m_source.GetLength();
}

void FileCache::StopThread(bool bWait /*= true*/)
{
	m_bStop = true;
	//Process could be waiting for seekEvent
	m_seekEvent.Set();
	CThread::StopThread(bWait);
}

std::string FileCache::GetContent()
{
	if (!m_source.GetImplemenation())
		return IFile::GetContent();

	return m_source.GetImplemenation()->GetContent();
}

std::string FileCache::GetContentCharset(void)
{
	IFile* impl = m_source.GetImplemenation();
	if (!impl)
		return IFile::GetContentCharset();

	return impl->GetContentCharset();
}

int FileCache::IoControl(EIoControl request, void* param)
{
	if (request == IOCTRL_CACHE_STATUS)
	{
		SCacheStatus* status = (SCacheStatus*)param;
		status->forward = m_pCache->WaitForData(0, 0);
		status->maxrate = m_writeRate;
		status->currate = m_writeRateActual;
		status->full = m_cacheFull;
		return 0;
	}

	if (request == IOCTRL_CACHE_SETRATE)
	{
		m_writeRate = *(unsigned*)param;
		return 0;
	}

	if (request == IOCTRL_SEEK_POSSIBLE)
		return m_seekPossible;

	return -1;
}

