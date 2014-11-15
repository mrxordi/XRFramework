#pragma once
#include <string>
#include "LogDefines.h"
#include "LogFile.h"
#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"

class LoggerDAL
{
public:
	LoggerDAL(LogFile* LogFileObject);
	~LoggerDAL();
	void Log(LOGLEVEL loglevel, const char* file, const int linenumber, const char* function, const char* message, ...);

	bool IsProblem() const { return !m_errorMessage.empty(); }
	const std::string& GetErrorMessage() const { return m_errorMessage; }

private:
	LoggerDAL(const LoggerDAL&);
	LoggerDAL& operator= (const LoggerDAL&);

	void SendMessageToLog(const char* file, const int lineNumber, const char* function, const char* buffer, LOGLEVEL level);

	std::string GetLogLevelAndTime(LOGLEVEL loglevel);

private:
	LogFile* m_logFileObj;
	std::string m_errorMessage;
	static CCriticalSection m_critSec;

};

