#pragma once
#include "filesystem/IDirectory.h"

class Win32Directory : public IDirectory
{
public:
	Win32Directory(void);
	virtual ~Win32Directory(void);
	virtual bool GetDirectory(const CUrl& url, FileItemList &items);
	virtual bool Create(const CUrl& url);
	virtual bool Exists(const CUrl& url);
	virtual bool Remove(const CUrl& url);
};

