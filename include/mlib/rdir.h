/*!
  \file RDIR.H - Recursive directory functions

  (c) Mircea Neacsu 2019
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

int r_mkdir (const std::string& dir);
int r_rmdir (const std::string& dir);

#ifdef MLIBSPACE
};
#endif
