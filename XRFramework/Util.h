#pragma once
#include "utils/StdString.h"

class CUtil
{
public:
	CUtil();
	~CUtil();

	static std::string ValidatePath(const std::string &path, bool bFixDoubleSlashes = false); ///< return a validated path, with correct directory separators.
	static CStdString ResolveExecutablePath();
	static CStdString ResolveSystemTempPath();
	static CStdString ResolveUserPath();
	static CStdString ResolveDocPath();
	static bool		  GetFocussedProcess(std::string &strProcessFile);
private:

};
