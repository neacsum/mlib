#pragma once

#if defined(_MSC_VER)
# ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
    /* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#   define _WINSOCK_DEPRECATED_NO_WARNINGS
# endif

# if !defined(_WINSOCK2API_) && defined(_WINSOCKAPI_)
#   error <winsock.h> has been included before <winsock2.h>
# else
#   ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#    define MLIB_MUST_UNDEF_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#   include <WinSock2.h>
#   ifdef MLIB_MUST_UNDEF_LEAN_AND_MEAN
#     undef WIN32_LEAN_AND_MEAN
#   endif
# endif
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