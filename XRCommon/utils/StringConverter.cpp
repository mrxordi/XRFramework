#include "stdafx.h"
#include "WIN32Utils.h"
#include "StringConverter.h"
#include "Converter.h"
#include "ConverterFactory.h"
#include "log/Log.h"



StringConverter::StringConverter()
{
}


StringConverter::~StringConverter()
{
}

bool StringConverter::utf8ToW(const std::string& utf8StringSrc, std::wstring& wStringDst, bool failOnBadChar) {
	return Convert(UTF8_SOURCE, WCHAR_CHARSET, utf8StringSrc, wStringDst, failOnBadChar);
}

bool StringConverter::WToUtf8(const std::wstring& wStringSrc, std::string& utf8StringDst, bool failOnBadChar) {
	return Convert(WCHAR_CHARSET, UTF8_SOURCE, wStringSrc, utf8StringDst, failOnBadChar);
}

bool StringConverter::Win1250ToUtf8(const std::string& win1250StringSrc, std::string& utf8StringDst, bool failOnBadChar) {
	return Convert("WINDOWS-1250", UTF8_SOURCE, win1250StringSrc, utf8StringDst, failOnBadChar);
}

bool StringConverter::Win1250ToW(const std::string& win1250StringSrc, std::wstring& wStringDst, bool failOnBadChar) {
	return Convert("WINDOWS-1250", WCHAR_CHARSET, win1250StringSrc, wStringDst, failOnBadChar);
}

bool StringConverter::Utf8ToWin1250(const std::string& utf8StringSrc, std::string& win1250StringDst, bool failOnBadChar) {
	return Convert(UTF8_SOURCE, "WINDOWS-1250", utf8StringSrc, win1250StringDst, failOnBadChar);
}

bool StringConverter::WToWin1250(const std::wstring& wStringSrc, std::string& win1250StringDst, bool failOnBadChar) {
	return Convert(WCHAR_CHARSET, "WINDOWS-1250", wStringSrc, win1250StringDst, failOnBadChar);
}


std::wstring StringConverter::ConvertPathToWin32Form(const std::string& pathUtf8)
{
	std::wstring result;
	if (pathUtf8.empty())
		return result;

	bool convertResult;

	if (pathUtf8.compare(0, 2, "\\\\", 2) != 0) // pathUtf8 don't start from "\\"
	{ // assume local file path in form 'C:\Folder\File.ext'
		std::string formedPath("\\\\?\\"); // insert "\\?\" prefix
		formedPath += URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(pathUtf8, '\\'), '\\'); // fix duplicated and forward slashes, resolve relative path
		convertResult = StringConverter::utf8ToW(formedPath, result, true);
	}
	else if (pathUtf8.compare(0, 8, "\\\\?\\UNC\\", 8) == 0) // pathUtf8 starts from "\\?\UNC\"
	{
		std::string formedPath("\\\\?\\UNC"); // start from "\\?\UNC" prefix
		formedPath += URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(pathUtf8.substr(7), '\\'), '\\'); // fix duplicated and forward slashes, resolve relative path, don't touch "\\?\UNC" prefix,
		convertResult = StringConverter::utf8ToW(formedPath, result, true);
	}
	else if (pathUtf8.compare(0, 4, "\\\\?\\", 4) == 0) // pathUtf8 starts from "\\?\", but it's not UNC path
	{
		std::string formedPath("\\\\?"); // start from "\\?" prefix
		formedPath += URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(pathUtf8.substr(3), '\\'), '\\'); // fix duplicated and forward slashes, resolve relative path, don't touch "\\?" prefix,
		convertResult = StringConverter::utf8ToW(formedPath, result, true);
	}
	else // pathUtf8 starts from "\\", but not from "\\?\UNC\"
	{ // assume UNC path in form '\\server\share\folder\file.ext'
		std::string formedPath("\\\\?\\UNC"); // append "\\?\UNC" prefix
		formedPath += URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(pathUtf8), '\\'); // fix duplicated and forward slashes, resolve relative path, transform "\\" prefix to single "\"
		convertResult = StringConverter::utf8ToW(formedPath, result, true);
	}

	if (!convertResult)
	{
		LOGERR("Error converting path \"%s\" to Win32 wide string!", pathUtf8.c_str());
		return L"";
	}

	return result;
}

std::wstring StringConverter::ConvertPathToWin32Form(const CURL& url)
{
	assert(url.GetProtocol().empty() || url.IsProtocol("smb"));

	if (url.GetFileName().empty())
		return std::wstring(); // empty string

	if (url.GetProtocol().empty())
	{
		std::wstring result;
		if (StringConverter::utf8ToW("\\\\?\\" +
			URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(url.GetFileName(), '\\'), '\\'), result, true))
			return result;
	}
	else if (url.IsProtocol("smb"))
	{
		if (url.GetHostName().empty())
			return std::wstring(); // empty string

		std::wstring result;
		if (StringConverter::utf8ToW("\\\\?\\UNC\\" +
			URIUtils::CanonicalizePath(URIUtils::FixSlashesAndDups(url.GetHostName() + '\\' + url.GetFileName(), '\\'), '\\'),
			result, true))
			return result;
	}
	else
		return std::wstring(); // unsupported protocol, return empty string

	LOGERR("Error converting path \"%s\" to Win32 form", url.Get().c_str());
	return std::wstring(); // empty string
}

template<class INPUT, class OUTPUT>
bool StringConverter::Convert(const std::string& sourceCharset, const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
{
	strDest.clear();
	if (strSource.empty())
		return true;

	const int dstMultp = (targetCharset.compare(0, 5, "UTF-8") == 0) ? 4 : 1;

	Converter* converter = ConverterFactory::CreateConverter(sourceCharset, targetCharset, dstMultp);
	CSingleLock converterLock(*converter);

	return InnerConvert(converter->GetConverter(converterLock), converter->GetTargetSingleCharMaxLen(), strSource, strDest, failOnInvalidChar);
}

/* iconv may declare inbuf to be char** rather than const char** depending on platform and version,
so provide a wrapper that handles both */
struct charPtrPtrAdapter
{
	const char** pointer;
	charPtrPtrAdapter(const char** p) :
		pointer(p) { }
	operator char**()
	{
		return const_cast<char**>(pointer);
	}
	operator const char**()
	{
		return pointer;
	}
};

template<class INPUT, class OUTPUT>
bool StringConverter::InnerConvert(iconv_t type, int multiplier, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
{
	if (type == NO_ICONV)
		return false;

	//input buffer for iconv() is the buffer from strSource
	size_t      inBufSize = (strSource.length() + 1) * sizeof(typename INPUT::value_type);
	const char* inBuf = (const char*)strSource.c_str();

	//allocate output buffer for iconv()
	size_t      outBufSize = (strSource.length() + 1) * sizeof(typename OUTPUT::value_type) * multiplier;
	char*       outBuf = (char*)malloc(outBufSize);
	if (outBuf == NULL)
	{
		LOGSEVERE("Malloc failed.");
		return false;
	}

	size_t      inBytesAvail = inBufSize;  //how many bytes iconv() can read
	size_t      outBytesAvail = outBufSize; //how many bytes iconv() can write
	const char* inBufStart = inBuf;      //where in our input buffer iconv() should start reading
	char*       outBufStart = outBuf;     //where in out output buffer iconv() should start writing

	size_t returnV;

	while (1)
	{
		//iconv() will update inBufStart, inBytesAvail, outBufStart and outBytesAvail
		returnV = iconv(type, charPtrPtrAdapter(&inBufStart), &inBytesAvail, &outBufStart, &outBytesAvail);

		if (returnV == (size_t)-1)
		{
			if (errno == E2BIG) //output buffer is not big enough
			{
				//save where iconv() ended converting, realloc might make outBufStart invalid
				size_t bytesConverted = outBufSize - outBytesAvail;

				//make buffer twice as big
				outBufSize *= 2;
				char* newBuf = (char*)realloc(outBuf, outBufSize);
				if (!newBuf)
				{
					LOGSEVERE("Realloc failed with errno=%d(%s)",
						errno, strerror(errno));
					break;
				}
				outBuf = newBuf;

				//update the buffer pointer and counter
				outBufStart = outBuf + bytesConverted;
				outBytesAvail = outBufSize - bytesConverted;

				//continue in the loop and convert the rest
				continue;
			}
			else if (errno == EILSEQ) //An invalid multibyte sequence has been encountered in the input
			{
				if (failOnInvalidChar)
					break;

				//skip invalid byte
				inBufStart++;
				inBytesAvail--;
				//continue in the loop and convert the rest
				continue;
			}
			else if (errno == EINVAL) /* Invalid sequence at the end of input buffer */
			{
				if (!failOnInvalidChar)
					returnV = 0; /* reset error status to use converted part */

				break;
			}
			else //iconv() had some other error
			{
				LOGERR("iconv() failed, errno=%d (%s)", errno, strerror(errno));
			}
		}
		break;
	}

	//complete the conversion (reset buffers), otherwise the current data will prefix the data on the next call
	if (iconv(type, NULL, NULL, &outBufStart, &outBytesAvail) == (size_t)-1)
		LOGERR("Failed cleanup errno=%d(%s)", errno, strerror(errno));

	if (returnV == (size_t)-1)
	{
		free(outBuf);
		return false;
	}
	//we're done

	const typename OUTPUT::size_type sizeInChars = (typename OUTPUT::size_type) (outBufSize - outBytesAvail) / sizeof(typename OUTPUT::value_type);
	typename OUTPUT::const_pointer strPtr = (typename OUTPUT::const_pointer) outBuf;
	/* Make sure that all buffer is assigned and string is stopped at end of buffer */
	if (strPtr[sizeInChars - 1] == 0 && strSource[strSource.length() - 1] != 0)
		strDest.assign(strPtr, sizeInChars - 1);
	else
		strDest.assign(strPtr, sizeInChars);

	free(outBuf);

	return true;
}