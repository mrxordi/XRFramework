#pragma once
#include <Windows.h>
class LogImplementation
{
public:
	LogImplementation();
	~LogImplementation();

	bool OpenLogFile(const std::string& logFilename, const std::string& backupOldLogToFilename);
	void CloseLogFile(void);
	bool WriteStringToLog(const std::string& logString);
	static void PrintDebugString(const std::string& debugString);
	static void GetCurrentLocalTime(int& hour, int& minute, int& second);
private:
	HANDLE m_hFile;
};

