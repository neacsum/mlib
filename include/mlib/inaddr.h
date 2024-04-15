#pragma once
/*
  MLIB Library
  (c) Mircea Neacsu 2002-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file inaddr.h Definition of inaddr class.

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <winsock2.h>
#include <ostream>

#include "errorcode.h"

namespace mlib {

/// sockaddr wrapper
class inaddr
{
public:
  inaddr (unsigned long host = INADDR_ANY, unsigned short port = 0);
  inaddr (const std::string& hostname, unsigned short port);
  inaddr (const std::string& hostname, const std::string& service,
          const std::string& proto = std::string ());

  inaddr (const sockaddr& adr);
  inaddr& operator= (const inaddr& rhs);

  /// convert to a pointer to sockaddr
  operator sockaddr* () const
  {
    return (sockaddr*)&sa;
  };
  bool operator== (const inaddr& other) const;
  bool operator!= (const inaddr& other) const;

  /// convert to a reference to sockaddr (const variant)
  operator const sockaddr& () const
  {
    return *(sockaddr*)&sa;
  };

  /// convert to a reference to sockaddr (nonconst variant)
  operator sockaddr& ()
  {
    return *(sockaddr*)&sa;
  };

  /// return port number in host order
  unsigned short port () const
  {
    return ntohs (sa.sin_port);
  };

  /// set port number
  void port (unsigned short p)
  {
    sa.sin_port = htons (p);
  };

  /// set port number based on well-known service ports
  erc port (const std::string& service, const std::string& proto = std::string ());

  /// return host address in host order
  unsigned host () const
  {
    return ntohl (sa.sin_addr.s_addr);
  };

  /// set host address
  void host (unsigned int h)
  {
    sa.sin_addr.s_addr = htonl (h);
  };

  erc host (const std::string& hostname);

  std::string hostname () const;

  /// return host address in dotted format
  const char* ntoa () const;

  /// check if it's multicast host
  bool is_multicast () const
  {
    return ((host () & 0xf0000000) == 0xe0000000);
  };

  static unsigned localhost ();

private:
  sockaddr_in sa;
};

/// Assignment operator
inline inaddr& inaddr::operator= (const inaddr& rhs)
{
  memcpy (&sa, &rhs.sa, sizeof (sa));
  return *this;
}

/// Equality operator
inline bool inaddr::operator== (const inaddr& other) const
{
  return (sa.sin_addr.S_un.S_addr == other.sa.sin_addr.S_un.S_addr)
         && (sa.sin_port == other.sa.sin_port);
}

/// Inequality operator
inline bool inaddr::operator!= (const inaddr& other) const
{
  return (!operator== (other));
}

} // namespace mlib

/// Serialize an address as '<hostname>:<port>'
inline std::ostream& operator<< (std::ostream& strm, const mlib::inaddr& addr)
{
  strm << addr.ntoa () << ':' << addr.port ();
  return strm;
}
