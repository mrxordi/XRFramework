#pragma once
#include "Lockables.h"

namespace XR {
	class ConditionVariable;

	class RecursiveMutex {
		//Mutex
		CRITICAL_SECTION mutex;
		// needs acces to 'mutex'
		friend class ConditionVariable;
		
	public:
		//default constructor
		inline RecursiveMutex() {
			InitializeCriticalSection(&mutex);
		}
		//Destructor
		inline ~RecursiveMutex() {
			DeleteCriticalSection(&mutex);
		}

		inline void lock() {
			EnterCriticalSection(&mutex);
		}

		inline void unlock() {
			LeaveCriticalSection(&mutex);
		}

		inline bool try_lock() {
			return TryEnterCriticalSection(&mutex) ? true : false;
		}
	};
}

/**
 * A CCriticalSection is a CountingLockable whose implementation is a 
 *  native recursive mutex.
 *
 * This is not a typedef because of a number of "class CCriticalSection;" 
 *  forward declarations in the code that break when it's done that way.
 */
namespace XR {
	class CCriticalSection : public XR::CountingLockable < XR::RecursiveMutex > {};
}
