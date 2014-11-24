#pragma once

#include <map>
#include "../utils/StdString.h"

// static class for path translation from our special:// URLs.

/* paths are as follows:

 special://app/          - the main XBMC folder (i.e. where the app resides).
 special://appbin/          - the main XBMC folder (i.e. where the app resides).
 special://home/          - a writeable version of the main XBMC folder
 Win32: ~/Application Data/XBMC/
 special://doc/      - a writable version of the user home directory
 Win32: home directory of user
 special://temp/          - the temporary directory.
 Win32: ~/Application Data/XBMC/cache
 */
class CURL;
class CSpecialProtocol
{
public:
	static void SetAPPPath(const CStdString &path);
	static void SetAPPBinPath(const CStdString &path);
	static void SetDocPath(const CStdString &path);
	static void SetHomePath(const CStdString &path);
	static void SetTempPath(const CStdString &path);

	static bool ComparePath(const CStdString &path1, const CStdString &path2);
	static void LogPaths();

	static std::string TranslatePath(const std::string &path);
	static std::string TranslatePath(const CURL &url);
	static CStdString TranslatePathConvertCase(const CStdString& path);

private:
	static void SetPath(const CStdString &key, const CStdString &path);
	static CStdString GetPath(const CStdString &key);

	static std::map<CStdString, CStdString> m_pathMap;
};

#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_STRING "\\"

