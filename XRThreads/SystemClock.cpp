#undef max

#include "stdafx.h"
//#include <windows.h>
#include <Mmsystem.h>
#include <ctime>
#include "SystemClock.h"
#include "../XRFramework/utils/DateTime.h"

namespace XR
{

	int64_t CurrentHostCounter(void) 
	{
		LARGE_INTEGER PerformanceCount;
		QueryPerformanceCounter(&PerformanceCount);
		return((int64_t)PerformanceCount.QuadPart);
	}

	int64_t CurrentHostFrequency(void) 
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		return((int64_t)Frequency.QuadPart);
	}

	CDateTime GetLocalTime(time_t time) 
	{
		CDateTime result;

		tm *local;
		local = localtime(&time); // Conversion to local time
		/*
		* Microsoft implementation of localtime returns NULL if on or before epoch.
		* http://msdn.microsoft.com/en-us/library/bf12f0hc(VS.80).aspx
		*/
		if (local)
			result = *local;
		else
			result = time; // Use the original time as close enough.

		return result;
	}

	unsigned int SystemClockMillis() 
	{
		uint64_t now_time;
		static uint64_t start_time = 0;
		static bool start_time_set = false;

		now_time = (uint64_t)timeGetTime();

		if (!start_time_set) 
		{
			start_time = now_time;
			start_time_set = true;
		}
		return (unsigned int)(now_time - start_time);
	}
	const unsigned int EndTime::InfiniteValue = std::numeric_limits<unsigned int>::max();
}