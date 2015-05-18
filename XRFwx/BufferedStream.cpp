#include "stdafxf.h"
#include <inttypes.h>
#include "BufferedStream.h"


CBufferedStream::CBufferedStream() : IInputStream(EStreamType::DVDSTREAM_TYPE_RTMP), CThread("CBufferedStream")
{
	m_nSeekResult = 0;
	m_seekPos = 0;
	m_readPos = 0;
	m_writePos = 0;
	m_streamCache = new CStreamCache;
	m_sourceStream = new CRTMPStream;
	m_bData = new uint8_t[m_buffersize];
}


CBufferedStream::~CBufferedStream()
{
	SAFE_DELETE_ARRAY(m_bData);
}

bool CBufferedStream::Open(const char* strFileName, const std::string& content)
{
	Close();

	XR::CSingleLock lock(m_sync);
	IInputStream::Open(strFileName, content);
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
	// opening the source stream.
	if (!m_sourceStream->Open(strFileName, content))
	{
		LOGERR("Failed to open source rtmp stream");
		return false;
	}
	m_readPos = 0;
	m_writePos = 0;
	
	m_seekEvent.Reset();
	m_seekEnded.Reset();

	CThread::Create(false);

	return true;
}

void CBufferedStream::Close()
{
	StopThread();

	XR::CSingleLock lock(m_sync);
	if (m_streamCache)
		m_streamCache->Close();

	m_sourceStream->Close();
}

int CBufferedStream::Read(uint8_t* buf, int buf_size)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int64_t CBufferedStream::Seek(int64_t offset, int whence)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CBufferedStream::Pause(double dTime)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int64_t CBufferedStream::GetLength()
{
	return m_writePos;
}

void CBufferedStream::Abort()
{

}

bool CBufferedStream::IsEOF()
{
	return m_streamCache->IsEndOfInput();
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
