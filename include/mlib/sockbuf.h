/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file sockbuf.h Definition of sockbuf class

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include "sock.h"
#include "trace.h"
#include <iostream>

namespace mlib {

#if !defined(SOCKBUF_BUFSIZ)
/// Default buffer size for socket streams
#define SOCKBUF_BUFSIZ 1024
#endif

/*!
  \ingroup sockets
  \class sockbuf
  Provide functions required by `streambuf` interface using an underlying socket

  You can simultaneously read and write into a `sockbuf` just like you can
  listen and talk through a telephone. Hence, the read and the write buffers
  are different.

  ### Read
  `eback()` points to the start of the get area.
  The unread chars are `gptr()` to `egptr()`.

  `eback()` is set to `base()` so that `pbackfail()` is called only when there
  is no place to putback a char. And `pbackfail()` always returns EOF.

  ### Write
  `pbase()` points to the start of the put area
  The unflushed chars are `pbase() - pptr()`
  `epptr()` points to the end of the write buffer.

  Output is flushed whenever one of the following conditions holds:
  (1) `pptr() == epptr()`
  (2) EOF is written

  ### Unbuffered
  Input buffer size is assumed to be of size 1 and output buffer is of size 0.
  That is, `egptr() <= base()+1` and `epptr() == pbase()`.
*/

template <typename T>
class sockbuf : public sock, public std::basic_streambuf<T>
{
public:
  // constructors

  /// Default constructor
  sockbuf () 
    : sock ()
    , x_flags (0)
  {
    setbuf (0, SOCKBUF_BUFSIZ);
  }

  /// Build a sockbuf object and the attached socket with the given parameters
  sockbuf (int type, int domain, int proto)
    : sock(type, domain, proto)
    , x_flags (0)
  {
    setbuf (0, SOCKBUF_BUFSIZ);
  }

  ///  Build a sockbuf object from a sock base
  sockbuf (const sock& s)
    : sock (s)
    , x_flags (0)
  {
    setbuf (0, SOCKBUF_BUFSIZ);
  }

  ///  Copy constructor
  sockbuf (const sockbuf<T>& sb)
    : sock (sb)
    , x_flags (sb.x_flags)
  {
    if (x_flags & flags::allocbuf)
      setbuf (0, (int)(const_cast<sockbuf<T>&> (sb).epptr () - const_cast<sockbuf<T>&> (sb).pbase ()));
  }

  /// Assignment operator.
  /// Maintains current buffering mode
  sockbuf<T>& operator= (const sockbuf<T>& rhs)
  {
    sock::operator = (rhs);
    std::basic_streambuf<T>::operator= (rhs);
    x_flags = (x_flags & flags::allocbuf) | (rhs.x_flags & ~flags::allocbuf);
    return *this;
  }

  ///  Destructor
  ~sockbuf ()
  {
    if (is_open ())
    {
      overflow ();
      shutdown (shut_readwrite).deactivate (); // ignore any errors
      close ().deactivate ();                  // ignore errors
    }

    if (x_flags & flags::allocbuf)
    {
      delete[] std::basic_streambuf<T>::pbase ();
      delete[] std::basic_streambuf<T>::eback ();
    }
  }

   
protected:
  using int_type = std::char_traits<T>::int_type;
  /// \name Redefined virtuals from parent streambuf
  ///\{

  int_type underflow () override
  {
    if (x_flags & flags::no_reads)
      return EOF; // reads blocked for this socket

    if (std::basic_streambuf<T>::gptr () < std::basic_streambuf<T>::egptr ())
      return *(unsigned char*)std::basic_streambuf<T>::gptr (); // return available char from buffer

    if (std::basic_streambuf<T>::eback ())
    {
      // buffered mode
      size_t rval = recv (std::basic_streambuf<T>::eback (), ibsize);
      if (rval != (size_t)EOF)
      {
        std::basic_streambuf<T>::setg (std::basic_streambuf<T>::eback (),
                                       std::basic_streambuf<T>::eback (),
                                       std::basic_streambuf<T>::eback () + rval);
        return rval ? *(unsigned char*)std::basic_streambuf<T>::gptr () : EOF;
      }
      return EOF;
    }
    else
    {
      // unbuffered mode
      unsigned char ch;
      size_t rval = recv (&ch, 1);
      if (rval == (size_t)EOF || rval == 0)
        return EOF;
      return ch;
    }
  }

  /*!
    If `c == EOF` or buffer full, return sync();
    otherwise insert `c` into the buffer and return `c`
  */
  int_type overflow (int_type c = EOF) override
  {
    if (x_flags & flags::no_writes)
      return EOF; // socket blocked for writing, can't do anything

    if (sync () == EOF) // flush output
      return EOF;

    if (c == EOF)
      return EOF;

    if (std::basic_streambuf<T>::pbase ())
    {
      // buffered mode
      *std::basic_streambuf<T>::pptr () = (T)c;
      std::basic_streambuf<T>::pbump (1);
    }
    else
    {
      // unbuffered mode
      unsigned char ch = (unsigned char)c;
      size_t rval = send (&ch, 1);
      if (rval == (size_t)EOF || rval == 0)
        return EOF;
    }
    return c;
  }

  ///  Return 0 if all chars flushed or -1 if error
  int sync () override
  {
    if (std::basic_streambuf<T>::pptr () <= std::basic_streambuf<T>::pbase ())
      return 0; // unbuffered or empty buffer
    if (!(x_flags & flags::no_writes))
    {
      size_t wlen = std::basic_streambuf<T>::pptr () - std::basic_streambuf<T>::pbase ();
      size_t wval = send (std::basic_streambuf<T>::pbase (), wlen);
      std::basic_streambuf<T>::setp (std::basic_streambuf<T>::pbase (),
                                     std::basic_streambuf<T>::epptr ());
      if (wval == wlen)
        return 0;
      TRACE ("sockbuf::sync failed - wanted %d sent %d", wlen, wval);
    }
    return -1;
  }

  /*!
    Change buffering mode.

    If \p buf is NULL, switches to automatic buffering mode with separate buffers
    for input and output. Otherwise the user supplied buffer is used for output only.
  */
  std::basic_streambuf<T>* setbuf (T* buf, std::streamsize sz) override
  {
    overflow (EOF); // flush current output buffer

    // switch to unbuffered mode
    if (x_flags & flags::allocbuf)
    {
      delete[] std::basic_streambuf<T>::eback ();
      delete[] std::basic_streambuf<T>::pbase ();
    }
    std::basic_streambuf<T>::setp (NULL, NULL);
    std::basic_streambuf<T>::setg (NULL, NULL, NULL);
    x_flags &= ~flags::allocbuf;
    ibsize = 0;
    if (!buf)
    {
      // automatic buffer - allocate separate input and output buffers
      x_flags |= flags::allocbuf;
      T* ptr = new T[(size_t)sz + 1];
      std::basic_streambuf<T>::setp (ptr, ptr + sz);
      ptr = new T[(size_t)sz];
      std::basic_streambuf<T>::setg (ptr, ptr + sz, ptr + sz);
      ibsize = (int)sz;
    }
    else
      std::basic_streambuf<T>::setp (buf,
                                     buf + sz - 1); // user specified buffer used only for output
    return this;
  }

  ///  Return number of characters available in socket buffer
  std::streamsize showmanyc () override
  {
    return nread ();
  }

  /// \}
private:
  enum flags
  {
    allocbuf = 0x0002,  ///< locally allocated buffer
    no_reads = 0x0004,  ///< write only flag
    no_writes = 0x0008, ///< read only flag
    eof_seen = 0x0010   ///< read EOF condition
  };

  int x_flags;
  int ibsize; // input buffer size
};

} // namespace mlib