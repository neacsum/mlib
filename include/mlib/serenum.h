/*!
  \file serenum.h Functions for enumerating serial ports

  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#include <vector>

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

bool SerEnum_UsingCreateFile (std::vector<int>& ports);
bool SerEnum_UsingSetupAPI (std::vector<int>& ports, std::vector<std::string>& names);
bool SerEnum_UsingRegistry (std::vector<int>& ports);

#ifdef MLIBSPACE
};
#endif
