#include "stdafxf.h"
#include "LoggerContext.h"
#include "../XRThreads/Thread.h"
#include "log/Log.h"
#include "log/ilog.h"

class ContextOpaque
{
public:
	XR::ILogger* loggerImpl;

	ContextOpaque() : loggerImpl(NULL) {}
};

LoggerContext::LoggerContext()
{
	impl = new ContextOpaque;

	// instantiate
	impl->loggerImpl = new XR::LogImpl;

	CThread::SetLogger(impl->loggerImpl);
}

LoggerContext::~LoggerContext()
{
	// cleanup
	CThread::SetLogger(NULL);
	delete impl->loggerImpl;

	delete impl;
}