#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include "errorcode.h"
#include "inaddr.h"
#include "tvops.h"

namespace mlib {

// clang-format off
/// \{
/// \ingroup sockets
/// Encapsulation of a Windows socket
class sock
{
public:
  /// socket types
  enum type
  {
    stream = SOCK_STREAM,
    dgram = SOCK_DGRAM,
    raw = SOCK_RAW
  };

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
  explicit sock (type t, int domain = AF_INET, int proto = 0);
  sock (const sock&);
  sock (sock&&);

  virtual ~sock ();
  sock& operator= (const sock&);
  sock& operator= (sock&&);
  HANDLE       handle () const;
  bool         operator== (const sock& other) const;
  bool         operator!= (const sock& other) const;
  operator     SOCKET () const;

  virtual erc  open (type t, int domain = AF_INET, int proto = 0);
  virtual void close ();
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

  void                      sendtimeout (std::chrono::milliseconds tmo) const;
  std::chrono::milliseconds sendtimeout () const;
  void                      recvtimeout (std::chrono::milliseconds tmo) const;
  std::chrono::milliseconds recvtimeout () const;
  bool         is_readready (std::chrono::milliseconds tmo = std::chrono::milliseconds{0}) const;
  bool         is_writeready (std::chrono::milliseconds tmo = std::chrono::milliseconds{0}) const;
  bool         is_exceptionpending (std::chrono::milliseconds tmo = std::chrono::milliseconds{0}) const;
  unsigned int nread () const;

  erc          bind (const inaddr&) const;
  erc          bind () const;
  erc          connect (const inaddr& peer) const;
  erc          connect (const inaddr& peer, std::chrono::milliseconds tmo) const;
  bool         connected () const;
  erc          listen (int num = SOMAXCONN) const;
  erc          accept (sock& client, inaddr* sa = nullptr) const;
  erc          accept (sock& client, std::chrono::milliseconds tmo, inaddr* sa = nullptr) const;
  checked<inaddr>  name () const;
  checked<inaddr>  peer () const;

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
#ifdef _WIN32
  erc          setevent (HANDLE evt, long mask) const;
  long         enumevents () const;
#endif
  void         linger (bool on_off, unsigned short seconds) const;
  bool         linger (unsigned short* seconds = 0) const;
  void         nodelay (bool on_off);
  bool         nodelay () const;

  /// Return error facility used by all sock-derived classes.
  static errfac& Errors ();

  /// Set the error facility use by all sock-derived classes 
  static void Errors (errfac& facitlity);

protected:
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

/*---------------------- support classes ------------------------------------*/
/// Keeps an instance counter for calls to WSAStartup/WSACleanup
/// \ingroup sockets
struct sock_initializer
{
  sock_initializer ();
  ~sock_initializer ();
};
inline sock_initializer sock_nifty_counter;


/*==================== INLINE FUNCTIONS ===========================*/

/// Default constructor creates a closed socket
inline 
sock::sock ()
  : sl{nullptr}
{}

/// Retrieve Windows socket handle
inline 
HANDLE sock::handle () const
{
  return sl ? (HANDLE)(std::intptr_t)sl->handle : INVALID_HANDLE_VALUE;
}

/// Conversion operator 
inline
sock::operator SOCKET () const
{
  return sl ? sl->handle : INVALID_SOCKET;
}

/// Check if socket is opened
inline
bool sock::is_open () const
{
  return sl && sl->handle != INVALID_SOCKET;
}

/// Establishes a connection to specified peer
inline
erc sock::connect (const inaddr& peer) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  int ret = ::connect (sl->handle, peer, sizeof (peer));
  if (ret == INVALID_SOCKET)
    return last_error ();

  return erc::success;
}

/// Check is socket is connected
inline
bool sock::connected () const
{
  return is_writeready ();
}

/// Permits an incoming connection attempt on the socket.
inline
erc sock::accept (sock& client, inaddr* addr) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  sockaddr sa;
  socklen_t len = sizeof (sa);
  client = sock (::accept (sl->handle, &sa, &len));
  if (addr)
    *addr = sa;
  return erc ();
}

/// Places the socket in a state in which it is listening for incoming connections.
inline
erc sock::listen (int num) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

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
  \param  tmo    timeout value in milliseconds
*/
inline
void sock::sendtimeout (std::chrono::milliseconds tmo) const
{
  assert (sl && sl->handle != INVALID_SOCKET);

#ifdef _WIN32
  socklen_t optlen = sizeof (int);
  int par = (int)tmo.count();
#else
  socklen_t optlen = sizeof(timeval);
  timeval par = from_chrono (tmo);
#endif
  setsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&par, optlen);
}

///  Returns the send timeout value
inline
std::chrono::milliseconds sock::sendtimeout () const
{
  assert (sl && sl->handle != INVALID_SOCKET);

#ifdef _WIN32
  socklen_t optlen = sizeof (int);
  int tmo;
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&tmo, &optlen);
  return std::chrono::milliseconds(tmo);
#else
  socklen_t optlen = sizeof(timeval);
  timeval tv;
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, &optlen);
  return std::chrono::duration_cast<std::chrono::milliseconds>(to_chrono (tv));
#endif
}

/*!
  Set receive timeout value
  \param  tmo    timeout value in milliseconds
*/
inline
void sock::recvtimeout (std::chrono::milliseconds tmo) const
{
  assert (sl && sl->handle != INVALID_SOCKET);
#ifdef _WIN32
  socklen_t optlen = sizeof (int);
  int par = (int)tmo.count();
#else
  socklen_t optlen = sizeof(timeval);
  timeval par = from_chrono (tmo);
#endif
  setsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&par, optlen);
}

///  Returns the send timeout value
inline
std::chrono::milliseconds sock::recvtimeout () const
{
  assert (sl && sl->handle != INVALID_SOCKET);
#ifdef _WIN32
  socklen_t optlen = sizeof (int);
  int tmo;
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&tmo, &optlen);
  return std::chrono::milliseconds (tmo);
#else
  socklen_t optlen = sizeof(timeval);
  timeval tv;
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, &optlen);
  return std::chrono::duration_cast<std::chrono::milliseconds> (to_chrono(tv));
#endif
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
int sock::getopt (int op, void* buf, int len, int level) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  socklen_t rlen = len;
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
inline
erc sock::setopt (int op, void* buf, int len, int level) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  if (::setsockopt (sl->handle, level, op, (char*)buf, len) == SOCKET_ERROR)
    return last_error ();

  return erc::success;
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
  return (old != 0);
}

/// Set the debug flag
inline
void sock::debug (bool b) const
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
  return (old != 0);
}

/// Set the "reuse address" flag
inline
void sock::reuseaddr (bool b) const
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
  return (old != 0);
}

/// Set "keep alive" flag
inline
void sock::keepalive (bool b) const
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
  return (old != 0);
}

/// Turn on or off the "don't route" flag
inline
void sock::dontroute (bool b) const
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
  return (old != 0);
}

/*!
  Turn on or off the "broadcast" option

  \note Socket semantics require that an application set this option
  before attempting to send a datagram to broadcast address.
*/
inline
void sock::broadcast (bool b) const
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
  return (old != 0);
}

/*!
  Set the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
inline
void sock::oobinline (bool b) const
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
void sock::sendbufsz (size_t sz) const
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
void sock::recvbufsz (size_t sz) const
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
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  unsigned long mode = on_off ? 0 : 1;
  if (ioctlsocket (sl->handle, FIONBIO, &mode) == SOCKET_ERROR)
    last_error ().raise ();
}

#ifdef _WIN32
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
erc sock::setevent (HANDLE evt, long mask) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

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
long sock::enumevents () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  WSANETWORKEVENTS netev;
  if (WSAEnumNetworkEvents (sl->handle, NULL, &netev) == SOCKET_ERROR)
    last_error ().raise ();
  return netev.lNetworkEvents;
}
#endif

/// Turn on or off linger mode and lingering timeout.
inline
void sock::linger (bool on_off, unsigned short seconds) const
{
  struct linger opt;
  opt.l_onoff = on_off;
  opt.l_linger = seconds;
  setopt (SO_LINGER, &opt, sizeof (opt));
}

/// Return linger mode and lingering timeout.
inline
bool sock::linger (unsigned short* seconds) const
{
  struct linger opt;
  getopt (SO_LINGER, &opt, sizeof (opt));
  if (seconds)
    *seconds = opt.l_linger;
  return (opt.l_onoff == 0);
}

/// Set TCP_NODELAY option
inline
void sock::nodelay (bool on_off)
{
  int value = on_off;
  setopt (TCP_NODELAY, &value, sizeof (int), IPPROTO_TCP);
}

/// Return status of TCP_NODELAY option
inline
bool sock::nodelay () const
{
  int value;
  getopt (TCP_NODELAY, &value, sizeof (int), IPPROTO_TCP);
  return (value != 0);
}

/// Return an error code with the value returned by WSAGetLastError
inline
erc sock::last_error ()
{
#ifdef _WIN32
  int code = WSAGetLastError ();
#else
  int code = errno;
#endif
  if (!code)
    return erc::success;
  else
    return erc (code, Errors());
}

/*!
   Equality comparison operator

   Returns `true` if both objects point to the same underlining socket control
   structure
*/
inline
bool sock::operator== (const sock& other) const
{
  return (sl == other.sl);
}

/*!
   Inequality comparison operator

   Returns `false` if both objects have the same handle
*/
inline
bool sock::operator!= (const sock& other) const
{
  return !operator== (other);
}

/// Bitwise OR operator for send message flags
inline 
sock::mflags operator| (sock::mflags f1, sock::mflags f2)
{
  return (sock::mflags) ((int)f1 | (int)f2);
}

} // namespace mlib

/// Extraction operator shows socket handle
inline 
std::ostream& operator<< (std::ostream& strm, const mlib::sock& s)
{
  strm << s.handle ();
  return strm;
}

