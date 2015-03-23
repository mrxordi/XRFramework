#pragma once

#include <sys/stat.h>
#include "Base.h"
#include "XRCommon/utils/URL.h"

/*!
* \class IFileCallback
* \brief Callback for file operations.
*/
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

struct SNativeIoControl
{
	unsigned long int   request;
	void*               param;
};

struct SCacheStatus
{
	uint64_t forward;  /**< number of bytes cached forward of current position */
	unsigned maxrate;  /**< maximum number of bytes per second cache is allowed to fill */
	unsigned currate;  /**< average read rate from source file since last position change */
	bool     full;     /**< is the cache full */
};

typedef enum {
	IOCTRL_NATIVE = 1, /**< SNativeIoControl structure, containing what should be passed to native ioctrl */
	IOCTRL_SEEK_POSSIBLE = 2, /**< return 0 if known not to work, 1 if it should work */
	IOCTRL_CACHE_STATUS = 3, /**< SCacheStatus structure */
	IOCTRL_CACHE_SETRATE = 4, /**< unsigned int with speed limit for caching in bytes per second */
	IOCTRL_SET_CACHE = 8, /** <CFileCache */
} EIoControl;

/*!
 * \class IFile
 * \brief Used as an interface for all file types.
 */
class IFile
{
public:
	IFile() {};
	virtual ~IFile() {};

	virtual bool Open(const CURL& url) = 0;
	virtual bool OpenForWrite(const CURL& url, bool bOverWrite = false) { return false; };
	virtual void Close() = 0;

	virtual bool Exists(const CURL& url) = 0;

	virtual int Stat(const CURL& url, struct __stat64* buffer) = 0;
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

	virtual int IoControl(EIoControl request, void* param) { return -1; }

	virtual std::string GetContent()                            { return "application/octet-stream"; }
	virtual std::string GetContentCharset(void)                { return ""; }
};

