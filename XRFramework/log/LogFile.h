#pragma once
#include <string>

bool FileExistsW(const wchar_t* filePath);
bool FileExistsA(const char* filePath);

std::wstring WStrFileAndExt(const wchar_t* filePath);
std::string  StrFileAndExt(const char* filePath);

class LogFile {
public:
	LogFile(const char* logPathName, bool asciiFile, bool overwriteExistingLog);

	virtual void LogMessage(std::string& message);

	virtual bool IsProblem() const {
		return !m_errorMessage.empty();
	}

	virtual const std::string& GetErrorMessage() const {
		return m_errorMessage;
	}

	virtual ~LogFile(void);

private:
	void OpenLogFile(const char* logPathName);
	void CreateLogFile(const char* logPathName);
	void FileOpenOrCreateError(const char* logPathName);
	void WriteLogMessageError(const char* logMssg, const int errCode);

private:
	FILE*	m_file;
	bool	m_overwriteLog;
	std::string m_errorMessage;
	bool	m_writeToStderr;
};
