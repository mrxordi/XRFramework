#pragma once
#include "LogDefines.h"


namespace XR {

	class ILogger {
	public:
		virtual ~ILogger() {}
		void Log(int loglevel, const char* file, const int linenumber, 
			const char* function, const char* message, ...);

		virtual void log(int loglevel, const char* file, const int linenumber, 
			const char* function, const char* message) = 0;
	};

}