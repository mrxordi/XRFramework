#pragma once
#include <windows.h>

#include "iLibraryLoader.h"
class Win32DllLoader : public iLibraryLoader
{
public:
	class Import
	{
	public:
		void *table;
		DWORD function;
	};

	Win32DllLoader(const char *dll);
	~Win32DllLoader();

	virtual bool Load();
	virtual void Unload();

	virtual int ResolveExport(const char* symbol, void** ptr, bool logging = true);
	virtual bool IsSystemDll();
	virtual HMODULE GetHModule();
	virtual bool HasSymbols();

private:
	void OverrideImports(const std::string &dll);
	void RestoreImports();
	static bool ResolveImport(const char *dllName, const char *functionName, void **fixup);
	static bool ResolveOrdinal(const char *dllName, unsigned long ordinal, void **fixup);
	bool NeedsHooking(const char *dllName);

	HMODULE m_dllHandle;
	bool bIsSystemDll;

	std::vector<Import> m_overriddenImports;
	std::vector<HMODULE> m_referencedDlls;
};

