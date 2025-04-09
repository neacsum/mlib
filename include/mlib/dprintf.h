/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file dprintf.h Declaration of mlib::dprintf() function
#pragma once

#define MAX_DPRINTF_CHARS 1024 //!< maximum message size

namespace mlib {

/// A printf-style function for debug messages.
bool dprintf (const char* fmt, ...);

} //namespace mlib
