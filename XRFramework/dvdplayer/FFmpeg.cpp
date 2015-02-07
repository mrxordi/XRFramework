#include "stdafxf.h"
#include "FFmpeg.h"

#include <map>
#include "log/Log.h"
#include "../XRThreads/SingleLock.h"
#include "../XRThreads/CriticalSection.h"
#include "../XRThreads/Thread.h"
#include "../XRCommon/utils/StringUtils.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

/* callback for the ffmpeg lock manager */
int ffmpeg_lockmgr_cb(void **mutex, enum AVLockOp operation)
{
	if (nullptr == mutex)
		return 0;

	switch (operation)
	{
	case AV_LOCK_CREATE:
	{
		*mutex = NULL;
		XR::CCriticalSection* m = new XR::CCriticalSection();
		*mutex = static_cast<void*>(m);
		break;
	}
	case AV_LOCK_OBTAIN:
	{
		XR::CCriticalSection* m = static_cast<XR::CCriticalSection*>(*mutex);
		m->lock();
		break;
	}
	case AV_LOCK_RELEASE:
	{
		XR::CCriticalSection* m = static_cast<XR::CCriticalSection*>(*mutex);
		m->unlock();
		break;
	}
	case AV_LOCK_DESTROY:
	{
		XR::CCriticalSection* m = static_cast<XR::CCriticalSection*>(*mutex);
		delete m;
		m = nullptr;
		break;
	}

	default:
		break;
	}
	return 0;
}

static XR::CCriticalSection m_logSection;
std::map<uintptr_t, std::string> g_logbuffer;

void ff_flush_avutil_log_buffers(void)
{
	XR::CSingleLock lock(m_logSection);
	/* Loop through the logbuffer list and remove any blank buffers
	If the thread using the buffer is still active, it will just
	add a new buffer next time it writes to the log */
	std::map<uintptr_t, std::string>::iterator it;
	for (it = g_logbuffer.begin(); it != g_logbuffer.end();)
		if ((*it).second.empty())
			g_logbuffer.erase(it++);
		else
			++it;
}

void ff_avutil_log(void* ptr, int level, const char* format, va_list va)
{
	XR::CSingleLock lock(m_logSection);
	uintptr_t threadId = (uintptr_t)CThread::GetCurrentThreadId();
	std::string &buffer = g_logbuffer[threadId];

	AVClass* avc = ptr ? *(AVClass**)ptr : NULL;

	if (level >= AV_LOG_DEBUG &&
		!g_LogPtr->IsLogExtraLogged(LOGFFMPEG))
		return;

	int type;
	switch (level)
	{
	case AV_LOG_INFO: type = LOG_INFO;    break;
	case AV_LOG_ERROR: type = LOG_ERROR;   break;
	case AV_LOG_DEBUG:
	default: type = LOG_DEBUG;   break;
	}

	std::string message = StringUtils::FormatV(format, va);
	std::string prefix = StringUtils::Format("ffmpeg[%lX]: ", threadId);
	if (avc)
	{
		if (avc->item_name)
			prefix += std::string("[") + avc->item_name(ptr) + "] ";
		else if (avc->class_name)
			prefix += std::string("[") + avc->class_name + "] ";
	}

	buffer += message;
	int pos, start = 0;
	while ((pos = buffer.find_first_of('\n', start)) >= 0)
	{
		if (pos>start)
			g_LogPtr->Log(type, "", 0, "", "%s%s", prefix.c_str(), buffer.substr(start, pos - start).c_str());
		start = pos + 1;
	}
	buffer.erase(0, start);
}