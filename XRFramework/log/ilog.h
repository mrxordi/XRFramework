#pragma once
#include "LogDefines.h"


namespace XR {

	class ILogger {
	public:
		virtual ~ILogger() {}
		void Log(LOGLEVEL loglevel, const char* file, const int linenumber, 
			const char* function, const char* message, ...);

		virtual void log(LOGLEVEL loglevel, const char* file, const int linenumber, 
			const char* function, const char* message) = 0;
	};

}