#pragma once
#include "IFile.h"
class CWin32File :
	public IFile
{
public:
	CWin32File();
	virtual ~CWin32File();
};

