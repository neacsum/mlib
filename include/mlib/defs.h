#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>

#define MLIBSPACE mlib

#pragma comment (lib, "mlib.lib")

