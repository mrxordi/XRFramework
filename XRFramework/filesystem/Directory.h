#pragma once
#include "IDirectory.h"
class Directory
{
public:
	Directory();
	~Directory();

	class Hints
	{
	public:
		Hints() : flags(DIR_FLAG_DEFAULTS)
		{
		};
		std::string mask;
		int flags;
	};

	static bool GetDirectory(const CUrl& url
		, FileItemList &items
		, const std::string &strMask = ""
		, int flags = DIR_FLAG_DEFAULTS
		, bool allowThreads = false);

	static bool GetDirectory(const CUrl& url
		, FileItemList &items
		, const Hints &hints
		, bool allowThreads = false);

	static bool GetDirectory(const std::string& strPath
		, FileItemList &items
		, const std::string &strMask = ""
		, int flags = DIR_FLAG_DEFAULTS
		, bool allowThreads = false);

	static bool GetDirectory(const std::string& strPath
		, FileItemList &items
		, const Hints &hints
		, bool allowThreads = false);

	static bool Create(const CUrl& url);
	static bool Exists(const CUrl& url, bool bUseCache = true);
	static bool Remove(const CUrl& url);

	static bool Create(const std::string& strPath);
	static bool Exists(const std::string& strPath, bool bUseCache = true);
	static bool Remove(const std::string& strPath);

};

IDirectory* DirectoryFactory(const CUrl& url);