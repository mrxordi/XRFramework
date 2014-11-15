#pragma once
// 
// Copyright (c) Kris Gustafson, 2012
// krisgus@ieee.org
// 
#include<memory>	// for shared_ptr<>()

// extra masks - from bit 5
#define LOGMASKBIT 5
#define LOGMASK   ((1 << LOGMASKBIT)-1)

#define LOGCURL   (1 << (LOGMASKBIT+0))


#ifdef LOG_QUIET_MODE
#define LOGERRW(x, ...) Logger::GetLog()->Log(LOG_ERROR, "", 0, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGERR(x, ...) Logger::GetLog()->Log(LOG_ERROR, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGINFOW(x, ...) Logger::GetLog()->Log(LOG_INFO, "", 0, L"", x, __VA_ARGS__);
#define LOGINFO(x, ...) Logger::GetLog()->Log(LOG_INFO, "", 0, "", x, __VA_ARGS__);
#define LOGDEBUGW(x, ...) Logger::GetLog()->Log(LOG_DEBUG, "", 0, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGDEBUG(x, ...) Logger::GetLog()->Log(LOG_DEBUG, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGWARNW(x, ...) Logger::GetLog()->Log(LOG_WARNING, "", 0, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGWARN(x, ...) Logger::GetLog()->Log(LOG_WARNING, "", 0, __FUNCTION__, x, __VA_ARGS__);
#else
#define LOGERRW(x, ...) Logger::GetLog()->Log(LOG_ERROR, __FILE__, __LINE__, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGERR(x, ...) Logger::GetLog()->Log(LOG_ERROR, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGINFOW(x, ...) Logger::GetLog()->Log(LOG_INFO, "", 0, L"", x, __VA_ARGS__);
#define LOGINFO(x, ...) Logger::GetLog()->Log(LOG_INFO, "", 0, "", x, __VA_ARGS__);
#define LOGDEBUGW(x, ...) Logger::GetLog()->Log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGDEBUG(x, ...) Logger::GetLog()->Log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGWARNW(x, ...) Logger::GetLog()->Log(LOG_WARNING, __FILE__, __LINE__, __FUNCTIONW__, x, __VA_ARGS__);
#define LOGWARN(x, ...) Logger::GetLog()->Log(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#endif



class ILogTarget;
typedef ILogTarget* LogObj_t;

class LoggerDAL;
typedef std::tr1::shared_ptr<LoggerDAL> LoggerDAL_t;

typedef enum _logLevel
{
	LOG_INFO,
	LOG_DEBUG,
	LOG_WARNING,
	LOG_ERROR,
} LOGLEVEL;
