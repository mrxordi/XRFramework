#include "stdafxf.h"
#include <string>
#include "LoaderContainer.h"
#include "log\Log.h"
#include "utils\StringUtils.h"
#include "Win32DllLoader.h"
#include "utils\URL.h"

//#define LOGALL

LibraryLoader* LoaderContainer::m_dlls[64] = {};
int LoaderContainer::m_iNrOfDlls = 0;
bool LoaderContainer::m_bTrack = true;

void LoaderContainer::Clear(){

}

HMODULE LoaderContainer::GetModuleAddress(const char* sName) {
	return (HMODULE)GetModule(sName);
}

int LoaderContainer::GetNrOfModules() {
	return m_iNrOfDlls;
}

LibraryLoader* LoaderContainer::GetModule(int iPos) {
	if (iPos < m_iNrOfDlls) return m_dlls[iPos];
	return NULL;
}

LibraryLoader* LoaderContainer::GetModule(const char* sName)
{
	for (int i = 0; i < m_iNrOfDlls && m_dlls[i] != NULL; i++)
	{
		if (_stricmp(m_dlls[i]->GetName(), sName) == 0) return m_dlls[i];
		if (!m_dlls[i]->IsSystemDll() && _stricmp(m_dlls[i]->GetFileName(), sName) == 0) return m_dlls[i];
	}

	return NULL;
}

LibraryLoader* LoaderContainer::GetModule(HMODULE hModule)
{
	for (int i = 0; i < m_iNrOfDlls && m_dlls[i] != NULL; i++)
	{
		if (m_dlls[i]->GetHModule() == hModule) return m_dlls[i];
	}
	return NULL;
}

LibraryLoader* LoaderContainer::LoadModule(const char* sName, const char* sCurrentDir/*=NULL*/, bool bLoadSymbols/*=false*/)
{
	LibraryLoader* pDll = NULL;

	if (sCurrentDir)
	{
		std::string strPath = sCurrentDir;
		strPath += sName;
		pDll = GetModule(strPath.c_str());
	}

	if (!pDll)
	{
		pDll = GetModule(sName);
	}

	if (!pDll)
	{
		pDll = FindModule(sName, sCurrentDir, bLoadSymbols);
	}

	return pDll;
}

void LoaderContainer::ReleaseModule(LibraryLoader*& pDll)
{
	if (!pDll)
		return;

	int iRefCount = pDll->DecrRef();
	if (iRefCount == 0)
	{

#ifdef LOGALL
		LOGDEBUG("Releasing Dll %s", pDll->GetFileName());
#endif
		if (!pDll->HasSymbols())
		{
			pDll->Unload();
			delete pDll;
			pDll = NULL;
		}
		else
			LOGINFO("has symbols loaded and can never be unloaded", pDll->GetName());
	}
#ifdef LOGALL
	else
	{
		LOGINFO("Dll %s is still referenced with a count of %d", pDll->GetFileName(), iRefCount);
	}
#endif
}

void LoaderContainer::RegisterDll(LibraryLoader* pDll)
{
	for (int i = 0; i < 64; i++)
	{
		if (m_dlls[i] == NULL)
		{
			m_dlls[i] = pDll;
			m_iNrOfDlls++;
			break;
		}
	}
}

void LoaderContainer::UnRegisterDll(LibraryLoader* pDll)
{
	if (pDll)
	{
		if (pDll->IsSystemDll())
		{
			LOGERR(" is a system dll and should never be removed", pDll->GetName());
		}
		else
		{
			// remove from the list
			bool bRemoved = false;
			for (int i = 0; i < m_iNrOfDlls && m_dlls[i]; i++)
			{
				if (m_dlls[i] == pDll) bRemoved = true;
				if (bRemoved && i + 1 < m_iNrOfDlls)
				{
					m_dlls[i] = m_dlls[i + 1];
				}
			}
			if (bRemoved)
			{
				m_iNrOfDlls--;
				m_dlls[m_iNrOfDlls] = NULL;
			}
		}
	}
}

LibraryLoader* LoaderContainer::FindModule(const char* sName, const char* sCurrentDir, bool bLoadSymbols)
{
	if (CURL::IsFullPath(sName))
	{ //  Has a path, just try to load
		return LoadDll(sName, bLoadSymbols);
	}

	if (sCurrentDir)
	{ // in the path of the parent dll?
		std::string strPath = sCurrentDir;
		strPath += sName;

		LibraryLoader* pLoader = LoadDll(strPath.c_str(), bLoadSymbols);
		if (pLoader)
			return pLoader;
	}

	//  in environment variable?
	StringArray vecEnv;

	StringUtils::SplitString(ENV_PATH, ";", vecEnv);
	LibraryLoader* pDll = NULL;

	for (int i = 0; i < (int)vecEnv.size(); ++i)
	{
		std::string strPath = vecEnv[i];
		strPath += '/';

#ifdef LOGALL
		LOGDEBUG("Searching for the dll %s in directory %s", sName, strPath.c_str());
#endif
		strPath += sName;

		// Have we already loaded this dll
		if ((pDll = GetModule(strPath.c_str())) != NULL)
			return pDll;

		if ((pDll = LoadDll(strPath.c_str(), bLoadSymbols)) != NULL)
			return pDll;
	}

	// can't find it in any of our paths - could be a system dll
	if ((pDll = LoadDll(sName, bLoadSymbols)) != NULL)
		return pDll;

	LOGDEBUG("Dll %s was not found in path", sName);
	return NULL;
}
//to do
LibraryLoader* LoaderContainer::LoadDll(const char* sName, bool bLoadSymbols)
{

#ifdef LOGALL
	LOGDEBUG("Loading dll %s", sName);
#endif

	LibraryLoader* pLoader = new Win32DllLoader(sName);

	if (!pLoader)
	{
		LOGERR("Unable to create dll %s", sName);
		return NULL;
	}

	if (!pLoader->Load())
	{
		delete pLoader;
		return NULL;
	}
#ifdef LOGALL
	LOGDEBUG("Dll %s has been succesfully loaded.", sName);
#endif
	return pLoader;
}

bool LoaderContainer::IsSystemDll(const char* sName)
{
	for (int i = 0; i < m_iNrOfDlls && m_dlls[i] != NULL; i++)
	{
		if (m_dlls[i]->IsSystemDll() && _stricmp(m_dlls[i]->GetName(), sName) == 0) return true;
	}

	return false;
}

