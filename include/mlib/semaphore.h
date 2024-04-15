#pragma once

/*!
  \file semaphore.h semaphore class definition

  (c) Mircea Neacsu 1999

*/

#include "syncbase.h"
#include <limits.h>

namespace mlib {

class semaphore : public syncbase
{
public:
  semaphore (int limit = INT_MAX, const std::string& name = std::string ());
  int signal (int count = 1);
  operator bool ();
};

} // namespace mlib
