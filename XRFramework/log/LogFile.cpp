#include "stdafxf.h"
// 
// Copyright (c) Kris Gustafson, 2012
// krisgus@ieee.org
// 
#include "LogFile.h"
#include "utils/StringUtils.h"
#include <cstdlib>
#include <cstdio>
#include <share.h>
#include <wchar.h>
#include <string>
#include <Windows.h>


using namespace std;

LogFile::LogFile(const char* logPathName, bool isAsciiFile, bool overwriteExistingLog)
	: m_file(NULL), m_overwriteLog(overwriteExistingLog),
	m_errorMessage(string("")), m_writeToStderr(false) {

	if (logPathName == NULL || logPathName[0] == 0) {
		// write output to stderr
		m_file = stderr;
		m_writeToStderr = true;
	}
	else {
		if (FileExistsA(logPathName)) {
			// open log file
			OpenLogFile(logPathName);
		}
		else {
			// create the log file
			CreateLogFile(logPathName);
		}

		if (m_file == NULL)	{
			FileOpenOrCreateError(logPathName);
		}
	}
}

void LogFile::OpenLogFile(const char* logPathName) {
	//string attributes;
	string attributes = m_overwriteLog ? string("wt") : string("at");

	m_file = _fsopen(logPathName, attributes.c_str(), _SH_DENYWR);
}

void LogFile::CreateLogFile(const char* logPathName) {
	m_file = _fsopen(logPathName, "w+t", _SH_DENYWR);
}

void LogFile::FileOpenOrCreateError(const char* logPathName) {
	m_errorMessage.append("\"");
	m_errorMessage.append(logPathName);
	m_errorMessage.append("\" failed to open. Sending output to stderr.");
	// write log output to stderr
	fprintf_s(stderr, m_errorMessage.c_str());
	m_file = stderr;
	m_writeToStderr = true;
}

void LogFile::LogMessage(std::string& message) {

	// write formatted string to log file
	if (m_writeToStderr) {
		message.append("\n");
		OutputDebugStringA(message.c_str());
	}
	else {
		int writeResultCode;
		int count = fputs(message.c_str(), m_file);

		fflush(m_file);
		message.append("\n");
		OutputDebugStringA(message.c_str());
		_get_errno(&writeResultCode);
		char result = fputc('\n', m_file);
		if (result != '\n') {
			WriteLogMessageError(message.c_str(), writeResultCode);
		}
	}
}

void LogFile::WriteLogMessageError(const char* logMssg, const int errCode) {
	m_errorMessage.clear();
	m_errorMessage += StringUtils::Format("failed to log message \"%s\" due to a write problem. Error code: %d", logMssg, errCode);
}

LogFile::~LogFile(void) {
	if (m_file != NULL) {
		const char* endMssg = "====[ END OF CURRENT LOG ]====\n";
		fputs(endMssg, m_file);
		fflush(m_file);
		fclose(m_file);
		m_file = NULL;
	}
}

bool FileExistsW(const wchar_t* filePath) {
	unsigned int fileAttr = GetFileAttributesW(filePath);
	return (fileAttr != 0xFFFFFFFF);
}


bool FileExistsA(const char* filePath) {
	unsigned int fileAttr = GetFileAttributesA(filePath);
	return (fileAttr != 0xFFFFFFFF);
}


std::wstring WStrFileAndExt(const wchar_t* filePath) {
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	std::wstring name;
	std::wstring extension;

	if (0 == _wsplitpath_s(filePath, drive, dir, fname, ext) || fname != NULL) {
		name = fname;
		extension = ext;
	}
	else {
		name = L"";
		extension = L"";
	}

	return name + extension;
}

std::string  StrFileAndExt(const char* filePath) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	std::string name;
	std::string extension;

	if (0 == _splitpath_s(filePath, drive, dir, fname, ext) || fname != NULL) {
		name = fname;
		extension = ext;
	}
	else {
		name = "";
		extension = "";
	}

	return name + extension;
}
