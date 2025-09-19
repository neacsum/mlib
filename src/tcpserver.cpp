/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <algorithm>

namespace mlib {

/*!
  \class    tcpserver
  \brief    multi-threaded TCP server.
  \ingroup  sockets

  A tcpserver object is a thread that, when started, listens on a socket and
  accepts new connections. Typical use is to create a derived class that
  overrides #initconn and #termconn functions to provide appropriate actions
  when a client connects and disconnects.

  By default, the server waits indefinitely until a client connects. You can change
  this behavior by calling the timeout() function to force server's run loop to
  execute periodically.

  Being a thread itself, the tcpserver object has to be started by calling
  the start() function. The listening port must be set before the server thread
  is started. It can be set either when the object is constructed or by 
  calling the `bind()` function on the listening socket.
  
  The following two examples are equivalent:

  Example 1:
  \code{.cpp}
    mlib::tcpserver srv (12345);

    //...

    srv.start ();
  \endcode

  Example 2:
  \code{.cpp}
    mlib::tcpserver srv;
    srv.socket().bind (mlib::inaddr (INADDR_ANY, 12345));

    //...

    srv.start ();
  \endcode


  When a new connection is received (`listen()` function returns a new socket)
  the server calls make_thread() to create a new thread servicing the
  connection. It then starts this new thread by calling initconn().
*/

/*!
  Opens the socket and initializes the connections table.

  \param port listening port number
  \param max_conn maximum number of accepted connections
  \param name server name used for debug messages

  If port address is 0, the listening address is the local loop-back interface.
  Otherwise, the listening address defaults to 'all interfaces' (`INADDR_ANY`).
*/
tcpserver::tcpserver (unsigned short port, unsigned int max_conn, const std::string& name)
  : srv_sock (sock::stream)
  , addr (port ? INADDR_ANY : INADDR_LOOPBACK, port)
  , thread (name)
  , limit (max_conn)
  , end_req (false)
  , idle (INFINITE)
  , connfunc (nullptr)
{}

/*!
  Terminate any connection that still exists
*/
tcpserver::~tcpserver ()
{
  for (auto& conn : contab)
    termconn (conn.socket, conn.thread);
}

/*!
  Binds the server socket to listening address and places it in listen mode.

  Also initializes an event flag to be signaled when a connection request is
  received. This function is automatically invoked by start ()
*/
bool tcpserver::init ()
{
  try
  {
    //if not opened, open it now
    if (!srv_sock.is_open ())
      srv_sock.open (sock::stream);

    //if not bound, bind it now
    if (srv_sock.name () != erc::success)
      srv_sock.bind (addr);

    srv_sock.setevent (evt.handle (), FD_ACCEPT);
    srv_sock.listen ();
    TRACE2 ("TCP server %s started", thread::name ().c_str ());
    return thread::init ();
  }
  catch (erc& x)
  {
    TRACE ("tcpserver::init failed - error %s - %d", x.message ().c_str (), int (x));
    return false;
  }
}

/*!
  Run loop.

  Waits for incoming connections. Every time a connection request is received
  the following actions take place:
*/
void tcpserver::run ()
{
  while (!end_req)
  {
    if (evt.wait (idle) == WAIT_TIMEOUT)
    {
      TRACE8 ("tcpserver::run - idle timeout");
      end_req = !idle_action ();
    }
    TRACE9 ("tcpserver::run - loop again end_req = %d", end_req);
    if (end_req) // bail out?
      continue;

    if (srv_sock.is_readready ())
    {
      /// - check if server can accept more connections
      if (limit && (contab.size () >= limit))
      {
        sock s;
        inaddr peer;
        srv_sock.accept (s, &peer);
        TRACE3 ("TCP server %s - rejected connection from %s", thread::name ().c_str (),
                peer.ntoa ());
        TRACE3 ("Max number of connections (%d) reached", limit);
        s.close ();
        continue;
      }
      contab_lock.enter ();

      inaddr peer;
      sock accepted;
      srv_sock.accept (accepted, &peer);

      /* clear inherited attributes (nonblocking mode and event mask)*/
      accepted.setevent (0, 0);
      accepted.blocking (true);

      TRACE7 ("tcpserver::run - request from %s:%d", peer.ntoa (), peer.port ());

      /// - invoke make_thread() to get a servicing thread
      auto th = make_thread (accepted);
      /// - invoke initconn() function
      initconn (accepted, th);
      contab.emplace_back (conndata{accepted, th, false});

      contab_lock.leave ();
    }
    else
    {
      // check if we've been signaled by close_connection
      contab_lock.enter ();
      auto p = std::find_if (contab.begin (), contab.end (), [] (const conndata& c) {
        return (c.condemned || !c.socket.is_open ());
      });
      if (p != contab.end ())
      {
        TRACE7 ("tcpserver::run - terminating condemned connection");
        termconn (p->socket, p->thread);
        contab.erase (p);
        evt.signal (); // loop again
      }
      contab_lock.leave ();
    }
    TRACE7 ("tcpserver::run %d connections active", contab.size ());
  }

  // End of run loop. Terminate all active connections
  contab_lock.enter ();
  while (!contab.empty ())
  {
    termconn (contab.begin ()->socket, contab.begin ()->thread);
    contab.erase (contab.begin ());
  }
  contab_lock.leave ();
}

/*!
  Closes a connection

  \param conn_sock socket associated with connection to be closed

  The connection is also removed from the connections table. In fact, the
  connection is only marked as condemned and the run loop will remove it next
  time it is run.
*/
void tcpserver::close_connection (const sock& conn_sock)
{
  if (conn_sock.is_open ())
  {
    auto p = std::find_if (contab.begin (), contab.end (),
                           [&conn_sock] (auto& c) { return c.socket == conn_sock; });
    if (p != contab.end ())
    {
      TRACE9 ("tcpserver::close_connection closing connection to %s",
              to_string (*p->socket.peer ()).c_str ());
      p->condemned = true;
    }
    conn_sock.shutdown (sock::shut_readwrite);
  }
  evt.signal ();
}

/*!
  Initializes a connection.

  \param conn_sock connection socket
  \param th thread that services the new connection

  If the connection has associated a servicing thread (\p th is not NULL),
  this thread is started now.
*/
void tcpserver::initconn (sock& conn_sock, thread* th)
{
  TRACE8 ("TCP server %s - Accepted connection with %s", thread::name ().c_str (),
          to_string (*conn_sock.peer ()).c_str ());
  if (th)
    th->start ();
}

/*!
  Set function object that becomes the body of the thread
  serving a new connection.
*/
void tcpserver::set_connfunc (std::function<int (const sock& conn)> f)
{
  connfunc = f;
}

/*!
  Return a servicing thread for each connection.

  \param conn_sock socket associated with the new connection

  If user has set a connection function (using set_connfunc() function), 
  the function returns a thread whose body is the connection function.
  Otherwise, it returns NULL and the server becomes a single-threaded server.

  \note The returned thread should not be started. It will be started from the
  initconn() function.
*/
thread* tcpserver::make_thread (sock& conn_sock)
{
  if (connfunc)
  {
    sock client (conn_sock);
    auto f = [=] () -> int {
      int ret = connfunc (client);
      close_connection (client);
      return ret;
    };
    return new thread (f);
  }
  return NULL;
}

/*!
  Finalizes a connection.

  \param conn_sock socket associated with the connection
  \param th connection servicing thread

  Wait for the servicing thread to terminate (if there is one) than close
  the socket
*/
void tcpserver::termconn (sock& conn_sock, thread* th)
{
  TRACE9 ("tcpserver::termconn");
  if (th)
  {
    th->wait (10);
    delete th;
  }
  if (conn_sock.is_open ())
  {
    try
    {
      TRACE8 ("TCP server %s - Closed connection with %s", thread::name ().c_str (),
              to_string (*conn_sock.peer ()).c_str ());
      conn_sock.linger (true, 1);
      conn_sock.shutdown (sock::shut_readwrite);

      // read and discard any pending data
      char buf[256];
      while (conn_sock.is_readready () && conn_sock.recv (buf, sizeof (buf)) != EOF)
        ;
      conn_sock.close ();
    }
    catch (erc& x)
    {
      if (x != WSAECONNRESET)
        TRACE ("tcpserver::termconn caught %d - %s", (int)x, x.message ().c_str ());
    }
  }
}

/*!
  Return the thread servicing a connection

  \param conn_sock socket associated with the connection
  \return pointer to connection servicing thread or NULL of there is no
          associated thread
*/
thread* tcpserver::get_connection_thread (const sock& conn_sock)
{
  lock l (contab_lock);
  auto p = std::find_if (contab.begin (), contab.end (), [&conn_sock] (auto& c) {
    return !c.condemned && c.socket == conn_sock;
  });

  return (p != contab.end ()) ? p->thread : nullptr;
}

/*!
  Terminate the tcp server.

*/
void tcpserver::terminate ()
{
  srv_sock.close ();
  end_req = true;
  evt.signal ();
  if (is_running ())
  {
    if (current_thread ().id () == id ())
      TRACE ("WARNING - tcpserver::terminate called from own thread - cannot self-destruct!");
    else
    {
      wait ();
      TRACE2 ("TCP server %s stopped", thread::name ().c_str ());
    }
  }
}

/*!
  Invoke an iteration function for each active connection

  \param f function to call
  \param param arbitrary value passed as second parameter to iteration function
*/
void tcpserver::foreach (conn_iter_func f, void* param)
{
  auto fp = [&] (conndata& c) {
    if (!c.condemned)
      f (c.socket, param);
  };

  std::for_each (contab.begin (), contab.end (), fp);
}

/*!
  Set max number of accepted connections
  \param n maximum number of concurrent connections
*/
void tcpserver::maxconn (unsigned int n)
{
  limit = n;
}

} // namespace mlib
