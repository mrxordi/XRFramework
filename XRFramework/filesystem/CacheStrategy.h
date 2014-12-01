#pragma once

#include <stdint.h>
#include <string>
#include "../XRThreads/CriticalSection.h"
#include "../XRThreads/Event.h"

#define CACHE_RC_OK  0
#define CACHE_RC_ERROR -1
#define CACHE_RC_WOULD_BLOCK -2
#define CACHE_RC_TIMEOUT -3

class IFile; // forward declaration

////////////////////////////////////////////////////////////////////////////////////
// CacheStrategy
////////////////////////////////////////////////////////////////////////////////////
class CacheStrategy
{
public:
	CacheStrategy();
	virtual ~CacheStrategy();

	virtual int Open() = 0;
	virtual void Close() = 0;

	virtual int WriteToCache(const char *pBuffer, size_t iSize) = 0;
	virtual int ReadFromCache(char *pBuffer, size_t iMaxSize) = 0;
	virtual int64_t WaitForData(unsigned int iMinAvail, unsigned int iMillis) = 0;

	virtual int64_t Seek(int64_t iFilePosition) = 0;
	virtual void Reset(int64_t iSourcePosition, bool clearAnyway = true) = 0;

	virtual void EndOfInput(); // mark the end of the input stream so that Read will know when to return EOF
	virtual bool IsEndOfInput();
	virtual void ClearEndOfInput();

	virtual int64_t CachedDataEndPosIfSeekTo(int64_t iFilePosition) = 0;
	virtual int64_t CachedDataEndPos() = 0;
	virtual bool IsCachedPosition(int64_t iFilePosition) = 0;

	virtual CacheStrategy *CreateNew() = 0;

	CEvent m_space;
protected:
	bool  m_bEndOfInput;
};

////////////////////////////////////////////////////////////////////////////////////
// SimpleFileCache
////////////////////////////////////////////////////////////////////////////////////
class SimpleFileCache : public CacheStrategy 
{
public:
	SimpleFileCache();
	virtual ~SimpleFileCache();

	virtual int Open();
	virtual void Close();

	virtual int WriteToCache(const char *pBuffer, size_t iSize);
	virtual int ReadFromCache(char *pBuffer, size_t iMaxSize);
	virtual int64_t WaitForData(unsigned int iMinAvail, unsigned int iMillis);

	virtual int64_t Seek(int64_t iFilePosition);
	virtual void Reset(int64_t iSourcePosition, bool clearAnyway = true);
	virtual void EndOfInput();

	virtual int64_t CachedDataEndPosIfSeekTo(int64_t iFilePosition);
	virtual int64_t CachedDataEndPos();
	virtual bool IsCachedPosition(int64_t iFilePosition);

	virtual CacheStrategy *CreateNew();

	int64_t  GetAvailableRead();

protected:
	std::string m_filename;
	IFile*   m_cacheFileRead;
	IFile*   m_cacheFileWrite;
	CEvent*  m_hDataAvailEvent;
	volatile int64_t m_nStartPosition;
	volatile int64_t m_nWritePosition;
	volatile int64_t m_nReadPosition;
};

////////////////////////////////////////////////////////////////////////////////////
// SimpleDoubleCache
////////////////////////////////////////////////////////////////////////////////////
class SimpleDoubleCache : public CacheStrategy
{
public:
	SimpleDoubleCache(CacheStrategy *impl);
	virtual ~SimpleDoubleCache();

	virtual int Open();
	virtual void Close();

	virtual int WriteToCache(const char *pBuffer, size_t iSize);
	virtual int ReadFromCache(char *pBuffer, size_t iMaxSize);
	virtual int64_t WaitForData(unsigned int iMinAvail, unsigned int iMillis);

	virtual int64_t Seek(int64_t iFilePosition);
	virtual void Reset(int64_t iSourcePosition, bool clearAnyway = true);
	virtual void EndOfInput();
	virtual bool IsEndOfInput();
	virtual void ClearEndOfInput();

	virtual int64_t CachedDataEndPosIfSeekTo(int64_t iFilePosition);
	virtual int64_t CachedDataEndPos();
	virtual bool IsCachedPosition(int64_t iFilePosition);

	virtual CacheStrategy *CreateNew();

protected:
	CacheStrategy *m_pCache;
	CacheStrategy *m_pCacheOld;

};

////////////////////////////////////////////////////////////////////////////////////
// CircularCache
////////////////////////////////////////////////////////////////////////////////////
class CircularCache : public CacheStrategy
{
public:
	CircularCache(size_t front, size_t back);
	virtual ~CircularCache();

	virtual int Open();
	virtual void Close();

	virtual int WriteToCache(const char *buf, size_t len);
	virtual int ReadFromCache(char *buf, size_t len);
	virtual int64_t WaitForData(unsigned int minimum, unsigned int iMillis);

	virtual int64_t Seek(int64_t pos);
	virtual void Reset(int64_t pos, bool clearAnyway = true);

	virtual int64_t CachedDataEndPosIfSeekTo(int64_t iFilePosition);
	virtual int64_t CachedDataEndPos();
	virtual bool IsCachedPosition(int64_t iFilePosition);

	virtual CacheStrategy *CreateNew();
protected:
	int64_t           m_beg;       /**< index in file (not buffer) of beginning of valid data */
	int64_t           m_end;       /**< index in file (not buffer) of end of valid data */
	int64_t           m_cur;       /**< current reading index in file */
	uint8_t          *m_buf;       /**< buffer holding data */
	size_t            m_size;      /**< size of data buffer used (m_buf) */
	size_t            m_size_back; /**< guaranteed size of back buffer (actual size can be smaller, or larger if front buffer doesn't need it) */
	CCriticalSection  m_sync;
	CEvent            m_written;
	HANDLE            m_handle;
};
