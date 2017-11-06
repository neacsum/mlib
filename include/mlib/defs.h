#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>

#define MLIBSPACE mlib

#ifndef NODEFAULTLIB
#pragma comment (lib, "mlib.lib")
#endif

