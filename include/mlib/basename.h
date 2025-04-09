/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file basename.h Declarations for mlib::basename() and mlib::dirname() functions.

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

/// Return a pointer to pathname of the file deleting any trailing '\\' character.
const char* dirname (const char* filename);

/// Return a pointer to the filename without any path component.
const char* basename (const char* filename);

}; // namespace mlib
