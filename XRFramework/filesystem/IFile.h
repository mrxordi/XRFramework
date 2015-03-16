#pragma once

#include "Base.h"
#include "XRCommon/utils/URL.h"

/*!
 * \class IFile
 * \brief Used as an interface for all file types.
 */
class IFile
{
public:
	IFile() {};
	virtual ~IFile() {};

	virtual bool Open(const CURL& url) = delete;
	virtual bool OpenForWrite(const CURL& url, bool bOverWrite = false) { return false; };
	virtual void Close() = delete;

	virtual bool Exists(const CURL& url) = delete;

	virtual int Stat(const CURL& url, struct __stat64* buffer) = delete;
	virtual int Stat(struct __stat64* buffer);

	virtual ssize_t Read(void* lpBuf, int64_t uiBufSize) = 0;
	virtual ssize_t Write(const void* lpBuf, int64_t uiBufSize) { return -1; };

	virtual bool ReadString(char *szLine, int iLineLength);
	virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET) = 0;


	virtual int64_t GetPosition() = 0;
	virtual int64_t GetLength() = 0;
	virtual void Flush() { }
	virtual int Truncate(int64_t size) { return -1; };

	/* Returns the minium size that can be read from input stream.   *
	* For example cdrom access where access could be sector based.  *
	* This will cause file system to buffer read requests, to       *
	* to meet the requirement of CFile.                             *
	* It can also be used to indicate a file system is non buffered *
	* but accepts any read size, have it return the value 1         */
	virtual int  GetChunkSize() { return 0; }

	virtual bool SkipNext(){ return false; }

	virtual bool Delete(const CURL& url) { return false; }
	virtual bool Rename(const CURL& url, const CURL& urlnew) { return false; }
	virtual bool SetHidden(const CURL& url, bool hidden) { return false; }

	//virtual int IoControl(EIoControl request, void* param) { return -1; }

	virtual std::string GetContent()                            { return "application/octet-stream"; }
	virtual std::string GetContentCharset(void)                { return ""; }
};

