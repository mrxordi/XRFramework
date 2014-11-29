#pragma once
#include <iconv.h>
#include <string>

#if defined(TARGET_WINDOWS)
#define WCHAR_IS_UTF16 1
#define UTF16_CHARSET "UTF-16" "LE"
#define UTF32_CHARSET "UTF-32" "LE"
#define UTF8_SOURCE "UTF-8"
#define WCHAR_CHARSET UTF16_CHARSET 
//#pragma comment(lib, "libiconv.lib")
#endif

class CURL;

class StringConverter
{
public:
	StringConverter();
	~StringConverter();

	static bool utf8ToW(const std::string& utf8StringSrc, std::wstring& wStringDst, bool failOnBadChar = false);
	static bool WToUtf8(const std::wstring& wStringSrc, std::string& utf8StringDst, bool failOnBadChar = false);
	static bool Win1250ToUtf8(const std::string& win1250StringSrc, std::string& utf8StringDst, bool failOnBadChar = false);
	static bool Win1250ToW(const std::string& win1250StringSrc, std::wstring& wStringDst, bool failOnBadChar = false);
	static bool Utf8ToWin1250(const std::string& win1250StringSrc, std::string& utf8StringDst, bool failOnBadChar = false);
	static bool WToWin1250(const std::wstring& win1250StringSrc, std::string& wStringDst, bool failOnBadChar = false);

	static std::wstring ConvertPathToWin32Form(const std::string& pathUtf8);
	static std::wstring ConvertPathToWin32Form(const CURL& url);

	template<class INPUT, class OUTPUT>
	static bool Convert(const std::string& sourceCharset, const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar = false);

private:

	template<class INPUT, class OUTPUT>
	static bool InnerConvert(iconv_t type, int multiplier, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar = false);

	iconv_t GetConverter(std::string& sourceCharset, std::string& targetCharset);


};

