#pragma once

#include <vector>

#include "Condition.h"
#include "SingleLock.h"

// forward declare the CEventGroup
namespace XR
{
  class CEventGroup;
}


/**
 * This is an Event class built from a ConditionVariable. The Event adds the state
 * that the condition is gating as well as the mutex/lock.
 *
 * This Event can be 'interruptible' (even though there is only a single place
 * in the code that uses this behavior).
 *
 * This class manages 'spurious returns' from the condition variable.
 */
class CEvent : public XR::NonCopyable
{
  bool manualReset;
  volatile bool signaled;
  unsigned int numWaits;

  XR::CCriticalSection groupListMutex; // lock for the groups list
  std::vector<XR::CEventGroup*> * groups;

  /**
   * To satisfy the TightConditionVariable requirements and allow the 
   *  predicate being monitored to include both the signaled and interrupted
   *  states.
   */
  XR::ConditionVariable actualCv;
  XR::TightConditionVariable<volatile bool&> condVar;
  XR::CCriticalSection mutex;

  friend class XR::CEventGroup;

  void addGroup(XR::CEventGroup* group);
  void removeGroup(XR::CEventGroup* group);

  // helper for the two wait methods
  inline bool prepReturn() { bool ret = signaled; if (!manualReset && numWaits == 0) signaled = false; return ret; }

public:
  inline CEvent(bool manual = false, bool signaled_ = false) : 
    manualReset(manual), signaled(signaled_), numWaits(0), groups(NULL), condVar(actualCv,signaled) {}

  inline void Reset() { XR::CSingleLock lock(mutex); signaled = false; }
  void Set();

  /**
   * This will wait up to 'milliSeconds' milliseconds for the Event
   *  to be triggered. The method will return 'true' if the Event
   *  was triggered. Otherwise it will return false.
   */
  inline bool WaitMSec(unsigned int milliSeconds) 
  { XR::CSingleLock lock(mutex); numWaits++; condVar.wait(mutex,milliSeconds); numWaits--; return prepReturn(); }

  /**
   * This will wait for the Event to be triggered. The method will return 
   * 'true' if the Event was triggered. If it was either interrupted
   * it will return false. Otherwise it will return false.
   */
  inline bool Wait()
  { XR::CSingleLock lock(mutex); numWaits++; condVar.wait(mutex); numWaits--; return prepReturn(); }

  /**
   * This is mostly for testing. It allows a thread to make sure there are 
   *  the right amount of other threads waiting.
   */
  inline int getNumWaits() { XR::CSingleLock lock(mutex); return numWaits; }

};

namespace XR
{
  /**
   * CEventGroup is a means of grouping CEvents to wait on them together.
   * It is equivalent to WaitOnMultipleObject that returns when "any" Event
   * in the group signaled.
   */
  class CEventGroup : public NonCopyable
  {
    std::vector<::CEvent*> events;
    ::CEvent* signaled;
    ConditionVariable actualCv;
    TightConditionVariable<::CEvent*&> condVar;
    CCriticalSection mutex;

    unsigned int numWaits;

    // This is ONLY called from CEvent::Set.
    inline void Set(::CEvent* child) { XR::CSingleLock l(mutex); signaled = child; condVar.notifyAll(); }

    friend class ::CEvent;

  public:

    /**
     * Create a CEventGroup from a number of CEvents. num is the number
     *  of Events that follow. E.g.:
     *
     *  CEventGroup g(3, event1, event2, event3);
     */
    CEventGroup(int num, ::CEvent* v1, ...);

    /**
     * Create a CEventGroup from a number of CEvents. The parameters
     *  should form a NULL terminated list of CEvent*'s
     *
     *  CEventGroup g(event1, event2, event3, NULL);
     */
    CEventGroup(::CEvent* v1, ...);
    ~CEventGroup();

    /**
     * This will block until any one of the CEvents in the group are
     * signaled at which point a pointer to that CEvents will be 
     * returned.
     */
    ::CEvent* wait();

    /**
     * This will block until any one of the CEvents in the group are
     * signaled or the timeout is reachec. If an event is signaled then
     * it will return a pointer to that CEvent, otherwise it will return
     * NULL.
     */
    ::CEvent* wait(unsigned int milliseconds);

    /**
     * This is mostly for testing. It allows a thread to make sure there are 
     *  the right amount of other threads waiting.
     */
    inline int getNumWaits() { XR::CSingleLock lock(mutex); return numWaits; }

  };
}
