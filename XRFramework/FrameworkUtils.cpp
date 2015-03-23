#include "stdafxf.h"
#include "FrameworkUtils.h"
#include "utils/StringUtils.h"
#include "utils/UrlUtils.h"
#include "utils/SpecialProtocol.h"
#include "filesystem/File.h"
#include "filesystem/FileItem.h"
#include "filesystem/Directory.h"
#include <shlobj.h>

std::string CFUtil::GetNextFilename(const std::string &fn_template, int max)
{
	if (fn_template.find("%03d") == std::string::npos)
		return "";

	std::string searchPath = UrlUtils::GetDirectory(fn_template);
	std::string mask = UrlUtils::GetExtension(fn_template);
	std::string name = StringUtils::Format(fn_template.c_str(), 0);

	CFileItemList items;
	if (!CDirectory::GetDirectory(searchPath, items, mask, DIR_FLAG_NO_FILE_DIRS))
		return name;

	items.SetFastLookup(true);
	for (int i = 0; i <= max; i++)
	{
		std::string name = StringUtils::Format(fn_template.c_str(), i);
		if (!items.Get(name))
			return name;
	}
	return "";
}

std::string CFUtil::GetNextPathname(const std::string &path_template, int max)
{
	if (path_template.find("%04d") == std::string::npos)
		return "";

	for (int i = 0; i <= max; i++)
	{
		std::string name = StringUtils::Format(path_template.c_str(), i);
		if (!CFile::Exists(name) && !CDirectory::Exists(name))
			return name;
	}
	return "";
}