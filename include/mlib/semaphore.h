/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file semaphore.h Definition of mlib::semaphore class

#pragma once

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
