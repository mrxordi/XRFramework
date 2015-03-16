#include "stdafxf.h"
#include "DynamicDll.h"
#include "LoaderFactory.h"
#include "log/Log.h"
#include "utils/URL.h"
#include "utils/SpecialProtocol.h"

DllDynamic::DllDynamic()
{
	m_dll = NULL;
	m_DelayUnload = true;
}

DllDynamic::DllDynamic(const std::string& strDllName)
{
	m_strDllName = strDllName;
	m_dll = NULL;
	m_DelayUnload = true;
}

DllDynamic::~DllDynamic()
{
	UnloadAll();
}

bool DllDynamic::Load()
{
	if (m_dll)
		return true;

	if (!CanLoad())
		return false;

	if (!(m_dll = LoaderFactory::Get()->LoadDLL(m_strDllName, m_DelayUnload, LoadSymbols())))
		return false;

	if (!ResolveExports()) 
	{
		LOGERR("Unable to resolve exports from dll %s", m_strDllName.c_str());
		Unload();
		return false;
	}

	return true;
}

void DllDynamic::Unload()
{
	if (m_dll)
		LoaderFactory::Get()->UnloadDLL(m_strDllName);
	m_dll = NULL;
}

bool DllDynamic::CanLoad()
{
	std::string szPath = CSpecialProtocol::TranslatePath(m_strDllName);
	DWORD dwAttrib = GetFileAttributesA(szPath.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	//return CFile::Exists(m_strDllName);
}

bool DllDynamic::EnableDelayedUnload(bool bOnOff)
{
	if (m_dll)
		return false;

	m_DelayUnload = bOnOff;

	return true;
}

bool DllDynamic::SetFile(const std::string& strDllName)
{
	if (m_dll)
		return false;

	m_strDllName = strDllName;
	return true;
}

