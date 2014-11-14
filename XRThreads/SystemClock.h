#pragma once

#undef max
#include <limits>


namespace XR
{
  /**
   * This function returns the system clock's number of milliseconds but with
   *  an arbitrary reference point. It handles the wrapping of any underlying
   *  system clock by setting a starting point at the first call. It should
   *  only be used for measuring time durations.
   *
   * Of course, on windows it just calls timeGetTime, so you're on your own.
   */
  unsigned int SystemClockMillis();

  /**
   * DO NOT compare the results from SystemClockMillis() to an expected end time
   *  that was calculated by adding a number of milliseconds to some start time.
   *  The reason is because the SystemClockMillis could wrap. Instead use this
   *  class which uses differences (which are safe accross a wrap).
   */
  class EndTime {
    unsigned int startTime;
    unsigned int totalWaitTime;
  public:
    static const unsigned int InfiniteValue;
    inline EndTime() : startTime(0), totalWaitTime(0) {}
    inline EndTime(unsigned int millisecondsIntoTheFuture) : startTime(SystemClockMillis()), totalWaitTime(millisecondsIntoTheFuture) {}

    inline void Set(unsigned int millisecondsIntoTheFuture) { startTime = SystemClockMillis(); totalWaitTime = millisecondsIntoTheFuture; }

    inline bool IsTimePast() const { return totalWaitTime == InfiniteValue ? false : (totalWaitTime == 0 ? true : (SystemClockMillis() - startTime) >= totalWaitTime); }

    inline unsigned int MillisLeft() const
    {
      if (totalWaitTime == InfiniteValue)
        return InfiniteValue;
      if (totalWaitTime == 0)
        return 0;
      unsigned int timeWaitedAlready = (SystemClockMillis() - startTime);
      return (timeWaitedAlready >= totalWaitTime) ? 0 : (totalWaitTime - timeWaitedAlready);
    }

    inline void SetExpired() { totalWaitTime = 0; }
    inline void SetInfinite() { totalWaitTime = InfiniteValue; }
    inline bool IsInfinite(void) const { return (totalWaitTime == InfiniteValue); }
    inline unsigned int GetInitialTimeoutValue(void) const { return totalWaitTime; }
    inline unsigned int GetStartTime(void) const { return startTime; }
  };
}
