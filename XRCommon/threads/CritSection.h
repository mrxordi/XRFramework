#pragma once

/*
namespace XR {
	//Critical section implementation for MFC

	class CCriticalSection
	{
		// make copy constructor and assignment operator inaccessible
		CCriticalSection(const CCriticalSection &refCritSec);
		CCriticalSection &operator=(const CCriticalSection &refCritSec);

		CRITICAL_SECTION m_CritSection;
#ifdef _DEBUG
	public:
		CCriticalSection();
		~CCriticalSection();

		void Lock();
		void Unlock();
		void Try_enter();
	public:
		DWORD   m_currentOwner;
		DWORD   m_lockCount;
		BOOL    m_fTrace;        // Trace this one

#else // _DEBUG

	public:
		CCriticalSection() {
			InitializeCriticalSection(&m_CritSection);
		};
		~CCriticalSection() {
			DeleteCriticalSection(&m_CritSection);
		};

		void Lock() {
			EnterCriticalSection(&m_CritSection);
		};
		void Unlock(){
			LeaveCriticalSection(&m_CritSection);
		};
		bool Try_enter(){
			return TryEnterCriticalSection(&m_CritSection);
		};
#endif
	};

	//
	// To make deadlocks easier to track it is useful to insert in the
	// code an assertion that says whether we own a critical section or
	// not.  We make the routines that do the checking globals to avoid
	// having different numbers of member functions in the debug and
	// retail class implementations of CCritSec.  In addition we provide
	// a routine that allows usage of specific critical sections to be
	// traced.  This is NOT on by default - there are far too many.
	//

#ifdef DEBUG
	BOOL WINAPI CritCheckIn(CCriticalSection * pcCrit);
	BOOL WINAPI CritCheckIn(const CCriticalSection * pcCrit);
	BOOL WINAPI CritCheckOut(CCriticalSection * pcCrit);
	BOOL WINAPI CritCheckOut(const CCriticalSection * pcCrit);
	void WINAPI DbgLockTrace(CCriticalSection * pcCrit, BOOL fTrace);
#else
#define CritCheckIn(x) TRUE
#define CritCheckOut(x) TRUE
#define DbgLockTrace(pc, fT)
#endif

}*/



