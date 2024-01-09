/*!
  \file tcpserv.cpp Implementation of tcpserver class

  (c) Mircea Neacsu 2003


*/

#include <mlib/tcpserv.h>
#include <mlib/trace.h>
#include <algorithm>

///Allocation increment for connections table
#define ALLOC_INCR 5

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
  the start() function
*/


/*!
  Opens the socket and initializes the connections table.

  If port address is 0, the listening address is the local loop-back interface.
  Otherwise, the listening address defaults to 'all interfaces' (`INADDR_ANY`).
*/
tcpserver::tcpserver (unsigned short port, const std::string &name, unsigned int max_conn) 
  : srv_sock (SOCK_STREAM)
  , addr (port?INADDR_ANY:INADDR_LOOPBACK, port)
  , thread (name)
  , limit (max_conn)
  , end_req (false)
  , idle (INFINITE)
  , connfunc (nullptr)
{
}

/*!
  Terminate any connection that still exists
*/
tcpserver::~tcpserver ()
{
  for (auto &conn : contab)
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
    if (!srv_sock.is_open ())
      srv_sock.open (SOCK_STREAM);
    inaddr me;
    if (srv_sock.name (me) != erc::success)
      srv_sock.bind (addr);
    srv_sock.setevent (evt.handle (), FD_ACCEPT);
    srv_sock.listen ();
    TRACE2 ("TCP server %s started", thread::name ().c_str ());
    return thread::init ();  
  }
  catch (erc& x)
  {
    TRACE ("tcpserver::init failed - error %s - %d", x.message ().c_str () , int (x));
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
    if (end_req)        //bail out?
      continue;

    if (srv_sock.is_readready(0))
    {
      /// - check if can accept more connections
      if (limit && (contab.size () >= limit))
      {
        sock s;
        inaddr peer;
        srv_sock.accept (s, &peer);
        TRACE3 ("TCP server %s - rejected connection from %s",
          thread::name ().c_str (), peer.ntoa ());
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
      
      TRACE7 ("tcpserver::run - request from %s:%d", peer.ntoa(), peer.port());

      /// - invoke make_thread to get a servicing thread
      auto th = make_thread (accepted);
      /// - invoke initconn function
      initconn (accepted, th);
      contab.emplace_back (conndata{accepted, th, false});

      contab_lock.leave ();
    }
    else
    {
      //check if we've been signaled by close_connection
      contab_lock.enter ();
      auto p = std::find_if (contab.begin (), contab.end (), 
        [] (const conndata &c) { return (c.condemned || !c.socket.is_open()); });
      if (p != contab.end ())
      {
        TRACE7 ("tcpserver::run - terminating condemned connection");
        termconn (p->socket, p->thread);
        contab.erase (p);
        evt.signal (); //loop again
      }
      contab_lock.leave ();
    }
    TRACE7 ("tcpserver::run %d connections active", contab.size());
  }

  //End of run loop. Terminate all active connections
  contab_lock.enter ();
  while (!contab.empty ())
  {
    termconn (contab.begin ()->socket, contab.begin ()->thread);
    contab.erase (contab.begin ());
  }
  contab_lock.leave ();
}

/*!
  Delete connection from connections table
*/
void tcpserver::close_connection (const sock& s)
{
  if (s.is_open ())
  {
    auto p = std::find_if (contab.begin (), contab.end (), 
      [&s] (auto &c) { return c.socket == s; });
    if (p != contab.end ())
    {
#ifdef MLIB_TRACE
      inaddr other;
      p->socket.peer (other);
      TRACE9 ("tcpserver::close_connection closing connection to %s:%d", other.ntoa (),
              other.port ());
#endif
      p->condemned = true;
    }
  }
  evt.signal ();
}

/*!
  Initializes a connection.

  If the connection has associated a servicing thread 
  (returned by make_thread), this thread is started now.
*/
void tcpserver::initconn (sock& socket, thread* th)
{
#ifdef MLIB_TRACE
  inaddr other;
  socket.peer (other);
  TRACE8 ("TCP server %s - Accepted connection with %s:%d",
    thread::name ().c_str (), other.ntoa (), other.port ());
#endif
  if (th)
    th->start ();
}

/*!
  Set function or lambda expression that becomes the body of the thread
  serving a new connection.
*/
void tcpserver::set_connfunc (std::function<int (const sock& conn)> f)
{
  connfunc = f;
}


/*!
  Return a servicing thread for each connection.

  If user has set a connection function (using set_connfunc() function), make_thread
  returns a thread whose body is the connection function.

  If the connection doesn't need a servicing thread (single-threaded TCP server)
  return NULL.

  The returned thread should not be started. It will be started from the initconn
  function.
*/
thread* tcpserver::make_thread (sock& connection)
{
  if (connfunc)
  {
    sock client (connection);
    auto f =
      [=] () -> int {
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

  Wait for the servicing thread to terminate (if there is one) than close
  the socket
*/
void tcpserver::termconn (sock& socket, thread *th)
{
  TRACE9 ("tcpserver::termconn");
  if (th)
  {
    th->wait (10);
    delete th;
  }
  if (socket.is_open())
  {
    try {
#ifdef MLIB_TRACE
      inaddr other;
      socket.peer (other);
      TRACE8 ("TCP server %s - Closed connection with %s:%d",
        thread::name ().c_str (), other.ntoa (), other.port ());
#endif
      socket.linger (true, 1);
      socket.shutdown (sock::shut_readwrite);

      //read and discard any pending data
      char buf[256];
      while (socket.is_readready (0) && socket.recv (buf, sizeof (buf)) != EOF)
        ;
      socket.close ();
    }
    catch (erc& x) {
      if (x != WSAECONNRESET)
        TRACE ("tcpserver::termconn caught %d - %s", (int)x, x.message().c_str());
    }
  }
}

/*!
  Return the thread servicing a connection
*/
thread *tcpserver::get_connection_thread(const sock &connection)
{
  lock l(contab_lock);
  auto p = std::find_if (contab.begin (), contab.end (),
                         [&connection] (auto &c) { return !c.condemned && c.socket == connection; });

  return (p != contab.end ())? p->thread : nullptr;
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
    if (current_thread().id () == id ()) 
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
*/
void tcpserver::foreach (conn_iter_func f, void *param)
{
  auto fp = [&] (conndata &c) {
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

}
