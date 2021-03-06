#include "stdafx.h"
#include "SpecialProtocol.h"
#include "Util.h"
#include "log/Log.h"
#include "URL.h"
#include "utils/UrlUtils.h"
#include "utils/StringUtils.h"

using namespace std;

map<string, string> CSpecialProtocol::m_pathMap;

void CSpecialProtocol::SetAPPPath(const std::string &dir)
{
	SetPath("app", dir);
}

void CSpecialProtocol::SetAPPBinPath(const std::string &dir)
{
	SetPath("appbin", dir);
}

void CSpecialProtocol::SetDocPath(const std::string &dir)
{
	SetPath("doc", dir);
}

void CSpecialProtocol::SetHomePath(const std::string &dir)
{
	SetPath("home", dir);
}

void CSpecialProtocol::SetTempPath(const std::string &dir)
{
	SetPath("temp", dir);
}

bool CSpecialProtocol::ComparePath(const std::string &path1, const std::string &path2)
{
	return TranslatePath(path1) == TranslatePath(path2);
}

std::string CSpecialProtocol::TranslatePath(const std::string &path)
{
	CURL url(path);
	// check for special-protocol, if not, return
	if (url.GetProtocol() != "special")
	{
		return path;
	}
	return TranslatePath(url);
}

std::string CSpecialProtocol::TranslatePath(const CURL &url)
{
	// check for special-protocol, if not, return
	if (!(url.GetProtocol() == "special"))
	{
		return url.Get();
	}

	std::string FullFileName = url.GetFileName();

	std::string translatedPath;
	std::string FileName;
	std::string RootDir;

	// Split up into the special://root and the rest of the filename
	size_t pos = FullFileName.find('/');
	if (pos != std::string::npos && pos > 1)
	{
		RootDir = FullFileName.substr(0, pos);

		if (pos < FullFileName.size())
			FileName = FullFileName.substr(pos + 1);
	}
	else
		RootDir = FullFileName;

	//   if (RootDir.Equals("subtitles"))
	//     translatedPath = UrlUtils::AddFileToFolder(CSettings::Get().GetString("subtitles.custompath"), FileName);
	//   else if (RootDir.Equals("userdata"))
	//     translatedPath = UrlUtils::AddFileToFolder(CProfilesManager::Get().GetUserDataFolder(), FileName);
	//   else if (RootDir.Equals("database"))
	//     translatedPath = UrlUtils::AddFileToFolder(CProfilesManager::Get().GetDatabaseFolder(), FileName);
	//   else if (RootDir.Equals("thumbnails"))
	//     translatedPath = UrlUtils::AddFileToFolder(CProfilesManager::Get().GetThumbnailsFolder(), FileName);
	//   else if (RootDir.Equals("recordings") || RootDir.Equals("cdrips"))
	//     translatedPath = UrlUtils::AddFileToFolder(CSettings::Get().GetString("audiocds.recordingpath"), FileName);
	//   else if (RootDir.Equals("screenshots"))
	//     translatedPath = UrlUtils::AddFileToFolder(CSettings::Get().GetString("debug.screenshotpath"), FileName);
	//   else if (RootDir.Equals("musicplaylists"))
	//     translatedPath = UrlUtils::AddFileToFolder(CUtil::MusicPlaylistsLocation(), FileName);
	//   else if (RootDir.Equals("videoplaylists"))
	//     translatedPath = UrlUtils::AddFileToFolder(CUtil::VideoPlaylistsLocation(), FileName);
	//   else if (RootDir.Equals("skin"))
	//     translatedPath = UrlUtils::AddFileToFolder(g_graphicsContext.GetMediaDir(), FileName);
	//   else if (RootDir.Equals("logpath"))
	//     translatedPath = UrlUtils::AddFileToFolder(g_advancedSettings.m_logFolder, FileName);


	// from here on, we have our "real" special paths
	if (RootDir == "app" ||
		RootDir == "appbin" ||
		RootDir == "doc" ||
		RootDir == "home" ||
		RootDir == "temp")

	{
		std::string basePath = GetPath(RootDir);
		if (!basePath.empty())
			translatedPath = UrlUtils::AddFileToFolder(basePath, FileName);
		else
			translatedPath.clear();
	}

	// Validate the final path, just in case
	return CUtil::ValidatePath(translatedPath);
}

std::string CSpecialProtocol::TranslatePathConvertCase(const std::string& path)
{
	std::string translatedPath = TranslatePath(path);

	return translatedPath;
}

void CSpecialProtocol::LogPaths()
{
	LOGINFO("/---- System paths are initialised  ----------------------------------------------------\\");
	LOGINFO("**special://app/ is mapped to: %s", GetPath("app").c_str());
	LOGINFO("**special://appbin/ is mapped to: %s", GetPath("appbin").c_str());
	LOGINFO("**special://home/ is mapped to: %s", GetPath("home").c_str());
	LOGINFO("**special://temp/ is mapped to: %s", GetPath("temp").c_str());
	LOGINFO("--special://doc/ is mapped to: %s", GetPath("doc").c_str());
	LOGINFO("\\---------------------------------------------------------------------------------------/");

}

// private routines, to ensure we only set/get an appropriate path
void CSpecialProtocol::SetPath(const std::string &key, const std::string &path)
{
	m_pathMap[key] = path;
//		m_pathMap.insert(std::pair<string, string>(key, path));
}

std::string CSpecialProtocol::GetPath(const std::string &key)
{
	map<std::string, std::string>::iterator it = m_pathMap.find(key);
	if (it != m_pathMap.end())
		return it->second;
	assert(false);
	return "";
}

void CSpecialProtocol::InitializePaths()
{
	std::string appPath, tempPath, docPath, homePath;

	appPath = CUtil::ResolveExecutablePath();
	tempPath = CUtil::ResolveSystemTempPath();
	docPath = CUtil::ResolveDocPath();
	homePath = CUtil::ResolveUserPath();

	CSpecialProtocol::SetAPPPath(appPath);
	CSpecialProtocol::SetAPPBinPath(appPath);
	CSpecialProtocol::SetTempPath(tempPath);
	CSpecialProtocol::SetDocPath(docPath);
	CSpecialProtocol::SetHomePath(homePath);
}
