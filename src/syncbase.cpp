/*!
  \file syncbase.cpp Member functions of syncbase class

  (c) Mircea Neacsu 1999-2024

*/
#include <mlib/mlib.h>
#pragma hdrstop

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
{}

/// Protected constructor
syncbase::syncbase (const std::string& a_name)
  : hl (new handle_life)
  , name_ (a_name)
{
  hl->handle_ = nullptr;
  hl->lives = 1;
}

/// Copy constructor
syncbase::syncbase (const syncbase& other)
  : name_ (other.name_)
{
  hl = other.hl;
  if (hl)
    hl->lives++;
}

/// Move constructor
syncbase::syncbase (syncbase&& other) noexcept
  : name_ (other.name_)
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
syncbase& syncbase::operator= (const syncbase& rhs)
{
  if (&rhs == this)
    return *this; // trivial assignment

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
syncbase& syncbase::operator= (syncbase&& rhs) noexcept
{
  if (&rhs == this)
    return *this; // trivial assignment

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
int syncbase::operator== (const syncbase& rhs) const
{
  return (!hl && !rhs.hl) || ((hl && rhs.hl) && (hl->handle_ == rhs.hl->handle_));
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

} // namespace mlib
