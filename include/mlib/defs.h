#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <WinSock2.h>
#include <windows.h>

#define MLIBSPACE mlib

#pragma comment (lib, "mlib.lib")

