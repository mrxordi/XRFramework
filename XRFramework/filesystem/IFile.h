#pragma once
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#include "IFileTypes.h"
#include "Base.h"

class CUrl;

class IFile
{
public:
	IFile();
	virtual ~IFile();

	virtual bool Open(const CUrl& url) = 0;
	virtual bool OpenForWrite(const CUrl& url, bool bOverWrite = false) { return false; };
	virtual void Close() = 0;

	virtual bool Exists(const CUrl& url) = 0;

	virtual int Stat(const CUrl& url, struct __stat64* buffer) = 0;
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

	virtual bool Delete(const CUrl& url) { return false; }
	virtual bool Rename(const CUrl& url, const CUrl& urlnew) { return false; }
	virtual bool SetHidden(const CUrl& url, bool hidden) { return false; }

	virtual int IoControl(EIoControl request, void* param) { return -1; }

	virtual std::string GetContent()                            { return "application/octet-stream"; }
	virtual std::string GetContentCharset(void)                { return ""; }
};

class CRedirectException
{
public:
	IFile *m_pNewFileImp;
	CUrl  *m_pNewUrl;

	CRedirectException();

	CRedirectException(IFile *pNewFileImp, CUrl *pNewUrl = NULL);
};