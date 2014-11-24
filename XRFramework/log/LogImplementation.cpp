#include "stdafxf.h"
#include "LogImplementation.h"
#include "utils/StringUtils.h"
#include "utils/StringConverter.h"


LogImplementation::LogImplementation() : m_hFile(INVALID_HANDLE_VALUE)
{}


LogImplementation::~LogImplementation() {
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
}

bool LogImplementation::OpenLogFile(const std::string& logFilename, const std::string& backupOldLogToFilename) {
	if (m_hFile != INVALID_HANDLE_VALUE)
		return false; // file was already opened

	if (!backupOldLogToFilename.empty())
	{
		(void)DeleteFile(backupOldLogToFilename.c_str());					 // if it's failed, try to continue
		(void)MoveFile(logFilename.c_str(), backupOldLogToFilename.c_str()); // if it's failed, try to continue
	}

	m_hFile = CreateFile(logFilename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE) {
		PrintDebugString("Failed to create the file log, logging only to debug output");
		return false;
	}

	static const unsigned char BOM[3] = { 0xEF, 0xBB, 0xBF };
	DWORD written;
	if (!WriteFile(m_hFile, BOM, sizeof(BOM), &written, NULL)) {			 // write BOM
		PrintDebugString("Failed to write to the file log, logging only to debug output");
		return false;
	}

	return true;
}

void LogImplementation::CloseLogFile(void)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

bool LogImplementation::WriteStringToLog(const std::string& logString)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return false;

	std::string strData(logString);
	StringUtils::Replace(strData, "\n", "\r\n");
	strData += "\r\n";

	DWORD written;
	const bool ret = (WriteFile(m_hFile, strData.c_str(), strData.length(), &written, NULL) != 0) && written == strData.length();
	(void)FlushFileBuffers(m_hFile);

	return ret;
}


void LogImplementation::PrintDebugString(const std::string& debugString)
{
#ifdef _DEBUG
	//	int bufSize = MultiByteToWideChar(CP_UTF8, 0, debugString.c_str(), debugString.length(), NULL, 0);
	//	void* buf = 0;
	//	size_t sizebuf = (sizeof(wchar_t) * (bufSize + 1)); // '+1' for extra safety
	//	buf = malloc(sizebuf);
	//	if (MultiByteToWideChar(CP_UTF8, 0, debugString.c_str(), debugString.length(), (wchar_t*)buf, sizebuf / sizeof(wchar_t)) == bufSize)
	//		::OutputDebugStringW(std::wstring((wchar_t*)buf, bufSize).c_str());
	//	else
	std::string outStr;
	StringConverter::Utf8ToWin1250(debugString, outStr, false);
	::OutputDebugStringA(outStr.c_str());
	::OutputDebugStringW(L"\n");
	//	free(buf);
#endif // _DEBUG
}

void LogImplementation::GetCurrentLocalTime(int& hour, int& minute, int& second)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	hour = time.wHour;
	minute = time.wMinute;
	second = time.wSecond;
}