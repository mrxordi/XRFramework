#include "stdafxf.h"
#include <inttypes.h>
#include "BufferedStream.h"

CBufferedStream::CBufferedStream() : IInputStream(EStreamType::DVDSTREAM_TYPE_RTMPBUFFERED), CThread("CBufferedStream")
{
	m_nSeekResult = 0;
	m_seekPos = 0;
	m_readPos = 0;
	m_writePos = 0;
	m_streamCache = new CStreamCache;
	m_sourceStream = new CRTMPStream;
	m_bData = new uint8_t[m_buffersize];
   LOGINFO("Creating Buffered steam.");
}

CBufferedStream::~CBufferedStream()
{
   Close();
	SAFE_DELETE_ARRAY(m_bData);
   
   SAFE_DELETE(m_sourceStream);
   SAFE_DELETE(m_streamCache);
   LOGINFO("Destroing Buffered steam.");

}

bool CBufferedStream::Open(const char* strFileName, const std::string& content)
{
	Close();

	XR::CSingleLock lock(m_sync);
	IInputStream::Open(strFileName, content);

	// opening the source stream.
	if (!m_sourceStream->Open(strFileName, content))
	{
		LOGERR("Failed to open source rtmp stream");
		Close();
		return false;
	}
	LOGDEBUG("Opening <%s> using cache.", m_strFileName.c_str());
	if (!m_streamCache)
	{
		LOGERR("Cache object not created.");
		return false;
	}
	if (m_streamCache->Open() != 0) {
		LOGERR("Failed to open cache.");
		Close();
		return false;
	}

	m_readPos = 0;
	m_writePos = 0;
	
	m_seekEvent.Reset();
	m_seekEnded.Reset();
	m_content = m_sourceStream->GetContent();

	CThread::Create(false);

	return true;
}

void CBufferedStream::Close()
{
   if (IsRunning())
	   StopThread();

	XR::CSingleLock lock(m_sync);
	if (m_streamCache)
		m_streamCache->Close();

	m_sourceStream->Close();

	m_CachedKeyframes.clear();
}

size_t CBufferedStream::Read(uint8_t* buf, size_t buf_size)
{
	XR::CSingleLock lock(m_sync);
	if (!m_streamCache)
	{
		LOGERR("Sanity failed. no cache strategy!");
		return -1;
	}
   if (!IsRunning())
   {
      LOGERR("Sanity failed. Buffered Stream is not running! No data to read.");
      return -1;
   }
	int64_t iRc;

	if (buf_size > SSIZE_MAX)
		buf_size = SSIZE_MAX;

	do {
		// attempt to read
		iRc = m_streamCache->ReadFromCache((char *)buf, (size_t)buf_size);
		if (iRc > 0)
		{
			m_readPos += iRc;
			return (int)iRc;
		}

		if (iRc == -2)
		{
			// just wait for some data to show up
			iRc = m_streamCache->WaitForData(1, 10000);
		}
	} while (iRc>0);

	if (iRc == -3)
	{
		LOGWARN("Timeout waiting for data", __FUNCTION__);
		return -1;
	}

	if (iRc == 0)
		return 0;

	// unknown error code
	LOGERR("Cache strategy returned unknown error code %" PRId64 ".", iRc);
	return -1;
}

//************************************
// Method:    Seek
// FullName:  CBufferedStream::Seek
// Access:    virtual public 
// Returns:   int64_t
// Qualifier: Function returns position after seek, as flv can seek only key frames
// Parameter: int64_t offset - Indicates offset from the begining of file
// Parameter: int whence - SEEK_SET, SEEK_CUR, SEEK_END
//************************************
int64_t CBufferedStream::Seek(int64_t offset, int whence)
{
	XR::CSingleLock lock(m_sync);

	if (!m_streamCache)
	{
		LOGERR("Sanity failed while seeking. no cache strategy running!");
		return -1;
	}
   if (!IsRunning())
   {
      LOGERR("Sanity failed. Buffered Stream is not running! No data to read.");
      return -1;
   }
	int64_t iCurPos = m_readPos;
	int64_t iTarget = offset;
	if (whence == SEEK_END)
		iTarget = GetLength() + iTarget;
	else if (whence == SEEK_CUR)
		iTarget = iCurPos + iTarget;
	else if (whence == SEEK_POSSIBLE)
		return true;
	else if (whence != SEEK_SET)
		return -1;

	if (iTarget == m_readPos)
		return m_readPos;

	if ((m_nSeekResult = m_streamCache->Seek(iTarget)) != iTarget)
	{
		return 0;
	}
	else
		m_readPos = iTarget;
	return m_nSeekResult;
}

//************************************
// Method:    SeekTime
// FullName:  CBufferedStream::SeekTime
// Access:    virtual public 
// Returns:   int64_t
// Qualifier: Function returns time after seek, as flv can seek only key frames
//			  so the time can be different than requested
// Parameter: int64_t time_ms
//************************************
int64_t CBufferedStream::SeekTime(int64_t time_ms)
{
	if (!m_streamCache)
	{
		LOGERR("Sanity failed while seeking. no cache strategy!");
		return -1;
	}
   if (!IsRunning())
   {
      LOGERR("Sanity failed. Buffered Stream is not running! No data to read.");
      return -1;
   }

	int64_t seek = 0;
	auto it = m_CachedKeyframes.begin();

	for (; it < (m_CachedKeyframes.end()-1); it++)
	{
		int64_t second = (it + 1)->first;
		if (it->first <= time_ms && second > time_ms) {
			seek = it->second;
			break;
		} 			
	}

	if (it == m_CachedKeyframes.end()-1) {
		LOGWARN("Tried to seek behind buffered position.");
		seek = (m_CachedKeyframes.end()-2)->second;
	}

	seek = Seek(seek, SEEK_SET);
	if (seek <= 0) {
		LOGERR("Failed to SeekTime(%" PRId64 "); returning NULL.", time_ms);
		return 0;
	}

	return seek;
}


bool CBufferedStream::Pause(double dTime)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int64_t CBufferedStream::GetLength()
{
	return -1;
}

bool CBufferedStream::IsEOF()
{
	if (m_streamCache->IsEndOfInput())
		m_bEof = true;
	return m_bEof;
}

// BitstreamStats CBufferedStream::GetBitstreamStats() const
// {
// 
// }

void CBufferedStream::StopThread(bool bWait /*= true*/)
{
	m_bStop = true;
	//Process could be waiting for seekEvent
	m_seekEvent.Set();
	CThread::StopThread(bWait);
}

void CBufferedStream::OnException()
{

}

void CBufferedStream::OnStartup()
{

}

void CBufferedStream::OnExit()
{
	m_bStop = true;

	// make sure cache is set to mark end of file (read may be waiting).
	if (m_streamCache)
		m_streamCache->EndOfInput();

	// just in case someone's waiting...
	m_seekEnded.Set();
}

void CBufferedStream::Process()
{
	if (!m_streamCache)
	{
		LOGERR("Sanity Failed. Cache object not created.");
		return;
	}
	if (!m_bData)
	{
		LOGERR("Sanity Failed. Data buffer not created.");
		return;
	}

	bool cacheReachEOF = false;

	while (!m_bStop)
	{
		ssize_t iRead = 0;
		if (!m_sourceStream->IsEOF())
		{
			iRead = m_sourceStream->Read(m_bData, m_buffersize);
			if (iRead == 0)
			{
				LOGINFO("Hit eof.");
				m_streamCache->EndOfInput();
				m_bStop = true;
			}
			else if (iRead < 0) {
				LOGINFO("RTMP error while reading packet.");
				m_bStop = true;
			}

			if (!m_bStop) {
				ParsePacketHeader(m_bData);

				int iWrite = m_streamCache->WriteToCache((char*)m_bData, iRead);

				if (iWrite < 0)
				{
					LOGERR("Process - error writing to cache");
					m_bStop = true;
					break;
				}
				m_writePos += iWrite;
			}
		}
	}
}

void CBufferedStream::ParsePacketHeader(const uint8_t* pData)
{
	if (pData[0] == 0x09 && ((pData[11] & 0xF0) >> 4) == 1) {
		uint32_t ts = AMF_DecodeInt24((char*)pData + 4);
		ts |= (pData[7] << 24);
		LOGINFO("I have video keyframe, timestamp: %u, at cache offset %" PRId64 ".", ts, m_writePos);
		XR::CSingleLock lock(m_sync);
		m_CachedKeyframes.push_back(make_pair<>(ts, m_writePos));

	}
}

const std::string& CBufferedStream::GetFileName()
{
	return m_strFileName;
}

int CBufferedStream::IoControl(EIoControl request, void* param)
{
   if (m_streamCache)
      return m_streamCache->IoControl(request, param);
   return -1;
}
