#include "stdafxf.h"
#include "BufferedRTMPStream.h"


CBufferedRTMPStream::CBufferedRTMPStream()
{
}


CBufferedRTMPStream::~CBufferedRTMPStream()
{
}

bool CBufferedRTMPStream::Open(const char* strFile, const std::string &content)
{
	if (!CRTMPStream::Open(strFile, ""))
		return false;

	m_cacheUrl = CURL("special://app/cache.bin");
	m_fCache.Open(m_cacheUrl);
}

bool CBufferedRTMPStream::Open(const CFileItem& fileItem)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void CBufferedRTMPStream::Close()
{
	throw std::logic_error("The method or operation is not implemented.");
}

int CBufferedRTMPStream::Read(uint8_t* buf, int buf_size)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int64_t CBufferedRTMPStream::Seek(int64_t offset, int whence)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CBufferedRTMPStream::Pause(double dTime)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CBufferedRTMPStream::IsEOF()
{
	throw std::logic_error("The method or operation is not implemented.");
}

int64_t CBufferedRTMPStream::GetLength()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void CBufferedRTMPStream::Abort()
{
	throw std::logic_error("The method or operation is not implemented.");
}

int CBufferedRTMPStream::GetBlockSize()
{
	throw std::logic_error("The method or operation is not implemented.");
}

BitstreamStats CBufferedRTMPStream::GetBitstreamStats() const
{
	throw std::logic_error("The method or operation is not implemented.");
}
