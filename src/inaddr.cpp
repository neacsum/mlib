/*!
  \file inaddr.cpp Implementation of inaddr class.

  (c) Mircea Neacsu 2002. All rights reserved.
*/

//get rid of a deprecation warning related to inet_ntoa (MN 22-Jan-17) 
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <mlib/inaddr.h>
#include <mlib/wsockstream.h>
#include <mlib/trace.h>

namespace mlib {


sock_initializer init;

/*!
  \class inaddr
  \ingroup sockets
*/

/*!
  Fill sockaddr structure with host number and port.
  Parameters are in HOST order
*/
inaddr::inaddr (unsigned long host, unsigned short port)
{
  memset (&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = htonl (host);
}

/// Construct from a sockaddr structure
inaddr::inaddr (const sockaddr& s)
{
  memcpy (&sa, &s, sizeof(sa));
}

/*!
  Fill sockaddr structure after resolving hostname
*/
inaddr::inaddr (const char *hostname, unsigned short port)
{
  memset (&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  if ((sa.sin_addr.s_addr=inet_addr (hostname)) == INADDR_NONE &&
      strcmp(hostname, "255.255.255.255"))
  {
    HOSTENT *he;
    if (NULL == (he = gethostbyname (hostname)))
      sock::last_error().raise ();
    else
      memcpy (&sa.sin_addr, he->h_addr_list[0], he->h_length);
  }
}

/*!
  Set host address after resolving name
*/
erc inaddr::host (const std::string& hostname)
{
  if ((sa.sin_addr.s_addr=inet_addr (hostname.c_str())) == INADDR_NONE &&
       hostname != "255.255.255.255")
  {
    HOSTENT *he = gethostbyname (hostname.c_str());
    if (he == NULL)
      return sock::last_error();
    else
      memcpy (&sa.sin_addr, he->h_addr_list[0], he->h_length);
  }
  return erc::success;
}

/*!
  Find hostname from the host address in sockaddr
*/
std::string inaddr::hostname ()
{
  HOSTENT *he = gethostbyaddr ((char*)&sa.sin_addr, sizeof(sa.sin_addr), AF_INET);
  if (he == NULL)
  {
    DWORD err = WSAGetLastError();
    if ( err == WSAHOST_NOT_FOUND ||
         err == WSANO_DATA ||
         err == WSANO_RECOVERY ||
         err == WSATRY_AGAIN)
      return inet_ntoa( sa.sin_addr );          //Seems like a DNS problem.
                                                //Make a char string from dotted address
    else
    {
      sock::last_error().raise ();
      return "0.0.0.0";
    }
  }
  else
    return he->h_name;
}

/*!
	Return local address in HOST order
*/
unsigned inaddr::localhost ()
{
  char lcl_host [256];
  HOSTENT *hostent;
  sockaddr_in lcl_addr;
  sockaddr_in rmt_addr;
  int addr_size = sizeof(sockaddr);
  SOCKET sock;

  // Init local address (to zero)
  lcl_addr.sin_addr.s_addr = INADDR_ANY;

  // Get the local hostname
  if (gethostname (lcl_host, sizeof(lcl_host)) != SOCKET_ERROR)
  {
    // Resolve hostname for local address
    if ((hostent = gethostbyname (lcl_host)) != NULL)
      return ntohl (*((UINT*)(hostent->h_addr)));
  }

  //If still not resolved, then try second strategy

  // Get a UDP socket
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET)
  {
    // Connect to arbitrary port and address (NOT loopback)
    rmt_addr.sin_family = AF_INET;
    rmt_addr.sin_port   = htons (IPPORT_ECHO);
    rmt_addr.sin_addr.s_addr = inet_addr ("128.127.50.1");
    if (connect (sock, (sockaddr *)&rmt_addr, sizeof(sockaddr)) != SOCKET_ERROR)
      getsockname (sock, (sockaddr *)&lcl_addr, &addr_size);	// Get local address
    closesocket (sock);   // we're done with the socket
  }
  return ntohl ((lcl_addr.sin_addr.s_addr));
}

const char *inaddr::ntoa ()
{
  return inet_ntoa (sa.sin_addr);
}

}
