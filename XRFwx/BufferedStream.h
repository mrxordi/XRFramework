#pragma once
#include "StreamCache.h"
#include "XRFramework/dvdplayer/InputSteams/IInputStream.h"
#include "XRFramework/dvdplayer/InputSteams/RTMPStream.h"
#include "XRThreads/Thread.h"

typedef std::vector<std::pair<uint32_t, int64_t> > KeyframesVector;

class CBufferedStream : public IInputStream, public CThread
{
public:
	CBufferedStream();
	virtual ~CBufferedStream();

	virtual bool Open(const char* strFileName, const std::string& content) override;
	virtual void Close() override;
	virtual size_t Read(uint8_t* buf, size_t buf_size) override;
	virtual int64_t Seek(int64_t offset, int whence) override;
	virtual int64_t SeekTime(int64_t offset);
	virtual bool Pause(double dTime) override;
	virtual int64_t GetLength() override;
	virtual const std::string& GetFileName() override;
	virtual int64_t GetCurrentReadPositon() {
		return m_readPos;
	}

//	virtual void Abort() override;
//	virtual int GetBlockSize() override;
/*	virtual BitstreamStats GetBitstreamStats() const override;*/

	virtual bool IsEOF() override;
	virtual void StopThread(bool bWait = true) override;
	virtual void OnException() override;
	virtual void OnStartup() override;
	virtual void OnExit() override;
	virtual void Process() override;
private:
	void ParsePacketHeader(const uint8_t* pData);

private:
	bool m_bEof;
	int64_t      m_nSeekResult;
	int64_t      m_seekPos;
	int64_t      m_readPos;
	int64_t      m_writePos;

	CRTMPStream* m_sourceStream;
	CEvent      m_seekEvent;
	CEvent      m_seekEnded;
	CStreamCache* m_streamCache;
	XR::CCriticalSection m_sync;

	//buffer for reading from stream
	int m_buffersize = 64 * 1024;
	uint8_t* m_bData;
	KeyframesVector m_CachedKeyframes;
};

