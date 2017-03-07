/*!
  \file wsockstream.cpp Implementation of sock and sock-derived classes

 Copyright (c) 2001 Mircea Neacsu


*/

//comment this line if you want debug messages from this module
#undef _TRACE

#include <mlib/defs.h>
#include <mlib/wsockstream.h>
#include <mlib/inaddr.h>
#include <mlib/dprintf.h>
#include <mlib/trace.h>

//default buffer size
#if !defined(BUFSIZ)
# define BUFSIZ 1024
#endif

#pragma comment (lib, "ws2_32.lib")

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

int sock_initializer_counter=0;

/*!
  \defgroup sockets Networking Objects
*/


///The initialization object
static sock_initializer init;

/// Default error handler
sock_facility error_handler;

/// Pointer to error handler
errfacility *sockerrors=&error_handler;


#define WSALASTERROR (errc( WSAGetLastError(), ERROR_PRI_ERROR, sockerrors))

/*!
  \class sock
  \ingroup sockets

  We keep a life counter associated with each sock object because sockets
  cannot be safely duplicated (it seems) and anyway would be more expensive 
  to call DuplicateHandle.
*/

/*!
  Create a sock object from a socket handle.
*/
sock::sock (SOCKET soc) :
  sl (new sock_life)
{
  sl->life = 1;
  sl->handle = soc;
  TRACE ("sock::sock (SOCKET %x)", sl->handle);
}


/*!
  Construct a sock object for the specified domain.
  \param  type      socket type (SOCK_DGRAM or SOCK_STREAM)
  \param  domain    normally AF_INET for TCP/IP sockets
  \param  proto     protocol

  Throws an exception if the socket cannot be created.
*/
sock::sock (int type, int domain, int proto) : 
  sl (new sock_life)
{
  sl->life = 1;
  if ((sl->handle = ::socket (domain, type, proto)) == INVALID_SOCKET) 
  {
    delete sl;
    WSALASTERROR;
  }
  TRACE ("sock::sock (type %d domain %d proto %d)=%x", type, domain, proto, sl->handle);
}

/*!
  Copy constructor.

  Increments the life counter associated with this socket
*/
sock::sock (const sock& sb)
{
  sl = sb.sl;
  sl->life++;
  TRACE ("sock::sock (sock %x) has %d lives", sl->handle,sl->life);

}

/*!
  Assignment operator
*/
sock& sock::operator = (const sock& rhs)
{
  if (this == &rhs)
    return *this;       //trivial assignment
    
  if (--sl->life == 0)
  {
    if (sl->handle != INVALID_SOCKET)
    {
      TRACE ("sock::operator= -- closesocket(%x)", sl->handle);
      closesocket (sl->handle);
    }
    delete sl;
    TRACE ("sock::operator= deleting old sl");
  }
  sl = rhs.sl;
  sl->life++;
  TRACE ("sock::operator= -- handle=%x has %d lives", sl->handle, sl->life);
  return *this;
}

/*!
  Destructor.

  If the life counter is 0 calls closesocket to free the Winsock handle.
*/
sock::~sock ()
{
  if (--sl->life == 0)
  {
    if (sl->handle != INVALID_SOCKET)
    {
      TRACE ("sock::~sock -- closesocket(%x)", sl->handle);
      closesocket (sl->handle);
    }
    delete sl;
    TRACE ("sock::~sock deleting sl");
  }
  else
    TRACE ("sock::~sock -- handle=%x has %d lives)", sl->handle, sl->life);
}

/*!
  Open the socket.
  
  If the socket was previously opened it calls first closesocket.
*/
errc sock::open (int type, int domain, int proto)
{
  TRACE ("sock::open (type=%d, domain=%d, proto=%d)", type, domain, proto);
  if (sl->handle != INVALID_SOCKET)
    closesocket (sl->handle);

  if ((sl->handle = ::socket (domain, type, proto)) == INVALID_SOCKET)
    return WSALASTERROR;
  TRACE ("sock::open handle=%x", sl->handle);
  return ERR_SUCCESS;
}


/*!
  Close socket.
*/
void sock::close()
{
  if (sl->handle != INVALID_SOCKET)
  {
    TRACE ("sock::close (%x)", sl->handle);
    closesocket (sl->handle);
    sl->handle = INVALID_SOCKET;
  }
}

/*!
  Return the local name for the object.

  This function is especially useful when a connect() call has been made without
  doing a bind first or after calling bind() without an explicit address. 
  name function provides the only way to determine the local association 
  that has been set by the system.
*/
sockaddr sock::name () const
{
  sockaddr sa;
  int len = sizeof(sa);
  if (getsockname (sl->handle, &sa, &len) == SOCKET_ERROR)
    WSALASTERROR;
  return sa;
}

/*!
  Retrieves the name of the peer to which the socket is connected. 
  The socket must be connected.
*/
sockaddr sock::peer () const
{
  sockaddr sa;
  int len = sizeof(sa);
  if (getpeername (sl->handle, &sa, &len) == SOCKET_ERROR)
    WSALASTERROR;
  return sa;
}

/*!
  Associates a local address with the socket.
*/
errc sock::bind (const sockaddr& sa)
{
  TRACE ("sock::bind (%x) to %x:%d", 
    sl->handle, ntohl(((sockaddr_in&)sa).sin_addr.S_un.S_addr), 
    ntohs(((sockaddr_in&)sa).sin_port));
  if (::bind (sl->handle, &sa, sizeof(sa)) == SOCKET_ERROR)
    return WSALASTERROR;
  return ERR_SUCCESS;
}

/*!
  Associates a local address with the socket.
  
  Winsock assigns a unique port to the socket. The application can use 
  name() after calling bind to learn the address and the port that has been
  assigned to it.
*/
errc sock::bind()
{
  sockaddr sa;
  memset (&sa, 0, sizeof(sa));
  sa.sa_family = AF_INET;

  if (::bind (sl->handle, &sa, sizeof(sa)) == SOCKET_ERROR)
    return WSALASTERROR;
  return ERR_SUCCESS;
}

/*!
  Establishes a connection to a specified address

  If a timeout is specified, the socket switches to non-blocking mode and the 
  the function waits the specified interval for the connection to be established.
  If it is not established the function returns WSAETIMEDOUT.
*/
errc sock::connect (const sockaddr& sa, int wp_sec)
{
  if (wp_sec != INFINITE)
  {
    blocking (false);
    if (!::connect (sl->handle, &sa, sizeof (sa)))
      return ERR_SUCCESS;
    DWORD ret = WSAGetLastError ();
    if (WSAGetLastError () != WSAEWOULDBLOCK)
      return WSALASTERROR;
    if (is_writeready (wp_sec))
      return ERR_SUCCESS;
    return erc (WSAETIMEDOUT, ERROR_PRI_INFO);
  }
  if (::connect(sl->handle, &sa, sizeof(sa)) == SOCKET_ERROR)
    return WSALASTERROR;
  return ERR_SUCCESS;
}

/*!
  Places the socket in a state in which it is listening for
  incoming connections.
*/
errc sock::listen (int num)
{
  if (::listen (sl->handle, num) == SOCKET_ERROR)
    return WSALASTERROR;
  return ERR_SUCCESS;
}

/*!
  Permits an incoming connection attempt on the socket.
  \param  sa  address of the connecting peer.

  The connection is actually made with the socket that is returned by accept.
*/
sock sock::accept (sockaddr& sa)
{
  SOCKET soc;
  int len = sizeof(sa);
  if ((soc = ::accept (sl->handle, &sa, &len)) == INVALID_SOCKET)
    WSALASTERROR;
  return soc;
}

/*!
  Permits an incoming connection attempt on the socket.
  The connection is actually made with the socket that is returned by accept.
*/
sock sock::accept ()
{
  SOCKET soc;
  if ((soc = ::accept (sl->handle, 0, 0)) == INVALID_SOCKET)
    WSALASTERROR;
  return soc;
}

/*!
  Receives data from socket.
  \param  buf   buffer for received data
  \param  len   buffer length
  \param  msgf  message flags

  \return Number of characters received or EOF if connection closed by peer.

*/
int sock::recv (void* buf, int len, int msgf)
{
  int rval = ::recv (sl->handle, (char*) buf, len, msgf);
  if (rval == SOCKET_ERROR)
  {
    int what =  WSAGetLastError();
    if (what == WSAEWOULDBLOCK)
    {
      TRACE ("sock::recv - EWOULDBLOCK");
      return 0;
    }
    else if (what == WSAECONNABORTED || what == WSAECONNRESET || what == WSAETIMEDOUT)
      return EOF;
    WSALASTERROR;
  }
  return (rval==0) ? EOF: rval;
}

/*!
  Receives data from socket.
  \param  sa    peer address
  \param  buf   buffer for received data
  \param  len   buffer length
  \param  msgf  message flags

  \return Number of characters received or EOF if connection closed by peer.

*/
int sock::recvfrom (sockaddr& sa, void* buf, int len, int msgf)
{
  int rval;
  int sa_len = sizeof(sa);
  
  if ((rval = ::recvfrom (sl->handle, (char*) buf, len, msgf, &sa, &sa_len)) == SOCKET_ERROR)
  {
    int what =  WSAGetLastError();
    if (what == WSAEWOULDBLOCK)
    {
      TRACE ("sock::recv - EWOULDBLOCK");
      return 0;
    }
    else if (what == WSAECONNABORTED || what == WSAECONNRESET || what == WSAETIMEDOUT)
      return EOF;
    WSALASTERROR;
  }
  return (rval==0) ? EOF: rval;
}

/*!
  Send data to the connected peer.
  \param  buf     data to send
  \param  len     length of buffer
  \param  msgf    flags (MSG_OOB or MSG_DONTROUTE)

  The function returns the number of characters actually sent or EOF if 
  connection was closed by peer.
*/
int sock::send (const void* buf, int len, int msgf)
{
  int wlen=0;
  const char *cp = (const char *)buf;
  while (len>0) 
  {
    int wval;
    if ((wval = ::send (sl->handle, cp, len, msgf)) == SOCKET_ERROR)
    {
      int what =  WSAGetLastError();
      if (what == WSAETIMEDOUT)
        return 0;
      else if (what == WSAECONNABORTED || what == WSAECONNRESET)
        return EOF;
      WSALASTERROR;
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
int sock::sendto (const sockaddr& sa, const void* buf, int len, int msgf)
{
  int wlen=0;
  const char *cp = (const char *)buf;
  while (len>0) 
  {
    int wval;
    if ((wval = ::sendto (sl->handle, cp, len, msgf, &sa, sizeof(sa))) == SOCKET_ERROR) 
    {
      int what =  WSAGetLastError();
      if (what == WSAETIMEDOUT)
        return 0;
      else if (what == WSAECONNABORTED || what == WSAECONNRESET)
        return EOF;
      WSALASTERROR;
    }
    len -= wval;
    wlen += wval;
    cp += wval;
  }
  return wlen;
}

/*!
  Set send timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
int sock::sendtimeout (int wp_sec)
{
  int oldwtmo;
  int optlen = sizeof(int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&oldwtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&wp_sec, optlen);
  return oldwtmo/1000;
}

/*!
  Returns the send timeout value
*/
int sock::sendtimeout () const
{
  int oldwtmo;
  int optlen = sizeof(int);
  getsockopt (sl->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&oldwtmo, &optlen);
  return oldwtmo/1000;
}

/*!
  Set receive timeout value
  \param  wp_sec    timeout value in seconds
  \retval Previous timeout value in seconds
*/
int sock::recvtimeout (int wp_sec)
{
  int oldrtmo;
  int optlen = sizeof(int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&oldrtmo, &optlen);
  wp_sec *= 1000;
  setsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&wp_sec, optlen);
  return oldrtmo/1000;
}

/*!
  Returns the send timeout value
*/
int sock::recvtimeout () const
{
  int oldrtmo;
  int optlen = sizeof(int);
  getsockopt (sl->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&oldrtmo, &optlen);
  return oldrtmo/1000;
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
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (sl->handle, &fds);
  
  timeval tv = { wp_sec, wp_usec };
  
  int ret = select (FD_SETSIZE, &fds, 0, 0, (wp_sec < 0) ? 0: &tv);
  if (ret == SOCKET_ERROR) 
  {
    WSALASTERROR;
    return false;
  }
  return (ret != 0);
}

/*!
  Check if socket is "writable".

  If wp_sec or wp_usec are not 0, the function waits the specified time for the
  socket to become "writeable".
  
  If the socket is processing a connect call, the socket is writeable if the 
  connection establishment successfully completes. If the socket is not 
  processing a connect call, writability means a send or sendto 
  are guaranteed to succeed.
*/
bool sock::is_writeready (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (sl->handle, &fds);
  timeval tv = { wp_sec, wp_usec };
  
  int ret = select (FD_SETSIZE, 0, &fds, 0, (wp_sec < 0) ? 0: &tv);
  if (ret == SOCKET_ERROR) 
  {
    WSALASTERROR;
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
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET  (sl->handle, &fds);
  timeval tv = { wp_sec, wp_usec};
  
  int ret = select (FD_SETSIZE, 0, 0, &fds, (wp_sec < 0) ? 0: &tv);
  if (ret == SOCKET_ERROR) 
  {
    WSALASTERROR;
    return false;
  }
  return (ret != 0);
}

/*!
  Return number of characters waiting in socket's buffer.
*/
unsigned int sock::nread ()
{
  unsigned long sz;
  if (ioctlsocket (sl->handle, FIONREAD, &sz) == SOCKET_ERROR)
    WSALASTERROR;
  return sz;
}

/*!
  Disables sends or receives on socket.

  \param  sh   describes what types of operation will no longer be allowed
*/
errc sock::shutdown (shuthow sh)
{
  if (::shutdown(sl->handle, sh) == SOCKET_ERROR)
    return WSALASTERROR;
	
  return ERR_SUCCESS;		
}

/*!
  Returns a socket option.
  \param  op    option to return
  \param  buf   buffer for option value
  \param  len   size of buffer
  \param  level level at which the option is defined

  \return Size of returned option
*/
int sock::getopt (int op, void* buf, int len, int level) const
{
  int	rlen = len;
  if (::getsockopt (sl->handle, level, op, (char*) buf, &rlen) == SOCKET_ERROR)
    WSALASTERROR;
  return rlen;
}

/*!
  Set a socket option.
  \param  op    option to return
  \param  buf   buffer for option value
  \param  len   size of buffer
  \param  level level at which the option is defined
*/
void sock::setopt (int op, void* buf, int len, int level) const
{
  if (::setsockopt (sl->handle, level, op, (char*) buf, len) == SOCKET_ERROR)
    WSALASTERROR;
}

/*!
  Return socket type (SOCK_DGRAM or SOCK_STREAM)
*/
int sock::gettype () const
{
  int ty=0;
  getopt (SO_TYPE, &ty, sizeof (ty));
  return ty;
}

/*!
  Clear the socket error flag.
  \return Error flag status

  The per socket–based error code is different from the per thread error
  code that is handled using the WSAGetLastError function call. 
  A successful call using the socket does not reset the socket based error code
  returned by this function.
*/
int sock::clearerror () const
{
  int err=0;
  getopt (SO_ERROR, &err, sizeof (err));
  return err;
}

/// Return the debug flag.
bool sock::debug () const
{
  BOOL old;
  getopt (SO_DEBUG, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the debug flag
void sock::debug (bool b)
{
  BOOL opt = b;
  setopt (SO_DEBUG, &opt, sizeof (opt));
}

/// Return the "reuse address" flag
bool sock::reuseaddr () const
{
  BOOL old;
  getopt (SO_REUSEADDR, &old, sizeof (old));
  return (old != FALSE);
}

/// Set the "reuse address" flag
void sock::reuseaddr (bool b)
{
  BOOL opt = b;
  setopt (SO_REUSEADDR, &opt, sizeof (opt));
}

/// Return "keep alive" flag
bool sock::keepalive () const
{
  BOOL old;
  getopt (SO_KEEPALIVE, &old, sizeof (old));
  return (old != FALSE);
}

/// Set "keep alive" flag
void sock::keepalive (bool b)
{
  BOOL opt = b;
  setopt (SO_KEEPALIVE, &opt, sizeof (opt));
}

/// Return "don't route" flag
bool sock::dontroute () const
{
  BOOL old;
  getopt (SO_DONTROUTE, &old, sizeof (old));
  return (old != FALSE);
}

/// Set "don't route" flag
void sock::dontroute (bool b)
{
  BOOL opt = b;
  setopt (SO_DONTROUTE, &opt, sizeof (opt));
}

/// Return "broadcast" option
bool sock::broadcast () const
{
  BOOL old;
  getopt (SO_BROADCAST, &old, sizeof (old));
  return (old != FALSE);
}

/*! 
  Set the "broadcast" option
  
  \note Socket semantics require that an application set this option 
  before attempting to send a datagram to broadcast address.
*/
void sock::broadcast (bool b)
{
  BOOL opt = b;
  setopt (SO_BROADCAST, &opt, sizeof (opt));
}

/*!
  Return the status of the OOB_INLINE flag.
  If set, OOB data is being received in the normal data stream.
*/
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
void sock::oobinline (bool b)
{
  BOOL opt = b;
  setopt (SO_OOBINLINE, &opt, sizeof (opt));
}

/// Return buffer size for send operations.
int sock::sendbufsz () const
{
  int old=0;
  getopt (SO_SNDBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for send operations.
void sock::sendbufsz (size_t sz)
{
  setopt (SO_SNDBUF, &sz, sizeof (sz));
}

/// Return buffer size for receive operations
int sock::recvbufsz () const
{
  int old=0;
  getopt (SO_RCVBUF, &old, sizeof (old));
  return old;
}

/// Set buffer size for receive operations
void sock::recvbufsz (size_t sz)
{
  setopt (SO_RCVBUF, &sz, sizeof (sz));
}

/*!
  Change blocking mode.
  sock objects are created by default as blocking sockets. They can be turned
  into non-blocking sockets using this function.
*/
void sock::blocking (bool on_off)
{
  unsigned long mode = on_off?0:1;
  if (ioctlsocket (sl->handle, FIONBIO, &mode) == SOCKET_ERROR)
    WSALASTERROR;
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
*/
errc sock::setevent (HANDLE evt, long mask)
{
  if (WSAEventSelect (sl->handle, (WSAEVENT)evt, mask) == SOCKET_ERROR)
    return WSALASTERROR;
  return ERR_SUCCESS;
}

/*!
  Indicates which of the FD_XXX network events have occurred.
  enumevents reports only network activity and errors for which setevent has 
  been called.
*/
long sock::enumevents ()
{
  WSANETWORKEVENTS netev;
  if (WSAEnumNetworkEvents (sl->handle, NULL, &netev) == SOCKET_ERROR)
    WSALASTERROR;
  return netev.lNetworkEvents;
}

/*!
  Set turn on or off linger mode and lingering timeout.
*/
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
bool sock::linger (unsigned short* seconds)
{
  struct linger opt;
  getopt (SO_LINGER, &opt, sizeof (opt));
  if (seconds)
    *seconds = opt.l_linger;
  return (opt.l_onoff == 0);
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
  The unflushed chars are pbase() - pptr()
  epptr() points to the end of the write buffer.
  
  Output is flushed whenever one of the following conditions
  holds:
  (1) pptr() == epptr()
  (2) EOF is written
 
 Unbuffered:
  Input buffer size is assumed to be of size 1 and output
  buffer is of size 0. That is, egptr() <= base()+1 and
  epptr() == pbase().
 */


/*!
  Build a sockbuf object from an existing socket
*/
sockbuf::sockbuf (SOCKET s) :
  sock (s),
  x_flags (0)
{
  setbuf (0, BUFSIZ);
}

/*!
  Build a sockbuf object and the attached socket with the given parameters
*/
sockbuf::sockbuf (int type, int domain, int proto) : 
  sock (type, domain, proto),
  x_flags (0)
{
  setbuf (0, BUFSIZ);
}

/*!
  Build a sockbuf object from a sock base
*/
sockbuf::sockbuf (const sock& s) :
  sock (s),
  x_flags (0)
{
  setbuf (0, BUFSIZ);
}

/*!
  Copy ctor
*/
sockbuf::sockbuf (const sockbuf& sb) : 
  sock (sb),
  x_flags (sb.x_flags)
{
  if (x_flags & _S_ALLOCBUF)
    setbuf (0, (int)(const_cast<sockbuf&>(sb).epptr()-const_cast<sockbuf&>(sb).pbase()));
}

/*!
  Assignment operator.

  Maintains current buffering mode
*/
sockbuf& sockbuf::operator = (const sockbuf& rhs)
{
  sock::operator =(rhs);
  x_flags = (x_flags & _S_ALLOCBUF) | (rhs.x_flags & ~_S_ALLOCBUF);
  return *this;
}

/*!
  Destructor
*/
sockbuf::~sockbuf ()
{
  if (handle() != INVALID_SOCKET)
    overflow ();

  if (x_flags & _S_ALLOCBUF)
  {
    delete [] pbase();
    delete [] eback();
  }
}


/*!
  Return 0 if all chars flushed or -1 if error
*/
int sockbuf::sync() 
{
  if (pptr () <= pbase ()) 
    return 0;                                     //unbuffered or empty buffer
  if (!(x_flags & _S_NO_WRITES)) 
  {
    int wlen   = (int)(pptr () - pbase ());
    int wval   = send (pbase (), wlen);
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
std::streambuf* sockbuf::setbuf( char *buf, int sz )
{
  overflow(EOF);                                  //flush current output buffer
  
  //switch to unbuffered mode
  if (x_flags & _S_ALLOCBUF)
  {
    delete []eback();
    delete []pbase();
  }
  setp (NULL, NULL);
  setg (NULL, NULL, NULL);
  x_flags &= ~_S_ALLOCBUF;
  ibsize = 0;
  if (!buf)
  {
    //automatic buffer - allocate separate input and output buffers
    x_flags |= _S_ALLOCBUF;
    char *ptr = new char[sz+1];
    setp (ptr, ptr+sz);
    ptr = new char[sz];
    setg( ptr, ptr+sz, ptr+sz );
    ibsize = sz;
  }
  else
    setp( buf, buf+sz-1 );                        //user specified buffer used only for output
  return this;
}

/*!
  underflow
*/
int sockbuf::underflow ()
{
  if (x_flags & _S_NO_READS) 
    return EOF;                                   //reads blocked for this socket
  
  if (gptr () < egptr ()) 
    return *(unsigned char*)gptr ();              //return available char from buffer
  
  if (eback())
  {
    //buffered mode
    int rval = recv (eback (), ibsize);
    if (rval != EOF) 
    {
      setg (eback(), eback(), eback() + rval);
      return rval?*(unsigned char*)gptr ():EOF;
    }
    return EOF;
  }
  else
  {
    //unbuffered mode
    unsigned char ch;
    int rval = recv (&ch, 1);
    if (rval == EOF || rval == 0)
      return EOF;
    return ch;
  }
}

/*!
  If c == EOF or buffer full , return sync();
  otherwise insert c into the buffer and return c
*/
int sockbuf::overflow (int c)
{
  if (x_flags & _S_NO_WRITES) 
    return EOF;                                   //socket blocked for writing, can't do anything

  if (sync () == EOF)     //flush output
    return EOF;

  if (c == EOF)
    return EOF;

  if (pbase ())
  {
    //buffered mode
    *pptr () = (char) c;
    pbump (1);
  }
  else
  {
    //unbuffered mode
    unsigned char ch = (unsigned char)c;
    int rval = send (&ch, 1);
    if (rval == EOF || rval == 0)
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
  of the static object sock_nifty_counter. Because it is a static object there
  will be one such object in each translation unit that includes the header file.
  Moreover, because it is declared before any other global object in that translation
  unit, sock_nifty_counter constructor will be called first. 

*/

/// First instance calls WSAStartup to initialize Winsock library
sock_initializer::sock_initializer ()
{
  TRACE ("sock_initializer::sock_initializer cnt=%d", sock_initializer_counter);
  if (sock_initializer_counter++ == 0)
  {
    WSADATA wsadata; 
    int err = WSAStartup (MAKEWORD(2,0), &wsadata);
    TRACE ("sock_initializer - WSAStartup result = %d", err);
  }
}

/// Last instance calls WSACleanup
sock_initializer::~sock_initializer()
{
  if (--sock_initializer_counter == 0)
  {
    int err = WSACleanup ();
    TRACE ("sock_initializer - WSACleanup result=%d", err);
  }
  TRACE ("sock_initializer::~sock_initializer cnt=%d", sock_initializer_counter);
}

/*!
  \class sock_facility
  \ingroup sockets

  Converts from error number to error name. 
*/
/// Constructor
sock_facility::sock_facility () : errfacility ("SOCKSTREAM ERROR") 
{
  TRACE ("sock_facility::sock_facility");
}

//Create an entry in the error table
#define ENTRY(A) {A, #A}

/*!
  Return the error message text.
*/
const char* sock_facility::msg (const errc& e)
{
  static struct errtab {
    int code;
    char *str;
  } errors[] = {

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
  for (i=0; errors[i].code && errors[i].code != ec; i++)
    ;
  return errors[i].str;
}

/*!
  Output either the message text or message number if no text.
*/
void sock_facility::log (const errc& e)
{
  const char *str = msg (e);
  if (str)
    dprintf ("%s  - %s(%d)", name(), str, e.code());
  else
    dprintf ("%s - %d", name(), e.code());
}

#ifdef MLIBSPACE
};
#endif

