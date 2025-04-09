/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/


/// \file critsect.h Definitions of mlib::criticalsection and mlib::lock classes

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif
#include "safe_winsock.h"

namespace mlib {

/*!
  \class criticalsection
  \ingroup syncro
  \brief Lightweight inter-thread synchronization.

  Only one %thread at a time can enter a critical section. Critical
  sections must be leaved as many times as they were entered.
*/
class criticalsection
{
public:
  criticalsection ();
  ~criticalsection ();
  virtual void enter ();
  virtual bool try_enter ();
  virtual void leave ();

private:
  CRITICAL_SECTION section;

  // critical sections cannot be copied or assigned
  criticalsection (const criticalsection&) = delete;
  criticalsection& operator= (const criticalsection&) = delete;
};

/*!
  \class lock
  \ingroup syncro
  \brief Automatic wrapper for critical sections.

  Used in conjunction with criticalsection objects, locks simplify
  critical sections' management by taking advantage the automatic
  destruction of static objects in C++.

  Use it as in example below:
\code
  criticalsection section;  //Initialize critical section object

  void func()
  {
    lock inuse( section );  //Acquire critical section

    //... code protected by critical section

  }//Here destructor for lock is invoked and critical section is released.
\endcode

  The main advantage however is that any exception thrown inside
  func or any called function will also invoke the destructor for
  lock thus releasing the critical section.
*/

class lock
{
public:
  /// Acquire critical section
  lock (criticalsection& cs);

  /// Copy constructor
  lock (const lock& t);

  /// Leave critical section
  ~lock ();

private:
  criticalsection& section;
};

/// Initializes critical section object
inline criticalsection::criticalsection ()
{
  InitializeCriticalSection (&section);
}

/// Deletes the critical section object
inline criticalsection::~criticalsection ()
{
  DeleteCriticalSection (&section);
}

/// Enter critical section
inline void criticalsection::enter ()
{
  EnterCriticalSection (&section);
}

/// Return \b true if critical section was entered
inline bool criticalsection::try_enter ()
{
  return (TryEnterCriticalSection (&section) != 0);
}

/// Leave critical section
inline void criticalsection::leave ()
{
  LeaveCriticalSection (&section);
}

inline lock::lock (criticalsection& cs)
  : section (cs)
{
  section.enter ();
}

inline lock::lock (const lock& t)
  : section (t.section)
{
  section.enter ();
}

inline lock::~lock ()
{
  section.leave ();
}

} // namespace mlib
