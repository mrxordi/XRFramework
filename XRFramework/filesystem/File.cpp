#include "stdafxf.h"
#include "File.h"
#include "Util.h"
#include "FileCache.h"
#include "Directory.h"
#include "Win32File.h"
#include "CurlFile.h"

#include "utils/UrlUtils.h"
#include "XRCommon/log/Log.h"
#include "XRCommon/utils/StringUtils.h"
#include "XRCommon/utils/SpecialProtocol.h"



CFile::CFile()
{
	m_pFile = NULL;
	m_flags = 0;
	//m_bitStreamStats = NULL;
	m_pBuffer = NULL;
}

CFile::CFile(const std::string& path, const unsigned int flags /*= 0*/)
{
	m_curl = CURL(path);
	m_flags = flags;
}

CFile::CFile(const CURL& curl, const unsigned int flags /*= 0*/)
{
	m_curl = CURL(curl);
	m_flags = flags;
}


CFile::~CFile()
{
	Close();
	if (m_pFile)
		SAFE_DELETE(m_pFile);
// 	if (m_pBuffer)
// 		SAFE_DELETE(m_pBuffer);
// 	if (m_bitStreamStats)
// 		SAFE_DELETE(m_bitStreamStats);
}

bool CFile::Open()
{
	if (m_pFile)
		Close();

	if (m_curl.Get().empty())
		return false;

	try
	{
		if (!(m_flags & READ_NO_CACHE))
		{
			const std::string pathToUrl(m_curl.Get());
			if (UrlUtils::IsInternetStream(m_curl, true) && !CUtil::IsPicture(pathToUrl))
				m_flags |= READ_CACHED;

			if (m_flags & READ_CACHED)
			{
				//for internet stream, if it contains multiple stream, file cache need handle it specially.
				m_pFile = new CFileCache((m_flags & READ_MULTI_STREAM) == READ_MULTI_STREAM);
				return m_pFile->Open(m_curl);
				//LOGERR("File cache not implemented yet. Copy the file and then you can open the file.");
				return false;
			}
		}

		m_pFile = FileFactory(m_curl);
		if (!m_pFile)
			return false;

		//Catching all exceptions caused in the file opening implementation
		try {
			if (!m_pFile->Open(m_curl))
			{
				SAFE_DELETE(m_pFile);
				return false;
			}
		}
		catch (...)
		{
			LOGERR("Unknown exception when opening %s", m_curl.GetRedacted().c_str());
			SAFE_DELETE(m_pFile);
			return false;
		}
// 		if (m_pFile->GetChunkSize() && !(m_flags & READ_CHUNKED))
// 		{
// 			m_pBuffer = new FileStreamBuffer(0);
// 			m_pBuffer->Attach(m_pFile);
// 		}
// 
// 		if (m_flags & READ_BITRATE)
// 		{
// 			m_bitStreamStats = new BitstreamStats();
// 			m_bitStreamStats->Start();
// 		}

		return true;
	}
	catch (...){
		LOGERR("Error opening %s", m_curl.GetRedacted().c_str());
		return false;
	}

	SAFE_DELETE(m_pFile);
	return false;
}

bool CFile::Open(const CURL& file, const unsigned int flags /*= 0*/)
{
	m_curl = CURL(file);
	m_flags = flags;
	return Open();
}
void CFile::Close()
{
	try
	{
		if (m_pFile)
			m_pFile->Close();

		SAFE_DELETE(m_pBuffer);
		SAFE_DELETE(m_pFile);

	}
	catch (...)
	{
		LOGERR("Unhandled exception while closing file.");
	}
	return;
}

bool CFile::Open(const std::string& strFileName, const unsigned int flags /*= 0*/)
{
	m_curl = CURL(strFileName);
	m_flags = flags;
	return Open();
}

bool CFile::OpenForWrite(const std::string& strFileName, bool bOverWrite /*= false*/)
{
	return OpenForWrite(CURL(strFileName), bOverWrite);
}

bool CFile::OpenForWrite(const CURL& file, bool bOverWrite /*= false*/)
{
	m_curl = CURL(file);
	return OpenForWrite(bOverWrite);
}

bool CFile::OpenForWrite(bool bOverWrite /*= false*/)
{
	if (m_pFile)
		Close();

	//Catching all exceptions caused in the whole file opening operation
	try
	{
		m_pFile = FileFactory(m_curl);
		if (m_pFile && m_pFile->OpenForWrite(m_curl, bOverWrite))
			return true;
	}
	catch (...)
	{
		LOGERR("Unhandled exception opening %s for write.", m_curl.GetRedacted().c_str());
	}

	LOGERR("Error opening %s for write.", m_curl.GetRedacted().c_str());
	SAFE_DELETE(m_pFile);
	return false;
}

ssize_t CFile::LoadFile(const std::string& file, auto_buffer& outputBuffer)
{
	m_curl = CURL(file);
	return LoadFile(outputBuffer);
}

ssize_t CFile::LoadFile(const CURL& file, auto_buffer& outputBuffer)
{
	m_curl = CURL(file);
	return LoadFile(outputBuffer);
}

ssize_t CFile::LoadFile(auto_buffer& outputBuffer)
{
	static const size_t max_file_size = 0x7FFFFFFF;
	static const size_t min_chunk_size = 64 * 1024U;
	static const size_t max_chunk_size = 2048 * 1024U;

	outputBuffer.clear();

	if (!Open(m_curl, READ_TRUNCATED))
		return 0;

	/*
	GetLength() will typically return values that fall into three cases:
	1. The real filesize. This is the typical case.
	2. Zero. This is the case for some http:// streams for example.
	3. Some value smaller than the real filesize. This is the case for an expanding file.

	In order to handle all three cases, we read the file in chunks, relying on Read()
	returning 0 at EOF.  To minimize (re)allocation of the buffer, the chunksize in
	cases 1 and 3 is set to one byte larger than the value returned by GetLength().
	The chunksize in case 2 is set to the lowest value larger than min_chunk_size aligned
	to GetChunkSize().

	We fill the buffer entirely before reallocation.  Thus, reallocation never occurs in case 1
	as the buffer is larger than the file, so we hit EOF before we hit the end of buffer.

	To minimize reallocation, we double the chunksize each read while chunksize is lower
	than max_chunk_size.
	*/

	int64_t filesize = GetLength();
	if (filesize > (int64_t)max_file_size)
	{
		LOGERR("File %s is to large to load into memory. Use other technique to read file.");
		return 0; /* file is too large for this function */
	}

	size_t chunksize = (filesize > 0) ? (size_t)(filesize + 1) : (size_t)GetChunkSize(GetChunkSize(), min_chunk_size);
	size_t total_read = 0;

	while (true)
	{
		if (total_read == outputBuffer.size())
		{ // (re)alloc
			if (outputBuffer.size() + chunksize > max_file_size)
			{
				outputBuffer.clear();
				return -1;
			}
			outputBuffer.resize(outputBuffer.size() + chunksize);
			if (chunksize < max_chunk_size)
				chunksize *= 2;
		}
		ssize_t read = Read((char*)outputBuffer.get() + total_read, outputBuffer.size() - total_read);
		if (read < 0)
		{
			outputBuffer.clear();
			return -1;
		}
		total_read += read;
		if (!read)
			break;
	}

	outputBuffer.resize(total_read);
	Close();

	return total_read;
}

ssize_t CFile::Read(void* bufPtr, size_t bufSize)
{
	if (!m_pFile)
		return -1;
	if (bufPtr == NULL && bufSize != 0)
		return -1;

	if (bufSize > SSIZE_MAX)
		bufSize = SSIZE_MAX;

/*
	if (m_pBuffer)
	{
		if (m_flags & READ_TRUNCATED)
		{
			const ssize_t nBytes = m_pBuffer->sgetn(
				(char *)lpBuf, std::min<streamsize>((streamsize)uiBufSize,
				m_pBuffer->in_avail()));
			if (m_bitStreamStats && nBytes > 0)
				m_bitStreamStats->AddSampleBytes(nBytes);
			return nBytes;
		}
		else
		{
			const ssize_t nBytes = m_pBuffer->sgetn((char*)lpBuf, uiBufSize);
			if (m_bitStreamStats && nBytes > 0)
				m_bitStreamStats->AddSampleBytes(nBytes);
			return nBytes;
		}
	}*/

	try
	{
		if (m_flags & READ_TRUNCATED)
		{
			const ssize_t nBytes = m_pFile->Read(bufPtr, bufSize);
/*			if (m_bitStreamStats && nBytes > 0)					//Implement bitstream stats
				m_bitStreamStats->AddSampleBytes(nBytes);*/
			return nBytes;
		}
		else
		{
			ssize_t done = 0;
			while ((bufSize - done) > 0)
			{
				const ssize_t curr = m_pFile->Read((char*)bufPtr + done, bufSize - done);
				if (curr <= 0)
				{
					if (curr < 0 && done == 0)
						return -1;

					break;
				}
				done += curr;
			}
/*			if (m_bitStreamStats && done > 0)					//Implement bitstream stats
				m_bitStreamStats->AddSampleBytes(done);*/
			return done;
		}

	}
	catch (...)
	{
		LOGERR("Unhandled exception while reading file.");
		return -1;
	}

	return 0;
}

bool CFile::ReadString(char *szLine, int iLineLength)
{
	if (!m_pFile || !szLine)
		return false;

	try {
		return m_pFile->ReadString(szLine, iLineLength);
	}
	catch (...)
	{
		LOGERR("Unhandled exception while reading string from file.");

	}
	return false;
}

ssize_t CFile::Write(const void* bufPtr, size_t bufSize)
{
	if (!m_pFile)
		return -1;
	if (bufPtr == NULL && bufSize != 0)
		return -1;

	try {
		return m_pFile->Write(bufPtr, bufSize);
	}
	catch (...)
	{
		LOGERR("Unhandled exception while writing to file.");
	}
	return -1;
}

int64_t CFile::Seek(int64_t iFilePosition, int iWhence /*= SEEK_SET*/)
{
	if (!m_pFile)
		return -1;

/*	if (m_pBuffer)
	{
		if (iWhence == SEEK_CUR)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::cur);
		else if (iWhence == SEEK_END)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::end);
		else if (iWhence == SEEK_SET)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::beg);
	}*/

	try 
	{
		return m_pFile->Seek(iFilePosition, iWhence);
	}
	catch (...)
	{
		LOGERR("Unhandled exception while seeking file.");
	}
	return -1;
}

void CFile::Flush()
{
	try {
		if (m_pFile)
			m_pFile->Flush();
	}
	catch (...)
	{
		LOGERR("Unhandled exception while flushing file.");
	}
	return;
}

int64_t CFile::GetPosition() const
{
	if (!m_pFile)
		return -1;

// 	if (m_pBuffer)
// 		return m_pBuffer->pubseekoff(0, ios_base::cur);

	try {
		return m_pFile->GetPosition();
	}
	catch (...)
	{
		LOGERR("Unhandled exception while getting position in file.");
	}
	return -1;
}

int64_t CFile::GetLength()
{
	try {
		if (m_pFile)
			return m_pFile->GetLength();
		return 0;
	}
	catch (...)
	{
		LOGERR("Unhandled exception while getting size of file.");
	}
	return 0;
}

int CFile::Truncate(int64_t iSize)
{
	if (!m_pFile)
		return -1;

	try {
		return m_pFile->Truncate(iSize);
	}
	catch (...)
	{
		LOGERR("Unhandled exception while trunkate file.");
	}
	return -1;
}

bool CFile::Exist()
{
	if (!m_pFile)
		return false;

	try 
	{
		return m_pFile->Exists(m_curl);
	}
	catch (...)
	{
		LOGERR("Unhandled exception checking %s does exist.", m_curl.GetRedacted().c_str());
	}
	LOGERR("Error checking for %s does exist.", m_curl.GetRedacted().c_str());
	return false;
}

bool CFile::Delete()
{
	if (!m_pFile)
		return false;

	try {
		if (m_pFile->Delete(m_curl))
			return true;
	}
	catch (...)
	{
		LOGERR("Unhandled exception checking %s does exist.", m_curl.GetRedacted().c_str());
	}
	LOGERR("Error checking for %s does exist.", m_curl.GetRedacted().c_str());
	return false;
}

bool CFile::Rename(const std::string& newFileName)
{
	if (!m_curl.Get().empty())
	{
		const CURL newUrlFileName(newFileName);
		return CFile::Rename(m_curl, newUrlFileName);
	}
	return false;
}

int CFile::Stat(struct __stat64 *buffer)
{
	if (!buffer)
		return -1;

	if (!m_pFile)
	{
		memset(buffer, 0, sizeof(struct __stat64));
		errno = ENOENT;
		return -1;
	}

	return m_pFile->Stat(buffer);
}

int CFile::GetChunkSize()
{
	if (m_pFile)
		return m_pFile->GetChunkSize();
	return 0;
}

std::string CFile::GetContentMimeType(void)
{
	if (!m_pFile)
		return "";
	return m_pFile->GetContent();
}

std::string CFile::GetContentCharset(void)
{
	if (!m_pFile)
		return "";
	return m_pFile->GetContent();
}

int CFile::IoControl(EIoControl request, void* param)
{
	int result = -1;
	if (m_pFile == NULL)
		return -1;
	result = m_pFile->IoControl(request, param);

	if (result == -1 && request == IOCTRL_SEEK_POSSIBLE)
	{
		if (m_pFile->GetLength() >= 0 && m_pFile->Seek(0, SEEK_CUR) >= 0)
			return 1;
		else
			return 0;
	}

	return result;
}

#pragma region CURL interface
int CFile::Stat(const std::string& strFileName, struct __stat64* buffer)
{
	const CURL pathToUrl(strFileName);
	return Stat(pathToUrl, buffer);
}
int CFile::Stat(const CURL& file, struct __stat64* buffer)
{
	if (!buffer)
		return -1;

	CURL url(file);

	try {
		std::auto_ptr<IFile> pFile(FileFactory(url));
		if (!pFile.get())
			return -1;
		pFile->Open(url);
		return pFile->Stat(buffer);
	}
	catch (...)
	{
		LOGERR("Unhandled exception while statting file.");
	}
	return -1;
}

bool CFile::Exists(const std::string& strFileName, bool bUseCache /* = true */)
{
	const CURL pathToUrl(strFileName);
	return Exists(pathToUrl, bUseCache);
}
bool CFile::Exists(const CURL& url, bool bUseCache /* = true */)
{
	CURL url2(url);

	try {

		std::auto_ptr<IFile>pFile(FileFactory(url2));
		if (!pFile.get())
			return false;

		return pFile->Exists(url2);
	}
	catch (...)
	{
		LOGERR("Unhandled exception checking %s does exist.", url.GetRedacted().c_str());
	}
	LOGERR("Error checking for %s does exist.", url.GetRedacted().c_str());
	return false;
}

bool CFile::Delete(const std::string& strFileName)
{
	const CURL pathToUrl(strFileName);
	return Delete(pathToUrl);
}

bool CFile::Delete(const CURL& url)
{
	CURL url2(url);

	try {
		std::auto_ptr<IFile>pFile(FileFactory(url2));
		if (!pFile.get())
			return false;

		if (pFile->Delete(url2))
			return true;
	}
	catch (...)
	{
		LOGERR("Unhandled exception checking %s does exist.", url.GetRedacted().c_str());
	}
	LOGERR("Error checking for %s does exist.", url.GetRedacted().c_str());
	return false;
}

bool CFile::Rename(const std::string& strFileName, const std::string& strNewFileName)
{
	const CURL pathToUrl(strFileName);
	const CURL pathToUrlNew(strNewFileName);
	return Rename(pathToUrl, pathToUrlNew);
}

bool CFile::Rename(const CURL& url, const CURL& newUrl)
{
	CURL url2 = CURL(url);
	CURL newurl2 = CURL(newUrl);

	try {
		std::auto_ptr<IFile> pFile(FileFactory(url2));
		if (!pFile.get())
			return false;
		if (pFile->Rename(url2, newurl2))
		{
			return true;
		}
	}
	catch (...)
	{
		LOGERR("Unhandled exception renaming the file %s.", url.GetRedacted().c_str());
	}

	LOGERR("Error renaming the file %s.", url.GetRedacted().c_str());
	return false;
}

bool CFile::Copy(const std::string& strFileName, const std::string& strDest, IFileCallback* pCallback, void* pContext)
{
	const CURL pathToUrl(strFileName);
	const CURL pathToUrlDest(strDest);
	return Copy(pathToUrl, pathToUrlDest, pCallback, pContext);
}
bool CFile::Copy(const CURL& url2, const CURL& dest, IFileCallback* pCallback, void* pContext)
{
	CFile file;

	const std::string pathToUrl(dest.Get());
	if (pathToUrl.empty())
		return false;

	CURL url(url2);
	if (file.Open(url.Get(), READ_TRUNCATED))
	{
		CFile newFile;

		if (UrlUtils::IsHD(pathToUrl)) // create possible missing dirs
		{
			std::vector<std::string> tokens;
			std::string strDirectory = UrlUtils::GetDirectory(pathToUrl);
			UrlUtils::RemoveSlashAtEnd(strDirectory);  // for the test below
			if (!(strDirectory.size() == 2 && strDirectory[1] == ':'))
			{
				CURL url(strDirectory);
				std::string pathsep;
				pathsep = "/";
				StringUtils::Tokenize(url.GetFileName(), tokens, pathsep.c_str());
				std::string strCurrPath;
				// Handle special
				if (!url.GetProtocol().empty())
				{
					pathsep = "/";
					strCurrPath += url.GetProtocol() + "://";
				} // If the directory has a / at the beginning, don't forget it
				else if (strDirectory[0] == pathsep[0])
					strCurrPath += pathsep;
				for (std::vector<std::string>::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
				{
					strCurrPath += *iter + pathsep;
					CDirectory::Create(strCurrPath);
				}
			}
		}

		if (CFile::Exists(dest))
			CFile::Delete(dest);

		if (!newFile.OpenForWrite(dest, true))
		{  // overwrite always
			file.Close();
			return false;
		}

		static const int iBufferSize = 128 * 1024;

		auto_buffer buffer(iBufferSize);
		ssize_t iRead, iWrite;

		UINT64 llFileSize = file.GetLength();
		UINT64 llPos = 0;


		float start = XR::SystemClockMillis() / 1000.0f;   //Perfor some checks here

		while (true)
		{
			iRead = file.Read(buffer.get(), iBufferSize);
			if (iRead == 0)
				break;

			else if (iRead < 0)
			{
				LOGERR("Failed read from file %s", url2.GetRedacted().c_str());
				llFileSize = (uint64_t)-1;
				break;
			}

			/* write data and make sure we managed to write it all */
			iWrite = 0;
			while (iWrite < iRead)
			{
				ssize_t iWrite2 = newFile.Write((char*)buffer.get() + iWrite, iRead - iWrite);
				if (iWrite2 <= 0)
					break;
				iWrite += iWrite2;
			}

			if (iWrite != iRead)
			{
				LOGERR("Failed write to file %s", dest.GetRedacted().c_str());
				llFileSize = (uint64_t)-1;
				break;
			}

			llPos += iRead;
			// calculate the current and average speeds
			float end = XR::SystemClockMillis() / 1000.0f;

			if (pCallback && end - start > 0.5 && end)
			{
				start = end;

				float averageSpeed = llPos / end;
				int ipercent = 0;
				if (llFileSize)
					ipercent = 100 * llPos / llFileSize;

				if (!pCallback->OnFileCallback(pContext, ipercent, averageSpeed))
				{
					LOGERR("User aborted copy file.");
					llFileSize = (uint64_t)-1;
					break;
				}
			}
		}

		/* close both files */
		newFile.Close();
		file.Close();

		/* verify that we managed to completed the file */
		if (llFileSize && llPos != llFileSize)
		{
			CFile::Delete(dest);
			return false;
		}
		return true;
	}
	return false;
}

bool CFile::SetHidden(const std::string& fileName, bool hidden)
{
	const CURL pathToUrl(fileName);
	return SetHidden(pathToUrl, hidden);
}
bool CFile::SetHidden(const CURL& url, bool hidden)
{
	CURL url2(url);

	try {
		std::auto_ptr<IFile>pFile(FileFactory(url2));
		if (!pFile.get())
			return false;

		if (pFile->SetHidden(url2, hidden))
			return true;
	}
	catch (...)
	{
		LOGERR("Unhandled exception checking %s does exist.", url.GetRedacted().c_str());
	}
	LOGERR("Error checking for %s does exist.", url.GetRedacted().c_str());
	return false;

}
#pragma endregion


IFile* CFile::FileFactory(const std::string& strFileName)
{
	CURL url(strFileName);
	return FileFactory(url);
}
IFile* CFile::FileFactory(CURL& url)
{
	if (url.IsProtocol("special"))
	{
		url = CURL(CSpecialProtocol::TranslatePath(url));
		return new CWin32File();
	}
	else if (url.IsProtocol("file") || url.GetProtocol().empty())
		return new CWin32File();
	else if (url.IsProtocol("http") || url.IsProtocol("https"))
		return new CCurlFile();

	LOGWARN("Unsupported protocol(%s) in %s", url.GetProtocol().c_str(), url.GetRedacted().c_str());
	return NULL;
}

