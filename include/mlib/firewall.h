#pragma once

#include <netfw.h>
#include "errorcode.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class firewall
{
public:
  firewall ();
  ~firewall ();

  bool is_enabled ();
  bool is_port_enabled (int portnum, bool tcp);
  bool has_port (int portnum, bool tcp);
  bool has_app (const char *appname);
  errc add_app (const char *appname, const char *filename);
  errc add_port (int portnum, bool tcp, const char *name);
  errc set_port (int portnum, bool tcp, bool enable);

private:
  INetFwMgr* fwmgr;
  INetFwPolicy* fwpolicy;
  INetFwProfile* fwprofile;
};

extern errfac *fw_errptr;

#ifdef MLIBSPACE
};
#endif
