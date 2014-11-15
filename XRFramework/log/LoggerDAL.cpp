#include "stdafxf.h"
#include "LoggerDAL.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include "utils/StringUtils.h"


LoggerDAL::LoggerDAL(LogFile* LogFileObject) {

	if (LogFileObject != NULL) {
		m_logFileObj = LogFileObject;
	}
	else {
		m_logFileObj = NULL;
		m_errorMessage = "Failed to open or create the log (log object is NULL).";
	}
}


LoggerDAL::~LoggerDAL()
{
	delete m_logFileObj;
}

void LoggerDAL::Log(LOGLEVEL loglevel, const char* file, const int linenumber, const char* function, const char* message, ...)
{
	CSingleLock mutex(m_critSec);

	if (this == NULL) return;

	va_list args;
	va_start(args, message);
	std::string buffer = StringUtils::FormatV(message, args);
	va_end(args);

	if (buffer.empty()) m_errorMessage += "Error allocating memory for a formatted string.";
	if (loglevel == LOG_INFO) {
		SendMessageToLog("", 0, "", buffer.c_str(), loglevel);
	}
	else {
		SendMessageToLog(file, linenumber, function, buffer.c_str(), loglevel);
	}
}

void LoggerDAL::SendMessageToLog(const char* file, const int lineNumber, const char* function, const char* buffer, LOGLEVEL level) {

	if (m_logFileObj != NULL) {

		std::ostringstream message("");

		//add time and log level
		message << GetLogLevelAndTime(level);
		const char* sourceFile = StrFileAndExt(file).c_str();
		if (file != NULL && sourceFile[0] != 0) {
			message << sourceFile;
		}
		if (lineNumber > 0) {
			message << StringUtils::Format("(%d)-", lineNumber);
		}
		if (function != NULL && function[0] != 0)	{
			message << function;
		}
		message << buffer;

		m_logFileObj->LogMessage(message.str());
	}
}


std::string LoggerDAL::GetLogLevelAndTime(LOGLEVEL loglevel) {
	std::ostringstream levelString;

	using namespace std;

	time_t et;
	time(&et);
	tm* etm = localtime(&et);

	if (!etm)
		return levelString.str();

	// clear sting stream
	levelString.str("");

	// write date
	levelString << setfill('0') << setw(2) << etm->tm_mday << '/' <<
		setfill('0') << setw(2) << 1 + etm->tm_mon << '/' <<
		setw(4) << (1900 + etm->tm_year) << ' ';

	// write time
	levelString << setfill('0') << setw(2) << etm->tm_hour << ':' <<
		setfill('0') << setw(2) << etm->tm_min << ':' <<
		setfill('0') << setw(2) << etm->tm_sec << ' ';

	switch (loglevel) {
	case LOG_INFO:
		levelString << "[INFO]:  ";
		break;
	case LOG_DEBUG:
		levelString << "[DEBUG]: ";
		break;
	case LOG_WARNING:
		levelString << "[WARN]:  ";
		break;
	case LOG_ERROR:
		levelString << "[ERROR]: ";
		break;
	default:
		levelString << "[unknown]";
	}
	return levelString.str();
}

