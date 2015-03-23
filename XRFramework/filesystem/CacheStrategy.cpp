#include "stdafxf.h"
#include "CacheStrategy.h"
#include "Win32File.h"
#include "utils/SpecialProtocol.h"
#include "Util.h"
#include "FrameworkUtils.h"
#include "utils/URL.h"
#include "utils/StringConverter.h"

////////////////////////////////////////////////////////////////////////////////////
// CacheStrategy
////////////////////////////////////////////////////////////////////////////////////
CacheStrategy::CacheStrategy() : m_bEndOfInput(false)
{}

CacheStrategy::~CacheStrategy()
{}

void CacheStrategy::EndOfInput() 
{
	m_bEndOfInput = true;
}

bool CacheStrategy::IsEndOfInput()
{
	return m_bEndOfInput;
}

void CacheStrategy::ClearEndOfInput()
{
	m_bEndOfInput = false;
}

////////////////////////////////////////////////////////////////////////////////////
// SimpleFileCache
////////////////////////////////////////////////////////////////////////////////////

SimpleFileCache::SimpleFileCache()
	: m_cacheFileRead(new CWin32File())
	, m_cacheFileWrite(new CWin32File())
	, m_hDataAvailEvent(NULL)
	, m_nStartPosition(0)
	, m_nWritePosition(0)
	, m_nReadPosition(0) {}

SimpleFileCache::~SimpleFileCache()
{
	Close();
	delete m_cacheFileRead;
	delete m_cacheFileWrite;
}

int SimpleFileCache::Open()
{
	Close();

	m_hDataAvailEvent = new CEvent;

	m_filename = CSpecialProtocol::TranslatePath(CFUtil::GetNextFilename("special://temp/filecache%03d.cache", 999));
	if (m_filename.empty())
	{
		LOGERR("Unable to generate a new filename");
		Close();
		return CACHE_RC_ERROR;
	}

	CURL fileURL(m_filename);

	if (!m_cacheFileWrite->OpenForWrite(fileURL, false))
	{
		LOGERR("failed to create file \"%s\" for writing", m_filename.c_str());
		Close();
		return CACHE_RC_ERROR;
	}

	if (!m_cacheFileRead->Open(fileURL))
	{
		LOGERR("failed to open file \"%s\" for reading", m_filename.c_str());
		Close();
		return CACHE_RC_ERROR;
	}

	return CACHE_RC_OK;
}

void SimpleFileCache::Close()
{
	if (m_hDataAvailEvent)
		delete m_hDataAvailEvent;

	m_hDataAvailEvent = NULL;

	m_cacheFileWrite->Close();
	m_cacheFileRead->Close();

	if (!m_cacheFileRead->Delete(CURL(m_filename)))
		LOGWARN("Failed to delete temporary file \"%s\"", m_filename.c_str());

	m_filename.clear();
}

int SimpleFileCache::WriteToCache(const char *pBuffer, size_t iSize)
{
	size_t written = 0;
	while (iSize > 0)
	{
		const ssize_t lastWritten = m_cacheFileWrite->Write(pBuffer, (iSize > SSIZE_MAX) ? SSIZE_MAX : iSize);
		if (lastWritten <= 0)
		{
			LOGERR("failed to write to file");
			return CACHE_RC_ERROR;
		}
		m_nWritePosition += lastWritten;
		iSize -= lastWritten;
		written += lastWritten;
	}

	// when reader waits for data it will wait on the event.
	m_hDataAvailEvent->Set();

	return written;
}

int64_t SimpleFileCache::GetAvailableRead()
{
	return m_nWritePosition - m_nReadPosition;
}

int SimpleFileCache::ReadFromCache(char *pBuffer, size_t iMaxSize)
{
	int64_t iAvailable = GetAvailableRead();
	if (iAvailable <= 0)
		return m_bEndOfInput ? 0 : CACHE_RC_WOULD_BLOCK;

	size_t toRead = ((int64_t)iMaxSize > iAvailable) ? (size_t)iAvailable : iMaxSize;

	size_t readBytes = 0;
	while (toRead > 0)
	{
		const ssize_t lastRead = m_cacheFileRead->Read(pBuffer, (toRead > SSIZE_MAX) ? SSIZE_MAX : toRead);
		if (lastRead == 0)
			break;
		if (lastRead < 0)
		{
			LOGERR("failed to read from file");
			return CACHE_RC_ERROR;
		}
		m_nReadPosition += lastRead;
		toRead -= lastRead;
		readBytes += lastRead;
	}

	if (readBytes > 0)
		m_space.Set();

	return readBytes;
}

int64_t SimpleFileCache::WaitForData(unsigned int iMinAvail, unsigned int iMillis)
{
	if (iMillis == 0 || IsEndOfInput())
		return GetAvailableRead();

	XR::EndTime endTime(iMillis);
	while (!IsEndOfInput())
	{
		int64_t iAvail = GetAvailableRead();
		if (iAvail >= iMinAvail)
			return iAvail;

		if (!m_hDataAvailEvent->WaitMSec(endTime.MillisLeft()))
			return CACHE_RC_TIMEOUT;
	}
	return GetAvailableRead();
}

int64_t SimpleFileCache::Seek(int64_t iFilePosition)
{
	int64_t iTarget = iFilePosition - m_nStartPosition;

	if (iTarget < 0)
	{
		LOGDEBUG("CSimpleFileCache::Seek, request seek before start of cache.");
		return CACHE_RC_ERROR;
	}

	int64_t nDiff = iTarget - m_nWritePosition;
	if (nDiff > 500000 || (nDiff > 0 && WaitForData((unsigned int)(iTarget - m_nReadPosition), 5000) == CACHE_RC_TIMEOUT))
	{
		LOGDEBUG("CSimpleFileCache::Seek - Attempt to seek past read data");
		return CACHE_RC_ERROR;
	}

	m_nReadPosition = m_cacheFileRead->Seek(iTarget, SEEK_SET);
	if (m_nReadPosition != iTarget)
	{
		LOGERR("Can't seek file");
		return CACHE_RC_ERROR;
	}

	m_space.Set();

	return iFilePosition;
}

void SimpleFileCache::Reset(int64_t iSourcePosition, bool clearAnyway)
{
	if (!clearAnyway && IsCachedPosition(iSourcePosition))
	{
		m_nReadPosition = m_cacheFileRead->Seek(iSourcePosition - m_nStartPosition, SEEK_SET);
		return;
	}

	m_nStartPosition = iSourcePosition;
	m_nWritePosition = m_cacheFileWrite->Seek(0, SEEK_SET);
	m_nReadPosition = m_cacheFileRead->Seek(0, SEEK_SET);
}

void SimpleFileCache::EndOfInput()
{
	CacheStrategy::EndOfInput();
	m_hDataAvailEvent->Set();
}

int64_t SimpleFileCache::CachedDataEndPosIfSeekTo(int64_t iFilePosition)
{
	if (iFilePosition >= m_nStartPosition && iFilePosition <= m_nStartPosition + m_nWritePosition)
		return m_nStartPosition + m_nWritePosition;
	return iFilePosition;
}

int64_t SimpleFileCache::CachedDataEndPos()
{
	return m_nStartPosition + m_nWritePosition;
}

bool SimpleFileCache::IsCachedPosition(int64_t iFilePosition)
{
	return iFilePosition >= m_nStartPosition && iFilePosition <= m_nStartPosition + m_nWritePosition;
}

CacheStrategy *SimpleFileCache::CreateNew()
{
	return new SimpleFileCache();
}

////////////////////////////////////////////////////////////////////////////////////
// SimpleDoubleCache
////////////////////////////////////////////////////////////////////////////////////
SimpleDoubleCache::SimpleDoubleCache(CacheStrategy *impl)
{
	assert(NULL != impl);
	m_pCache = impl;
	m_pCacheOld = NULL;
}

SimpleDoubleCache::~SimpleDoubleCache()
{
	delete m_pCache;
	delete m_pCacheOld;
}

int SimpleDoubleCache::Open()
{
	return m_pCache->Open();
}

void SimpleDoubleCache::Close()
{
	m_pCache->Close();
	if (m_pCacheOld)
	{
		delete m_pCacheOld;
		m_pCacheOld = NULL;
	}
}

int SimpleDoubleCache::WriteToCache(const char *pBuffer, size_t iSize)
{
	return m_pCache->WriteToCache(pBuffer, iSize);
}

int SimpleDoubleCache::ReadFromCache(char *pBuffer, size_t iMaxSize)
{
	return m_pCache->ReadFromCache(pBuffer, iMaxSize);
}

int64_t SimpleDoubleCache::WaitForData(unsigned int iMinAvail, unsigned int iMillis)
{
	return m_pCache->WaitForData(iMinAvail, iMillis);
}

int64_t SimpleDoubleCache::Seek(int64_t iFilePosition)
{
	return m_pCache->Seek(iFilePosition);
}

void SimpleDoubleCache::Reset(int64_t iSourcePosition, bool clearAnyway)
{
	if (!clearAnyway && m_pCache->IsCachedPosition(iSourcePosition)
		&& (!m_pCacheOld || !m_pCacheOld->IsCachedPosition(iSourcePosition)
		|| m_pCache->CachedDataEndPos() >= m_pCacheOld->CachedDataEndPos()))
	{
		m_pCache->Reset(iSourcePosition, clearAnyway);
		return;
	}
	if (!m_pCacheOld)
	{
		CacheStrategy *pCacheNew = m_pCache->CreateNew();
		if (pCacheNew->Open() != CACHE_RC_OK)
		{
			delete pCacheNew;
			m_pCache->Reset(iSourcePosition, clearAnyway);
			return;
		}
		pCacheNew->Reset(iSourcePosition, clearAnyway);
		m_pCacheOld = m_pCache;
		m_pCache = pCacheNew;
		return;
	}
	m_pCacheOld->Reset(iSourcePosition, clearAnyway);
	CacheStrategy *tmp = m_pCacheOld;
	m_pCacheOld = m_pCache;
	m_pCache = tmp;
}

void SimpleDoubleCache::EndOfInput()
{
	m_pCache->EndOfInput();
}

bool SimpleDoubleCache::IsEndOfInput()
{
	return m_pCache->IsEndOfInput();
}

void SimpleDoubleCache::ClearEndOfInput()
{
	m_pCache->ClearEndOfInput();
}

int64_t SimpleDoubleCache::CachedDataEndPos()
{
	return m_pCache->CachedDataEndPos();
}

int64_t SimpleDoubleCache::CachedDataEndPosIfSeekTo(int64_t iFilePosition)
{
	int64_t ret = m_pCache->CachedDataEndPosIfSeekTo(iFilePosition);
	if (m_pCacheOld)
		return std::max(ret, m_pCacheOld->CachedDataEndPosIfSeekTo(iFilePosition));
	return ret;
}

bool SimpleDoubleCache::IsCachedPosition(int64_t iFilePosition)
{
	return m_pCache->IsCachedPosition(iFilePosition) || (m_pCacheOld && m_pCacheOld->IsCachedPosition(iFilePosition));
}

CacheStrategy *SimpleDoubleCache::CreateNew()
{
	return new SimpleDoubleCache(m_pCache->CreateNew());
}

////////////////////////////////////////////////////////////////////////////////////
// CircularCache
////////////////////////////////////////////////////////////////////////////////////
CircularCache::CircularCache(size_t front, size_t back)
	: CacheStrategy()
	, m_beg(0)
	, m_end(0)
	, m_cur(0)
	, m_buf(NULL)
	, m_size(front + back)
	, m_size_back(back)
	, m_handle(INVALID_HANDLE_VALUE)
{
}

CircularCache::~CircularCache()
{
	Close();
}

int CircularCache::Open()
{
	m_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_size, NULL);
	if (m_handle == NULL)
		return CACHE_RC_ERROR;
	m_buf = (uint8_t*)MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_buf == 0)
		return CACHE_RC_ERROR;
	m_beg = 0;
	m_end = 0;
	m_cur = 0;
	return CACHE_RC_OK;
}

void CircularCache::Close()
{
#ifdef TARGET_WINDOWS
	UnmapViewOfFile(m_buf);
	CloseHandle(m_handle);
	m_handle = INVALID_HANDLE_VALUE;
#else
	delete[] m_buf;
#endif
	m_buf = NULL;
}

/**
* Function will write to m_buf at m_end % m_size location
* it will write at maximum m_size, but it will only write
* as much it can without wrapping around in the buffer
*
* It will always leave m_size_back of the backbuffer intact
* but if the back buffer is less than that, that space is
* usable to write.
*
* If back buffer is filled to an larger extent than
* m_size_back, it will allow it to be overwritten
* until only m_size_back data remains.
*
* The following always apply:
*  * m_end <= m_cur <= m_end
*  * m_end - m_beg <= m_size
*
* Multiple calls may be needed to fill buffer completely.
*/
int CircularCache::WriteToCache(const char *buf, size_t len)
{
	XR::CSingleLock lock(m_sync);

	// where are we in the buffer
	size_t pos = m_end % m_size;
	size_t back = (size_t)(m_cur - m_beg);
	size_t front = (size_t)(m_end - m_cur);

	size_t limit = m_size - std::min(back, m_size_back) - front;
	size_t wrap = m_size - pos;

	// limit by max forward size
	if (len > limit)
		len = limit;

	// limit to wrap point
	if (len > wrap)
		len = wrap;

	if (len == 0)
		return 0;

	// write the data
	memcpy(m_buf + pos, buf, len);
	m_end += len;

	// drop history that was overwritten
	if (m_end - m_beg > (int64_t)m_size)
		m_beg = m_end - m_size;

	m_written.Set();

	return len;
}

/**
* Reads data from cache. Will only read up till
* the buffer wrap point. So multiple calls
* may be needed to empty the whole cache
*/
int CircularCache::ReadFromCache(char *buf, size_t len)
{
	XR::CSingleLock lock(m_sync);

	size_t pos = m_cur % m_size;
	size_t front = (size_t)(m_end - m_cur);
	size_t avail = std::min(m_size - pos, front);

	if (avail == 0)
	{
		if (IsEndOfInput())
			return 0;
		else
			return CACHE_RC_WOULD_BLOCK;
	}

	if (len > avail)
		len = avail;

	if (len == 0)
		return 0;

	memcpy(buf, m_buf + pos, len);
	m_cur += len;

	m_space.Set();

	return len;
}

int64_t CircularCache::WaitForData(unsigned int minumum, unsigned int millis)
{
	XR::CSingleLock lock(m_sync);
	int64_t avail = m_end - m_cur;

	if (millis == 0 || IsEndOfInput())
		return avail;

	if (minumum > m_size - m_size_back)
		minumum = m_size - m_size_back;

	XR::EndTime endtime(millis);
	while (!IsEndOfInput() && avail < minumum && !endtime.IsTimePast())
	{
		lock.Leave();
		m_written.WaitMSec(50); // may miss the deadline. shouldn't be a problem.
		lock.Enter();
		avail = m_end - m_cur;
	}

	return avail;
}

int64_t CircularCache::Seek(int64_t pos)
{
	XR::CSingleLock lock(m_sync);

	// if seek is a bit over what we have, try to wait a few seconds for the data to be available.
	// we try to avoid a (heavy) seek on the source
	if (pos >= m_end && pos < m_end + 100000)
	{
		lock.Leave();
		WaitForData((size_t)(pos - m_cur), 5000);
		lock.Enter();
	}

	if (pos >= m_beg && pos <= m_end)
	{
		m_cur = pos;
		return pos;
	}

	return CACHE_RC_ERROR;
}

void CircularCache::Reset(int64_t pos, bool clearAnyway)
{
	XR::CSingleLock lock(m_sync);
	if (!clearAnyway && IsCachedPosition(pos))
	{
		m_cur = pos;
		return;
	}
	m_end = pos;
	m_beg = pos;
	m_cur = pos;
}

int64_t CircularCache::CachedDataEndPosIfSeekTo(int64_t iFilePosition)
{
	if (IsCachedPosition(iFilePosition))
		return m_end;
	return iFilePosition;
}

int64_t CircularCache::CachedDataEndPos()
{
	return m_end;
}

bool CircularCache::IsCachedPosition(int64_t iFilePosition)
{
	return iFilePosition >= m_beg && iFilePosition <= m_end;
}

CacheStrategy *CircularCache::CreateNew()
{
	return new CircularCache(m_size - m_size_back, m_size_back);
}