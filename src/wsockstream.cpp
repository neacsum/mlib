/*
  MLIB Library
  (c) Mircea Neacsu 2002-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file wsockstream.cpp Implementation of sock and sock-derived classes

// comment this line if you want debug messages from this module
#undef _TRACE

#include <mlib/mlib.h>
#pragma hdrstop
#pragma comment(lib, "ws2_32.lib")

namespace mlib {

/// Router for socket errors
/*!
  \class sock_facility
  \ingroup sockets

  Converts from error number to error name.
*/
class sock_facility : public errfac
{
public:
  sock_facility ()
    : errfac ("SOCKSTREAM ERROR"){};
  void log (const erc& e) const override;
  std::string message (const erc& e) const override;
};

static sock_facility default_sockstream_errors;

static errfac* errors = &default_sockstream_errors;

errfac& sock::Errors ()
{
  return *errors;
}

void sock::Errors (errfac& facility)
{
  errors = &facility;
}

int sock_initializer_counter = 0;

/*!
  \defgroup sockets Networking Objects
*/

/// The initialization object
static sock_initializer init;

/*!
  \class sock
  \ingroup sockets

  We keep a reference counter associated with each sock object because it is
  more expensive to duplicate a socket handle using WSADuplicateHandle
*/

/*!
  Create a sock object from a socket handle.
*/
sock::sock (SOCKET soc)
  : sl (nullptr)
{
  if (soc != INVALID_SOCKET)
  {
    sl = new sock_ref;
    sl->handle = soc;
  }
  TRACE9 ("sock::sock (SOCKET %x)", sl->handle);
}

/*!
  Construct a sock object for the specified domain.
  \param  type      socket type (SOCK_DGRAM or SOCK_STREAM)
  \param  domain    normally AF_INET for TCP/IP sockets
  \param  proto     protocol

  Throws an exception if the socket cannot be created.
*/
sock::sock (int type, int domain, int proto)
  : sl (nullptr)
{
  SOCKET h;
  if ((h = ::socket (domain, type, proto)) == INVALID_SOCKET)
    last_error ().raise ();

  sl = new sock_ref;
  sl->handle = h;
  TRACE9 ("sock::sock (type %d domain %d proto %d)=%x", type, domain, proto, sl->handle);
}

/*!
  Copy constructor

  The new object shares the handle with the original. This constructor only
  increments the reference counter associated with the socket handle.
*/
sock::sock (const sock& sb)
{
  sl = sb.sl;
  if (sl)
  {
    sl->ref_count++;
    TRACE9 ("sock::sock (sock %x) has %d lives", sl->handle, sl->ref_count);
  }
}

/*!
    Move constructor

    The new object takes over the handle of the original.
*/
sock::sock (sock&& sb)
{
  sl = sb.sl;
  sb.sl = nullptr;
}

/*!
  Assignment operator

  The left hand object shares the handle of the right hand object.
*/
sock& sock::operator= (const sock& rhs)
{
  if (this == &rhs)
    return *this; // trivial assignment

  close (); // close previous handle

  if ((sl = rhs.sl) != nullptr)
  {
    sl->ref_count++;
    TRACE9 ("sock::operator= -- handle=%x has %d lives", sl->handle, sl->ref_count);
  }
  return *this;
}

/// Move assignment
sock& sock::operator= (sock&& rhs)
{
  if (this == &rhs)
    return *this; // trivial assignment

  close (); // close previous handle

  sl = rhs.sl;
  rhs.sl = nullptr;
  return *this;
}

/*!
  Destructor.

  If the 'ref_count' counter is 0, calls closesocket() to free the Winsock handle.
*/
sock::~sock ()
{
  if (sl && --sl->ref_count == 0)
  {
    if (sl->handle != INVALID_SOCKET)
    {
      TRACE8 ("sock::~sock -- closesocket(%x)", sl->handle);
      closesocket (sl->handle);
    }
    delete sl;
    TRACE9 ("sock::~sock deleting sl");
  }
}

/*!
  Open the socket.

  If the socket was previously opened it calls first close().
*/
erc sock::open (int type, int domain, int proto)
{
  TRACE8 ("sock::open (type=%d, domain=%d, proto=%d)", type, domain, proto);
  if (sl && sl->handle != INVALID_SOCKET)
  {
    if (--sl->ref_count == 0)
      closesocket (sl->handle); // no other references to this handle
    else
      sl = nullptr;
  }
  if (!sl)
    sl = new sock_ref;
  if ((sl->handle = ::socket (domain, type, proto)) == INVALID_SOCKET)
    return last_error ();
  TRACE8 ("sock::open handle=%x", sl->handle);
  return erc::success;
}

/*!
  Close socket.
*/
erc sock::close ()
{
  if (sl && sl->handle != INVALID_SOCKET)
  {
    TRACE8 ("sock::close (%x)", sl->handle);
    if (sl->ref_count == 1)
    {
      // no other references to this handle
      if (closesocket (sl->handle) == SOCKET_ERROR)
        return last_error ();

      delete sl;
    }
    else
      --sl->ref_count;
    sl = nullptr;
  }
  return erc::success;
}

/*!
  Return the local name for the object.

  This function is especially useful when a connect() call has been made without
  doing a bind first or after calling bind() without an explicit address.
  name function provides the only way to determine the local association
  that has been set by the system.
*/
checked<inaddr> sock::name () const
{
  inaddr local;
  if (!sl || sl->handle == INVALID_SOCKET)
    return {local, erc (WSAENOTSOCK, Errors ())};

  int len = sizeof (sockaddr);
  getsockname (sl->handle, local, &len);
  return {local, last_error ()};
}

/*!
  Retrieves the name of the peer to which the socket is connected.
  The socket must be connected.
*/
checked<inaddr> sock::peer () const
{
  inaddr remote;
  if (!sl || sl->handle == INVALID_SOCKET)
    return {remote, erc (WSAENOTSOCK, Errors ())};

  int len = sizeof (sockaddr);
  getpeername (sl->handle, remote, &len);
  return {remote, last_error ()};
}

/*!
  Associates a local address with the socket.
*/
erc sock::bind (const inaddr& sa) const
{
  assert (sl && sl->handle != INVALID_SOCKET);

  TRACE8 ("sock::bind (%x) to %s:%d", sl->handle, sa.ntoa (), sa.port ());
  if (::bind (sl->handle, sa, sizeof (sa)) == SOCKET_ERROR)
    return last_error ();
  return erc::success;
}

/*!
  Associates a local address with the socket.

  Winsock assigns a unique port to the socket. The application can use
  name() function to learn the address and the port that has been assigned
  to it.
*/
erc sock::bind () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  sockaddr sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_family = AF_INET;

  if (::bind (sl->handle, &sa, sizeof (sa)) == SOCKET_ERROR)
    return last_error ();
  return erc::success;
}

/*!
  Establishes a connection to a specified address
  \param  peer   address of peer
  \param  wp_sec  timeout interval (in seconds)

  Waits the specified interval for a connection to be established.
  If it is not established the function returns WSAETIMEDOUT.
*/
erc sock::connect (const inaddr& peer, int wp_sec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  if (is_writeready (wp_sec))
  {
    ::connect (sl->handle, peer, sizeof (peer));
    return last_error ();
  }
  return erc (WSAETIMEDOUT, Errors(), erc::info);
}

/*!
  Permits an incoming connection attempt on the socket.

  \param  client  socket for communication with connected peer.
  \param  addr    optional pointer to address of the connecting peer.
  \param  wp_sec  timeout interval (in seconds)

  The function waits the specified interval for a connecting peer.
  If no connection request is made the function returns WSAETIMEDOUT.
*/
erc sock::accept (sock& client, int wp_sec, inaddr* addr) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  sockaddr sa;
  int len = sizeof (sa);
  if (is_readready (wp_sec))
    client = sock (::accept (sl->handle, &sa, &len));
  else
  {
    client = sock ();
    return erc (WSAETIMEDOUT, Errors(), erc::info);
  }
  if (addr)
    *addr = sa;
  return last_error ();
}

/*!
  Receives data from socket.
  \param  buf   buffer for received data
  \param  len   buffer length
  \param  msgf  message flags

  \return Number of characters received or EOF if connection closed by peer.

*/
size_t sock::recv (void* buf, size_t len, mflags msgf) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  int rval = ::recv (sl->handle, (char*)buf, (int)len, msgf);
  if (rval == SOCKET_ERROR)
  {
    int what = WSAGetLastError ();
    if (what == WSAEWOULDBLOCK)
    {
      TRACE8 ("sock::recv - EWOULDBLOCK");
      return 0;
    }
    else if (what == WSAECONNABORTED || what == WSAECONNRESET || what == WSAETIMEDOUT
             || what == WSAESHUTDOWN)
      return EOF;
    last_error ().raise ();
  }
  return (rval == 0) ? EOF : rval;
}

/*!
  Receives data from socket.
  \param  sa    peer address
  \param  buf   buffer for received data
  \param  len   buffer length
  \param  msgf  message flags

  \return Number of characters received or EOF if connection closed by peer.

*/
size_t sock::recvfrom (sockaddr& sa, void* buf, size_t len, mflags msgf) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  int rval;
  int sa_len = sizeof (sa);

  if ((rval = ::recvfrom (sl->handle, (char*)buf, (int)len, msgf, &sa, &sa_len)) == SOCKET_ERROR)
  {
    int what = WSAGetLastError ();
    if (what == WSAEWOULDBLOCK)
    {
      TRACE8 ("sock::recv - EWOULDBLOCK");
      return 0;
    }
    else if (what == WSAECONNABORTED || what == WSAECONNRESET || what == WSAETIMEDOUT)
      return EOF;
    last_error ().raise ();
  }
  return (rval == 0) ? (size_t)EOF : rval;
}

/*!
  Send data to the connected peer.
  \param  buf     data to send
  \param  len     length of buffer.
  \param  msgf    flags (MSG_OOB or MSG_DONTROUTE)

  The function returns the number of characters actually sent or EOF if
  connection was closed by peer.
*/
size_t sock::send (const void* buf, size_t len, mflags msgf) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  size_t wlen = 0;
  const char* cp = (const char*)buf;
  while (len > 0)
  {
    int wval;
    if ((wval = ::send (sl->handle, cp, (int)len, msgf)) == SOCKET_ERROR)
    {
      int what = WSAGetLastError ();
      if (what == WSAETIMEDOUT)
        return 0;
      else if (what == WSAECONNABORTED || what == WSAECONNRESET)
        return EOF;
      last_error ().raise ();
    }
    len -= wval;
    wlen += wval;
    cp += wval;
  }
  return wlen;
}

/*!
  Send data to a peer.
  \param  sa      peer address
  \param  buf     data to send
  \param  len     length of buffer
  \param  msgf    flags (MSG_OOB or MSG_DONTROUTE)

  The function returns the number of characters actually sent or EOF if
  connection was closed by peer.
*/
size_t sock::sendto (const sockaddr& sa, const void* buf, size_t len, mflags msgf) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    throw erc (WSAENOTSOCK, Errors());

  int wlen = 0;
  const char* cp = (const char*)buf;
  while (len > 0)
  {
    int wval;
    if ((wval = ::sendto (sl->handle, cp, (int)len, msgf, &sa, sizeof (sa))) == SOCKET_ERROR)
    {
      int what = WSAGetLastError ();
      if (what == WSAETIMEDOUT)
        return 0;
      else if (what == WSAECONNABORTED || what == WSAECONNRESET)
        return EOF;
      last_error ().raise ();
    }
    len -= wval;
    wlen += wval;
    cp += wval;
  }
  return wlen;
}

/*!
  Check if socket is "readable".

  If wp_sec or wp_usec are not 0, the function waits the specified time for the
  socket to become "readable".

  If the socket is currently in the listen state, it will be marked as readable
  if an incoming connection request has been received such that accept()
  is guaranteed to complete without blocking. For other sockets, readability
  means that queued data is available for reading such that a call to recv() or
  recvfrom() is guaranteed not to block.
*/
bool sock::is_readready (int wp_sec, int wp_usec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return false;

  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (sl->handle, &fds);

  timeval tv = {wp_sec, wp_usec};

  int ret = ::select (FD_SETSIZE, &fds, 0, 0, (wp_sec < 0) ? 0 : &tv);
  if (ret == SOCKET_ERROR)
  {
    last_error ().raise ();
    return false;
  }
  return (ret != 0);
}

/*!
  Check if socket is "writable".

  If wp_sec or wp_usec are not 0, the function waits the specified time for the
  socket to become "writable".

  If the socket is processing a connect call, the socket is writable if the
  connection establishment successfully completes. If the socket is not
  processing a connect call, being writable means a send() or sendto()
  are guaranteed to succeed.
*/
bool sock::is_writeready (int wp_sec, int wp_usec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return false;

  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (sl->handle, &fds);
  timeval tv = {wp_sec, wp_usec};

  int ret = ::select (FD_SETSIZE, 0, &fds, 0, (wp_sec < 0) ? 0 : &tv);
  if (ret == SOCKET_ERROR)
  {
    last_error ().raise ();
    return false;
  }
  return (ret != 0);
}

/*!
  Check if socket has OOB data or any exceptional error conditions.

  If wp_sec or wp_usec are not 0, the function waits the specified time.
*/
bool sock::is_exceptionpending (int wp_sec, int wp_usec) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return false;

  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (sl->handle, &fds);
  timeval tv = {wp_sec, wp_usec};

  int ret = ::select (FD_SETSIZE, 0, 0, &fds, (wp_sec < 0) ? 0 : &tv);
  if (ret == SOCKET_ERROR)
  {
    last_error ().raise ();
    return false;
  }
  return (ret != 0);
}

/*!
  Return number of characters waiting in socket's buffer.
*/
unsigned int sock::nread () const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return 0;

  unsigned long sz;
  if (ioctlsocket (sl->handle, FIONREAD, &sz) == SOCKET_ERROR)
    last_error ().raise ();
  return sz;
}

/*!
  Disables sends or receives on socket.

  \param  sh   describes what types of operation will no longer be allowed
*/
erc sock::shutdown (shuthow sh) const
{
  if (!sl || sl->handle == INVALID_SOCKET)
    return erc (WSAENOTSOCK, Errors());

  if (::shutdown (sl->handle, sh) == SOCKET_ERROR)
    return last_error ();

  return erc::success;
}

//---------------------------- sockbuf class --------------------------------//
/*!
  \class sockbuf
  \ingroup sockets

  You can simultaneously read and write into a sockbuf just like you can
  listen and talk through a telephone. Hence, the read and the write buffers
  are different.

  Read:
  eback() points to the start of the get area.
  The unread chars are gptr() to egptr().

  eback() is set to base() so that pbackfail()
  is called only when there is no place to
  putback a char. And pbackfail() always returns EOF.

  Write:
  pbase() points to the start of the put area
  The unflushed chars are `pbase() - pptr()`
  epptr() points to the end of the write buffer.

  Output is flushed whenever one of the following conditions
  holds:
  (1) `pptr() == epptr()`
  (2) EOF is written

 Unbuffered:
  Input buffer size is assumed to be of size 1 and output buffer is of size 0.
  That is, `egptr() <= base()+1` and `epptr() == pbase()`.
*/

///  Build a sockbuf object from an existing socket
sockbuf::sockbuf (SOCKET s)
  : sock (s)
  , x_flags (0)
{
  setbuf (0, SOCKBUF_BUFSIZ);
}

///  Build a sockbuf object and the attached socket with the given parameters
sockbuf::sockbuf (int type, int domain, int proto)
  : sock (type, domain, proto)
  , x_flags (0)
{
  setbuf (0, SOCKBUF_BUFSIZ);
}

///  Build a sockbuf object from a sock base
sockbuf::sockbuf (const sock& s)
  : sock (s)
  , x_flags (0)
{
  setbuf (0, SOCKBUF_BUFSIZ);
}

///  Copy constructor
sockbuf::sockbuf (const sockbuf& sb)
  : sock (sb)
  , x_flags (sb.x_flags)
{
  if (x_flags & flags::allocbuf)
    setbuf (0, (int)(const_cast<sockbuf&> (sb).epptr () - const_cast<sockbuf&> (sb).pbase ()));
}

/*!
  Assignment operator.

  Maintains current buffering mode
*/
sockbuf& sockbuf::operator= (const sockbuf& rhs)
{
  sock::operator= (rhs);
  std::streambuf::operator= (rhs);
  x_flags = (x_flags & flags::allocbuf) | (rhs.x_flags & ~flags::allocbuf);
  return *this;
}

///  Destructor
sockbuf::~sockbuf ()
{
  if (is_open ())
  {
    overflow ();
    shutdown (shut_readwrite).deactivate (); // ignore any errors
    close ().deactivate ();                  // ignore errors
  }

  if (x_flags & flags::allocbuf)
  {
    delete[] pbase ();
    delete[] eback ();
  }
}

/*!
  Return 0 if all chars flushed or -1 if error
*/
int sockbuf::sync ()
{
  if (pptr () <= pbase ())
    return 0; // unbuffered or empty buffer
  if (!(x_flags & flags::no_writes))
  {
    size_t wlen = pptr () - pbase ();
    size_t wval = send (pbase (), wlen);
    setp (pbase (), epptr ());
    if (wval == wlen)
      return 0;
    TRACE ("sockbuf::sync failed - wanted %d sent %d", wlen, wval);
  }
  return -1;
}

/*!
  Change buffering mode.

  If \p buf is NULL, switches to automatic buffering mode with separate buffers
  for input and output.
*/
std::streambuf* sockbuf::setbuf (char* buf, std::streamsize sz)
{
  overflow (EOF); // flush current output buffer

  // switch to unbuffered mode
  if (x_flags & flags::allocbuf)
  {
    delete[] eback ();
    delete[] pbase ();
  }
  setp (NULL, NULL);
  setg (NULL, NULL, NULL);
  x_flags &= ~flags::allocbuf;
  ibsize = 0;
  if (!buf)
  {
    // automatic buffer - allocate separate input and output buffers
    x_flags |= flags::allocbuf;
    char* ptr = new char[(size_t)sz + 1];
    setp (ptr, ptr + sz);
    ptr = new char[(size_t)sz];
    setg (ptr, ptr + sz, ptr + sz);
    ibsize = (int)sz;
  }
  else
    setp (buf, buf + sz - 1); // user specified buffer used only for output
  return this;
}

/*!
  underflow
*/
int sockbuf::underflow ()
{
  if (x_flags & flags::no_reads)
    return EOF; // reads blocked for this socket

  if (gptr () < egptr ())
    return *(unsigned char*)gptr (); // return available char from buffer

  if (eback ())
  {
    // buffered mode
    size_t rval = recv (eback (), ibsize);
    if (rval != (size_t)EOF)
    {
      setg (eback (), eback (), eback () + rval);
      return rval ? *(unsigned char*)gptr () : EOF;
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
  If c == EOF or buffer full, return sync();
  otherwise insert c into the buffer and return c
*/
int sockbuf::overflow (int c)
{
  if (x_flags & flags::no_writes)
    return EOF; // socket blocked for writing, can't do anything

  if (sync () == EOF) // flush output
    return EOF;

  if (c == EOF)
    return EOF;

  if (pbase ())
  {
    // buffered mode
    *pptr () = (char)c;
    pbump (1);
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

/*!
  Return number of characters available in socket buffer
*/
std::streamsize sockbuf::showmanyc ()
{
  return nread ();
}

/*!
  \struct sock_initializer
  \ingroup sockets

  This is a wrapper around WSAStartup/WSACleanup functions.
  When the first instance is created it calls WSAStartup and WSACleanup is
  called when the last instance is destroyed.

  It uses the "Nifty Counter" (https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter)
  idiom to ensure Winsock library is properly
  initialized before usage. The header file wsockstream.h contains a declaration
  of the static object `sock_nifty_counter`. Because it is a static object there
  will be one such object in each translation unit that includes the header file.
  Moreover, because it is declared before any other global object in that translation
  unit, sock_nifty_counter constructor will be called first.

*/

/// First instance calls WSAStartup to initialize Winsock library
sock_initializer::sock_initializer ()
{
  TRACE9 ("sock_initializer::sock_initializer cnt=%d", sock_initializer_counter);
  if (sock_initializer_counter++ == 0)
  {
    WSADATA wsadata;
    int err = WSAStartup (MAKEWORD (2, 0), &wsadata);
    TRACE8 ("sock_initializer - WSAStartup result = %d", err);

    // Set default error handling facility
    errors = &default_sockstream_errors;
  }
}

/// Last instance calls WSACleanup
sock_initializer::~sock_initializer ()
{
  if (--sock_initializer_counter == 0)
  {
    int err = WSACleanup ();
    TRACE8 ("sock_initializer - WSACleanup result=%d", err);
  }
  TRACE9 ("sock_initializer::~sock_initializer cnt=%d", sock_initializer_counter);
}

// Create an entry in the error table
#define ENTRY(A)                                                                                   \
  {                                                                                                \
    A, #A                                                                                          \
  }

/*!
  Return the error message text.

  Values from: https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
*/
std::string sock_facility::message (const erc& e) const
{
  static struct errtab
  {
    int code;
    const char* str;
  } errors[] = {
    ENTRY (WSA_INVALID_HANDLE),
    ENTRY (WSA_NOT_ENOUGH_MEMORY),
    ENTRY (WSA_INVALID_PARAMETER),
    ENTRY (WSA_OPERATION_ABORTED),
    ENTRY (WSA_IO_INCOMPLETE),
    ENTRY (WSA_IO_PENDING),
    ENTRY (WSAEINTR),
    ENTRY (WSAEBADF),
    ENTRY (WSAEACCES),
    ENTRY (WSAEFAULT),
    ENTRY (WSAEINVAL),
    ENTRY (WSAEMFILE),
    ENTRY (WSAEWOULDBLOCK),
    ENTRY (WSAEINPROGRESS),
    ENTRY (WSAEALREADY),
    ENTRY (WSAENOTSOCK),
    ENTRY (WSAEDESTADDRREQ),
    ENTRY (WSAEMSGSIZE),
    ENTRY (WSAEPROTOTYPE),
    ENTRY (WSAENOPROTOOPT),
    ENTRY (WSAEPROTONOSUPPORT),
    ENTRY (WSAESOCKTNOSUPPORT),
    ENTRY (WSAEOPNOTSUPP),
    ENTRY (WSAEPFNOSUPPORT),
    ENTRY (WSAEAFNOSUPPORT),
    ENTRY (WSAEADDRINUSE),
    ENTRY (WSAEADDRNOTAVAIL),
    ENTRY (WSAENETDOWN),
    ENTRY (WSAENETUNREACH),
    ENTRY (WSAENETRESET),
    ENTRY (WSAECONNABORTED),
    ENTRY (WSAECONNRESET),
    ENTRY (WSAENOBUFS),
    ENTRY (WSAEISCONN),
    ENTRY (WSAENOTCONN),
    ENTRY (WSAESHUTDOWN),
    ENTRY (WSAETOOMANYREFS),
    ENTRY (WSAETIMEDOUT),
    ENTRY (WSAECONNREFUSED),
    ENTRY (WSAELOOP),
    ENTRY (WSAENAMETOOLONG),
    ENTRY (WSAEHOSTDOWN),
    ENTRY (WSAEHOSTUNREACH),
    ENTRY (WSAENOTEMPTY),
    ENTRY (WSAEPROCLIM),
    ENTRY (WSAEUSERS),
    ENTRY (WSAEDQUOT),
    ENTRY (WSAESTALE),
    ENTRY (WSAEREMOTE),
    ENTRY (WSASYSNOTREADY),
    ENTRY (WSAVERNOTSUPPORTED),
    ENTRY (WSANOTINITIALISED),
    ENTRY (WSAEDISCON),
    ENTRY (WSAENOMORE),
    ENTRY (WSAECANCELLED),
    ENTRY (WSAEINVALIDPROCTABLE),
    ENTRY (WSAEINVALIDPROVIDER),
    ENTRY (WSAEPROVIDERFAILEDINIT),
    ENTRY (WSASYSCALLFAILURE),
    ENTRY (WSASERVICE_NOT_FOUND),
    ENTRY (WSATYPE_NOT_FOUND),
    ENTRY (WSA_E_NO_MORE),
    ENTRY (WSA_E_CANCELLED),
    ENTRY (WSAEREFUSED),
    ENTRY (WSAHOST_NOT_FOUND),
    ENTRY (WSATRY_AGAIN),
    ENTRY (WSANO_RECOVERY),
    ENTRY (WSANO_DATA),
    //  ENTRY (WSA_QOS_RECEIVERS),
    //  ENTRY (WSA_QOS_SENDERS),
    //  ENTRY (WSA_QOS_NO_SENDERS),
    //  ENTRY (WSA_QOS_NO_RECEIVERS),
    //  ENTRY (WSA_QOS_REQUEST_CONFIRMED),
    //  ENTRY (WSA_QOS_ADMISSION_FAILURE),
    //  ENTRY (WSA_QOS_POLICY_FAILURE),
    //  ENTRY (WSA_QOS_BAD_STYLE),
    //  ENTRY (WSA_QOS_BAD_OBJECT),
    //  ENTRY (WSA_QOS_TRAFFIC_CTRL_ERROR),
    //  ENTRY (WSA_QOS_GENERIC_ERROR),
    {0, 0},
  };

  int ec = e.code ();
  int i;
  for (i = 0; errors[i].code && errors[i].code != ec; i++)
    ;
  return errors[i].str;
}

/*!
  Output either the message text or message number if no text.
*/
void sock_facility::log (const erc& e) const
{
  auto str = message (e);
  if (!str.empty ())
    dprintf ("%s  - %s(%d)", name ().c_str (), str.c_str (), e.code ());
  else
    dprintf ("%s - %d", name ().c_str (), e.code ());
}

} // namespace mlib
