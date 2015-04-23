#pragma once
#include "IInputStream.h"
#include "librtmp/log.h"
#include "librtmp/rtmp.h"
#include "../XRThreads/CriticalSection.h"

class CRTMPStream :
	public IInputStream
{
public:
	CRTMPStream();
	virtual ~CRTMPStream();
	virtual bool    Open(const char* strFile, const std::string &content);
	virtual bool	Open(const CFileItem& fileItem);
	virtual void    Close();
	virtual int     Read(uint8_t* buf, int buf_size);
	virtual int64_t Seek(int64_t offset, int whence);
	bool            SeekTime(int iTimeInMsec);
	bool            CanSeek()  { return m_canSeek; }
	bool            CanPause() { return m_canPause; }
	virtual bool    Pause(double dTime);
	virtual bool    IsEOF();
	virtual int64_t GetLength();

	XR::CCriticalSection m_RTMPSection;
	RTMP*		m_hRTMP;

private:
	bool		m_eof;
	bool		m_bPaused;
	bool		m_canSeek;
	bool		m_canPause;
	char* m_sStreamPlaying;
	std::vector<std::string> m_optionvalues;
};

