#include "stdafxf.h"
#include "URIUtils.h"
#include "filesystem/URL.h"
#include "StringUtils.h"


URIUtils::URIUtils(void)
{
}

URIUtils::~URIUtils(void)
{
}

const CStdString URIUtils::GetFileName(const CStdString& strFileNameAndPath)
{
	if (IsURL(strFileNameAndPath))
	{
		CURL url(strFileNameAndPath);
		return GetFileName(url.GetFileName());
	}

	/* find the last slash */
	const size_t slash = strFileNameAndPath.find_last_of("/\\");
	return strFileNameAndPath.substr(slash + 1);
}

void URIUtils::AddSlashAtEnd(std::string& strFolder)
{
	if (IsURL(strFolder))
	{
		CURL url(strFolder);
		std::string file = url.GetFileName();
		if (!file.empty() && file != strFolder)
		{
			AddSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
		}
		return;
	}

	if (!HasSlashAtEnd(strFolder))
	{
		if (IsDOSPath(strFolder))
			strFolder += '\\';
		else
			strFolder += '/';
	}
}

bool URIUtils::HasSlashAtEnd(const std::string& strFile, bool checkURL /* = false */)
{
	if (strFile.empty()) return false;
	if (checkURL && IsURL(strFile))
	{
		CURL url(strFile);
		CStdString file = url.GetFileName();
		return file.empty() || HasSlashAtEnd(file, false);
	}
	char kar = strFile.c_str()[strFile.size() - 1];

	if (kar == '/' || kar == '\\')
		return true;

	return false;
}

void URIUtils::RemoveSlashAtEnd(std::string& strFolder)
{
	if (IsURL(strFolder))
	{
		CURL url(strFolder);
		std::string file = url.GetFileName();
		if (!file.empty() && file != strFolder)
		{
			RemoveSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
			return;
		}
		if (url.GetHostName().empty())
			return;
	}

	while (HasSlashAtEnd(strFolder))
		strFolder.erase(strFolder.size() - 1, 1);
}

bool URIUtils::IsURL(const CStdString& strFile)
{
	return strFile.find("://") != std::string::npos;
}

bool URIUtils::IsURL(const std::string& strFile)
{
	return strFile.find("://") != std::string::npos;
}

bool URIUtils::IsDOSPath(const std::string& path)
{
	if (path.size() > 1 && path[1] == ':' && isalpha(path[0]))
		return true;

	// windows network drives
	if (path.size() > 1 && path[0] == '\\' && path[1] == '\\')
		return true;

	return false;
}

bool URIUtils::IsDOSPath(const CStdString& path)
{
	if (path.size() > 1 && path[1] == ':' && isalpha(path[0]))
		return true;

	// windows network drives
	if (path.size() > 1 && path[0] == '\\' && path[1] == '\\')
		return true;

	return false;
}

CStdString URIUtils::AddFileToFolder(const CStdString& strFolder,
	const CStdString& strFile)
{
	if (IsURL(strFolder))
	{
		CURL url(strFolder);
		if (url.GetFileName() != strFolder)
		{
			url.SetFileName(AddFileToFolder(url.GetFileName(), strFile));
			return url.Get();
		}
	}

	CStdString strResult = strFolder;
	if (!strResult.empty())
		AddSlashAtEnd(strResult);

	// Remove any slash at the start of the file
	if (strFile.size() && (strFile[0] == '/' || strFile[0] == '\\'))
		strResult += strFile.substr(1);
	else
		strResult += strFile;

	// correct any slash directions
	if (!IsDOSPath(strFolder))
		StringUtils::Replace(strResult, '\\', '/');
	else
		StringUtils::Replace(strResult, '/', '\\');

	return strResult;
}

bool URIUtils::IsInternetStream(const std::string &path, bool bStrictCheck /* = false */)
{
	const CURL pathToUrl(path);
	return IsInternetStream(pathToUrl, bStrictCheck);
}

bool URIUtils::IsInternetStream(const CURL& url, bool bStrictCheck /* = false */)
{
	if (url.GetProtocol().empty())
		return false;

	// there's nothing to stop internet streams from being stacked
	if (url.IsProtocol("stack"))
		return false;

	// Special case these
	if (url.IsProtocol("ftp") || url.IsProtocol("ftps") ||
		url.IsProtocol("dav") || url.IsProtocol("davs") ||
		url.IsProtocol("sftp"))
		return bStrictCheck;

	std::string protocol = url.GetTranslatedProtocol();
	if (CURL::IsProtocolEqual(protocol, "http") || CURL::IsProtocolEqual(protocol, "https") ||
		CURL::IsProtocolEqual(protocol, "tcp") || CURL::IsProtocolEqual(protocol, "udp") ||
		CURL::IsProtocolEqual(protocol, "rtp") || CURL::IsProtocolEqual(protocol, "sdp") ||
		CURL::IsProtocolEqual(protocol, "mms") || CURL::IsProtocolEqual(protocol, "mmst") ||
		CURL::IsProtocolEqual(protocol, "mmsh") || CURL::IsProtocolEqual(protocol, "rtsp") ||
		CURL::IsProtocolEqual(protocol, "rtmp") || CURL::IsProtocolEqual(protocol, "rtmpt") ||
		CURL::IsProtocolEqual(protocol, "rtmpe") || CURL::IsProtocolEqual(protocol, "rtmpte") ||
		CURL::IsProtocolEqual(protocol, "rtmps"))
		return true;

	return false;
}