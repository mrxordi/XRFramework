#include "stdafxf.h"
#include "File.h"
#include "Base.h"
#include "IFile.h"
#include "MimeTypes.h"
#include "log/Log.h"
#include "utils/UrlUtils.h"
#include "utils/Timer.h"
#include "utils/StringUtils.h"
#include "Util.h"
#include "utils/URL.h"
#include "utils/SpecialProtocol.h"
#include "../XRThreads/SystemClock.h"
#include "FileCache.h"
#include "CurlFile.h"
#include "Win32File.h"
#include "Directory.h"

class FileStreamBuffer;

File::File(){}

File::~File(){
	SAFE_DELETE(m_pFile);
}

bool File::Open(const std::string& strFileName, const unsigned int flags)
{
	const CUrl pathToUrl(strFileName);
	return Open(pathToUrl, flags);
}
bool File::Open(const CUrl& url, const unsigned int flags)
{
	CUrl url2(url);
	m_flags = flags;
	
	//Catching all exceptions caused in the whole file opening operation
	try {

		if (!(m_flags & READ_NO_CACHE)) 
		{
			const std::string pathToUrl(url2.Get());
			if (UrlUtils::IsInternetStream(url2, true) && !CUtil::IsPicture(pathToUrl))
				m_flags |= READ_CACHED;

			if (m_flags & READ_CACHED)
			{
				//for internet stream, if it contains multiple stream, file cache need handle it specially.
				m_pFile = new FileCache((m_flags & READ_MULTI_STREAM) == READ_MULTI_STREAM);
				return m_pFile->Open(url);
				//LOGERR("File cache not implemented yet. Copy the file and then you can open the file.");
				return false;
			}
		}

		m_pFile = FileFactory(url2);
		if (!m_pFile)
			return false;

		//Catching all exceptions caused in the file opening implementation
		try {
			if (!m_pFile->Open(url2)) 
			{
				SAFE_DELETE(m_pFile);
				return false;
			}
		}
		catch (...) 
		{
			LOGERR("Unknown exception when opening %s", url.GetRedacted().c_str());
			SAFE_DELETE(m_pFile);
			return false;
		}

		if (m_pFile->GetChunkSize() && !(m_flags & READ_CHUNKED))
		{
			m_pBuffer = new FileStreamBuffer(0);
			m_pBuffer->Attach(m_pFile);
		}

		if (m_flags & READ_BITRATE)
		{
			m_bitStreamStats = new BitstreamStats();
			m_bitStreamStats->Start();
		}

		return true;
	}
	catch (...){
		LOGERR("Error opening %s", url.GetRedacted().c_str());
		return false;
	}

	SAFE_DELETE(m_pFile);
	return false;
}

bool File::OpenForWrite(const std::string& strFileName, bool bOverWrite)
{
	const CUrl pathToUrl(strFileName);
	return OpenForWrite(pathToUrl, bOverWrite);
}
bool File::OpenForWrite(const CUrl& file, bool bOverWrite)
{
	CUrl url(file);
	//Catching all exceptions caused in the whole file opening operation
	try 
	{
		m_pFile = FileFactory(url);
		if (m_pFile && m_pFile->OpenForWrite(url, bOverWrite)) 
			return true;
	} 
	catch (...) 
	{
		LOGERR("Unhandled exception opening %s for write.", url.GetRedacted().c_str());
	}

	LOGERR("Error opening %s for write.", url.GetRedacted().c_str());
	SAFE_DELETE(m_pFile);
	return false;
}

ssize_t File::LoadFile(const std::string &filename, auto_buffer& outputBuffer)
{
	const CUrl pathToUrl(filename);
	return LoadFile(pathToUrl, outputBuffer);
}
ssize_t File::LoadFile(const CUrl& file, auto_buffer& outputBuffer)
{ 
	static const size_t max_file_size = 0x7FFFFFFF;
	static const size_t min_chunk_size = 64 * 1024U;
	static const size_t max_chunk_size = 2048 * 1024U;

	outputBuffer.clear();

	if (!Open(file, READ_TRUNCATED))
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

ssize_t File::Read(void *lpBuf, size_t uiBufSize)
{
	if (!m_pFile)
		return -1;
	if (lpBuf == NULL && uiBufSize != 0)
		return -1;

	if (uiBufSize > SSIZE_MAX)
		uiBufSize = SSIZE_MAX;

	if (m_pBuffer)
	{
		if (m_flags & READ_TRUNCATED)
		{
			const ssize_t nBytes = m_pBuffer->sgetn(
				(char *)lpBuf, std::min<streamsize>((streamsize)uiBufSize,
				m_pBuffer->in_avail()));
			if (m_bitStreamStats && nBytes>0)
				m_bitStreamStats->AddSampleBytes(nBytes);
			return nBytes;
		}
		else
		{
			const ssize_t nBytes = m_pBuffer->sgetn((char*)lpBuf, uiBufSize);
			if (m_bitStreamStats && nBytes>0)
				m_bitStreamStats->AddSampleBytes(nBytes);
			return nBytes;
		}
	}

	try 
	{
		if (m_flags & READ_TRUNCATED)
		{
			const ssize_t nBytes = m_pFile->Read(lpBuf, uiBufSize);
			if (m_bitStreamStats && nBytes>0)					//Implement bitstream stats
				m_bitStreamStats->AddSampleBytes(nBytes);
			return nBytes;
		}
		else
		{
			ssize_t done = 0;
			while ((uiBufSize - done) > 0)
			{
				const ssize_t curr = m_pFile->Read((char*)lpBuf + done, uiBufSize - done);
				if (curr <= 0)
				{
					if (curr < 0 && done == 0)
						return -1;

					break;
				}
				done += curr;
			}
			if (m_bitStreamStats && done > 0)					//Implement bitstream stats
				m_bitStreamStats->AddSampleBytes(done);
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

bool File::ReadString(char *szLine, int iLineLength)
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

ssize_t File::Write(const void* lpBuf, size_t uiBufSize)
{
	if (!m_pFile)
		return -1;
	if (lpBuf == NULL && uiBufSize != 0)
		return -1;

	try {
		return m_pFile->Write(lpBuf, uiBufSize);
	}
	catch (...) 
	{
		LOGERR("Unhandled exception while writing to file.");
	}
	return -1;
}

void File::Flush()
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

int64_t File::Seek(int64_t iFilePosition, int iWhence)
{
	if (!m_pFile)
		return -1;

	if (m_pBuffer)         
	{
		if (iWhence == SEEK_CUR)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::cur);
		else if (iWhence == SEEK_END)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::end);
		else if (iWhence == SEEK_SET)
			return m_pBuffer->pubseekoff(iFilePosition, ios_base::beg);
	}

	try {
		return m_pFile->Seek(iFilePosition, iWhence);
	}
	catch (...) 
	{
		LOGERR("Unhandled exception while seeking file.");
	}
	return -1;
}

int File::Truncate(int64_t iSize)
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

int64_t File::GetPosition() const
{
	if (!m_pFile)
		return -1;

	if (m_pBuffer)
		return m_pBuffer->pubseekoff(0, ios_base::cur);

	try {
		return m_pFile->GetPosition();
	}
	catch (...) 
	{
		LOGERR("Unhandled exception while getting position in file.");
	}
	return -1;
}

int64_t File::GetLength()
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

void File::Close()
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

int File::GetChunkSize()
{
	if (m_pFile)
		return m_pFile->GetChunkSize();
	return 0;
}

std::string File::GetContentMimeType(void)
{
	if (!m_pFile)
		return "";
	return m_pFile->GetContent();
}

std::string File::GetContentCharset(void)
{
	if (!m_pFile)
		return "";
	return m_pFile->GetContentCharset();
}

bool File::SkipNext()
{
	if (m_pFile)
		return m_pFile->SkipNext();
	return false;
}

int File::IoControl(EIoControl request, void* param)
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

int File::Stat(struct __stat64 *buffer)
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

int File::Stat(const std::string& strFileName, struct __stat64* buffer)
{
	const CUrl pathToUrl(strFileName);
	return Stat(pathToUrl, buffer);
}
int File::Stat(const CUrl& file, struct __stat64* buffer)
{
	if (!buffer)
		return -1;

	CUrl url(file);

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

bool File::Exists(const std::string& strFileName, bool bUseCache /* = true */)
{
	const CUrl pathToUrl(strFileName);
	return Exists(pathToUrl, bUseCache);
}
bool File::Exists(const CUrl& url, bool bUseCache /* = true */)
{
	CUrl url2(url);

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

bool File::Delete(const std::string& strFileName)
{
	const CUrl pathToUrl(strFileName);
	return Delete(pathToUrl);
}

bool File::Delete(const CUrl& url)
{
	CUrl url2(url);

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

bool File::Rename(const std::string& strFileName, const std::string& strNewFileName)
{
	const CUrl pathToUrl(strFileName);
	const CUrl pathToUrlNew(strNewFileName);
	return Rename(pathToUrl, pathToUrlNew);
}

bool File::Rename(const CUrl& url, const CUrl& newUrl)
{
		CUrl url2 = CUrl(url);
		CUrl newurl2 = CUrl(newUrl);

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

bool File::Copy(const std::string& strFileName, const std::string& strDest, IFileCallback* pCallback, void* pContext)
{
	const CUrl pathToUrl(strFileName);
	const CUrl pathToUrlDest(strDest);
	return Copy(pathToUrl, pathToUrlDest, pCallback, pContext);
}
bool File::Copy(const CUrl& url2, const CUrl& dest, IFileCallback* pCallback, void* pContext)
{ 
	File file;

	const std::string pathToUrl(dest.Get());
	if (pathToUrl.empty())
		return false;

	CUrl url(url2);
	if (file.Open(url.Get(), READ_TRUNCATED)) 
	{
		File newFile;

		if (UrlUtils::IsHD(pathToUrl)) // create possible missing dirs
		{
			std::vector<std::string> tokens;
			std::string strDirectory = UrlUtils::GetDirectory(pathToUrl);
			UrlUtils::RemoveSlashAtEnd(strDirectory);  // for the test below
			if (!(strDirectory.size() == 2 && strDirectory[1] == ':'))
			{
				CUrl url(strDirectory);
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
					Directory::Create(strCurrPath);
				}
			}
		}

		if (File::Exists(dest))
			File::Delete(dest);

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


		float start = XR::SystemClockMillis()/1000.0f;   //Perfor some checks here

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
			float end = XR::SystemClockMillis()/1000.0f;

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
			File::Delete(dest);
			return false;
		}
		return true;
	}
	return false;
}

bool File::SetHidden(const std::string& fileName, bool hidden)
{
	const CUrl pathToUrl(fileName);
	return SetHidden(pathToUrl, hidden);
}
bool File::SetHidden(const CUrl& url, bool hidden)
{
	CUrl url2(url);

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

IFile* File::FileFactory(const std::string& strFileName) 
{
	CUrl url(strFileName);
	return FileFactory(url);
}
IFile* File::FileFactory(CUrl& url) 
{
	if (url.IsProtocol("special")) 
	{
		url = CUrl(CSpecialProtocol::TranslatePath(url));
		return new CWin32File();
	}
	else if (url.IsProtocol("file") || url.GetProtocol().empty())
		return new CWin32File();
	else if (url.IsProtocol("http") || url.IsProtocol("https"))
		return new CCurlFile();

	LOGWARN("Unsupported protocol(%s) in %s", url.GetProtocol().c_str(), url.GetRedacted().c_str());
	return NULL;
}

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
FileStreamBuffer::~FileStreamBuffer()
{
	sync();
	Detach();
}

FileStreamBuffer::FileStreamBuffer(int backsize)
	: streambuf()
	, m_file(NULL)
	, m_buffer(NULL)
	, m_backsize(backsize)
	, m_frontsize(0)
{
}

void FileStreamBuffer::Attach(IFile *file)
{
	m_file = file;
	m_frontsize = File::GetChunkSize(m_file->GetChunkSize(), 64 * 1024);

	m_buffer = new char[m_frontsize + m_backsize];
	setg(0, 0, 0);
	setp(0, 0);
}

void FileStreamBuffer::Detach()
{
	setg(0, 0, 0);
	setp(0, 0);
	delete[] m_buffer;
	m_buffer = NULL;
}

FileStreamBuffer::int_type FileStreamBuffer::underflow()
{
	if (gptr() < egptr())
		return traits_type::to_int_type(*gptr());

	if (!m_file)
		return traits_type::eof();

	size_t backsize = 0;
	if (m_backsize)
	{
		backsize = (size_t)min<ptrdiff_t>((ptrdiff_t)m_backsize, egptr() - eback());
		memmove(m_buffer, egptr() - backsize, backsize);
	}

	unsigned int size = m_file->Read(m_buffer + backsize, m_frontsize);

	if (size == 0)
		return traits_type::eof();

	setg(m_buffer, m_buffer + backsize, m_buffer + backsize + size);
	return traits_type::to_int_type(*gptr());
}

FileStreamBuffer::pos_type FileStreamBuffer::seekoff(
	off_type offset,
	ios_base::seekdir way,
	ios_base::openmode mode)
{
	// calculate relative offset
	off_type pos = m_file->GetPosition() - (egptr() - gptr());
	off_type offset2;
	if (way == ios_base::cur)
		offset2 = offset;
	else if (way == ios_base::beg)
		offset2 = offset - pos;
	else if (way == ios_base::end)
		offset2 = offset + m_file->GetLength() - pos;
	else
		return streampos(-1);

	// a non seek shouldn't modify our buffer
	if (offset2 == 0)
		return pos;

	// try to seek within buffer
	if (gptr() + offset2 >= eback() && gptr() + offset2 < egptr())
	{
		gbump(offset2);
		return pos + offset2;
	}

	// reset our buffer pointer, will
	// start buffering on next read
	setg(0, 0, 0);
	setp(0, 0);

	int64_t position = -1;
	if (way == ios_base::cur)
		position = m_file->Seek(offset, SEEK_CUR);
	else if (way == ios_base::end)
		position = m_file->Seek(offset, SEEK_END);
	else
		position = m_file->Seek(offset, SEEK_SET);

	if (position<0)
		return streampos(-1);

	return position;
}

FileStreamBuffer::pos_type FileStreamBuffer::seekpos(
	pos_type pos,
	ios_base::openmode mode)
{
	return seekoff(pos, ios_base::beg, mode);
}

streamsize FileStreamBuffer::showmanyc()
{
	underflow();
	return egptr() - gptr();
}

/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
// very basic file input stream
FileStream::FileStream(int backsize /*= 0*/) : istream(&m_buffer), m_buffer(backsize), m_file(NULL)
{
}

FileStream::~FileStream()
{
	Close();
}

bool FileStream::Open(const CUrl& filename)
{
	Close();

	CUrl url(filename);
	m_file = File::FileFactory(url);
	if (m_file && m_file->Open(url))
	{
		m_buffer.Attach(m_file);
		return true;
	}

	setstate(failbit);
	return false;
}

int64_t FileStream::GetLength()
{
	return m_file->GetLength();
}

void FileStream::Close()
{
	if (!m_file)
		return;

	m_buffer.Detach();
	SAFE_DELETE(m_file);
}

bool FileStream::Open(const std::string& filename)
{
	const CUrl pathToUrl(filename);
	return Open(pathToUrl);
}
