#pragma once
#include "RTMPStream.h"
#include "rtmp/rtmp.h"
#include "filesystem/FileCache.h"
struct RTMPKeyframeInfo
{
	
};

class CBufferedRTMPStream : public CRTMPStream
{
public:
	CBufferedRTMPStream();
	virtual ~CBufferedRTMPStream();

	virtual bool Open(const char* strFile, const std::string &content) override;

	virtual bool Open(const CFileItem& fileItem) override;

	virtual void Close() override;

	virtual int Read(uint8_t* buf, int buf_size) override;

	virtual int64_t Seek(int64_t offset, int whence) override;

	virtual bool Pause(double dTime) override;

	virtual bool IsEOF() override;

	virtual int64_t GetLength() override;

	virtual void Abort() override;

	virtual int GetBlockSize() override;

	virtual BitstreamStats GetBitstreamStats() const override;

private:
	CURL m_cacheUrl;
	CFileCache m_fCache;
};

