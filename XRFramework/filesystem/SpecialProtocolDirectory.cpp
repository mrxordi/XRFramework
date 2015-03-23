#include "stdafxf.h"
#include "SpecialProtocolDirectory.h"
#include "utils/SpecialProtocol.h"
#include "Directory.h"
#include "utils/StringUtils.h"
#include "utils/UrlUtils.h"
#include "FileItem.h"
#include "utils/URL.h"


SpecialProtocolDirectory::SpecialProtocolDirectory(void)
{
}

SpecialProtocolDirectory::~SpecialProtocolDirectory(void)
{
}

bool SpecialProtocolDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
	const std::string pathToUrl(url.Get());
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	if (CDirectory::GetDirectory(translatedPath, items, m_strFileMask, m_flags | DIR_FLAG_GET_HIDDEN))
	{ // replace our paths as necessary
		items.SetURL(url);
		for (int i = 0; i < items.Size(); i++)
		{
			CFileItemPtr item = items[i];
			if (StringUtils::StartsWith(item->GetPath(), translatedPath))
				item->SetPath(UrlUtils::AddFileToFolder(pathToUrl, item->GetPath().substr(translatedPath.size())));
		}
		return true;
	}
	return false;
}

bool SpecialProtocolDirectory::Create(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return CDirectory::Create(translatedPath.c_str());
}

bool SpecialProtocolDirectory::Remove(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return CDirectory::Remove(translatedPath.c_str());
}

bool SpecialProtocolDirectory::Exists(const CURL& url)
{
	std::string translatedPath = CSpecialProtocol::TranslatePath(url);
	return CDirectory::Exists(translatedPath.c_str());
}
