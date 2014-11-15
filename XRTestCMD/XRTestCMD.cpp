// XRTestCMD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <regex>
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

	// text to transform
	std::string text = "This is a element and this a unique ID.";

	// regular expression with two capture groups
	const std::tr1::regex pattern("(\\ba (a|e|i|u|o))+");

	// the pattern for the transformation, using the second
	// capture group
	std::string replace = "an $2";

	std::string newtext = std::tr1::regex_replace(text, pattern, replace);

	std::cout << newtext << std::endl;
	std::cin >> newtext;




	return 0;
}

