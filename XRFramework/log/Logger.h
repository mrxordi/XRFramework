#pragma once

#include <string>
#include "LogDefines.h"
#include "LogFile.h"
#include "utils/Singleton.h"
#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"

class Logger : public Singleton < Logger >
{
public:
	Logger();
	~Logger();

	static bool Init(const std::string& fileName);

	static LoggerDAL_t GetLog();

	static bool IsAlreadyInit();

	/*!
	* \brief GetLogNameA returns the ASCII (char* or string) log name (optional path) sent in the call to Init(),
	*  or empty string ("") if the log name was sent as UNICODE.
	*/
	static const std::string& GetLogNameA() {
		return m_LogFileName;
	}

	/*!
	* \brief IsProblem returns true if an error condition was detected (and false otherwise).
	* \see GetErrorMessage() to return the error string.
	*/
	static const bool IsProblem() {
		return !m_errorMessage.empty();
	}

	/*!
	* \brief GetErrorMessage returns the string for the last error condition. An empty string is
	*	returned if no errors have been detected.
	*/
	static const std::string& GetErrorMessage() {
		return m_errorMessage;
	}

	static void ResetLogger();

	static void SetExtraLogLevels(int level);
private:
	static bool InitializeLog(std::string& newFilename, std::string& oldFilename);

private:
	static LoggerDAL_t		m_loggerDAL;
	static LogFile*			m_LogFile;
	static int				m_extraLogLevels;
	static std::string		m_errorMessage;
	static std::string		m_LogFileName;
	static CCriticalSection m_critSec;


};

