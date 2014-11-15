#include "stdafxf.h"
#include "Logger.h"
#include "LoggerDAL.h"
#include "utils/URIUtils.h"
#include "filesystem/SpecialProtocol.h"
#include <iostream>
#include <sys/stat.h>

#define stat64 __stat64

/*************************************************************************
Static Data Definitions
*************************************************************************/
// singleton instance pointer
template<> Logger* Singleton<Logger>::ms_Singleton = 0;

LoggerDAL_t Logger::m_loggerDAL = nullptr;
LogFile* Logger::m_LogFile = nullptr;
std::string  Logger::m_LogFileName = std::string("");
std::string  Logger::m_errorMessage = std::string("");
CCriticalSection Logger::m_critSec;

using namespace std;

bool Logger::Init(const std::string& pathFileName) {

	string logPathName, logPathNameOld;

	if (pathFileName.empty()) {
		m_errorMessage = "Logger::Init() - the \"logPathName\" parameter must not be empty (but is); a log name must be supplied.";
		cerr << m_errorMessage << endl;
		return false;
	}
	else {
		if (!URIUtils::IsURL(pathFileName))
		{
			logPathName += "special://app/";
			logPathName += pathFileName;
		}
		else {
			logPathName = pathFileName;
		}
		logPathName = CSpecialProtocol::TranslatePath(logPathName);
		logPathNameOld = logPathName + ".old";
		if (!InitializeLog(logPathName, logPathNameOld))
			return false;
	}

	// create log header
	m_loggerDAL->Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	m_loggerDAL->Log(LOG_INFO, "", 0, "", "+                   XRFrameWork event log has been initiated                  +");
	m_loggerDAL->Log(LOG_INFO, "", 0, "", "+                          (http://github.com/mrxordi)                        +");
	m_loggerDAL->Log(LOG_INFO, "", 0, "", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	char addr_buff[32];
	sprintf(addr_buff, "(%p)", static_cast<void*>(m_loggerDAL.get()));
	m_loggerDAL->Log(LOG_INFO, "", 0, "", "Logger singleton created. %s \n", string(addr_buff).c_str());

	return true;
}

bool Logger::InitializeLog(std::string& newFilename, std::string& oldFilename) {

	CSingleLock singleLock(m_critSec);

	m_LogFileName = newFilename;

	struct stat64 info;
	if (_stat64(oldFilename.c_str(), &info) == 0 && !DeleteFileA(oldFilename.c_str())){
		m_errorMessage = "Logger::Init() - Failed to delete old log";
		OutputDebugStringA(m_errorMessage.c_str());
		return false;
	}
	if (_stat64(m_LogFileName.c_str(), &info) == 0 && !MoveFileA(newFilename.c_str(), oldFilename.c_str())){
		m_errorMessage = "Logger::Init() - Failed to move exsisting old log";
		OutputDebugStringA(m_errorMessage.c_str());
		return false;
	}

	if (!IsAlreadyInit()) {
		// Init the log factory (for creating the log target object)
		m_LogFile = new LogFile(m_LogFileName.c_str(), true, true);

		// Create DAL (data access layer) object through which the user intracts with the logger
		if (m_LogFile != NULL) {
			LoggerDAL *dal = new LoggerDAL(m_LogFile);
			if (dal == NULL) {
				m_errorMessage += "Failed to create pointer needed for logging messages (\"LogFileDAL\" is NULL).";
				OutputDebugStringA(m_errorMessage.c_str());
			}
			else {
				m_loggerDAL.reset(dal);
			}

			if (m_loggerDAL->IsProblem()) {
				m_errorMessage = m_loggerDAL->GetErrorMessage();
				OutputDebugStringA(m_errorMessage.c_str());
			}
		}

		if (IsProblem()) {
			ResetLogger();
			return false;
		}
	}
	return true;
}

LoggerDAL_t Logger::GetLog() {
	//Lock before checking, for possibility to initializing while trying to get instance.
	CSingleLock singleLock(m_critSec);
	m_errorMessage.clear();
	if (IsAlreadyInit() == false) {
		m_errorMessage = "Logger::GetLog() - Log file has not been initialized.\nLogger::GetLog() - Use Init(...) to initialize the log file instead.\n ";
		OutputDebugStringA(m_errorMessage.c_str());
	}

	if (m_loggerDAL.get() == NULL) {
		m_errorMessage = "Logger::GetLog() - Log file object failed to initialize (object pointer is NULL). Log cannot be used.\n";
		OutputDebugStringA(m_errorMessage.c_str());
	}
	return m_loggerDAL;
}

bool Logger::IsAlreadyInit() {
	return  !(m_LogFileName.empty() || m_loggerDAL.get() == NULL);
}