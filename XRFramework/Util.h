#pragma once
#include "utils/StdString.h"

class CUtil
{
public:
	CUtil();
	~CUtil();

	static std::string ValidatePath(const std::string &path, bool bFixDoubleSlashes = false); ///< return a validated path, with correct directory separators.
	static std::string ResolveExecutablePath();
	static std::string ResolveSystemTempPath();
	static std::string ResolveUserPath();
	static std::string ResolveDocPath();

	static bool IsPicture(const CStdString& strFile);

	static bool GetFocussedProcess(std::string &strProcessFile);
private:

};
