#include "stdafxf.h"
#include "Win32Directory.h"
#include "utils/URL.h"
#include "FileItem.h"
#include "log/Log.h"
#include "utils/StringConverter.h"

// check for empty string, remove trailing slash if any, convert to win32 form
inline static std::wstring prepareWin32DirectoryName(const std::string& strPath)
{
	if (strPath.empty())
		return std::wstring(); // empty string

	std::wstring nameW(StringConverter::ConvertPathToWin32Form(strPath));
	if (!nameW.empty())
	{
		if (nameW.back() == L'\\')
			nameW.pop_back(); // remove slash at the end if any
		if (nameW.length() == 6 && nameW.back() == L':') // 6 is the length of "\\?\x:"
			nameW.push_back(L'\\'); // always add backslash for root folders
	}
	return nameW;
}



Win32Directory::Win32Directory()
{}


Win32Directory::~Win32Directory()
{}

bool Win32Directory::GetDirectory(const CURL& url, FileItemList &items)
{
	items.Clear();

	std::string pathWithSlash(url.Get());
	if (pathWithSlash.back() != '\\')
		pathWithSlash.push_back('\\');

	std::wstring searchMask(StringConverter::ConvertPathToWin32Form(pathWithSlash));
	if (searchMask.empty())
		return false;

	// TODO: support m_strFileMask, require rewrite of internal caching
	searchMask += L'*';

	HANDLE hSearch;
	WIN32_FIND_DATAW findData = {};

	hSearch = FindFirstFileExW(searchMask.c_str(), FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);

	if (hSearch == INVALID_HANDLE_VALUE)
		return GetLastError() == ERROR_FILE_NOT_FOUND ? Exists(url) : false; // return true if directory exist and empty

	do
	{
		std::wstring itemNameW(findData.cFileName);
		if (itemNameW == L"." || itemNameW == L"..")
			continue;

		std::string itemName;
		if (!StringConverter::WToUtf8(itemNameW, itemName, true) || itemName.empty())
		{
			LOGERR("Can't convert wide string name to UTF-8 encoding.");
			continue;
		}

		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && (m_flags & DIR_FLAG_NO_FILE_DIRS) == DIR_FLAG_NO_FILE_DIRS)
			continue; 

		FileItemPtr pItem(new FileItem);

		pItem->m_bIsFolder = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
		if (pItem->m_bIsFolder)
			pItem->SetPath(pathWithSlash + itemName + '\\');
		else
			pItem->SetPath(pathWithSlash + itemName);

		if ((findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0
			|| itemName.front() == '.') // mark files starting from dot as hidden
			pItem->SetProperty("file:hidden", true);

		// calculation of size and date costs a little on win32 
		// so DIR_FLAG_NO_FILE_INFO flag is ignored
		FILETIME localTime;
		if (FileTimeToLocalFileTime(&findData.ftLastWriteTime, &localTime) == TRUE)
			pItem->m_dateTime = localTime;
		else
			pItem->m_dateTime = 0;

		if (!pItem->m_bIsFolder)
			pItem->m_dwSize = (__int64(findData.nFileSizeHigh) << 32) + findData.nFileSizeLow;

		items.Add(pItem);
	} while (FindNextFileW(hSearch, &findData));

	FindClose(hSearch);

	return true;
}

bool Win32Directory::Create(const CURL& url)
{
	std::wstring nameW(prepareWin32DirectoryName(url.Get()));
	if (nameW.empty())
		return false;

	if (!CreateDirectoryW(nameW.c_str(), NULL))
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return Exists(url); // is it file or directory?
		else
			return false;
	}

	// if directory name starts from dot, make it hidden
	const size_t lastSlashPos = nameW.rfind(L'\\');
	if (lastSlashPos < nameW.length() - 1 && nameW[lastSlashPos + 1] == L'.')
	{
		DWORD dirAttrs = GetFileAttributesW(nameW.c_str());
		if (dirAttrs != INVALID_FILE_ATTRIBUTES && SetFileAttributesW(nameW.c_str(), dirAttrs | FILE_ATTRIBUTE_HIDDEN))
			return true;
	}

	return true;
}

bool Win32Directory::Exists(const CURL& url)
{
	std::wstring nameW(prepareWin32DirectoryName(url.Get()));
	if (nameW.empty())
		return false;

	DWORD fileAttrs = GetFileAttributesW(nameW.c_str());
	if (fileAttrs == INVALID_FILE_ATTRIBUTES || (fileAttrs & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return false;

	return true;
}

bool Win32Directory::Remove(const CURL& url)
{
	std::wstring nameW(prepareWin32DirectoryName(url.Get()));
	if (nameW.empty())
		return false;

	if (RemoveDirectoryW(nameW.c_str()))
		return true;

	return !Exists(url);
}