#include "stdafxf.h"
#include "LoaderFactory.h"
#include "XRThreads/SingleLock.h"
#include "log/Log.h"
#include "DLLoader/LoaderContainer.h"
#include "XRThreads/SystemClock.h"

//  delay for unloading dll's
#define UNLOAD_DELAY 30*1000 // 30 sec.

#define LOGALL


LoaderFactory::LoaderFactory(void)
{
}


LoaderFactory::~LoaderFactory(void)
{
	UnloadAll();
}

LoaderFactory_t LoaderFactory::GetSharedPtr() {
	CSingleLock lock(m_critSect);

	if (!m_thisInstance) {
		m_thisInstance.reset(new LoaderFactory);
	}
	return m_thisInstance;

}

LoaderFactory* LoaderFactory::Get()
{
	CSingleLock lock(m_critSect);
	if (!m_thisInstance) {
		m_thisInstance.reset(new LoaderFactory);
	}
	return m_thisInstance.get();
}

LibraryLoader* LoaderFactory::LoadDLL(const std::string& strDllName, bool bDelayUnload/*=true*/, bool bLoadSymbols/*=false*/)
{
	CSingleLock lock(m_critSect);

	if (strDllName.empty()) return NULL;
	//	check if it's already loaded, and increase the reference count if so
	for (int i = 0; i < (int)m_vecLoadedDLLs.size(); ++i)
	{
		CDll& dll = m_vecLoadedDLLs[i];
		if (dll.m_strDllName == strDllName)
		{
			dll.m_lReferenceCount++;
			LOGDEBUG("Dll (%s) is already loaded. Increasing reference (%d)", strDllName.c_str(), dll.m_lReferenceCount);
			return dll.m_pDll;
		}
	}

	// ok, now load the dll
	LOGDEBUG("Dll (%s) library is not loaded - Loading...", strDllName.c_str());

	LibraryLoader* pDll = LoaderContainer::LoadModule(strDllName.c_str(), NULL, bLoadSymbols);
	if (!pDll)
		return NULL;

	CDll newDll;
	newDll.m_strDllName = strDllName;
	newDll.m_lReferenceCount = 1;
	newDll.m_bDelayUnload = bDelayUnload;
	newDll.m_pDll=pDll;
	m_vecLoadedDLLs.push_back(newDll);
	LOGDEBUG("Dll (%s) has been succesfully loaded.", strDllName.c_str());
	return newDll.m_pDll;
}

void LoaderFactory::UnloadDLL(const std::string& strDllName)
{
	CSingleLock lock(m_critSect);

	if (strDllName.empty()) return;
	// check if it's already loaded, and decrease the reference count if so
	for (int i = 0; i < (int)m_vecLoadedDLLs.size(); ++i)
	{
		CDll& dll = m_vecLoadedDLLs[i];
		if (dll.m_strDllName == strDllName)
		{
			dll.m_lReferenceCount--;
			if (0 == dll.m_lReferenceCount)
			{
				if (dll.m_bDelayUnload) {
					dll.m_unloadDelayStartTick = XR::SystemClockMillis();
					LOGDEBUG("Dll (%s) has no more references added to delayed unload.", strDllName.c_str());
				} else {
					LOGDEBUG("Dll (%s) has no more references. Unloading.", strDllName.c_str());
					if (dll.m_pDll)
						LoaderContainer::ReleaseModule(dll.m_pDll);
					m_vecLoadedDLLs.erase(m_vecLoadedDLLs.begin() + i);
				}

				return;
			}
			LOGDEBUG("Dll (%s) decreasing reference, still referenced with a count of %i.", strDllName.c_str(), dll.m_lReferenceCount);
		}
	}
}

void LoaderFactory::UnloadDelayed()
{
	CSingleLock lock(m_critSect);

	// check if we can unload any unreferenced dlls
	for (int i = 0; i < (int)m_vecLoadedDLLs.size(); ++i)
	{
		CDll& dll = m_vecLoadedDLLs[i];
		unsigned int tickcount = XR::SystemClockMillis() - dll.m_unloadDelayStartTick;
		if (dll.m_lReferenceCount == 0 && (tickcount > UNLOAD_DELAY))
		{
			LOGDEBUG("Dll (%s) unloaded after delay %u ms.", dll.m_strDllName.c_str(), tickcount);

			if (dll.m_pDll)
				LoaderContainer::ReleaseModule(dll.m_pDll);
			m_vecLoadedDLLs.erase(m_vecLoadedDLLs.begin() + i);
			return;
		}
	}
}

void LoaderFactory::UnloadAll()
{
	// delete the dll's
	CSingleLock lock(m_critSect);
	std::vector<CDll>::iterator it = m_vecLoadedDLLs.begin();
	while (it != m_vecLoadedDLLs.end())
	{
		CDll& dll = *it;
		LOGDEBUG("Unloading (%s) library at LoaderFactory destroy.", dll.m_strDllName.c_str());
		if (dll.m_pDll)
			LoaderContainer::ReleaseModule(dll.m_pDll);
		it = m_vecLoadedDLLs.erase(it);
	}
}

std::vector<LoaderFactory::CDll> LoaderFactory::m_vecLoadedDLLs;

CCriticalSection LoaderFactory::m_critSect;

LoaderFactory_t LoaderFactory::m_thisInstance = NULL;
