#pragma once
#include "IDirectory.h"
class CDirectory
{
public:
	CDirectory();
	~CDirectory();

	class Hints
	{
	public:
		Hints() : flags(DIR_FLAG_DEFAULTS)
		{
		};
		std::string mask;
		int flags;
	};

	static bool GetDirectory(const CURL& url
		, CFileItemList &items
		, const std::string &strMask = ""
		, int flags = DIR_FLAG_DEFAULTS
		, bool allowThreads = false);

	static bool GetDirectory(const CURL& url
		, CFileItemList &items
		, const Hints &hints
		, bool allowThreads = false);

	static bool GetDirectory(const std::string& strPath
		, CFileItemList &items
		, const std::string &strMask = ""
		, int flags = DIR_FLAG_DEFAULTS
		, bool allowThreads = false);

	static bool GetDirectory(const std::string& strPath
		, CFileItemList &items
		, const Hints &hints
		, bool allowThreads = false);

	static bool Create(const CURL& url);
	static bool Exists(const CURL& url, bool bUseCache = true);
	static bool Remove(const CURL& url);

	static bool Create(const std::string& strPath);
	static bool Exists(const std::string& strPath, bool bUseCache = true);
	static bool Remove(const std::string& strPath);

};

IDirectory* DirectoryFactory(const CURL& url);