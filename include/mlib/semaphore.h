#pragma once

/*!
  \file semaphore.h semaphore class implementation

	(c) Mircea Neacsu 1999

\verbatim
  $Revision$
  $Author$
\endverbatim
*/

#include "syncbase.h"
#include <limits.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class semaphore : public syncbase
{
public:
  semaphore (int limit=INT_MAX, const char *name=NULL);
  int signal (int count=1);
  operator bool ();
};

#ifdef MLIBSPACE
};
#endif
