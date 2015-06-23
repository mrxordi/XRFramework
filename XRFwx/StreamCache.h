#pragma once
#include "XRFramework/filesystem/File.h"
#include "XRThreads/Event.h"

class CStreamCache {
public:
	CStreamCache();
	virtual ~CStreamCache();

	int Open();
	void Close();
	bool Reset();

	int		WriteToCache(const char *pBuffer, size_t iSize);
	int		ReadFromCache(char *pBuffer, size_t iMaxSize);
	int64_t WaitForData(unsigned int iMinAvail, unsigned int iMillis);
	int64_t Seek(int64_t iFilePosition);

	int64_t CachedDataEndPos();
	bool	IsCachedPosition(int64_t iFilePosition);

	int64_t  GetAvailableRead();
	void	 EndOfInput();
	bool	 IsEndOfInput();
   int IoControl(EIoControl request, void* param);

private:
	bool  m_bEndOfInput;
	std::string m_filename;

	IFile*   m_cacheFileRead;
	IFile*   m_cacheFileWrite;

	CEvent*  m_hDataAvailEvent;
	volatile int64_t m_nWritePosition;
	volatile int64_t m_nReadPosition;
};

