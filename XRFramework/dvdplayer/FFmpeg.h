#pragma once


extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/ffversion.h"
#include "libavfilter/avfilter.h"
#include "libpostproc/postprocess.h"
}

// callback used for locking
int ffmpeg_lockmgr_cb(void **mutex, enum AVLockOp operation);

// callback used for logging
void ff_avutil_log(void* ptr, int level, const char* format, va_list va);
void ff_flush_avutil_log_buffers(void);
