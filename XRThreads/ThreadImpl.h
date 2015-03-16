#pragma once

struct threadOpaque
{
  HANDLE handle;
};

typedef DWORD ThreadIdentifier;
typedef threadOpaque ThreadOpaque;
typedef DWORD THREADFUNC;

namespace XR
{
  inline static void ThreadSleep(unsigned int millis) { Sleep(millis); }
}