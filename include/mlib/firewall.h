/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file firewall.h Definition of mlib::firewall class
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"

#include <netfw.h>
#include "errorcode.h"

namespace mlib {

/// Light wrapper for Windows firewall
class firewall
{
public:
  firewall ();
  ~firewall ();

  bool is_enabled ();
  bool is_port_enabled (int portnum, bool tcp);
  bool has_port (int portnum, bool tcp);
  bool has_app (const char* appname);
  erc add_app (const char* appname, const char* filename);
  erc add_port (int portnum, bool tcp, const char* name);
  erc set_port (int portnum, bool tcp, bool enable);

  static errfac& Errors ();
  static void Errors (errfac& facility);

private:
  INetFwMgr* fwmgr;
  INetFwPolicy* fwpolicy;
  INetFwProfile* fwprofile;
};

extern errfac* fw_errptr;

}; // namespace mlib
