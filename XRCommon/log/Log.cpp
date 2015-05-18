#include "stdafx.h"
#include "Log.h"

#include <inttypes.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

#include "FilePathUtils.h"
#include "utils/UrlUtils.h"
#include "utils/StringUtils.h"
#include "utils/SpecialProtocol.h"
#include "../XRThreads/SingleLock.h"
#include "../XRThreads/Thread.h"

template<>
CLog*  Singleton<CLog>::ms_Singleton = 0;

CLog::~CLog() {
	// create log footer
	Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	Log(LOG_INFO, "", 0, "", "+                          That is the end of your log                        +");
	Log(LOG_INFO, "", 0, "", "+                          (http://github.com/mrxordi)                        +");
	Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	Close();
}

bool CLog::Init(std::string& path) {
	XR::CSingleLock waitlock(m_CritSection);
	std::string pathFileName;

	if (m_isInit)     //Check is already initialised
		return true;

	m_Iplementation.reset(new LogImplementation);

	if (path.empty()) {
		m_errorMessage = "Logger::Init() - the \"logPathName\" parameter must not be empty (but is); a log name must be supplied.";
		LogImplementation::PrintDebugString(m_errorMessage);
		return false;
	}
	else {
		if (!UrlUtils::IsURL(path))			//If only filename without path 
		{
			pathFileName += "special://app/";
			pathFileName += path;
		}
		else {
			pathFileName = path;
		}
		pathFileName = CSpecialProtocol::TranslatePath(pathFileName);   //Translate special path to windows path
		if (m_Iplementation->OpenLogFile(pathFileName, pathFileName + ".old")) {
			m_bConsoleOnly = false;
		}

	}
#if _DEBUG
	if (!m_hConsoleHandle)
		AllocConsole();
	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

#endif
	m_hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);


	// create log header
	Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	Log(LOG_INFO, "", 0, "", "+                   XRFrameWork event log has been initiated                  +");
	Log(LOG_INFO, "", 0, "", "+                          (http://github.com/mrxordi)                        +");
	Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	char addr_buff[32];

	sprintf(addr_buff, "(%p)", static_cast<void*>(g_LogPtr));
	Log(LOG_INFO, "", 0, "", "Logger singleton created. %s \n", std::string(addr_buff).c_str());

	m_isInit = true;

	return true;
}

void CLog::Close() {
	XR::CSingleLock lock(m_CritSection);

	m_Iplementation->CloseLogFile();
	m_repeatLine.clear();
	m_isInit = false;
	m_bConsoleOnly = true;
}

void CLog::Log(int loglevel, const char* file, const int lineNumber, const char* functionName, const char* format, ...) {
	if (IsLogLevelLogged(loglevel))
	{
		va_list va;
		va_start(va, format);
		LogString(loglevel, file, lineNumber, functionName, StringUtils::FormatV(format, va));
		va_end(va);
	}
}

void CLog::LogString(int loglevel, const char* file, const int lineNumber, const char* functionName, std::string& logString) {
	//Check that our CLog object existing
	//Otherwise send only WriteLogString which will output only in Debug Window in that case.
	if (this) {
		XR::CSingleLock waitLock(m_CritSection);

		std::string strData(logString);
		StringUtils::TrimRight(strData);

		if (!strData.empty()) {
			if (m_repeatLogLevel == loglevel && m_repeatLine == logString) {
				m_repeatCount++;
			}
			else if (m_repeatCount) {
				std::string strData2 = StringUtils::Format("Previous line repeats %d times.",
					m_repeatCount);

				WriteLogString(m_repeatLogLevel, "", 0, "", strData2);
				WriteLogString(loglevel, file, lineNumber, functionName, strData);

				m_repeatCount = 0;
			}
			else {
				m_repeatLine = strData;
				m_repeatLogLevel = loglevel;

				WriteLogString(loglevel, file, lineNumber, functionName, strData);
			}
		}
	}
	else {
		WriteLogString(loglevel, file, lineNumber, functionName, logString);
	}
}

bool CLog::WriteLogString(int logLevel, const char* file, const int lineNumber, const char* functionName, const std::string& logString) {
	static const char* prefixFormat = "%02.2d:%02.2d:%02.2d T:%" PRIu64"%7s: ";

	std::string extraString;
	std::string sourceFile = StrFileAndExt(file);
	if (file != NULL && file[0] != 0 && lineNumber > 0) {
		extraString = StringUtils::Format("[%s(%i)%s] ", sourceFile.c_str(), lineNumber, functionName);
	}
	else {
		extraString = "";
	}

	std::string strData(logString);
	/* fixup newline alignment, number of spaces should equal prefix length */
	//StringUtils::Replace(strData, "\n", "\n                                            ");

	int hour, minute, second;
	LogImplementation::GetCurrentLocalTime(hour, minute, second);

	strData = StringUtils::Format(prefixFormat,
		hour,
		minute,
		second,
		(uint64_t)CThread::GetCurrentThreadId(),
		levelNames[logLevel]) + "+" + extraString + strData;

	if (!this)
		strData = "[CAUTION CLOG NOT INITIALISED] " + strData;

	if (m_hConsoleHandle) {
		switch (logLevel) {
		case LOG_ERROR:
			SetConsoleTextAttribute(m_hConsoleHandle, ConColor::RED);
			break;
		case LOG_INFO:
			SetConsoleTextAttribute(m_hConsoleHandle, ConColor::YELLOW);
			break;
		default:
			SetConsoleTextAttribute(m_hConsoleHandle, ConColor::WHITE);
		}
// 		std::cout << StringUtils::Format("%02.2d:%02.2d:%02.2d %7s: ", 
// 			hour, minute, second,
// 			levelNames[logLevel]) << logString << std::endl;
		std::cout << strData << std::endl;
	}

	PrintDebugString(strData);

	if (!this)
		return true;

	if (m_bConsoleOnly)
		return true;

	return m_Iplementation->WriteStringToLog(strData);
}

void CLog::PrintDebugString(const std::string& line) {
//#if defined(_DEBUG) || defined(PROFILE)
	LogImplementation::PrintDebugString(line);
//#endif // defined(_DEBUG) || defined(PROFILE)
}

void CLog::SetLogLevel(int level)
{
	XR::CSingleLock waitLock(m_CritSection);
	if (level >= LOG_LEVEL_NONE && level <= LOG_LEVEL_MAX)
	{
		m_logLevel = level;
		Log(LOG_NOTICE, "", 0, __FUNCTION__, "Log level changed to \"%s\"", logLevelNames[m_logLevel + 1]);
	}
	else
		Log(LOG_ERROR, "", 0, __FUNCTION__, "Invalid log level requested: %d", level);
}

void CLog::SetExtraLogLevels(int level)
{
	XR::CSingleLock waitLock(m_CritSection);
	if (level == m_extraLogLevels)
		return;

	std::string msg { "" };
	if (level & LOGCURL)
		msg += "LOGCURL";
	if (level & LOGFFMPEG) 
	{
		if (!msg.empty())
			msg += " | ";
		msg += "LOGFFMPEG";
	}
	if (level & LOGRTMP)
	{
		if (!msg.empty())
			msg += " | ";
		msg += "LOGRTMP";
	}
	m_extraLogLevels = level;
	waitLock.Leave();

	Log(LOG_INFO, "", 0, "", "Extra Log flags set to: %s", msg.c_str());
}

bool CLog::IsLogExtraLogged(int loglevel) {
	if (loglevel <= 0)
		return false;

	return ((m_extraLogLevels & loglevel) == loglevel);
}

bool CLog::IsLogLevelLogged(int loglevel)
{
	const int extras = (loglevel & ~LOGMASK);
	if (extras != 0 && (m_extraLogLevels & extras) == 0)
		return false;

#if defined(_DEBUG) || defined(PROFILE)
	return true;
#else
	// "m_logLevel" is "LOG_LEVEL_NORMAL"
	return (loglevel & LOGMASK) >= LOG_NOTICE;
#endif
}

CLog& CLog::Create() {
	return *new CLog();
}

void CLog::Destroy() {
	delete CLog::getSingletonPtr();
}