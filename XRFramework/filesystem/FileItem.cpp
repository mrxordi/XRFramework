#include "stdafxf.h"
#include "FileItem.h"
#include "File.h"
#include "CurlFile.h"
#include "MimeTypes.h"
#include "Directory.h"
#include "utils/StringUtils.h"
#include "../XRThreads/SingleLock.h"



FileItem::FileItem()
{
	Initialize();
}

FileItem::FileItem(const FileItem& item)
{
	*this = item;
}

FileItem::FileItem(const CURL& path, bool bIsFolder)
{
	Initialize();
	m_strPath = path.Get();
	m_bIsFolder = bIsFolder;
	m_url = CURL(path);
	if (m_bIsFolder && !m_strPath.empty())
		URIUtils::AddSlashAtEnd(m_strPath);
	FillInMimeType(false);
}

FileItem::FileItem(const std::string& strPath, bool bIsFolder)
{
	Initialize();
	m_strPath = strPath;
	m_bIsFolder = bIsFolder;
	m_url = CURL(strPath);
	if (m_bIsFolder && !m_strPath.empty())
		URIUtils::AddSlashAtEnd(m_strPath);
	FillInMimeType(false);
}

FileItem::~FileItem()
{
}

void FileItem::SetURL(const CURL& url)
{
	m_strPath = url.Get();
	m_url = CURL(url);
	m_properties.clear();
	FillInMimeType();
}

bool FileItem::IsURL(const CURL& url) const
{
	return URIUtils::PathEquals(url.Get(), m_strPath);
}

void FileItem::Reset()
{
	m_bIsFolder = false;
	m_strPath.clear();
	m_mimetype.clear();
	m_properties.clear();

	Initialize();
}

const FileItem& FileItem::operator=(const FileItem& item)
{
	if (this == &item) return *this;
	m_strPath = item.GetPath();
	m_url = CURL(m_strPath);
	FillInMimeType();
	return *this;
}

bool FileItem::Exists(bool bUseCache /* = true */) const
{
	if (m_strPath.empty())
		return true;

	std::string strPath = m_strPath;

	if (m_bIsFolder) {
		return Directory::Exists(strPath, bUseCache);
		return false;
	}
	else
		return File::Exists(strPath, bUseCache);

	return false;
}

void FileItem::FillInMimeType(bool lookup /*= true*/)
{
	// TODO: adapt this to use CMime::GetMimeType()
	if (m_mimetype.empty())
	{
		if (m_bIsFolder)
			m_mimetype = "x-directory/normal";
		else if (StringUtils::StartsWithNoCase(m_strPath, "http://")
			|| StringUtils::StartsWithNoCase(m_strPath, "https://"))
		{
			// If lookup is false, bail out early to leave mime type empty
			if (!lookup)
				return;

			CCurlFile::GetMimeType(GetURL(), m_mimetype);

			// make sure there are no options set in mime-type
			// mime-type can look like "video/x-ms-asf ; charset=utf8"
			size_t i = m_mimetype.find(';');
			if (i != std::string::npos)
				m_mimetype.erase(i, m_mimetype.length() - i);
			StringUtils::Trim(m_mimetype);
		}
		else
			m_mimetype = Mime::GetMimeType(*this);

		// if it's still empty set to an unknown type
		if (m_mimetype.empty())
			m_mimetype = "application/octet-stream";
	}
}

bool FileItem::IsPath(const std::string& path) const {
	return (m_strPath == path);
}

void FileItem::SetProperty(const std::string &strKey, const CVariant &value)
{
	PropertyMap::iterator iter = m_mapProperties.find(strKey);
	if (iter == m_mapProperties.end())
	{
		m_mapProperties.insert(make_pair(strKey, value));
	}
	else if (iter->second != value)
	{
		iter->second = value;
	}
}

CVariant FileItem::GetProperty(const std::string &strKey) const
{
	PropertyMap::const_iterator iter = m_mapProperties.find(strKey);
	if (iter == m_mapProperties.end())
		return CVariant(CVariant::VariantTypeNull);

	return iter->second;
}

void FileItem::Initialize() {
	m_dwSize = 0;
	m_dateTime = CDateTime();
}

bool FileItem::icompare::operator()(const std::string &s1, const std::string &s2) const
{
	return StringUtils::CompareNoCase(s1, s2) < 0;
}


/////////////////////////////////////////////////////////////////////////////////
/////
///// CFileItemList
/////
//////////////////////////////////////////////////////////////////////////////////


FileItemList::FileItemList() {
	m_fastLookup = false;
	m_bIsFolder = true;
}

FileItemList::FileItemList(const std::string& strPath) {
	m_fastLookup = false;
	m_bIsFolder = true;
}

FileItemList::~FileItemList() {
	Clear();
}

FileItemPtr FileItemList::operator[] (int iItem)
{
	return Get(iItem);
}

const FileItemPtr FileItemList::operator[] (int iItem) const
{
	return Get(iItem);
}

FileItemPtr FileItemList::operator[] (const std::string& strPath)
{
	return Get(strPath);
}

const FileItemPtr FileItemList::operator[] (const std::string& strPath) const
{
	return Get(strPath);
}

void FileItemList::Clear() {
	CSingleLock lock(m_critSect);
	m_items.clear();
	m_map.clear();
}

void FileItemList::Add(const FileItemPtr &pItem)
{
	CSingleLock lock(m_critSect);

	m_items.push_back(pItem);
	if (m_fastLookup) {
		m_map.insert(MAPFILEITEMSPAIR(pItem->GetPath(), pItem));
	}
}

void FileItemList::AddFront(const FileItemPtr &pItem, int itemPosition)
{
	CSingleLock lock(m_critSect);

	if (itemPosition >= 0)
	{
		m_items.insert(m_items.begin() + itemPosition, pItem);
	}
	else
	{
		m_items.insert(m_items.begin() + (m_items.size() + itemPosition), pItem);
	}
	if (m_fastLookup)
	{
		m_map.insert(MAPFILEITEMSPAIR(pItem->GetPath(), pItem));
	}
}

void FileItemList::Remove(FileItem* pItem)
{
	CSingleLock lock(m_critSect);

	for (IVECFILEITEMS it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (pItem == it->get())
		{
			m_items.erase(it);
			if (m_fastLookup)
			{
				m_map.erase(pItem->GetPath());
			}
			break;
		}
	}
}

void FileItemList::Remove(int iItem)
{
	CSingleLock lock(m_critSect);

	if (iItem >= 0 && iItem < (int)Size())
	{
		FileItemPtr pItem = *(m_items.begin() + iItem);
		if (m_fastLookup)
		{
			m_map.erase(pItem->GetPath());
		}
		m_items.erase(m_items.begin() + iItem);
	}
}

void FileItemList::Append(const FileItemList& itemlist)
{
	CSingleLock lock(m_critSect);

	for (int i = 0; i < itemlist.Size(); ++i)
		Add(itemlist[i]);
}

void FileItemList::Assign(const FileItemList& itemlist, bool append)
{
	CSingleLock lock(m_critSect);
	if (!append)
		Clear();
	Append(itemlist);
	SetPath(itemlist.GetPath());
	
}

bool FileItemList::Copy(const FileItemList& items, bool copyItems /* = true */)
{
	// assign all CFileItem parts
	*(FileItem*)this = *(FileItem*)&items;

	if (copyItems)
	{
		// make a copy of each item
		for (int i = 0; i < items.Size(); i++)
		{
			FileItemPtr newItem(new FileItem(*items[i]));
			Add(newItem);
		}
	}

	return true;
}

FileItemPtr FileItemList::Get(int iItem)
{
	CSingleLock lock(m_critSect);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return FileItemPtr();
}

const FileItemPtr FileItemList::Get(int iItem) const
{
	CSingleLock lock(m_critSect);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return FileItemPtr();
}

FileItemPtr FileItemList::Get(const std::string& strPath)
{
	CSingleLock lock(m_critSect);

	if (m_fastLookup)
	{
		IMAPFILEITEMS it = m_map.find(strPath);
		if (it != m_map.end())
			return it->second;

		return FileItemPtr();
	}
	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		FileItemPtr pItem = m_items[i];
		if (pItem->IsPath(strPath))
			return pItem;
	}

	return FileItemPtr();
}

const FileItemPtr FileItemList::Get(const std::string& strPath) const
{
	CSingleLock lock(m_critSect);

	if (m_fastLookup)
	{
		std::map<std::string, FileItemPtr>::const_iterator it = m_map.find(strPath);
		if (it != m_map.end())
			return it->second;

		return FileItemPtr();
	}
	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		FileItemPtr pItem = m_items[i];
		if (pItem->IsPath(strPath))
			return pItem;
	}

	return FileItemPtr();
}

int FileItemList::Size() const
{
	CSingleLock lock(m_critSect);
	return (int)m_items.size();
}

bool FileItemList::IsEmpty() const
{
	CSingleLock lock(m_critSect);
	return (m_items.size() <= 0);
}

void FileItemList::Reserve(int iCount)
{
	CSingleLock lock(m_critSect);
	m_items.reserve(iCount);
}

int FileItemList::GetFolderCount() const
{
	CSingleLock lock(m_critSect);
	int nFolderCount = 0;
	for (int i = 0; i < (int)m_items.size(); i++)
	{
		FileItemPtr pItem = m_items[i];
		if (pItem->m_bIsFolder)
			nFolderCount++;
	}

	return nFolderCount;
}

int FileItemList::GetFileCount() const
{
	CSingleLock lock(m_critSect);
	int nFileCount = 0;
	for (int i = 0; i < (int)m_items.size(); i++)
	{
		FileItemPtr pItem = m_items[i];
		if (!pItem->m_bIsFolder)
			nFileCount++;
	}

	return nFileCount;
}

void FileItemList::SetFastLookup(bool fastLookup)
{
	CSingleLock lock(m_critSect);

	if (fastLookup && !m_fastLookup)
	{ // generate the map
		m_map.clear();
		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			FileItemPtr pItem = m_items[i];
			m_map.insert(MAPFILEITEMSPAIR(pItem->GetPath(), pItem));
		}
	}
	if (!fastLookup && m_fastLookup)
		m_map.clear();
	m_fastLookup = fastLookup;
}

bool FileItemList::Contains(const std::string& fileName) const
{
	CSingleLock lock(m_critSect);

	if (m_fastLookup)
		return m_map.find(fileName) != m_map.end();

	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		const FileItemPtr pItem = m_items[i];
		if (pItem->IsPath(fileName))
			return true;
	}
	return false;
}

