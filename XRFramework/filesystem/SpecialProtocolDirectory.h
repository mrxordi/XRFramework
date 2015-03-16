#pragma once
#include "IDirectory.h"

class SpecialProtocolDirectory : public IDirectory
{
public:
	SpecialProtocolDirectory(void);
	virtual ~SpecialProtocolDirectory(void);
	virtual bool GetDirectory(const CUrl& url, FileItemList &items);
	virtual bool Create(const CUrl& url);
	virtual bool Exists(const CUrl& url);
	virtual bool Remove(const CUrl& url);
};
