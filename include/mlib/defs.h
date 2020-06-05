#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>

/*!
  Namespace for all objects in MLIB. You can move everything into a different
  namespace or the global namespace by changing (or removing) this definition
*/
#define MLIBSPACE mlib

#ifndef NODEFAULTLIB
#pragma comment (lib, "mlib.lib")
#endif

/// log facility claimed by MLIB
#define MLIB_LOGFAC (25<<3)

