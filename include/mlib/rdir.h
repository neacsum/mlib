/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file rdir.h Recursive directory functions

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>

namespace mlib {

int r_mkdir (const std::string& dir);
int r_rmdir (const std::string& dir);

} // namespace mlib
