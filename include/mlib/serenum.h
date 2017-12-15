/*!
  \file serenum.h - Functions for enumerating serial ports
  (c) Mircea Neacsu 2017. All rights reserved.

  \defgroup serenum Serial Port Enumeration

  These functions are heavily inspired from [CEnumerateSerial] (http://www.naughter.com/enumser.html)
  code.

  From the original code I've removed methods that don't seem to be working
  (ComDBOpen) or that take a ridiculous amount of time to execute (GetDefaultCommConfig).

  In the end I was left with three methods: using CreateFile, using SetupAPI and using
  the registry. The CreateFile method has no particular advantage except that it is
  conceptually very simple. The setup API is a bit slower (about 15 ms per port on my machine)
  but it returns also the friendly port name. The registry method is blazing fast
  (under 1 ms) but does not provide the friendly name.
*/
#pragma once

#include "defs.h"

#include <vector>


#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

bool SerEnum_UsingCreateFile (std::vector<int>& ports);
bool SerEnum_UsingSetupAPI (std::vector<int>& ports, std::vector<std::string>& names);
bool SerEnum_UsingRegistry (std::vector<int>& ports);

#ifdef MLIBSPACE
};
#endif
