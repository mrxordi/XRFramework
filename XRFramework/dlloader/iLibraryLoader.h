#pragma once
//#include <windows.h>

class iLibraryLoader
{
public:
	iLibraryLoader(const char* libraryFile);
	virtual ~iLibraryLoader();

	virtual bool Load() = 0;
	virtual void Unload() = 0;

	virtual int ResolveExport(const char* symbol, void** ptr, bool logging = true) = 0;
	virtual int ResolveOrdinal(unsigned long ordinal, void** ptr);
	virtual bool IsSystemDll() = 0;
	virtual HMODULE GetHModule() = 0;
	virtual bool HasSymbols() = 0;

	char* GetName(); // eg "mplayer.dll"
	char* GetFileName(); // "special://xbmcbin/system/mplayer/players/mplayer.dll"
	char* GetPath(); // "special://xbmcbin/system/mplayer/players/"

	int IncrRef();
	int DecrRef();
	int GetRef();

private:
	iLibraryLoader(const iLibraryLoader&);
	iLibraryLoader& operator=(const iLibraryLoader&);
	char* m_sFileName;
	char* m_sPath;
	int   m_iRefCount;
};

typedef iLibraryLoader LibraryLoader;

