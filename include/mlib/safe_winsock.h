#pragma once
/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#if defined(_MSC_VER) && !defined(_WINSOCK2API_)
#ifdef _WINSOCKAPI_
#error <winsock.h> has been included before <winsock2.h>
#else
#include <winsock2.h>
#endif
#endif
