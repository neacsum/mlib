/*! 
  \file wsockstream.h sock and sockstream classes

 Copyright (c) 2001-2019 Mircea Neacsu
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifndef _WINSOCK2API_
#error WINSOCK2 required
#endif

#include <iostream>

#include "errorcode.h"
#include "inaddr.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


///Error facility used by all sock derived classes.
extern errfac *sockerrors;

//sockbuf flags
#define _S_ALLOCBUF   0x0002    ///< locally allocated buffer
#define _S_NO_READS   0x0004    ///< write only flag
#define _S_NO_WRITES  0x0008    ///< read only flag
#define _S_EOF_SEEN   0x0010    ///< read eof condition

/// Encapsulation of a Windows socket
class sock
{
public:

  ///operation blocked by shutdown function
  enum shuthow {
    shut_read=SD_RECEIVE,   ///< blocked for reading
    shut_write=SD_SEND,     ///< blocked for writing
    shut_readwrite=SD_BOTH  ///< blocked for both
  };

  sock (SOCKET soc = INVALID_SOCKET);
  sock (int type, int domain=AF_INET, int proto=0);
  sock (const sock&);

  virtual           ~sock ();
  sock&             operator = (const sock&);

  ///Convert into a Windows socket 
  operator          SOCKET() const { return sl->handle; }
  
  ///Retrieve Windows socket handle
  SOCKET            handle () const {return sl->handle;};
  
  virtual erc       open (int type, int domain=AF_INET, int proto=0);
  void              close ();
  virtual erc       shutdown (shuthow sh);

  ///Check if socket is opened
  virtual bool      is_open () const  { return sl->handle != INVALID_SOCKET; }
  int               recv (void* buf, int maxlen, int msgf=0);
  int               recvfrom (sockaddr& sa,void* buf, int maxlen, int msgf=0);
  int               send (const void* buf, int len, int msgf=0);
  int               sendto (const sockaddr& sa,const void* buf, int len, int msgf=0);
  int               sendtimeout (int wp_sec);
  int               sendtimeout () const;
  int               recvtimeout (int wp_sec);
  int               recvtimeout () const;
  bool              is_readready (int wp_sec, int wp_usec=0) const;
  bool              is_writeready (int wp_sec, int wp_usec=0) const;
  bool              is_exceptionpending (int wp_sec, int wp_usec=0) const;
  unsigned int      nread ();

  erc               bind (const sockaddr&);
  erc               bind ();
  erc               connect (const sockaddr& peer, int wp_sec = INFINITE);
  erc               listen (int num=SOMAXCONN);
  sockaddr          name () const;
  sockaddr          peer () const;

  virtual sock      accept ();
  virtual sock      accept (sockaddr& sa);

  int               getopt (int op, void* buf,int len, int level=SOL_SOCKET) const;
  void              setopt (int op, void* buf,int len, int level=SOL_SOCKET) const;

  int               gettype () const;
  int               clearerror () const;
  bool              debug () const;
  void              debug (bool opt);
  bool              reuseaddr () const;
  void              reuseaddr (bool opt);
  bool              keepalive () const;
  void              keepalive (bool opt);
  bool              dontroute () const;
  void              dontroute (bool opt);
  bool              broadcast () const;
  void              broadcast (bool opt);
  bool              oobinline () const;
  void              oobinline (bool opt);
  int               sendbufsz () const;
  void              sendbufsz (size_t sz);
  int               recvbufsz () const;
  void              recvbufsz (size_t sz);
  void              blocking (bool on_off);
  erc               setevent (HANDLE evt, long mask);
  long              enumevents ();
  void              linger (bool on_off, unsigned short seconds);
  bool              linger (unsigned short *seconds = 0);

protected:

private:
    struct sock_life {
    SOCKET  handle;
    int     lives;
  } *sl;
};

///Provide functions required by streambuf interface using an underlying socket
class sockbuf: public sock, public std::streambuf {
public:

  //constructors
  sockbuf (SOCKET soc = INVALID_SOCKET);
  sockbuf (int type, int domain=AF_INET, int proto=0);
  sockbuf (const sockbuf&);
	sockbuf (const sock& soc);

  virtual           ~sockbuf ();
  sockbuf&          operator = (const sockbuf&);
  
protected:
  /// \name Redefined virtuals from parent streambuf
  ///\{
  virtual int       underflow ();
  virtual int       overflow (int c = EOF);
  virtual int       sync ();
  virtual std::streambuf* setbuf (char *buf, int sz);
  virtual std::streamsize showmanyc ();
  ///\}


private:
  int     x_flags;
  int     ibsize;   //input buffer size
};

/*!
  \class generic_sockstream
  \ingroup Sockets

  An IO stream using a sockbuf object as the underlying streambuf.
  The streambuf object can be retrieved using rdbuf() function or the -> operator.
*/

template <class strm> 
class generic_sockstream : public strm
{
public:
  ///Default constructor
  generic_sockstream (): strm (new sockbuf()) {};

  ///Create from an existing streambuffer
  generic_sockstream (const sockbuf& sb) : strm (new sockbuf(sb)) {};

  /// Create from an existing socket
  generic_sockstream (const sock& s) : strm (new sockbuf(s)) {};

  /// Create a SOCK_STREAM or SOCK_DGRAM  stream
  generic_sockstream (int type, int domain=AF_INET, int proto=0) : 
      strm (new sockbuf (type, domain, proto)) {};

  /// Create a SOCK_STREAM connected to a remote peer
  generic_sockstream (const inaddr& remote, int type=SOCK_STREAM);

  ~generic_sockstream ();

  ///Return the buffer used by this stream 
  sockbuf*      rdbuf () { return (sockbuf*)std::ios::rdbuf(); }
  ///Return the buffer used by this stream 
  sockbuf*      operator ->() { return rdbuf(); }
};

template <class strm>
generic_sockstream<strm>::generic_sockstream (const inaddr& remote, int type) :
strm (new sockbuf ())
{
  rdbuf ()->open (type);
  rdbuf ()->connect (remote);
}

template <class strm>
generic_sockstream<strm>::~generic_sockstream ()
{
  delete std::ios::rdbuf ();
  init (0);
}

///Input socket stream
typedef generic_sockstream<std::istream> isockstream;

///Output socket stream
typedef generic_sockstream<std::ostream> osockstream;

///Bidirectional socket stream
typedef generic_sockstream<std::iostream> sockstream;

/*---------------------- support classes ------------------------------------*/
/// Keeps an instance counter for calls to WSAStartup/WSACleanup
static struct sock_initializer
{
  sock_initializer ();
  ~sock_initializer ();
} sock_nifty_counter;

///Router for socket errors
class sock_facility : public errfac
{
public:
  sock_facility ();
  void log (const erc& e);
  const char *msg (const erc& e);
};

#ifdef MLIBSPACE
}
#endif

