#pragma once
#include <memory>
#include <string>
#include <vector>
#include "iLibraryLoader.h"
#include "XRThreads/CriticalSection.h"

class LoaderFactory;


typedef std::tr1::shared_ptr<LoaderFactory> LoaderFactory_t;

class LoaderFactory {
public:
	class CDll {
	public:
		std::string m_strDllName;
		long m_lReferenceCount;
		LibraryLoader* m_pDll;
		unsigned int m_unloadDelayStartTick;
		bool m_bDelayUnload;
	};

	LoaderFactory(void);
	virtual ~LoaderFactory(void);

	static LibraryLoader* LoadDLL(const std::string& strDllName, bool bDelayUnload=true, bool bLoadSymbols=false);
	static void UnloadDLL(const std::string& strDllName);
	static void UnloadDelayed();
	void UnloadAll();

	static LoaderFactory* Get();
	static LoaderFactory_t GetSharedPtr();

protected:
	static std::vector<CDll> m_vecLoadedDLLs;
	static CCriticalSection m_critSect;
private:
	static LoaderFactory_t m_thisInstance;
};
