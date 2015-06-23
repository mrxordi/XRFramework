#pragma once
#include "Event.h"
#include "ThreadImpl.h"
#include "log/ilog.h"
#include "SystemClock.h"
#include <stdint.h>

//Interface of runnable object
class IRunnable {
public:
	virtual void Run() = 0;
	virtual ~IRunnable() {}
};


class CThread 
{
	static XR::ILogger* logger;

protected:
	CThread(const char* ThreadName);

public:
	CThread(IRunnable* pRunnable, const char* ThreadName);
	virtual ~CThread(void);

	void Create(bool bAutoDelete = false, unsigned stacksize = 0);
	void Sleep(unsigned int miliseconds);
	virtual void StopThread(bool bWait = true);
	bool WaitForThreadExit(unsigned int milliseconds);

	int	 GetSchedRRPriority(void);
	int	 GetPriority(void);
	bool SetPriority(const int iPriority);
	int	 GetMinPriority(void);
	int	 GetMaxPriority(void);
	int	 GetNormalPriority(void);

	bool IsAutoDelete() const { return m_bAutoDelete; }
	bool IsRunning() const { return m_ThreadId ? true : false; }

	bool        IsCurrentThread() const { return IsCurrentThread(ThreadId()); }
	static bool IsCurrentThread(const ThreadIdentifier tid);

	float		GetRelativeUsage();  // returns the relative cpu usage of this thread since last call
	int64_t		GetAbsoluteUsage();

	static ThreadIdentifier GetCurrentThreadId();
	static CThread*			GetCurrentThread();

	static inline void		   SetLogger(XR::ILogger* logger_) { CThread::logger = logger_; }
	static inline XR::ILogger* GetLogger() { return CThread::logger; }

	virtual void OnException(){} // signal termination handler
protected:
	virtual void OnStartup(){};
	virtual void OnExit(){};
	virtual void Process();

	volatile bool m_bStop;

	enum WaitResponse { WAIT_INTERRUPTED = -1, WAIT_SIGNALED = 0, WAIT_TIMEDOUT = 1 };

	/**
	* This call will wait on a CEvent in an interruptible way such that if
	*  stop is called on the thread the wait will return with a response
	*  indicating what happened.
	*/
	inline WaitResponse AbortableWait(CEvent& event, int timeoutMillis = -1 /* indicates wait forever*/) 
	{
		XR::CEventGroup group(&event, &m_StopEvent, NULL);
		CEvent* result = timeoutMillis < 0 ? group.wait() : group.wait(timeoutMillis);
		return  result == &event ? WAIT_SIGNALED :
			(result == NULL ? WAIT_TIMEDOUT : WAIT_INTERRUPTED);
	}

private:
	static THREADFUNC staticThread(void *data);
	void Action();

	ThreadIdentifier ThreadId() const { return m_ThreadId; }
	void SetThreadInfo();
	void TermHandler();
	//void SetSignalHandlers();
	void SpawnThread(unsigned stacksize);

	ThreadIdentifier m_ThreadId;
	ThreadOpaque m_ThreadOpaque;
	bool m_bAutoDelete;
	::CEvent m_StopEvent;
	::CEvent m_TermEvent;
	::CEvent m_StartEvent;
	XR::CCriticalSection m_CriticalSection;
	IRunnable* m_pRunnable;
	uint64_t m_iLastUsage;
	uint64_t m_iLastTime;
	float m_fLastUsage;

	std::string m_ThreadName;
   
};

