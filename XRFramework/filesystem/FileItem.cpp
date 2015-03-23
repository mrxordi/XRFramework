#include "stdafxf.h"
#include "FileItem.h"
#include "File.h"
#include "CurlFile.h"
#include "MimeTypes.h"
#include "Directory.h"
#include "utils/StringUtils.h"
#include "../XRThreads/SingleLock.h"



CFileItem::CFileItem()
{
	Initialize();
}

CFileItem::CFileItem(const CFileItem& item)
{
	*this = item;
}

CFileItem::CFileItem(const CURL& path, bool bIsFolder)
{
	Initialize();
	m_strPath = path.Get();
	m_bIsFolder = bIsFolder;
	m_url = CURL(path);
	if (m_bIsFolder && !m_strPath.empty())
		UrlUtils::AddSlashAtEnd(m_strPath);
	FillInMimeType(false);
}

CFileItem::CFileItem(const std::string& strPath, bool bIsFolder)
{
	Initialize();
	m_strPath = strPath;
	m_bIsFolder = bIsFolder;
	m_url = CURL(strPath);
	if (m_bIsFolder && !m_strPath.empty())
		UrlUtils::AddSlashAtEnd(m_strPath);
	FillInMimeType(false);
}

CFileItem::~CFileItem()
{
}

void CFileItem::SetURL(const CURL& url)
{
	m_strPath = url.Get();
	m_url = CURL(url);
	m_properties.clear();
	FillInMimeType();
}

bool CFileItem::IsURL(const CURL& url) const
{
	return UrlUtils::PathEquals(url.Get(), m_strPath);
}

void CFileItem::Reset()
{
	m_bIsFolder = false;
	m_strPath.clear();
	m_mimetype.clear();
	m_properties.clear();

	Initialize();
}

const CFileItem& CFileItem::operator=(const CFileItem& item)
{
	if (this == &item) return *this;
	m_strPath = item.GetPath();
	m_url = CURL(m_strPath);
	FillInMimeType();
	return *this;
}

bool CFileItem::Exists(bool bUseCache /* = true */) const
{
	if (m_strPath.empty())
		return true;

	std::string strPath = m_strPath;

	if (m_bIsFolder) {
		return CDirectory::Exists(strPath, bUseCache);
		return false;
	}
	else
		return CFile::Exists(strPath, bUseCache);

	return false;
}

void CFileItem::FillInMimeType(bool lookup /*= true*/)
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

bool CFileItem::IsPath(const std::string& path) const {
	return (m_strPath == path);
}

void CFileItem::SetProperty(const std::string &strKey, const CVariant &value)
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

CVariant CFileItem::GetProperty(const std::string &strKey) const
{
	PropertyMap::const_iterator iter = m_mapProperties.find(strKey);
	if (iter == m_mapProperties.end())
		return CVariant(CVariant::VariantTypeNull);

	return iter->second;
}

void CFileItem::Initialize() {
	m_dwSize = 0;
	m_dateTime = CDateTime();
}

bool CFileItem::icompare::operator()(const std::string &s1, const std::string &s2) const
{
	return StringUtils::CompareNoCase(s1, s2) < 0;
}


/////////////////////////////////////////////////////////////////////////////////
/////
///// CFileItemList
/////
//////////////////////////////////////////////////////////////////////////////////


CFileItemList::CFileItemList() {
	m_fastLookup = false;
	m_bIsFolder = false;
}

CFileItemList::CFileItemList(const std::string& strPath) {
	m_fastLookup = false;
	m_bIsFolder = false;
}

CFileItemList::~CFileItemList() {
	Clear();
}

CFileItemPtr CFileItemList::operator[] (int iItem)
{
	return Get(iItem);
}

const CFileItemPtr CFileItemList::operator[] (int iItem) const
{
	return Get(iItem);
}

CFileItemPtr CFileItemList::operator[] (const std::string& strPath)
{
	return Get(strPath);
}

const CFileItemPtr CFileItemList::operator[] (const std::string& strPath) const
{
	return Get(strPath);
}

void CFileItemList::Clear() {
	XR::CSingleLock lock(m_critSect);
	m_items.clear();
	m_map.clear();
}

void CFileItemList::Add(const CFileItemPtr &pItem)
{
	XR::CSingleLock lock(m_critSect);

	m_items.push_back(pItem);
	if (m_fastLookup) {
		m_map.insert(PairFileItem(pItem->GetPath(), pItem));
	}
}

void CFileItemList::AddFront(const CFileItemPtr &pItem, int itemPosition)
{
	XR::CSingleLock lock(m_critSect);

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
		m_map.insert(PairFileItem(pItem->GetPath(), pItem));
	}
}

void CFileItemList::Remove(CFileItem* pItem)
{
	XR::CSingleLock lock(m_critSect);

	for (VecFileItemsIterator it = m_items.begin(); it != m_items.end(); ++it)
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

void CFileItemList::Remove(int iItem)
{
	XR::CSingleLock lock(m_critSect);

	if (iItem >= 0 && iItem < (int)Size())
	{
		CFileItemPtr pItem = *(m_items.begin() + iItem);
		if (m_fastLookup)
		{
			m_map.erase(pItem->GetPath());
		}
		m_items.erase(m_items.begin() + iItem);
	}
}

void CFileItemList::Append(const CFileItemList& itemlist)
{
	XR::CSingleLock lock(m_critSect);

	for (int i = 0; i < itemlist.Size(); ++i)
		Add(itemlist[i]);
}

void CFileItemList::Assign(const CFileItemList& itemlist, bool append)
{
	XR::CSingleLock lock(m_critSect);
	if (!append)
		Clear();
	Append(itemlist);
	SetPath(itemlist.GetPath());

}

bool CFileItemList::Copy(const CFileItemList& items, bool copyItems /* = true */)
{
	// assign all CFileItem parts
	*(CFileItem*)this = *(CFileItem*)&items;

	if (copyItems)
	{
		// make a copy of each item
		for (int i = 0; i < items.Size(); i++)
		{
			CFileItemPtr newItem(new CFileItem(*items[i]));
			Add(newItem);
		}
	}

	return true;
}

CFileItemPtr CFileItemList::Get(int iItem)
{
	XR::CSingleLock lock(m_critSect);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return CFileItemPtr();
}

const CFileItemPtr CFileItemList::Get(int iItem) const
{
	XR::CSingleLock lock(m_critSect);

	if (iItem > -1 && iItem < (int)m_items.size())
		return m_items[iItem];

	return CFileItemPtr();
}

CFileItemPtr CFileItemList::Get(const std::string& strPath)
{
	XR::CSingleLock lock(m_critSect);

	if (m_fastLookup)
	{
		MapFileItemIterator it = m_map.find(strPath);
		if (it != m_map.end())
			return it->second;

		return CFileItemPtr();
	}
	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (pItem->IsPath(strPath))
			return pItem;
	}

	return CFileItemPtr();
}

const CFileItemPtr CFileItemList::Get(const std::string& strPath) const
{
	XR::CSingleLock lock(m_critSect);

	if (m_fastLookup)
	{
		std::map<std::string, CFileItemPtr>::const_iterator it = m_map.find(strPath);
		if (it != m_map.end())
			return it->second;

		return CFileItemPtr();
	}
	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (pItem->IsPath(strPath))
			return pItem;
	}

	return CFileItemPtr();
}

int CFileItemList::Size() const
{
	XR::CSingleLock lock(m_critSect);
	return (int)m_items.size();
}

bool CFileItemList::IsEmpty() const
{
	XR::CSingleLock lock(m_critSect);
	return (m_items.size() <= 0);
}

void CFileItemList::Reserve(int iCount)
{
	XR::CSingleLock lock(m_critSect);
	m_items.reserve(iCount);
}

int CFileItemList::GetFolderCount() const
{
	XR::CSingleLock lock(m_critSect);
	int nFolderCount = 0;
	for (int i = 0; i < (int)m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (pItem->m_bIsFolder)
			nFolderCount++;
	}

	return nFolderCount;
}

int CFileItemList::GetFileCount() const
{
	XR::CSingleLock lock(m_critSect);
	int nFileCount = 0;
	for (int i = 0; i < (int)m_items.size(); i++)
	{
		CFileItemPtr pItem = m_items[i];
		if (!pItem->m_bIsFolder)
			nFileCount++;
	}

	return nFileCount;
}

void CFileItemList::SetFastLookup(bool fastLookup)
{
	XR::CSingleLock lock(m_critSect);

	if (fastLookup && !m_fastLookup)
	{ // generate the map
		m_map.clear();
		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			CFileItemPtr pItem = m_items[i];
			m_map.insert(PairFileItem(pItem->GetPath(), pItem));
		}
	}
	if (!fastLookup && m_fastLookup)
		m_map.clear();
	m_fastLookup = fastLookup;
}

bool CFileItemList::Contains(const std::string& fileName) const
{
	XR::CSingleLock lock(m_critSect);

	if (m_fastLookup)
		return m_map.find(fileName) != m_map.end();

	// slow method...
	for (unsigned int i = 0; i < m_items.size(); i++)
	{
		const CFileItemPtr pItem = m_items[i];
		if (pItem->IsPath(fileName))
			return true;
	}
	return false;
}

