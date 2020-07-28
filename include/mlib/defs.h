#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>


#ifndef NODEFAULTLIB
#pragma comment (lib, "mlib.lib")
#endif

/// log facility claimed by MLIB
#define MLIB_LOGFAC (25<<3)

