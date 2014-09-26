#pragma once

/*!
  \file mutex.h mutex class definition.

	(c) Mircea Neacsu 1999
*/

#include "syncbase.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class mutex : public syncbase
{
public:
  mutex (const char *name=NULL);
  operator bool ();
  void signal ();
};

/// Signal the mutex
inline
void mutex::signal () { ReleaseMutex (handle ()); }

#ifdef MLIBSPACE
};
#endif
