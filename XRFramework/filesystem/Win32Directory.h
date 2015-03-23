#pragma once
#include "filesystem/IDirectory.h"

class Win32Directory : public IDirectory
{
public:
	Win32Directory(void);
	virtual ~Win32Directory(void);
	virtual bool GetDirectory(const CURL& url, CFileItemList &items);
	virtual bool Create(const CURL& url);
	virtual bool Exists(const CURL& url);
	virtual bool Remove(const CURL& url);
};

