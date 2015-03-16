#pragma once
#include "File.h"
#include "utils/URL.h"
#include "utils/Variant.h"
#include "utils/DateTime.h"
#include <boost/shared_ptr.hpp>
#include "../XRThreads/CriticalSection.h"

class FileItem
{
public:
	FileItem();
	FileItem(const FileItem& item);
	FileItem(const CUrl& path, bool bIsFolder);
	FileItem(const std::string& strPath, bool bIsFolder);
	~FileItem();


	const CUrl GetURL() const { CUrl url(m_strPath); return url; };
	void SetURL(const CUrl& url);
	bool IsURL(const CUrl& url) const;
	const std::string &GetPath() const { return m_strPath; };
	void SetPath(const std::string &path) { m_strPath = path; m_url = CUrl(path); FillInMimeType(); };
	bool IsPath(const std::string& path) const;

	/*! \brief reset class to it's default values as per construction.
	Free's all allocated memory.
	\sa Initialize
	*/
	void Reset();
	const FileItem& operator=(const FileItem& item);
	bool Exists(bool bUseCache = true) const;
	void FillInMimeType(bool lookup = true/*= true*/);
	/* sets the mime-type if known beforehand */
	void SetMimeType(const std::string& mimetype) { m_mimetype = mimetype; };
	std::string GetMimeType() { return m_mimetype; };

	struct icompare
	{
		bool operator()(const std::string &s1, const std::string &s2) const;
	};

	typedef std::map<std::string, CVariant, icompare> PropertyMap;
	PropertyMap m_mapProperties;
	void SetProperty(const std::string &strKey, const CVariant &value);
	CVariant   GetProperty(const std::string &strKey) const;


	CDateTime m_dateTime;             ///< file creation date & time
	int64_t m_dwSize;             ///< file size (0 for folders)
	bool	m_bIsFolder;

private:
	void Initialize();

	std::string m_strPath,
				m_mimetype;
	CUrl		m_url;
	CVariant    m_properties;
};

/*!
\brief A shared pointer to CFileItem
\sa CFileItem
*/
typedef boost::shared_ptr<FileItem> FileItemPtr;

/*!
\brief A vector of pointer to CFileItem
\sa CFileItem
*/
typedef std::vector<FileItemPtr> VECFILEITEMS;

/*!
\brief Iterator for VECFILEITEMS
\sa CFileItemList
*/
typedef std::vector<FileItemPtr>::iterator IVECFILEITEMS;

/*!
\brief A map of pointers to CFileItem
\sa CFileItem
*/
typedef std::map<std::string, FileItemPtr> MAPFILEITEMS;

/*!
\brief Iterator for MAPFILEITEMS
\sa MAPFILEITEMS
*/
typedef std::map<std::string, FileItemPtr>::iterator IMAPFILEITEMS;

/*!
\brief Pair for MAPFILEITEMS
\sa MAPFILEITEMS
*/
typedef std::pair<std::string, FileItemPtr > MAPFILEITEMSPAIR;

typedef bool(*FILEITEMLISTCOMPARISONFUNC) (const FileItemPtr &pItem1, const FileItemPtr &pItem2);
typedef void(*FILEITEMFILLFUNC) (FileItemPtr &item);

/*!
\brief Represents a list of files
\sa CFileItemList, CFileItem
*/
class FileItemList : public FileItem
{
public:
	enum CACHE_TYPE { CACHE_NEVER = 0, CACHE_IF_SLOW, CACHE_ALWAYS };
	FileItemList();
	explicit FileItemList(const std::string& strPath);
	virtual ~FileItemList();


	FileItemPtr operator[] (int iItem);
	const FileItemPtr operator[] (int iItem) const;
	FileItemPtr operator[] (const std::string& strPath);
	const FileItemPtr operator[] (const std::string& strPath) const;

	void Clear();

	void Add(const FileItemPtr &pItem);
	void AddFront(const FileItemPtr &pItem, int itemPosition);
	void Remove(FileItem* pItem);
	void Remove(int iItem);
	void Append(const FileItemList& itemlist);
	void Assign(const FileItemList& itemlist, bool append = false);
	bool Copy(const FileItemList& item, bool copyItems = true);
	void Reserve(int iCount);

	FileItemPtr Get(int iItem);
	const FileItemPtr Get(int iItem) const;
	const VECFILEITEMS GetList() const { return m_items; }
	FileItemPtr Get(const std::string& strPath);
	const FileItemPtr Get(const std::string& strPath) const;

	int Size() const;
	bool IsEmpty() const;

	int GetFolderCount() const;
	int GetFileCount() const;
	 
	void SetFastLookup(bool fastLookup);
	bool Contains(const std::string& fileName) const;
	bool GetFastLookup() const { return m_fastLookup; };


private:
	MAPFILEITEMS m_map;
	VECFILEITEMS m_items;

	bool m_fastLookup;
	bool m_bIsFolder;
	XR::CCriticalSection m_critSect;
};