// 
// Copyright (c) Kris Gustafson, 2012
// krisgus@ieee.org
// 
#include "stdafx.h"
#include "FilePathUtils.h"
#include <cstdlib>
#include <exception>
#include <new>
#include <errno.h>


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
	} else {
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
	} else {
		name = "";
		extension = "";
	}

	return name + extension;
}

/*! std::string  wchar_to_char_string(...)
Converts a UNICODE string (wchar_t*) to an ANSI string (char*).

in: wideString: the string to be converted from wchar_t* to char*.
Returns: a STL "string" with the converted character string.

Any wide character too large for char size is converted to the
"substituteCharacter" letter (by default the old English letter "thorn").
*/
std::string wchar_to_char_string(const wchar_t* wideString, const char substituteCharacter) {
	std::string charString;
	size_t wcharCount = wcslen( wideString) + sizeof(wchar_t);
	size_t charBuffLen = wcharCount * sizeof(wchar_t);
	char* buffer = new (std::nothrow) char[charBuffLen];

	if (buffer != NULL) {
		memset(buffer, 0, charBuffLen);
		size_t converted;
		int errNum = wcstombs_s(&converted, buffer, charBuffLen, wideString, wcharCount);
		if ( errNum == 0 ) {
			charString = buffer;
		} else {
			// wideString was NULL, buffer is too small or one or more wchar_t was > 255
			size_t i;
			for (i = 0; i < wcslen(wideString) && i < charBuffLen; ++i) {
				if (wideString[i] >= 0x00ff)
					buffer[i] = substituteCharacter;
				else
					buffer[i] = (char)wideString[i];
			}
			buffer[i] = 0;

			if (buffer[0] != 0)
				charString = buffer;
			else
				charString = "";
		}
		delete[] buffer;
	} else {
		charString = "";
	}

	return charString;
}


/*! std::wstring  char_to_wchar_string(...)
Converts an ANSI string (char*) to an ANSI string (wchar_t*).

in: ansiString: the string to be converted from char* to wchar_t*.
Returns: a STL "wstring" with the converted characters.
*/
std::wstring char_to_wchar_string(const char* ansiString) {
	std::wstring wcharString;
	if (ansiString == NULL || ansiString[0] == 0) {
		wcharString = L"";
	} else {
		size_t charBuffLen = (strlen(ansiString) + 1) * sizeof(wchar_t);
		wchar_t* buffer = new (std::nothrow) wchar_t[charBuffLen];
		if (buffer != NULL) {
			buffer[0] = 0;
			int idx = 0;
			for (; ansiString[idx] != 0; ++idx) {
				buffer[idx] = static_cast<wchar_t>(ansiString[idx]);
			}
			buffer[idx] = 0;
			wcharString = buffer;
			delete[] buffer;
		} else {
			wcharString = L"";
		}
	}

	return wcharString;
}
