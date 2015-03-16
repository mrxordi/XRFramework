#pragma once

namespace XR
{
  /**
   * A thin wrapper around windows thread specific storage
   * functionality.
   */
  template <typename T> class ThreadLocal
  {
    DWORD key;
  public:
    inline ThreadLocal()
    {
       if ((key = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		   throw;
    }

    inline ~ThreadLocal() 
    {
       if (!TlsFree(key))
		   throw;
    }

    inline void set(T* val)
    {
       if (!TlsSetValue(key,(LPVOID)val))
		   throw;
    }

    inline T* get() { return (T*)TlsGetValue(key); }
  };
}

