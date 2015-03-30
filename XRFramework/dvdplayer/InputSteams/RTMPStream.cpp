#include "stdafxf.h"
#include "RTMPStream.h"
#include "log/Log.h"
#include "../XRThreads/SingleLock.h"

static int RTMP_level = RTMP_LOGALL;
extern "C"
{
	static void RTMPInputStream_Log(int level, const char *fmt, va_list args)
	{
		if (!g_LogPtr->IsLogExtraLogged(LOGRTMP))
			return;

		char buf[2048];

		if (level > RTMP_level)
			return;

		switch (level)
		{
		default:
		case RTMP_LOGCRIT:    level = LOG_FATAL;   break;
		case RTMP_LOGERROR:   level = LOG_ERROR;   break;
		case RTMP_LOGWARNING: level = LOG_WARNING; break;
		case RTMP_LOGINFO:    level = LOG_NOTICE;  break;
		case RTMP_LOGDEBUG:   level = LOG_INFO;    break;
		case RTMP_LOGDEBUG2:  level = LOG_DEBUG;   break;
		}

		vsnprintf(buf, sizeof(buf), fmt, args);
		g_LogPtr->Log(level, "%s", 0, "", buf);
	}
}

#define  SetAVal(av, cstr)  av.av_val = (char *)cstr.c_str(); av.av_len = cstr.length()
#undef AVC
#define AVC(str)  {(char *)str,sizeof(str)-1}

/* librtmp option names are slightly different */
static const struct {
	const char *name;
	AVal key;
} options[] = {
	{ "SWFPlayer", AVC("swfUrl") },
	{ "PageURL", AVC("pageUrl") },
	{ "PlayPath", AVC("playpath") },
	{ "TcUrl", AVC("tcUrl") },
	{ "IsLive", AVC("live") },
	{ NULL }
};

CRTMPStream::CRTMPStream() : IInputStream(EStreamType::DVDSTREAM_TYPE_RTMP)
{
	int iResult;
	WSADATA wsaData;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
	}

	LOGDEBUG("Using external libRTMP");

	RTMP_LogSetCallback(RTMPInputStream_Log);
	RTMP_LogSetLevel(RTMP_LOGDEBUG2);
	m_hRTMP = RTMP_Alloc();
	RTMP_Init(m_hRTMP);

	m_eof = true;
	m_bPaused = false;
	m_sStreamPlaying = nullptr;
}


CRTMPStream::~CRTMPStream()
{
	free(m_sStreamPlaying);
	m_sStreamPlaying = NULL;

	Close();
	if (m_hRTMP)
		RTMP_Free(m_hRTMP);
	m_hRTMP = NULL;
	m_bPaused = false;
}

bool CRTMPStream::Open(const char* strFile, const std::string &content)
{
	Close();

	if (!m_hRTMP || !IInputStream::Open(strFile, "video/x-flv"))
		return false;

	XR::CSingleLock lock(m_RTMPSection);

	// libRTMP can and will alter strFile, so take a copy of it
	m_sStreamPlaying = (char*)calloc(strlen(strFile) + 1, sizeof(char));
	int a = strlen(strFile);
	strcpy(m_sStreamPlaying, strFile);

 	if (!RTMP_SetupURL(m_hRTMP, m_sStreamPlaying))
 		return false;

	/* Look for protocol options in the URL.
	* Options are added to the URL in space separated key=value pairs.
	* We are only interested in the "live" option to disable seeking,
	* the rest is handled by librtmp internally
	*
	* example URL suitable for use with RTMP_SetupURL():
	* "rtmp://flashserver:1935/ondemand/thefile swfUrl=http://flashserver/player.swf swfVfy=1 live=1"
	* details: https://rtmpdump.mplayerhq.hu/librtmp.3.html
	*/
	std::string url = strFile;
	size_t iPosBlank = url.find(' ');
	if (iPosBlank != std::string::npos && (url.find("live=true") != std::string::npos || url.find("live=1") != std::string::npos))
	{
		m_canSeek = false;
		m_canPause = false;
	}
	LOGDEBUG("RTMP canseek: %s", m_canSeek ? "true" : "false");


	if (!RTMP_Connect(m_hRTMP, NULL) || !RTMP_ConnectStream(m_hRTMP, 0))
		return false;

	m_eof = false;

	return true;
}

bool CRTMPStream::Open(const CFileItem& fileItem)
{
	Close();

	if (!m_hRTMP || !IInputStream::Open(fileItem))
		return false;

	/* SetOpt and SetAVal copy pointers to the value. librtmp doesn't use the values until the Connect() call,
	* so value objects must stay allocated until then. To be extra safe, keep the values around until Close(),
	* in case librtmp needs them again.
	*/
	m_optionvalues.clear();
	for (int i = 0; options[i].name; i++)
	{
		std::string tmp = fileItem.GetProperty(options[i].name).asString();
		if (!tmp.empty())
		{
			m_optionvalues.push_back(tmp);
			AVal av_tmp;
			SetAVal(av_tmp, m_optionvalues.back());
			RTMP_SetOpt(m_hRTMP, &options[i].key, &av_tmp);
		}
	}
	
	if (!m_hRTMP || !IInputStream::Open(m_strFileName.c_str(), "video/x-flv"))
		return false;

	XR::CSingleLock lock(m_RTMPSection);

	// libRTMP can and will alter strFile, so take a copy of it
	m_sStreamPlaying = (char*)calloc(strlen(m_strFileName.c_str()) + 1, sizeof(char));
	strcpy(m_sStreamPlaying, m_strFileName.c_str());

	if (!RTMP_SetupURL(m_hRTMP, m_sStreamPlaying))
		return false;

	/* Look for protocol options in the URL.
	* Options are added to the URL in space separated key=value pairs.
	* We are only interested in the "live" option to disable seeking,
	* the rest is handled by librtmp internally
	*
	* example URL suitable for use with RTMP_SetupURL():
	* "rtmp://flashserver:1935/ondemand/thefile swfUrl=http://flashserver/player.swf swfVfy=1 live=1"
	* details: https://rtmpdump.mplayerhq.hu/librtmp.3.html
	*/

	size_t iPosBlank = m_strFileName.find(' ');
	if (iPosBlank != std::string::npos && (m_strFileName.find("live=true") != std::string::npos || m_strFileName.find("live=1") != std::string::npos))
	{
		m_canSeek = false;
		m_canPause = false;
	}
	LOGDEBUG("RTMP canseek: %s", m_canSeek ? "true" : "false");


	if (!RTMP_Connect(m_hRTMP, NULL) || !RTMP_ConnectStream(m_hRTMP, 0))
		return false;

	m_eof = false;

	return true;
}

void CRTMPStream::Close()
{
	XR::CSingleLock lock(m_RTMPSection);

	if (m_hRTMP)
		RTMP_Close(m_hRTMP);

	m_optionvalues.clear();
	m_eof = true;
	m_bPaused = false;
}

bool CRTMPStream::IsEOF()
{
	return m_eof;
}

int64_t CRTMPStream::Seek(int64_t offset, int whence)
{
	if (whence == SEEK_POSSIBLE)
		return 0;
	else
		return -1;
}

int64_t CRTMPStream::GetLength()
{
	return 1;
}

int CRTMPStream::Read(uint8_t* buf, int buf_size)
{
	if (!m_hRTMP)
		return -1;

	int i = RTMP_Read(m_hRTMP, (char *)buf, buf_size);
	if (i < 0)
		m_eof = true;

	return i;
}

bool CRTMPStream::Pause(double dTime)
{
	XR::CSingleLock lock(m_RTMPSection);

	m_bPaused = !m_bPaused;

	LOGINFO("RTMP Pause %s requested", m_bPaused ? "TRUE" : "FALSE");

	if (m_hRTMP)
		RTMP_Pause(m_hRTMP, m_bPaused);

	return true;
}
