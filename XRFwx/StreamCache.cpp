#include "stdafxf.h"
#include "StreamCache.h"
#include "XRFramework/filesystem/Win32File.h"
#include "XRFramework/FrameworkUtils.h"
#include "XRCommon/utils/SpecialProtocol.h"


CStreamCache::CStreamCache() : m_cacheFileRead(new CWin32File())
, m_cacheFileWrite(new CWin32File())
, m_hDataAvailEvent(nullptr), m_nWritePosition(0)
, m_nReadPosition(0)
, m_bEndOfInput(false)
{
}

CStreamCache::~CStreamCache()
{
	Close();

	delete m_cacheFileRead;
	delete m_cacheFileWrite;
}

int CStreamCache::Open()
{
	Close();
   m_hDataAvailEvent = new CEvent;

	m_filename = CSpecialProtocol::TranslatePath(CFUtil::GetNextFilename("special://temp/filecache%03d.cache", 999));
	if (m_filename.empty())
	{
		LOGERR("Unable to generate a new filename");
		Close();
		return -1;
	}

	CURL fileURL(m_filename);

	if (!m_cacheFileWrite->OpenForWrite(fileURL, false))
	{
		LOGERR("failed to create file \"%s\" for writing", m_filename.c_str());
		Close();
		return -1;
	}

	if (!m_cacheFileRead->Open(fileURL))
	{
		LOGERR("Failed to open file \"%s\" for reading", m_filename.c_str());
		Close();
		return -1;
	}

	return 0;
}

void CStreamCache::Close()
{
	if (m_hDataAvailEvent)
		delete m_hDataAvailEvent;

	m_hDataAvailEvent = NULL;

	m_cacheFileWrite->Close();
	m_cacheFileRead->Close();

// 	if (!m_filename.empty() && !m_cacheFileRead->Delete(CURL(m_filename)))
// 		LOGWARN("Failed to delete temporary file \"%s\"", m_filename.c_str());

	m_filename.clear();
}

bool CStreamCache::Reset()
{
	m_bEndOfInput = false;
	m_nWritePosition = m_cacheFileWrite->Seek(0, SEEK_SET);
	m_nReadPosition = m_cacheFileRead->Seek(0, SEEK_SET);
	return true;
}

int CStreamCache::WriteToCache(const char *pBuffer, size_t iSize)
{
	size_t written = 0;
	while (iSize > 0)
	{
		const ssize_t lastWritten = m_cacheFileWrite->Write(pBuffer, (iSize > SSIZE_MAX) ? SSIZE_MAX : iSize);
		if (lastWritten <= 0)
		{
			LOGERR("failed to write to file");
			return -1;
		}
		m_nWritePosition += lastWritten;
		iSize -= lastWritten;
		written += lastWritten;
	}

	// when reader waits for data it will wait on the event.
	m_hDataAvailEvent->Set();

	return written;
}

int CStreamCache::ReadFromCache(char *pBuffer, size_t iMaxSize)
{
	int64_t iAvailable = GetAvailableRead();
	if (iAvailable <= 0)
		return m_bEndOfInput ? 0 : -2;

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
			return -1;
		}
		m_nReadPosition += lastRead;
		toRead -= lastRead;
		readBytes += lastRead;
	}
	return readBytes;
}

int64_t CStreamCache::WaitForData(unsigned int iMinAvail, unsigned int iMillis)
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
			return -1;
	}
	return GetAvailableRead();
}

int64_t CStreamCache::CachedDataEndPos()
{
	return m_nWritePosition;
}

bool CStreamCache::IsCachedPosition(int64_t iFilePosition)
{
	return iFilePosition <= m_nWritePosition;
}

int64_t CStreamCache::GetAvailableRead()
{
	return m_nWritePosition - m_nReadPosition;
}

void CStreamCache::EndOfInput()
{
	m_bEndOfInput = true;
	m_hDataAvailEvent->Set();
	Close();
}

int64_t CStreamCache::Seek(int64_t iFilePosition)
{

	if (iFilePosition < 0)
	{
		LOGDEBUG("CSimpleFileCache::Seek, request seek before start of cache.");
		return -1;
	}

	int64_t nDiff = iFilePosition - m_nWritePosition;
	if (nDiff > 500000 || (nDiff > 0 && WaitForData((unsigned int)(iFilePosition - m_nReadPosition), 5000) == -1))
	{
		LOGDEBUG("Attempt to seek past read data");
		return -1;
	}

	m_nReadPosition = m_cacheFileRead->Seek(iFilePosition, SEEK_SET);
	if (m_nReadPosition != iFilePosition)
	{
		LOGERR("Can't seek file");
		return -1;
	}

	return iFilePosition;
}

bool CStreamCache::IsEndOfInput()
{
	return m_bEndOfInput;
}

int CStreamCache::IoControl(EIoControl request, void* param)
{
   int result = -1;
   if (m_cacheFileRead == NULL)
      return -1;
   result = m_cacheFileRead->IoControl(request, param);

   if (result == -1 && request == IOCTRL_SEEK_POSSIBLE)
   {
      if (m_cacheFileRead->GetLength() >= 0 && m_cacheFileRead->Seek(0, SEEK_CUR) >= 0)
         return 1;
      else
         return 0;
   }

   return result;
}
