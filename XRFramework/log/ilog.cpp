#include "stdafxf.h"
#include "ilog.h"
#include "utils/StringUtils.h"


void XR::ILogger::Log(int loglevel, const char* file, const int linenumber, const char* function, const char* message, ...)
{
	std::string strData;
	strData.reserve(16384);

	va_list va;
	va_start(va, message);
	strData = StringUtils::FormatV(message,va);
	va_end(va);

	log(loglevel, file, linenumber, function, strData.c_str());
}
