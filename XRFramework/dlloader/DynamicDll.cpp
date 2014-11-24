/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "stdafxf.h"
#include "DynamicDll.h"
#include "LoaderFactory.h"
#include "log/Log.h"
#include "filesystem/URL.h"
#include "filesystem/SpecialProtocol.h"

DllDynamic::DllDynamic()
{
	m_dll = NULL;
	m_DelayUnload = true;
}

DllDynamic::DllDynamic(const CStdString& strDllName)
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
	CStdString szPath = CSpecialProtocol::TranslatePath(m_strDllName);
	DWORD dwAttrib = GetFileAttributes(szPath.c_str());

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

bool DllDynamic::SetFile(const CStdString& strDllName)
{
	if (m_dll)
		return false;

	m_strDllName = strDllName;
	return true;
}

