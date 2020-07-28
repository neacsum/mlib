/*!
  \file basename.h Definitions for Unix-like basename() and dirname() functions.

  (c) Mircea Neacsu 2017
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

namespace mlib {
  
///Return a pointer to pathname of the file deleting any trailing '\\' character.
const char *dirname (const char *filename);

///Return a pointer to the filename without any path component.
const char *basename (const char* filename);

};
