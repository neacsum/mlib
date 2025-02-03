/*!
  \file semaphore.cpp semaphore class implementation

  (c) Mircea Neacsu 1999

*/
#include <mlib/mlib.h>
#pragma hdrstop
#include <assert.h>

#include <utf8/utf8.h>

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
semaphore::semaphore (int limit, const std::string& name)
  : syncbase (name)
{
  HANDLE h = CreateSemaphoreW (NULL, 0, limit, 
    !name.empty () ? utf8::widen (name).c_str () : NULL);

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
    signal ();
  return result;
}

} // namespace mlib
