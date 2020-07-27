/*!
  \file semaphore.cpp semaphore class implementation

  (c) Mircea Neacsu 1999

*/
#include <mlib/semaphore.h>
#include <assert.h>

#if __has_include(<utf8/utf8.h>)
#define HAS_UTF8
#include <utf8/utf8.h>
#endif

namespace mlib {

/*!
  \class semaphore
  \ingroup syncro
  \brief Wrapper for Windows semaphore objects.
  A semaphore is a counter incremented by signal and decremented by wait.
  If counter is negative any waiting thread is blocked until counter becomes
  positive.
*/

/*! 
  \param limit  maximum limit for counter
  \param name   object's name
*/
semaphore::semaphore (int limit, const char *name)
  : syncbase (name)
{
#ifdef HAS_UTF8
  HANDLE h = CreateSemaphoreW (NULL, 0, limit, name?utf8::widen(name).c_str():0);
#else
  HANDLE h = CreateSemaphoreA (NULL, 0, limit, name);
#endif

  assert (h);
  set_handle (h);
}

/// Signal a %semaphore object
int semaphore::signal (int cnt)
{
  long prev;
  ReleaseSemaphore (handle (), cnt, &prev);
  return (int)prev;
}

/// Test if semaphore is signaled
semaphore::operator bool ()
{
  bool result = syncbase::operator bool ();
  if (result)
    signal();
  return result;
}

}
