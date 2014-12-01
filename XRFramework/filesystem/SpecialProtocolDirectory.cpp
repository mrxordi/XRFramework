#include "stdafxf.h"
#include "SpecialProtocolDirectory.h"
#include "SpecialProtocol.h"
#include "Directory.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "FileItem.h"
#include "URL.h"


SpecialProtocolDirectory::SpecialProtocolDirectory(void)
{
}

SpecialProtocolDirectory::~SpecialProtocolDirectory(void)
{
}

bool SpecialProtocolDirectory::GetDirectory(const CURL& url, FileItemList &items)
{
	const std::string pathToUrl(url.Get());
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	if (Directory::GetDirectory(translatedPath, items, m_strFileMask, m_flags | DIR_FLAG_GET_HIDDEN))
	{ // replace our paths as necessary
		items.SetURL(url);
		for (int i = 0; i < items.Size(); i++)
		{
			FileItemPtr item = items[i];
			if (StringUtils::StartsWith(item->GetPath(), translatedPath))
				item->SetPath(URIUtils::AddFileToFolder(pathToUrl, item->GetPath().substr(translatedPath.size())));
		}
		return true;
	}
	return false;
}

bool SpecialProtocolDirectory::Create(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return Directory::Create(translatedPath.c_str());
}

bool SpecialProtocolDirectory::Remove(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return Directory::Remove(translatedPath.c_str());
}

bool SpecialProtocolDirectory::Exists(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return Directory::Exists(translatedPath.c_str());
}
