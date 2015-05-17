#pragma once
// 
// Copyright (c) Kris Gustafson, 2012
// krisgus@ieee.org
// 

#include<memory>	// for shared_ptr<>()

//#define LOG_QUIET_MODE

class LogImplementation;
typedef std::tr1::shared_ptr<LogImplementation> LogImplementation_t;

#define LOG_LEVEL_NONE         -1 // nothing at all is logged
#define LOG_LEVEL_NORMAL        0 // shows notice, error, severe and fatal
#define LOG_LEVEL_DEBUG         1 // shows all
#define LOG_LEVEL_DEBUG_FREEMEM 2 // shows all + shows freemem on screen
#define LOG_LEVEL_MAX           LOG_LEVEL_DEBUG_FREEMEM

// ones we use in the code
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_NOTICE  2
#define LOG_WARNING 3
#define LOG_ERROR   4
#define LOG_SEVERE  5
#define LOG_FATAL   6
#define LOG_NONE    7

// extra masks - from bit 5
#define LOGMASKBIT  5
#define LOGMASK     ((1 << LOGMASKBIT) - 1)

#define LOGCURL     (1 << (LOGMASKBIT + 1))
#define LOGFFMPEG   (1 << (LOGMASKBIT + 2))
#define LOGRTMP     (1 << (LOGMASKBIT + 3))

#ifdef LOG_QUIET_MODE
#define LOGDEBUG(x, ...)	g_LogPtr->Log(LOG_DEBUG, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGINFO(x, ...)		g_LogPtr->Log(LOG_INFO, "", 0, "", x, __VA_ARGS__);
#define LOGNOTICE(x, ...)	g_LogPtr->Log(LOG_NOTICE, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGWARN(x, ...)		g_LogPtr->Log(LOG_WARNING, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGERR(x, ...)		g_LogPtr->Log(LOG_ERROR, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGSEVERE(x, ...)	g_LogPtr->Log(LOG_SEVERE, "", 0, __FUNCTION__, x, __VA_ARGS__);
#define LOGFATAL(x, ...)	g_LogPtr->Log(LOG_FATAL, "", 0, __FUNCTION__, x, __VA_ARGS__);
#else
#define LOGDEBUG(x, ...)	g_LogPtr->Log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGINFO(x, ...)		g_LogPtr->Log(LOG_INFO, "", 0, "", x, __VA_ARGS__);
#define LOGNOTICE(x, ...)	g_LogPtr->Log(LOG_NOTICE, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGWARN(x, ...)		g_LogPtr->Log(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGERR(x, ...)		g_LogPtr->Log(LOG_ERROR, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGSEVERE(x, ...)	g_LogPtr->Log(LOG_SEVERE, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#define LOGFATAL(x, ...)	g_LogPtr->Log(LOG_FATAL, __FILE__, __LINE__, __FUNCTION__, x, __VA_ARGS__);
#endif

enum ConColor { DARKBLUE = 1, DARKGREEN, DARKTEAL, DARKRED, DARKPINK, DARKYELLOW, GRAY, DARKGRAY, BLUE, GREEN, TEAL, RED, PINK, YELLOW, WHITE };