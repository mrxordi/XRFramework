#pragma once

#include "CriticalSection.h"
#include "Lockables.h"

/**
 * This implements a "guard" pattern for a CCriticalSection that
 *  borrows most of it's functionality from boost's unique_lock.
 */
class CSingleLock : public XR::UniqueLock<CCriticalSection> {
public:
  inline CSingleLock(CCriticalSection& cs) : XR::UniqueLock<CCriticalSection>(cs) {}
  inline CSingleLock(const CCriticalSection& cs) : XR::UniqueLock<CCriticalSection> ((CCriticalSection&)cs) {}

  inline void Leave() { unlock(); }
  inline void Enter() { lock(); }
protected:
  inline CSingleLock(CCriticalSection& cs, bool dicrim) : XR::UniqueLock<CCriticalSection>(cs,true) {}
};

/**
 * This implements a "guard" pattern for a CCriticalSection that
 *  works like a CSingleLock but only "try"s the lock and so
 *  it's possible it doesn't actually get it..
 */
class CSingleTryLock : public CSingleLock {
public:
  inline CSingleTryLock(CCriticalSection& cs) : CSingleLock(cs,true) {}

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
  CCriticalSection& sec;
  unsigned int count;
public:
  inline CSingleExit(CCriticalSection& cs) : sec(cs), count(cs.exit()) { }
  inline ~CSingleExit() { sec.restore(count); }
};

