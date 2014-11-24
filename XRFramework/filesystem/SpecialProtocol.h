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
	static void SetAPPPath(const std::string &path);
	static void SetAPPBinPath(const std::string &path);
	static void SetDocPath(const std::string &path);
	static void SetHomePath(const std::string &path);
	static void SetTempPath(const std::string &path);

	static bool ComparePath(const std::string &path1, const std::string &path2);
	static void LogPaths();

	static std::string TranslatePath(const std::string &path);
	static std::string TranslatePath(const CURL &url);
	static std::string TranslatePathConvertCase(const std::string& path);

private:
	static void SetPath(const std::string &key, const std::string &path);
	static std::string GetPath(const std::string &key);

	static std::map<std::string, std::string> m_pathMap;
};

#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_STRING "\\"

