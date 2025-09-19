/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file sockstream.h Definition of `sockstream`, `isockstream` and `osockstream` classes.

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include <iostream>


#include <assert.h>
#include "sockbuf.h"

namespace mlib {

// clang-format on


/*!
  \class generic_sockstream
  \ingroup sockets

  An IO stream using a sockbuf object as the underlying streambuf.
  The streambuf object can be retrieved using rdbuf() function or the -> operator.
*/

template <typename T>
class generic_sockstream : public std::basic_iostream<T>
{
public:
  /// Default constructor
  generic_sockstream ()
    : std::basic_iostream<T> (new sockbuf<T> ()){};

  /// Create from an existing mlib::sockbuf
  explicit generic_sockstream (const sockbuf<T>& sb)
    : std::basic_iostream<T> (new sockbuf<T> (sb)){};

  /// Create from an existing mlib::sock
  explicit generic_sockstream (const sock& s)
    : std::basic_iostream<T> (new sockbuf<T> (s)){};

  /// Create a SOCK_STREAM or SOCK_DGRAM  stream
  explicit generic_sockstream (sock::type t, int domain = AF_INET, int proto = 0)
    : std::basic_iostream<T> (new sockbuf<T> (t, domain, proto)){};

  /// Create a SOCK_STREAM connected to a remote peer
  explicit generic_sockstream (const inaddr& remote, sock::type t = sock::stream);

  ~generic_sockstream ();

  /// Return the buffer used by this stream
  sockbuf<T>* rdbuf ()
  {
    return (sockbuf<T>*)std::basic_ios<T>::rdbuf ();
  }
  /// Return the buffer used by this stream
  sockbuf<T>* operator->()
  {
    return rdbuf ();
  }
};

template <typename T>
generic_sockstream<T>::generic_sockstream (const inaddr& remote, sock::type t)
  : std::basic_iostream<T> (new sockbuf<T> ())
{
  rdbuf ()->open (t);
  rdbuf ()->connect (remote);
}

template <typename T>
generic_sockstream< T >::~generic_sockstream ()
{
  delete std::basic_ios<T>::rdbuf ();
}

/// Input socket stream
/// \ingroup sockets
typedef generic_sockstream<char> isockstream;

/// Output socket stream
/// \ingroup sockets
typedef generic_sockstream<char> osockstream;

/// Bidirectional socket stream
/// \ingroup sockets
typedef generic_sockstream<char> sockstream;


} // namespace mlib
