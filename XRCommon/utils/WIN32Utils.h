#pragma once
#include "URL.h"

class WIN32Utils
{
public:
	WIN32Utils();
	virtual ~WIN32Utils();

	static inline __time64_t fileTimeToTimeT(const __int64 ftimei64)
	{
		// FILETIME is 100-nanoseconds from 00:00:00 UTC 01 Jan 1601
		// __time64_t is seconds from 00:00:00 UTC 01 Jan 1970
		return (ftimei64 - 116444736000000000) / 10000000;
	}
	static __time64_t fileTimeToTimeT(const FILETIME& ftimeft);
	static __time64_t fileTimeToTimeT(const LARGE_INTEGER& ftimeli);
};

