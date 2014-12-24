#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include "Base.h"
#include "IFileTypes.h"
#include "utils/URL.h"
#include "utils/AutoBuffer.h"
#include "utils/BitstreamStats.h"

class IFile;

class IFileCallback
{
public:
	virtual bool OnFileCallback(void* pContext, int ipercent, float avgSpeed) = 0;
	virtual ~IFileCallback() {};
};

/* indicate that caller can handle truncated reads, where function returns before entire buffer has been filled */
#define READ_TRUNCATED 0x01

/* indicate that that caller support read in the minimum defined chunk size, this disables internal cache then */
#define READ_CHUNKED   0x02

/* use cache to access this file */
#define READ_CACHED     0x04

/* open without caching. regardless to file type. */
#define READ_NO_CACHE  0x08

/* calcuate bitrate for file while reading */
#define READ_BITRATE   0x10

/* indicate the caller will seek between multiple streams in the file frequently */
#define READ_MULTI_STREAM 0x20

class FileStreamBuffer;

/////////////////////////////////////////////////////////////////////////////////////////
// CFile definition
/////////////////////////////////////////////////////////////////////////////////////////
class File
{
public:
	File();
	~File();

	bool Open(const std::string& strFileName, const unsigned int flags = 0);
	bool Open(const CURL& file, const unsigned int flags = 0);

	bool OpenForWrite(const std::string& strFileName, bool bOverWrite = false);
	bool OpenForWrite(const CURL& file, bool bOverWrite = false);

	//Function returns how large in memory is the file.
	ssize_t LoadFile(const std::string &filename, auto_buffer& outputBuffer);
	ssize_t LoadFile(const CURL &file, auto_buffer& outputBuffer);

	//Attempt to read bufSize bytes from currently opened file into buffer bufPtr.
	//Error returns -1, num bytes readed return when success
	ssize_t Read(void* bufPtr, size_t bufSize);

	bool ReadString(char *szLine, int iLineLength);

	//Attempt to write bufSize bytes from buffer bufPtr into currently opened file.
	ssize_t Write(const void* bufPtr, size_t bufSize);

	void Flush();
	int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
	int Truncate(int64_t iSize);
	int64_t GetPosition() const;
	int64_t GetLength();
	void Close();
	int GetChunkSize();

	std::string GetContentMimeType(void);
	std::string GetContentCharset(void);

	// will return a size, that is aligned to chunk size
	// but always greater or equal to the file's chunk size
	static int GetChunkSize(int chunk, int minimum) {
		if (chunk)
			return chunk * ((minimum + chunk - 1) / chunk);
		else
			return minimum;
	}

	bool SkipNext();
	BitstreamStats* GetBitstreamStats() { return m_bitStreamStats; }

	int IoControl(EIoControl request, void* param);

	IFile *GetImplemenation() { return m_pFile; }

	int Stat(struct __stat64 *buffer);

	// CURL interface
	/**
	* Fills struct __stat64 with information about file specified by filename
	* For st_mode function will set correctly _S_IFDIR (directory) flag and may set
	* _S_IREAD (read permission), _S_IWRITE (write permission) flags if such
	* information is available. Function may set st_size (file size), st_atime,
	* st_mtime, st_ctime (access, modification, creation times).
	* Any other flags and members of __stat64 that didn't updated with actual file
	* information will be set to zero (st_nlink can be set ether to 1 or zero).
	* @param strFileName specifies requested file
	* @param buffer      pointer to __stat64 buffer to receive information about file
	* @return zero of success, -1 otherwise.
	*/
	static int  Stat(const std::string& strFileName, struct __stat64* buffer);
	static int  Stat(const CURL& file, struct __stat64* buffer);

	static bool Exists(const std::string& strFileName, bool bUseCache = true);
	static bool Exists(const CURL& file, bool bUseCache = true);

	static bool Delete(const std::string& strFileName);
	static bool Delete(const CURL& file);

	static bool Rename(const std::string& strFileName, const std::string& strNewFileName);
	static bool Rename(const CURL& file, const CURL& urlNew);

	static bool Copy(const std::string& strFileName, const std::string& strDest, IFileCallback* pCallback = NULL, void* pContext = NULL);
	static bool Copy(const CURL& file, const CURL& dest, IFileCallback* pCallback = NULL, void* pContext = NULL);

	static bool SetHidden(const std::string& fileName, bool hidden);
	static bool SetHidden(const CURL& file, bool hidden);

	static IFile* FileFactory(const std::string& strFileName);
	static IFile* FileFactory(CURL& url);

private:
	unsigned int m_flags;
	IFile* m_pFile;

	FileStreamBuffer* m_pBuffer;
	BitstreamStats* m_bitStreamStats;
};

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
class FileStreamBuffer : public std::streambuf
{
public:
	~FileStreamBuffer();
	FileStreamBuffer(int backsize = 0);

	void Attach(IFile *file);
	void Detach();

private:
	virtual int_type underflow();
	virtual std::streamsize showmanyc();
	virtual pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
	virtual pos_type seekpos(pos_type, std::ios_base::openmode = std::ios_base::in | std::ios_base::out);

	IFile* m_file;
	char*  m_buffer;
	int    m_backsize;
	int    m_frontsize;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CFileStreamBuffer // streambuf for file io, only supports buffered input currently
/////////////////////////////////////////////////////////////////////////////////////////
// very basic file input stream
class FileStream : public std::istream
{
public:
	FileStream(int backsize = 0);
	~FileStream();

	bool Open(const std::string& filename);
	bool Open(const CURL& filename);
	void Close();

	int64_t GetLength();
private:
	FileStreamBuffer m_buffer;
	IFile*            m_file;
};
