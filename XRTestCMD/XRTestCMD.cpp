// XRTestCMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../XRFramework/filesystem/SpecialProtocol.h"
#include "../XRFramework/log/Log.h"
#include "../XRFramework/Util.h"


int _tmain(int argc, _TCHAR* argv[])
{
	CStdString appPath, tempPath, docPath, homePath;

	appPath = CUtil::ResolveExecutablePath();
	tempPath = CUtil::ResolveSystemTempPath();
	docPath = CUtil::ResolveDocPath();
	homePath = CUtil::ResolveUserPath();

	CSpecialProtocol::SetAPPPath(appPath);
	CSpecialProtocol::SetAPPBinPath(appPath);
	CSpecialProtocol::SetTempPath(tempPath);
	CSpecialProtocol::SetDocPath(docPath);
	CSpecialProtocol::SetHomePath(homePath);


	DWORD m_threadid = GetCurrentThreadId();

	if (!Logger::Init("special://app/log.txt"))
	{
		fprintf(stderr, "Could not init logging classes. Permission errors on (%s)\n",
			CSpecialProtocol::TranslatePath("special://home/").c_str());
		return false;
	}

	LOGDEBUG("cApplication Initialization. App Running on thread: %d", m_threadid);

	CSpecialProtocol::LogPaths();


	return 0;
}

