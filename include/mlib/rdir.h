/*!
  \file rdir.h - Recursive directory functions

  (c) Mircea Neacsu 2019
*/
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>

namespace mlib {

int r_mkdir (const std::string& dir);
int r_rmdir (const std::string& dir);

} // namespace mlib
