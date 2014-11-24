#pragma once
// 
// Copyright (c) Kris Gustafson, 2012
// krisgus@ieee.org
// 
#include <string>

bool FileExistsW(const wchar_t* filePath);
bool FileExistsA(const char* filePath);

std::wstring WStrFileAndExt(const wchar_t* filePath);
std::string  StrFileAndExt(const char* filePath);

std::string  wchar_to_char_string(const wchar_t* theWString, const char substituteCharacter = (char)0xfe);
std::wstring char_to_wchar_string(const char* theString);