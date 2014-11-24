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

Context::Context()
{
	impl = new ContextOpaque;

	// instantiate
	impl->loggerImpl = new XR::LogImpl;

	// set
	//XbmcCommons::Exception::SetLogger(impl->loggerImpl);
	CThread::SetLogger(impl->loggerImpl);
}

Context::~Context()
{
	// cleanup
	//XbmcCommons::Exception::SetLogger(NULL);
	CThread::SetLogger(NULL);
	delete impl->loggerImpl;

	delete impl;
}