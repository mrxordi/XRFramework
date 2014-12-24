#include "stdafx.h"
#include "SystemClock.h"
#include "Thread.h"
#include "SingleLock.h"
#include "ThreadLocal.h"

static XR::ThreadLocal<CThread> currentThread;


XR::ILogger* CThread::logger = NULL;

/************************************************************************/
/* CONSTRUCTORS/DESTRUCTORS                                             */
/************************************************************************/
#define LOG(lvl,mes,...) if(logger) logger->Log(lvl, __FILE__, __LINE__, __FUNCTION__, mes, __VA_ARGS__);


CThread::CThread(const char* ThreadName) 
	: m_StopEvent(true, true), m_TermEvent(true), m_StartEvent(true) {

		m_bStop = false;
		m_bAutoDelete = false;

		m_ThreadId = 0;
		m_iLastTime = 0;
		m_iLastUsage = 0;
		m_fLastUsage = 0.0f;

		m_pRunnable=NULL;

		if (ThreadName) 
			m_ThreadName = ThreadName;
}

CThread::CThread(IRunnable* pRunnable, const char* ThreadName) 
	: m_StopEvent(true,true), m_TermEvent(true), m_StartEvent(true) {

		m_bStop = false;
		m_bAutoDelete = false;
		m_ThreadId = 0;
		m_iLastTime = 0;
		m_iLastUsage = 0;
		m_fLastUsage = 0.0f;

		m_pRunnable=pRunnable;

		if (ThreadName)
			m_ThreadName = ThreadName;
}

CThread::~CThread(void) {
	StopThread();
}

void CThread::Create(bool bAutoDelete, unsigned stacksize /*= 0*/) {
	if (m_ThreadId != 0) {
		LOG(LOG_ERROR, "fatal error creating thread- old thread id not null", NULL);
		exit(1);
	}

	m_iLastTime = XR::SystemClockMillis() * 10000;
	m_iLastUsage = 0;
	m_fLastUsage = 0.0f;
	m_bAutoDelete = bAutoDelete;
	m_bStop = false;
	m_StopEvent.Reset();
	m_TermEvent.Reset();
	m_StartEvent.Reset();

	SpawnThread(stacksize);
}


void CThread::Sleep(unsigned int miliseconds)
{
	if(miliseconds > 10 && IsCurrentThread())
		m_StopEvent.WaitMSec(miliseconds);
	else
		XR::ThreadSleep(miliseconds);
}


THREADFUNC CThread::staticThread(void *data) {
	CThread* pThread = (CThread*)(data);
	std::string name;
	ThreadIdentifier id;
	bool autodelete;

	if (!pThread) {
		LOG(LOG_ERROR, "sanity failed. thread is NULL.", NULL);
		return 1;
	}

	name = pThread->m_ThreadName;
	id = pThread->m_ThreadId;
	autodelete = pThread->m_bAutoDelete;

	pThread->SetThreadInfo();

	LOG(LOG_INFO,"Thread %s start, auto delete: %s", name.c_str(), (autodelete ? "true" : "false"));

	currentThread.set(pThread);
	pThread->m_StartEvent.Set();

	pThread->Action();

	// lock during termination
	XR::CSingleLock lock(pThread->m_CriticalSection);

	pThread->m_ThreadId = 0;
	pThread->m_TermEvent.Set();
	pThread->TermHandler();

	lock.Leave();

	if (autodelete) {
		LOG(LOG_DEBUG,"Thread %s %d terminating (autodelete)", name.c_str(), (DWORD)id);
		delete pThread;
		pThread = NULL;
	} else {
		LOG(LOG_DEBUG,"Thread %s %d terminating", name.c_str(), (DWORD)id);
	}

	return 0;
}

void CThread::StopThread(bool bWait /*= true*/) {
	m_bStop = true;
	m_StopEvent.Set();
	XR::CSingleLock lock(m_CriticalSection);
	if (m_ThreadId && bWait) {
		lock.Leave();
		WaitForThreadExit(0xFFFFFFFF);
	}
}

void CThread::Process() {
	if (m_pRunnable)
		m_pRunnable->Run();
}

CThread* CThread::GetCurrentThread() {
	return currentThread.get();
}

void CThread::Action() {

	try
	{
		OnStartup();
	}
	catch (...)
	{
		LOG(LOG_ERROR, "thread %s, Unhandled exception caught in thread startup, aborting. auto delete: %d", m_ThreadName.c_str(), IsAutoDelete());
		if (IsAutoDelete())
			return;
	}

	try
	{
		Process();
	}
	catch (...)
	{
		LOG(LOG_ERROR, "thread %s, Unhandled exception caught in thread process, aborting. auto delete: %d", m_ThreadName.c_str(), IsAutoDelete());
	}

	try
	{
		OnExit();
	}
	catch (...)
	{
		LOG(LOG_ERROR, "thread %s, Unhandled exception caught in thread OnExit, aborting. auto delete: %d", m_ThreadName.c_str(), IsAutoDelete());
	}
}

bool CThread::WaitForThreadExit(unsigned int milliseconds) {
	bool bReturn = true;

	XR::CSingleLock lock(m_CriticalSection);
	if (m_ThreadId && m_ThreadOpaque.handle != NULL)
	{
		// boost priority of thread we are waiting on to same as caller
		int callee = GetThreadPriority(m_ThreadOpaque.handle);
		int caller = GetThreadPriority(::GetCurrentThread());
		if(caller != THREAD_PRIORITY_ERROR_RETURN && caller > callee)
			SetThreadPriority(m_ThreadOpaque.handle, caller);

		lock.Leave();
		bReturn = m_TermEvent.WaitMSec(milliseconds);
		lock.Enter();

		// restore thread priority if thread hasn't exited
		if(callee != THREAD_PRIORITY_ERROR_RETURN && caller > callee && m_ThreadOpaque.handle)
			SetThreadPriority(m_ThreadOpaque.handle, callee);
	}
	return bReturn;
}

int CThread::GetSchedRRPriority(void) {
	return GetNormalPriority();
}

int CThread::GetPriority(void) {
	XR::CSingleLock lock(m_CriticalSection);

	int iReturn = THREAD_PRIORITY_NORMAL;
	if (m_ThreadOpaque.handle)
	{
		iReturn = GetThreadPriority(m_ThreadOpaque.handle);
	}
	return iReturn;
}

bool CThread::SetPriority(const int iPriority) {
	bool bReturn = false;

	XR::CSingleLock lock(m_CriticalSection);
	if (m_ThreadOpaque.handle) {
		bReturn = SetThreadPriority(m_ThreadOpaque.handle, iPriority) == TRUE;
	}

	return bReturn;
}

int CThread::GetMinPriority(void)
{
	return(THREAD_PRIORITY_IDLE);
}

int CThread::GetMaxPriority(void)
{
	return(THREAD_PRIORITY_HIGHEST);
}

int CThread::GetNormalPriority(void)
{
	return(THREAD_PRIORITY_NORMAL);
}

bool CThread::IsCurrentThread(const ThreadIdentifier tid)
{
	return (::GetCurrentThreadId() == tid);
}

float CThread::GetRelativeUsage() {
	unsigned int iTime = XR::SystemClockMillis();
	iTime *= 10000; // convert into 100ns tics

	// only update every 1 second
	if( iTime < m_iLastTime + 1000*10000 ) return m_fLastUsage;

	int64_t iUsage = GetAbsoluteUsage();

	if (m_iLastUsage > 0 && m_iLastTime > 0)
		m_fLastUsage = (float)( iUsage - m_iLastUsage ) / (float)( iTime - m_iLastTime );

	m_iLastUsage = iUsage;
	m_iLastTime = iTime;

	return m_fLastUsage;
}

int64_t CThread::GetAbsoluteUsage()
{
	XR::CSingleLock lock(m_CriticalSection);

	if (!m_ThreadOpaque.handle)
		return 0;

	uint64_t time = 0;
	FILETIME CreationTime, ExitTime, UserTime, KernelTime;
	if( GetThreadTimes(m_ThreadOpaque.handle, &CreationTime, &ExitTime, &KernelTime, &UserTime ) )
	{
		time = (((uint64_t)UserTime.dwHighDateTime) << 32) + ((uint64_t)UserTime.dwLowDateTime);
		time += (((uint64_t)KernelTime.dwHighDateTime) << 32) + ((uint64_t)KernelTime.dwLowDateTime);
	}
	return time;
}

ThreadIdentifier CThread::GetCurrentThreadId() {
	return ::GetCurrentThreadId();
}

void CThread::SetThreadInfo()
{
	const unsigned int MS_VC_EXCEPTION = 0x406d1388;

#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in same addr space)
		DWORD dwThreadID; // thread ID (-1 caller thread)
		DWORD dwFlags; // reserved for future use, most be zero
	} info;
#pragma pack(pop)

	info.dwType = 0x1000;
	info.szName = m_ThreadName.c_str();
	info.dwThreadID = m_ThreadId;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

}

void CThread::TermHandler() {
	CloseHandle(m_ThreadOpaque.handle);
	m_ThreadOpaque.handle = NULL;
}

void CThread::SpawnThread(unsigned stacksize) {
	// Create in the suspended state, so that no matter the thread priorities and scheduled order, the handle will be assigned
	// before the new thread exits.
	m_ThreadOpaque.handle = CreateThread(NULL, stacksize, (LPTHREAD_START_ROUTINE)&staticThread, this, CREATE_SUSPENDED, &m_ThreadId);
	if (m_ThreadOpaque.handle == NULL)
	{
		LOG(LOG_ERROR, "fatal error %d creating thread", GetLastError());
		return;
	}

	if (ResumeThread(m_ThreadOpaque.handle) == -1)
		LOG(LOG_ERROR, "fatal error %d resuming thread", GetLastError());

}








