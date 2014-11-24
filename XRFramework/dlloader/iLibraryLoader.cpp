#include "stdafxf.h"
#include "iLibraryLoader.h"
#include <string.h>
#include <stdlib.h>
#include "log/Log.h"

iLibraryLoader::iLibraryLoader(const char* libraryFile)
{
	m_sFileName = _strdup(libraryFile);

	char* sPath = strrchr(m_sFileName, '\\');
	if (!sPath) sPath = strrchr(m_sFileName, '/');
	if (sPath)
	{
		sPath++;
		m_sPath = (char*)malloc(sPath - m_sFileName + 1);
		strncpy(m_sPath, m_sFileName, sPath - m_sFileName);
		m_sPath[sPath - m_sFileName] = 0;
	}
	else
		m_sPath = NULL;

	m_iRefCount = 1;
}

iLibraryLoader::~iLibraryLoader()
{
	free(m_sFileName);
	free(m_sPath);
}

char* iLibraryLoader::GetName()
{
	if (m_sFileName)
	{
		char* sName = strrchr(m_sFileName, '/');
		if (sName) return sName + 1;
		else return m_sFileName;
	}
	return (char*)"";
}

char* iLibraryLoader::GetFileName()
{
	if (m_sFileName) return m_sFileName;
	return (char*)"";
}

char* iLibraryLoader::GetPath()
{
	if (m_sPath) return m_sPath;
	return (char*)"";
}

int iLibraryLoader::IncrRef()
{
	m_iRefCount++;
	return m_iRefCount;
}

int iLibraryLoader::DecrRef()
{
	m_iRefCount--;
	return m_iRefCount;
}

int iLibraryLoader::ResolveOrdinal(unsigned long ordinal, void** ptr)
{
	LOGWARN("Unable to resolve %lu in dll %s", __FUNCTION__, ordinal, GetName());
	return 0;
}
