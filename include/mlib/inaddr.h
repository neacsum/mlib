#pragma once
/*!
  \file inaddr.h Definition of inaddr class.

  (c) Mircea Neacsu 2002. All rights reserved.

  inaddr is a wrapper class for SOCKADDR structure.
*/

//#include <winsock2.h>   //MS

#include "defs.h"
#include "errorcode.h"

#ifndef _WINSOCK2API_
#error WINSOCK2 required
#endif

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


///SOCKADDR wrapper
class inaddr
{
public:
  inaddr (unsigned long host=INADDR_ANY, unsigned short port=0);
  inaddr (const char *hostname, unsigned short port);
  inaddr (const SOCKADDR& adr);
  inaddr& operator =(const inaddr& rhs);

  /// convert to a pointer to SOCKADDR
  operator SOCKADDR* () const             { return (SOCKADDR*)&sa; };
  int operator ==(const inaddr& other) const;
  int operator != (const inaddr& other) const;

  /// convert to a reference to SOCKADDR (const variant)
  operator const SOCKADDR& () const       { return *(SOCKADDR*)&sa; };

  /// convert to a reference to SOCKADDR (nonconst variant)
  operator SOCKADDR& ()                   { return *(SOCKADDR*)&sa; };

  /// return port number in host order
  unsigned short port () const            { return ntohs(sa.sin_port); };

  /// set port number
  void port (unsigned short p)            { sa.sin_port = htons(p); };

  /// return host address in host order
  unsigned host () const                  { return ntohl(sa.sin_addr.s_addr); };

  /// set host address
  errc host (unsigned int h)           { sa.sin_addr.s_addr = htonl(h); return ERR_SUCCESS;};

  errc host (const char *hostname);
  void hostname (char *name, size_t sz);
  const char* hostname ();

  /// return host address in dotted format
  const char *ntoa()                      { return inet_ntoa(sa.sin_addr); };

  /// check if it's multicast host
  bool is_multicast () const             {return ((host() & 0xe0000000) ==  0xe0000000);};

  static unsigned localhost ();

private:
  sockaddr_in sa;
};

/// Assignment operator
inline
inaddr& inaddr::operator =(const inaddr &rhs)
{
  memcpy (&sa, &rhs.sa, sizeof(sa));
  return *this;
}

/// Equality operator
inline
int inaddr::operator == (const inaddr& other) const
{
  return (sa.sin_addr.S_un.S_addr == other.sa.sin_addr.S_un.S_addr) &&
         (sa.sin_port == other.sa.sin_port);
}

/// Inequality operator
inline
int inaddr::operator != (const inaddr& other) const
{
  return (! operator ==(other));
}


#ifdef MLIBSPACE
}
#endif
