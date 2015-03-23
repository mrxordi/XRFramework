#pragma once
#include "File.h"
#include "utils/URL.h"
#include "utils/Variant.h"
#include "utils/DateTime.h"
#include "../XRThreads/CriticalSection.h"

class CFileItem
{
public:
	CFileItem();
	CFileItem(const CFileItem& item);
	CFileItem(const CURL& path, bool bIsFolder);
	CFileItem(const std::string& strPath, bool bIsFolder);
	~CFileItem();


	const CURL GetURL() const { CURL url(m_strPath); return url; };
	void SetURL(const CURL& url);
	bool IsURL(const CURL& url) const;
	const std::string &GetPath() const { return m_strPath; };
	void SetPath(const std::string &path) { m_strPath = path; m_url = CURL(path); FillInMimeType(); };
	bool IsPath(const std::string& path) const;

	/*! \brief reset class to it's default values as per construction.
	Free's all allocated memory.
	\sa Initialize
	*/
	void Reset();
	const CFileItem& operator=(const CFileItem& item);
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
	CURL		m_url;
	CVariant    m_properties;
};

/*!
\brief A shared pointer to CFileItem
\sa CFileItem
*/
typedef std::shared_ptr<CFileItem> CFileItemPtr;

/*!
\brief A vector of pointer to CFileItem
\sa CFileItem
*/
typedef std::vector<CFileItemPtr> VecFileItem;

/*!
\brief Iterator for VECFILEITEMS
\sa CFileItemList
*/
typedef std::vector<CFileItemPtr>::iterator VecFileItemsIterator;

/*!
\brief A map of pointers to CFileItem
\sa CFileItem
*/
typedef std::map<std::string, CFileItemPtr> MapFileItem;

/*!
\brief Iterator for MAPFILEITEMS
\sa MAPFILEITEMS
*/
typedef std::map<std::string, CFileItemPtr>::iterator MapFileItemIterator;

/*!
\brief Pair for MAPFILEITEMS
\sa MAPFILEITEMS
*/
typedef std::pair<std::string, CFileItemPtr > PairFileItem;

typedef bool(*FILEITEMLISTCOMPARISONFUNC) (const CFileItemPtr &pItem1, const CFileItemPtr &pItem2);
typedef void(*FILEITEMFILLFUNC) (CFileItemPtr &item);

/*!
\brief Represents a list of files
\sa CFileItemList, CFileItem
*/
class CFileItemList : public CFileItem
{
public:
	enum CACHE_TYPE { CACHE_NEVER = 0, CACHE_IF_SLOW, CACHE_ALWAYS };
	CFileItemList();
	explicit CFileItemList(const std::string& strPath);
	virtual ~CFileItemList();


	CFileItemPtr operator[] (int iItem);
	const CFileItemPtr operator[] (int iItem) const;
	CFileItemPtr operator[] (const std::string& strPath);
	const CFileItemPtr operator[] (const std::string& strPath) const;

	void Clear();

	void Add(const CFileItemPtr &pItem);
	void AddFront(const CFileItemPtr &pItem, int itemPosition);
	void Remove(CFileItem* pItem);
	void Remove(int iItem);
	void Append(const CFileItemList& itemlist);
	void Assign(const CFileItemList& itemlist, bool append = false);
	bool Copy(const CFileItemList& item, bool copyItems = true);
	void Reserve(int iCount);

	CFileItemPtr Get(int iItem);
	const CFileItemPtr Get(int iItem) const;
	const VecFileItem GetList() const { return m_items; }
	CFileItemPtr Get(const std::string& strPath);
	const CFileItemPtr Get(const std::string& strPath) const;

	int Size() const;
	bool IsEmpty() const;

	int GetFolderCount() const;
	int GetFileCount() const;

	void SetFastLookup(bool fastLookup);
	bool Contains(const std::string& fileName) const;
	bool GetFastLookup() const { return m_fastLookup; };


private:
	MapFileItem m_map;
	VecFileItem m_items;

	bool m_fastLookup;
	bool m_bIsFolder;
	XR::CCriticalSection m_critSect;
};