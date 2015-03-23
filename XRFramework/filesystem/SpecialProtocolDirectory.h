#pragma once
#include "IDirectory.h"

class SpecialProtocolDirectory : public IDirectory
{
public:
	SpecialProtocolDirectory(void);
	virtual ~SpecialProtocolDirectory(void);
	virtual bool GetDirectory(const CURL& url, CFileItemList &items);
	virtual bool Create(const CURL& url);
	virtual bool Exists(const CURL& url);
	virtual bool Remove(const CURL& url);
};
