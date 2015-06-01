#pragma once

#include "librtmp/rtmp.h"
#include "utils/URL.h"
#include "utils/BitstreamStats.h"
#include "filesystem/FileItem.h"

#define SEEK_POSSIBLE 0x10 // flag used to check if protocol allows seeks

enum class EStreamType {
	DVDSTREAM_TYPE_NONE,
	DVDSTREAM_TYPE_RTMP,
};

class IInputStream
{

public:
	IInputStream(EStreamType streamType);
	virtual ~IInputStream();

	virtual bool Open(const char* strFileName, const std::string& content);
	virtual bool Open(const CFileItem& fileItem);
	virtual void Close() = 0;
	virtual size_t Read(uint8_t* buf, size_t buf_size) = 0;
	virtual int64_t Seek(int64_t offset, int whence) = 0;
	virtual bool Pause(double dTime) = 0;
	virtual int64_t GetLength() = 0;
	virtual std::string& GetContent() { return m_content; };
	virtual const std::string& GetFileName() { return m_strFileName; }
	virtual const CURL &GetURL() { return m_url; }
	virtual void Abort() {}
	virtual int GetBlockSize() { return 0; }
	bool IsStreamType(EStreamType type) const { return m_streamType == type; }
	virtual bool IsEOF() = 0;
	virtual BitstreamStats GetBitstreamStats() const { return m_stats; }

protected:
	EStreamType m_streamType;
	std::string m_strFileName;
	CURL m_url;
	BitstreamStats m_stats;
	std::string m_content;
};

