#include "stdafxf.h"
#include "WIN32Utils.h"
#include "utils/StringConverter.h"
#include "utils/URIUtils.h"


WIN32Utils::WIN32Utils()
{
}


WIN32Utils::~WIN32Utils()
{
}

__time64_t WIN32Utils::fileTimeToTimeT(const FILETIME& ftimeft)
{
	if (!ftimeft.dwHighDateTime && !ftimeft.dwLowDateTime)
		return 0;

	return fileTimeToTimeT((__int64(ftimeft.dwHighDateTime) << 32) + __int64(ftimeft.dwLowDateTime));
}

__time64_t WIN32Utils::fileTimeToTimeT(const LARGE_INTEGER& ftimeli)
{
	if (ftimeli.QuadPart == 0)
		return 0;

	return fileTimeToTimeT(__int64(ftimeli.QuadPart));
}