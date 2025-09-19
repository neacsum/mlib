#pragma once
/* inet_ntoa triggers some deprecation warnings. Disable them for now. */

#if defined(_MSC_VER)
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#if !defined(_WINSOCK2API_)
#ifdef _WINSOCKAPI_
#error <winsock.h> has been included before <winsock2.h>
#else
#include <winsock2.h>
#endif
#endif
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> /* for TCP_NODELAY */
#include <fcntl.h>       /* for nonblocking */
#include <netdb.h>      // for hostent
#include <cerrno>

#define ioctlsocket ioctl
inline int WSAGetLastError () {return errno;}

typedef int SOCKET;
typedef void* HANDLE;
typedef struct hostent HOSTENT;
#define INVALID_HANDLE        ((HANDLE)(-1))
#define INVALID_HANDLE_VALUE  ((HANDLE)(std::intptr_t) - 1)
#define INVALID_SOCKET        (-1)
#define SOCKET_ERROR          (-1)
#define WSAENOTSOCK           ENOTSOCK
#define WSAETIMEDOUT          ETIMEDOUT
#define WSAEINPROGRESS        EINPROGRESS
#define WSAEWOULDBLOCK        EWOULDBLOCK
#define WSAECONNABORTED       ECONNABORTED
#define WSAECONNRESET         ECONNRESET
#define WSAESHUTDOWN          ESHUTDOWN
#define WSAHOST_NOT_FOUND     HOST_NOT_FOUND
#define WSANO_DATA            NO_DATA
#define WSANO_RECOVERY        NO_RECOVERY
#define WSATRY_AGAIN          TRY_AGAIN
#endif