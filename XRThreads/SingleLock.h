#pragma once

#include "CriticalSection.h"
#include "Lockables.h"

namespace XR {
	/**
	 * This implements a "guard" pattern for a CCriticalSection that
	 *  borrows most of it's functionality from boost's unique_lock.
	 */
	class CSingleLock : public XR::UniqueLock < XR::CCriticalSection > {
	public:
		inline XR::CSingleLock(XR::CCriticalSection& cs) : XR::UniqueLock<XR::CCriticalSection>(cs) {}
		inline XR::CSingleLock(const XR::CCriticalSection& cs) : XR::UniqueLock<XR::CCriticalSection>((XR::CCriticalSection&)cs) {}

		inline void Leave() { unlock(); }
		inline void Enter() { lock(); }
	protected:
		inline XR::CSingleLock(XR::CCriticalSection& cs, bool dicrim) : XR::UniqueLock<XR::CCriticalSection>(cs, true) {}
	};

	/**
	 * This implements a "guard" pattern for a CCriticalSection that
	 *  works like a CSingleLock but only "try"s the lock and so
	 *  it's possible it doesn't actually get it..
	 */
	class CSingleTryLock : public CSingleLock {
	public:
		inline CSingleTryLock(XR::CCriticalSection& cs) : XR::CSingleLock(cs, true) {}

		inline bool IsOwner() const { return owns_lock(); }
	};

	/**
	 * This implements a "guard" pattern for exiting all locks
	 *  currently being held by the current thread and restoring
	 *  those locks on destruction.
	 *
	 * This class can be used on a CCriticalSection that isn't owned
	 *  by this thread in which case it will do nothing.
	 */
	class CSingleExit {
		XR::CCriticalSection& sec;
		unsigned int count;
	public:
		inline CSingleExit(XR::CCriticalSection& cs) : sec(cs), count(cs.exit()) { }
		inline ~CSingleExit() { sec.restore(count); }
	};

}