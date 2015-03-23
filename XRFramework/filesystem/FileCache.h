#pragma once
#include "IFile.h"
#include "File.h"
#include "CacheStrategy.h"
#include "XRThreads/Thread.h"
#include "XRThreads/CriticalSection.h"

class CFileCache : public IFile, public CThread
{
public:
	CFileCache(bool useDoubleCache = false);
	CFileCache(CacheStrategy *pCache, bool bDeleteCache = true);
	~CFileCache();

	// CThread methods
	virtual void Process();
	virtual void OnExit();
	virtual void StopThread(bool bWait = true);

	void SetCacheStrategy(CacheStrategy *pCache, bool bDeleteCache = true);

	// IFIle methods
	virtual bool          Open(const CURL& url);
	virtual void          Close();
	virtual bool          Exists(const CURL& url);
	virtual int           Stat(const CURL& url, struct __stat64* buffer);

	virtual ssize_t       Read(void* lpBuf, int64_t uiBufSize);

	virtual int64_t       Seek(int64_t iFilePosition, int iWhence);
	virtual int64_t       GetPosition();
	virtual int64_t       GetLength();

	virtual int           IoControl(EIoControl request, void* param);

	IFile *GetFileImp();

	virtual std::string GetContent();
	virtual std::string GetContentCharset(void);

private:
	CacheStrategy *m_pCache;
	bool      m_bDeleteCache;
	int        m_seekPossible;
	CFile      m_source;
	std::string    m_sourcePath;
	CEvent      m_seekEvent;
	CEvent      m_seekEnded;
	int64_t      m_nSeekResult;
	int64_t      m_seekPos;
	int64_t      m_readPos;
	int64_t      m_writePos;
	unsigned     m_chunkSize;
	unsigned     m_writeRate;
	unsigned     m_writeRateActual;
	bool         m_cacheFull;
	XR::CCriticalSection m_sync;
};

