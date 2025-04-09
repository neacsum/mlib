/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file mutex.h Definition of mlib::mutex class

#pragma once

#include "syncbase.h"

namespace mlib {

class mutex : public syncbase
{
public:
  mutex (const std::string& name = std::string ());
  operator bool ();
  void signal ();
};

/// Signal the mutex
inline void mutex::signal ()
{
  ReleaseMutex (handle ());
}

}; // namespace mlib
