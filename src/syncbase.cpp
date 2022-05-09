/*!
  \file syncbase.cpp Member functions of syncbase class

  (c) Mircea Neacsu 1999-2017

*/
#include <mlib/syncbase.h>

namespace mlib {
/*!
  \defgroup syncro  Synchronization Objects
  \brief Wrapper classes for Windows synchronization mechanisms.

  While, currently, most of these objects have standard conforming replacements
  (std::mutex, std::semaphore, std::thread, etc.) there are still use cases where
  standard versions lack functionality compared to objects in this library.

  One such case is when trying to achieve synchronization between different processes.
  Standard objects have no (portable) mechanism for sharing them between processes.
  Other limitations include wakeup on message received (syncbase::wait_alertable)
  and many limitations related to mlib::thread objects.
*/

/*!
  \class syncbase
  \ingroup syncro

  The Windows handle has an instance counter so copying and assignment are
  safe and rather cheap (doesn't call DuplicateHandle).

  The virtual bool conversion operator allows testing the signaled state of the
  object (it is true if object is signaled).
*/

/// Default constructor
syncbase::syncbase ()
  : hl (NULL)
{
}

/// Protected constructor
syncbase::syncbase (const char *a_name)
  : hl (new handle_life)
  , name_ (a_name?a_name:std::string())
{
  hl->handle_ = nullptr;
  hl->lives = 1;
}

/// Copy constructor
syncbase::syncbase (const syncbase& other) :
name_ (other.name_)
{
  hl = other.hl;
  if (hl)
    hl->lives++;
}

/// Move constructor
syncbase::syncbase (syncbase&& other) noexcept:
  name_ (other.name_)
{
  hl = other.hl;
  other.hl = 0;
  other.name_.clear ();
}

/// Destructor
syncbase::~syncbase ()
{
  if (hl && --hl->lives == 0)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
    delete hl;
    hl = 0;
  }
}

/// Assignment operator
syncbase& syncbase::operator =(const syncbase& rhs)
{
  if (&rhs == this)
    return *this;       //trivial assignment

  if (hl && --hl->lives == 0)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
    delete hl;
  }
  hl = rhs.hl;
  if (hl)
    hl->lives++;
  name_ = rhs.name_;
  return *this;
}

/// Move assignment operator
syncbase& syncbase::operator = (syncbase&& rhs) noexcept
{
  if (&rhs == this)
    return *this;       //trivial assignment

  if (hl && --hl->lives == 0)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
    delete hl;
  }
  hl = rhs.hl;
  rhs.hl = nullptr;
  rhs.name_.clear ();
  name_ = rhs.name_;
  return *this;
}


/// Equality operator
int syncbase::operator ==(const syncbase& rhs) const
{
  return (!hl && !rhs.hl) 
    || ((hl && rhs.hl) && (hl->handle_ == rhs.hl->handle_));
}

/// Wait for the object to become signaled
DWORD syncbase::wait (DWORD time_limit)
{
  assert (hl && hl->handle_);
  return WaitForSingleObject (hl->handle_, time_limit);
}

/// Wait for object to become signaled or an APC or IO completion routine 
/// to occur
DWORD syncbase::wait_alertable (DWORD time_limit)
{
  assert (hl && hl->handle_);
  return WaitForSingleObjectEx (hl->handle_, time_limit, TRUE);
}

/// Change object's handle. Closes the previous one.
void syncbase::set_handle (HANDLE h)
{
  if (hl)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
  }
  else
  {
    hl = new handle_life;
    hl->lives = 1;
  }
  hl->handle_ = h;
}

/// Change object's name
void syncbase::name (const char* nam)
{
  name_ = nam ? nam : std::string ();
}

/// Check if object is signaled
syncbase::operator bool ()
{
  return (WaitForSingleObject (hl->handle_, 0) == WAIT_OBJECT_0);
}

/// Wait for object to become signaled or a message to be queued
DWORD syncbase::wait_msg (DWORD time_limit, DWORD mask)
{
  return MsgWaitForMultipleObjects (1, &hl->handle_, FALSE, time_limit, mask);
}

/// Busy waiting for a number of microseconds
void udelay (unsigned short usec)
{
  LARGE_INTEGER interval, crt;
  static LARGE_INTEGER freq;
  if (!freq.QuadPart)
    QueryPerformanceFrequency (&freq);
  QueryPerformanceCounter (&interval);
  interval.QuadPart += (LONGLONG)usec * 1000000L / freq.QuadPart;
  do {
    QueryPerformanceCounter (&crt);
  } while (crt.QuadPart < interval.QuadPart);
}

}
