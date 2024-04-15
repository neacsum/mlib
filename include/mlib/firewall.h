#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <Winsock2.h>

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

private:
  INetFwMgr* fwmgr;
  INetFwPolicy* fwpolicy;
  INetFwProfile* fwprofile;
};

extern errfac* fw_errptr;

}; // namespace mlib
