#include "stdafxf.h"
#include "URIUtils.h"
#include "URL.h"
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
