/*
  MLIB Library
  (c) Mircea Neacsu 2001-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file wsockstream.h Definition of sock and sockstream classes
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include <iostream>

#include <mlib/errorcode.h>
#include <mlib/inaddr.h>

#include <assert.h>

namespace mlib {

// clang-format off
/// Encapsulation of a Windows socket
class sock
{
public:
  /// operation blocked by shutdown function
  enum shuthow
  {
    shut_read = 0,     ///< blocked for reading
    shut_write = 1,    ///< blocked for writing
    shut_readwrite = 2 ///< blocked for both
  };

  /// Flags for send/receive operations
  enum mflags
  {
    none = 0,                   ///< no flags
    out_of_band = MSG_OOB,      ///< send out of band data
    peek = MSG_PEEK,            ///< don't remove data from the input queue
    dont_route = MSG_DONTROUTE, ///< data should not be routed
    wait_all = MSG_WAITALL      ///< wait until buffer full or connection closed
  };

  sock ();
  explicit sock (SOCKET soc);
  explicit sock (int type, int domain = AF_INET, int proto = 0);
  sock (const sock&);
  sock (sock&&);

  virtual ~sock ();
  sock& operator= (const sock&);
  sock& operator= (sock&&);

  HANDLE       handle () const;
  bool         operator== (const sock& other) const;
  bool         operator!= (const sock& other) const;

  virtual erc  open (int type, int domain = AF_INET, int proto = 0);
  virtual erc  close ();
  virtual erc  shutdown (shuthow sh) const;

  virtual bool is_open () const;
  size_t       recv (void* buf, size_t maxlen, mflags msgf = mflags::none) const;
  size_t       recvfrom (sockaddr& sa, void* buf, size_t maxlen, mflags msgf = mflags::none) const;
  size_t       send (const void* buf, size_t len, mflags msgf = mflags::none) const;
  template <typename T>
  size_t       send (std::basic_string<T> buf, mflags msgf = mflags::none) const;
  size_t       sendto (const sockaddr& sa, const void* buf, size_t len, mflags msgf = mflags::none) const;
  template <typename T>
  size_t       sendto (const sockaddr& sa, std::basic_string<T> buf, mflags msgf = mflags::none) const;

  int          sendtimeout (int wp_sec) const;
  int          sendtimeout () const;
  int          recvtimeout (int wp_sec) const;
  int          recvtimeout () const;
  bool         is_readready (int wp_sec, int wp_usec = 0) const;
  bool         is_writeready (int wp_sec, int wp_usec = 0) const;
  bool         is_exceptionpending (int wp_sec, int wp_usec = 0) const;
  unsigned int nread () const;

  erc          bind (const inaddr&) const;
  erc          bind () const;
  erc          connect (const inaddr& peer) const;
  erc          connect (const inaddr& peer, int wp_sec) const;
  erc          listen (int num = SOMAXCONN) const;
  erc          accept (sock& client, inaddr* sa = nullptr) const;
  erc          accept (sock& client, int wp_sec, inaddr* sa = nullptr) const;
  erc          name (inaddr& addr) const;
  erc          peer (inaddr& addr) const;

  int          getopt (int op, void* buf, int len, int level = SOL_SOCKET) const;
  erc          setopt (int op, void* buf, int len, int level = SOL_SOCKET) const;

  int          gettype () const;
  int          clearerror () const;
  bool         debug () const;
  void         debug (bool opt) const;
  bool         reuseaddr () const;
  void         reuseaddr (bool opt) const;
  bool         keepalive () const;
  void         keepalive (bool opt) const;
  bool         dontroute () const;
  void         dontroute (bool opt) const;
  bool         broadcast () const;
  void         broadcast (bool opt) const;
  bool         oobinline () const;
  void         oobinline (bool opt) const;
  int          sendbufsz () const;
  void         sendbufsz (size_t sz) const;
  int          recvbufsz () const;
  void         recvbufsz (size_t sz) const;
  void         blocking (bool on_off);
  erc          setevent (HANDLE evt, long mask) const;
  long         enumevents () const;
  void         linger (bool on_off, unsigned short seconds) const;
  bool         linger (unsigned short* seconds = 0) const;

protected:
  /// Error facility used by all sock derived classes.
  static errfac* errors;
  static erc   last_error ();

private:
  struct sock_ref
  {
    sock_ref ()
      : handle{INVALID_SOCKET}
      , ref_count{1} {};
    SOCKET handle;
    int ref_count;
  }* sl;

  friend class inaddr;
  friend struct sock_initializer;
};

/// Extraction operator shows socket handle
inline std::ostream& operator<< (std::ostream& strm, const sock& s)
{
  strm << s.handle ();
  return strm;
}

// default buffer size
#if !defined(SOCKBUF_BUFSIZ)
#define SOCKBUF_BUFSIZ 1024
#endif

/// Provide functions required by `streambuf` interface using an underlying socket
class sockbuf : public sock, public std::streambuf
{
public:
  // constructors
  sockbuf (SOCKET soc = INVALID_SOCKET);
  sockbuf (int type, int domain = AF_INET, int proto = 0);
  sockbuf (const sockbuf&);
  sockbuf (const sock& soc);

  virtual ~sockbuf ();
  sockbuf& operator= (const sockbuf&);

protected:
  /// \name Redefined virtuals from parent `streambuf`
  ///\{
  virtual int             underflow () override;
  virtual int             overflow (int c = EOF) override;
  virtual int             sync () override;
  virtual std::streambuf* setbuf (char* buf, std::streamsize sz) override;
  virtual std::streamsize showmanyc () override;
  ///\}

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
// clang-format on

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
  /// Default constructor
  generic_sockstream ()
    : strm (new sockbuf ()){};

  /// Create from an existing sockbuf
  generic_sockstream (const sockbuf& sb)
    : strm (new sockbuf (sb)){};

  /// Create from an existing socket
  generic_sockstream (const sock& s)
    : strm (new sockbuf (s)){};

  /// Create a SOCK_STREAM or SOCK_DGRAM  stream
  generic_sockstream (int type, int domain = AF_INET, int proto = 0)
    : strm (new sockbuf (type, domain, proto)){};

  /// Create a SOCK_STREAM connected to a remote peer
  generic_sockstream (const inaddr& remote, int type = SOCK_STREAM);

  ~generic_sockstream ();

  /// Return the buffer used by this stream
  sockbuf* rdbuf ()
  {
    return (sockbuf*)std::ios::rdbuf ();
  }
  /// Return the buffer used by this stream
  sockbuf* operator->()
  {
    return rdbuf ();
  }
};

template <class strm>
generic_sockstream<strm>::generic_sockstream (const inaddr& remote, int type)
  : strm (new sockbuf ())
{
  rdbuf ()->open (type);
  rdbuf ()->connect (remote);
}

template <class strm>
generic_sockstream<strm>::~generic_sockstream ()
{
  delete std::ios::rdbuf ();
}

/// Input socket stream
typedef generic_sockstream<std::istream> isockstream;

/// Output socket stream
typedef generic_sockstream<std::ostream> osockstream;

/// Bidirectional socket stream
typedef generic_sockstream<std::iostream> sockstream;

/// \}

/*---------------------- support classes ------------------------------------*/
/// Keeps an instance counter for calls to WSAStartup/WSACleanup
static struct sock_initializer
{
  sock_initializer ();
  ~sock_initializer ();
} sock_nifty_counter;

inline errfac* sock::errors;

/*==================== INLINE FUNCTIONS ===========================*/

/// Default constructor creates a closed socket
inline sock::sock ()
  : sl{nullptr}
{}

/// Retrieve Windows socket handle
inline HANDLE sock::handle () const
{
  return sl ? (HANDLE)sl->handle : INVALID_HANDLE_VALUE;
}

/// Check if socket is opened
inline bool sock::is_open () const
{
  return sl && sl->handle != INVALID_SOCKET;
}

/// Establishes a connection to specified peer
inline erc sock::connect (const inaddr& peer) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int ret = ::connect (sl->handle, peer, sizeof (peer));
  if (ret == SOCKET_ERROR)
    return last_error ();

  return erc::success;
}

/// Permits an incoming connection attempt on the socket.
inline erc sock::accept (sock& client, inaddr* addr) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  sockaddr sa;
  int len = sizeof (sa);
  client = sock (::accept (sl->handle, &sa, &len));
  if (addr)
    *addr = sa;
  return erc ();
}

/// Places the socket in a state in which it is listening for incoming connections.
inline erc sock::listen (int num) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  ::listen (sl->handle, num);
  return last_error ();
}

/*!
  Send data to connected peer.
  \param  buf     data to send
  \param  msgf    flags (MSG_OOB or MSG_DONTROUTE)

  The function returns the number of characters actually sent or EOF if
  connection was closed by peer.
*/
template <typename T>
size_t sock::send (std::basic_string<T> buf, mflags msgf) const
{
  return send (buf.c_str (), buf.size () * sizeof (T), msgf);
}

/*!
  Send data to a peer.
  \param  sa      peer address
  \param  buf     data to send
  \param  msgf    flags (MSG_OOB or MSG_DONTROUTE)

  The function returns the number of characters actually sent or EOF if
  connection was closed by peer.
*/
template <typename T>
size_t sock::sendto (const sockaddr& sa, std::basic_string<T> buf, mflags msgf) const
{
  return sendto (sa, buf.c_str (), buf.size () * sizeof (T), msgf);
}

/*!
  Set send timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
inline int sock::sendtimeout (int wp_sec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int oldwtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&oldwtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&wp_sec, optlen);
  return oldwtmo / 1000;
}

///  Returns the send timeout value
inline int sock::sendtimeout () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int oldwtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&oldwtmo, &optlen);
  return oldwtmo / 1000;
}

/*!
  Set receive timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
inline int sock::recvtimeout (int wp_sec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int oldrtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&oldrtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&wp_sec, optlen);
  return oldrtmo / 1000;
}

///  Returns the send timeout value
inline int sock::recvtimeout () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int oldrtmo;
  int optlen = sizeof (int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&oldrtmo, &optlen);
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
inline int sock::getopt (int op, void* buf, int len, int level) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  int rlen = len;
  if (::getsockopt (sl->handle, level, op, (char*)buf, &rlen) == SOCKET_ERROR)
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
inline erc sock::setopt (int op, void* buf, int len, int level) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  if (::setsockopt (sl->handle, level, op, (char*)buf, len) == SOCKET_ERROR)
    return last_error ();

  return erc::success;
}

/*!
  Return socket type (SOCK_DGRAM or SOCK_STREAM)
*/
inline int sock::gettype () const
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
inline int sock::clearerror () const
{
  int err = 0;
  getopt (SO_ERROR, &err, sizeof (err));
  return err;
}

/// Return the debug flag.
inline bool sock::debug () const
{
  BOOL old;
  getopt (SO_DEBUG, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the debug flag
inline void sock::debug (bool b) const
{
  BOOL opt = b;
  setopt (SO_DEBUG, &opt, sizeof (opt));
}

/// Return the "reuse address" flag
inline bool sock::reuseaddr () const
{
  BOOL old;
  getopt (SO_REUSEADDR, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the "reuse address" flag
inline void sock::reuseaddr (bool b) const
{
  BOOL opt = b;
  setopt (SO_REUSEADDR, &opt, sizeof (opt));
}

/// Return "keep alive" flag
inline bool sock::keepalive () const
{
  BOOL old;
  getopt (SO_KEEPALIVE, &old, sizeof (old));
  return (old != FALSE);
}

/// Set "keep alive" flag
inline void sock::keepalive (bool b) const
{
  BOOL opt = b;
  setopt (SO_KEEPALIVE, &opt, sizeof (opt));
}

/// Return status of "don't route" flag
inline bool sock::dontroute () const
{
  BOOL old;
  getopt (SO_DONTROUTE, &old, sizeof (old));
  return (old != FALSE);
}

/// Turn on or off the "don't route" flag
inline void sock::dontroute (bool b) const
{
  BOOL opt = b;
  setopt (SO_DONTROUTE, &opt, sizeof (opt));
}

/// Return "broadcast" option
inline bool sock::broadcast () const
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
inline void sock::broadcast (bool b) const
{
  BOOL opt = b;
  setopt (SO_BROADCAST, &opt, sizeof (opt));
}

/*!
  Return the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
inline bool sock::oobinline () const
{
  BOOL old;
  getopt (SO_OOBINLINE, &old, sizeof (old));
  return (old != FALSE);
}

/*!
  Set the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
inline void sock::oobinline (bool b) const
{
  BOOL opt = b;
  setopt (SO_OOBINLINE, &opt, sizeof (opt));
}

/// Return buffer size for send operations.
inline int sock::sendbufsz () const
{
  int old = 0;
  getopt (SO_SNDBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for send operations.
inline void sock::sendbufsz (size_t sz) const
{
  setopt (SO_SNDBUF, &sz, sizeof (sz));
}

/// Return buffer size for receive operations
inline int sock::recvbufsz () const
{
  int old = 0;
  getopt (SO_RCVBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for receive operations
inline void sock::recvbufsz (size_t sz) const
{
  setopt (SO_RCVBUF, &sz, sizeof (sz));
}

/*!
  Change blocking mode.
  sock objects are created by default as blocking sockets. They can be turned
  into non-blocking sockets using this function.
*/
inline void sock::blocking (bool on_off)
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, erc::error, sock::errors);

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
inline erc sock::setevent (HANDLE evt, long mask) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, erc::error, sock::errors);

  if (WSAEventSelect (sl->handle, (WSAEVENT)evt, mask) == SOCKET_ERROR)
    return last_error ();
  return erc::success;
}

/*!
  Indicates which of the FD_XXX network events have occurred.

  The function reports only network activity and errors for which setevent()
  has been called.
*/
inline long sock::enumevents () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, erc::error, sock::errors);

  WSANETWORKEVENTS netev;
  if (WSAEnumNetworkEvents (sl->handle, NULL, &netev) == SOCKET_ERROR)
    last_error ().raise ();
  return netev.lNetworkEvents;
}

/// Turn on or off linger mode and lingering timeout.
inline void sock::linger (bool on_off, unsigned short seconds) const
{
  struct linger opt;
  opt.l_onoff = on_off;
  opt.l_linger = seconds;
  setopt (SO_LINGER, &opt, sizeof (opt));
}

/// Return linger mode and lingering timeout.
inline bool sock::linger (unsigned short* seconds) const
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
  int code = WSAGetLastError ();
  if (!code)
    return erc::success;
  else
    return erc (code, erc::error, sock::errors);
}

/*!
   Equality comparison operator

   Returns `true` if both objects point to the same underlining socket control
   structure
*/
inline bool sock::operator== (const sock& other) const
{
  return (sl == other.sl);
}

/*!
   Inequality comparison operator

   Returns `false` if both objects have the same handle
*/
inline bool sock::operator!= (const sock& other) const
{
  return !operator== (other);
}

/// Bitwise OR operator for send message flags
inline sock::mflags operator| (sock::mflags f1, sock::mflags f2)
{
  return (sock::mflags) ((int)f1 | (int)f2);
}

} // namespace mlib
