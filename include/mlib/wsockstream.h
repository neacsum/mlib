/*! 
  \file wsockstream.h sock and sockstream classes

 Copyright (c) 2001-2019 Mircea Neacsu
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <WinSock2.h>
#include <iostream>

#include "errorcode.h"
#include "inaddr.h"

namespace mlib {


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
  virtual erc       close ();
  virtual erc       shutdown (shuthow sh);

  ///Check if socket is opened
  virtual bool      is_open () const  { return sl->handle != INVALID_SOCKET; }
  size_t            recv (void* buf, size_t maxlen, int msgf=0);
  size_t            recvfrom (sockaddr& sa,void* buf, size_t maxlen, int msgf=0);
  size_t            send (const void* buf, size_t len, int msgf=0);
  size_t            sendto (const sockaddr& sa,const void* buf, size_t len, int msgf=0);
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
  inaddr            name () const;
  inaddr            peer () const;

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

  /// Error facility used by all sock derived classes.
  static errfac *errors;
  static erc last_error ();

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

/// \{
/// \ingroup sockets

/*!
  \class generic_sockstream

  An IO stream using a sockbuf object as the underlying streambuf.
  The streambuf object can be retrieved using rdbuf() function or the -> operator.
*/

template <class strm> 
class generic_sockstream : public strm
{
public:
  ///Default constructor
  generic_sockstream (): strm (new sockbuf()) {};

  ///Create from an existing sockbuf
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
}

///Input socket stream
typedef generic_sockstream<std::istream> isockstream;

///Output socket stream
typedef generic_sockstream<std::ostream> osockstream;

///Bidirectional socket stream
typedef generic_sockstream<std::iostream> sockstream;

/// \}

/*---------------------- support classes ------------------------------------*/
/// Keeps an instance counter for calls to WSAStartup/WSACleanup
static struct sock_initializer
{
  sock_initializer ();
  ~sock_initializer ();
} sock_nifty_counter;

inline errfac *sock::errors;

/*==================== INLINE FUNCTIONS ===========================*/

/*!
  Set send timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
inline
int sock::sendtimeout (int wp_sec)
{
  int oldwtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char *)&oldwtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char *)&wp_sec, optlen);
  return oldwtmo / 1000;
}

///  Returns the send timeout value
inline
int sock::sendtimeout () const
{
  int oldwtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char *)&oldwtmo, &optlen);
  return oldwtmo / 1000;
}

/*!
  Set receive timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
inline
int sock::recvtimeout (int wp_sec)
{
  int oldrtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&oldrtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&wp_sec, optlen);
  return oldrtmo / 1000;
}

///  Returns the send timeout value
inline
int sock::recvtimeout () const
{
  int oldrtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&oldrtmo, &optlen);
  return oldrtmo / 1000;
}

/*!
  Returns a socket option.
  \param  op    option to return
  \param  buf   buffer for option value
  \param  len   size of buffer
  \param  level level at which the option is defined

  \return Size of returned option
*/
inline
int sock::getopt (int op, void *buf, int len, int level) const
{
  int rlen = len;
  if (::getsockopt (sl->handle, level, op, (char *)buf, &rlen) == SOCKET_ERROR)
    last_error ().raise ();
  return rlen;
}

/*!
  Set a socket option.
  \param  op    option to return
  \param  buf   buffer for option value
  \param  len   size of buffer
  \param  level level at which the option is defined
*/
inline
void sock::setopt (int op, void *buf, int len, int level) const
{
  if (::setsockopt (sl->handle, level, op, (char *)buf, len) == SOCKET_ERROR)
    last_error ().raise ();
}

/*!
  Return socket type (SOCK_DGRAM or SOCK_STREAM)
*/
inline
int sock::gettype () const
{
  int ty = 0;
  getopt (SO_TYPE, &ty, sizeof (ty));
  return ty;
}

/*!
  Return and clear the socket error flag.
  \return Error flag status

  The per socket-based error code is different from the per thread error
  code that is handled using the WSAGetLastError function call.
  A successful call using the socket does not reset the socket based error code
  returned by this function.
*/
inline
int sock::clearerror () const
{
  int err = 0;
  getopt (SO_ERROR, &err, sizeof (err));
  return err;
}

/// Return the debug flag.
inline
bool sock::debug () const
{
  BOOL old;
  getopt (SO_DEBUG, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the debug flag
inline
void sock::debug (bool b)
{
  BOOL opt = b;
  setopt (SO_DEBUG, &opt, sizeof (opt));
}

/// Return the "reuse address" flag
inline
bool sock::reuseaddr () const
{
  BOOL old;
  getopt (SO_REUSEADDR, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the "reuse address" flag
inline
void sock::reuseaddr (bool b)
{
  BOOL opt = b;
  setopt (SO_REUSEADDR, &opt, sizeof (opt));
}

/// Return "keep alive" flag
inline
bool sock::keepalive () const
{
  BOOL old;
  getopt (SO_KEEPALIVE, &old, sizeof (old));
  return (old != FALSE);
}

/// Set "keep alive" flag
inline
void sock::keepalive (bool b)
{
  BOOL opt = b;
  setopt (SO_KEEPALIVE, &opt, sizeof (opt));
}

/// Return status of "don't route" flag
inline
bool sock::dontroute () const
{
  BOOL old;
  getopt (SO_DONTROUTE, &old, sizeof (old));
  return (old != FALSE);
}

/// Turn on or off the "don't route" flag
inline
void sock::dontroute (bool b)
{
  BOOL opt = b;
  setopt (SO_DONTROUTE, &opt, sizeof (opt));
}

/// Return "broadcast" option
inline
bool sock::broadcast () const
{
  BOOL old;
  getopt (SO_BROADCAST, &old, sizeof (old));
  return (old != FALSE);
}

/*!
  Turn on or off the "broadcast" option

  \note Socket semantics require that an application set this option
  before attempting to send a datagram to broadcast address.
*/
inline
void sock::broadcast (bool b)
{
  BOOL opt = b;
  setopt (SO_BROADCAST, &opt, sizeof (opt));
}

/*!
  Return the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
inline
bool sock::oobinline () const
{
  BOOL old;
  getopt (SO_OOBINLINE, &old, sizeof (old));
  return (old != FALSE);
}

/*!
  Set the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
inline
void sock::oobinline (bool b)
{
  BOOL opt = b;
  setopt (SO_OOBINLINE, &opt, sizeof (opt));
}

/// Return buffer size for send operations.
inline
int sock::sendbufsz () const
{
  int old = 0;
  getopt (SO_SNDBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for send operations.
inline
void sock::sendbufsz (size_t sz)
{
  setopt (SO_SNDBUF, &sz, sizeof (sz));
}

/// Return buffer size for receive operations
inline
int sock::recvbufsz () const
{
  int old = 0;
  getopt (SO_RCVBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for receive operations
inline
void sock::recvbufsz (size_t sz)
{
  setopt (SO_RCVBUF, &sz, sizeof (sz));
}

/*!
  Change blocking mode.
  sock objects are created by default as blocking sockets. They can be turned
  into non-blocking sockets using this function.
*/
inline
void sock::blocking (bool on_off)
{
  unsigned long mode = on_off ? 0 : 1;
  if (ioctlsocket (sl->handle, FIONBIO, &mode) == SOCKET_ERROR)
    last_error ().raise ();
}

/*!
  Associate an event object with this socket.

  \param  evt   handle to event object
  \param  mask  conditions that trigger the event Can be a combination of:
                  FD_READ, FD_WRITE, FD_OOB, FD_ACCEPT, FD_CONNECT, FD_CLOSE

  The event object must be a manual event. It will be set to signaled state if
  the corresponding socket status change.

  It is not possible to specify different event objects for different
  network events.

  The function automatically sets socket to nonblocking mode.
*/
inline
erc sock::setevent (HANDLE evt, long mask)
{
  if (WSAEventSelect (sl->handle, (WSAEVENT)evt, mask) == SOCKET_ERROR)
    return last_error ();
  return erc::success;
}

/*!
  Indicates which of the FD_XXX network events have occurred.

  The function reports only network activity and errors for which setevent()
  has been called.
*/
inline
long sock::enumevents ()
{
  WSANETWORKEVENTS netev;
  if (WSAEnumNetworkEvents (sl->handle, NULL, &netev) == SOCKET_ERROR)
    last_error ().raise ();
  return netev.lNetworkEvents;
}

/*!
  Turn on or off linger mode and lingering timeout.
*/
inline
void sock::linger (bool on_off, unsigned short seconds)
{
  struct linger opt;
  opt.l_onoff = on_off;
  opt.l_linger = seconds;
  setopt (SO_LINGER, &opt, sizeof (opt));
}

/*!
  Return linger mode and lingering timeout.
*/
inline
bool sock::linger (unsigned short *seconds)
{
  struct linger opt;
  getopt (SO_LINGER, &opt, sizeof (opt));
  if (seconds)
    *seconds = opt.l_linger;
  return (opt.l_onoff == 0);
}

/// Return an error code with the value returned by WSAGetLastError
inline erc sock::last_error ()
{
  return erc(WSAGetLastError (), erc::error, sock::errors);
}

}

