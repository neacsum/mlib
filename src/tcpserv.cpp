/*!
  \file tcpserv.cpp Implementation of tcpserver class

  (c) Mircea Neacsu 2003


*/

#include <mlib/tcpserv.h>
#include <mlib/trace.h>

///Allocation increment for connections table
#define ALLOC_INCR 5

namespace mlib {

/*!
  \class    tcpserver 
  \brief    multi-threaded TCP server. 
  \ingroup  sockets
  
  When started it listens on a socket and accepts new connections. 
  Typical use is to create a derived class that overrides #initconn 
  and #termconn functions to provide appropriate actions when a client 
  connects and disconnects.

  Being a thread itself, the tcpserver object has to be started by calling
  the start() function
*/


/*!
  Opens the socket and initializes the connections table
*/
tcpserver::tcpserver (unsigned int max_conn, DWORD idle_timeout, const std::string& name) 
  : sock (SOCK_STREAM)
  , thread (name)
  , limit (max_conn)
  , count (0)
  , alloc (ALLOC_INCR)
  , contab (new conndata* [ALLOC_INCR])
  , end_req (false)
  , idle (idle_timeout)
  , connfunc (nullptr)
{
  memset (contab, 0, alloc * sizeof (conndata*));
}

/*!
  Terminate any connection that still exists
*/
tcpserver::~tcpserver ()
{
  for (size_t i=0; i<alloc; i++)
  {
    if (contab[i])
      termconn (contab[i]->socket, contab[i]->thread);
  
    delete contab[i];
    contab[i] = 0;
  }
  delete []contab;
}

/*!
  Place socket in listen mode.
  
  Also initializes an event flag to be signaled when a connection request is 
  received. This function is automatically invoked by start ()
*/
bool tcpserver::init ()
{
  if (!is_open ())
    open (SOCK_STREAM); 
  setevent (evt.handle (), FD_ACCEPT);
  listen ();
  TRACE2 ("TCP server %s started", thread::name ().c_str ());
  return thread::init ();
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

    if (is_readready(0))
    {
      /// - check if there is space in connections table
      if (limit && count >= limit)
      {
        sock s = accept ();
        TRACE8 ("TCP server %s - rejected connection from %s",
          thread::name ().c_str (), s.peer ().ntoa ());
        TRACE8 ("Max number of connections (%d) reached", limit);
        s.close ();
        continue;
      }
      contab_lock.enter ();
      /// - find an empty slot in connections table
      size_t i;
      for (i=0; i<alloc && (contab[i] != NULL); i++)
          ;
      if (i == alloc)
      {
        //reallocate connections table
        conndata **new_table = new conndata*[alloc+ALLOC_INCR];
        memcpy (new_table, contab, alloc*sizeof(conndata*));
        memset (new_table+alloc, 0, ALLOC_INCR*sizeof(conndata*));
        delete []contab;
        contab = new_table;
        alloc += ALLOC_INCR;
        TRACE9 ("tcpserver::run - increased connection table size to %d", alloc);
      }

      inaddr peer;
      contab[i] = new conndata;
      contab[i]->socket = accept (peer);

      /* clear inherited attributes (nonblocking mode and event mask)*/
      contab[i]->socket.setevent (0, 0);
      contab[i]->socket.blocking (true);
      
      TRACE9 ("tcpserver::run contab[%d] - request from %s:%d",i, peer.ntoa(), peer.port());

      /// - invoke make_thread to get a servicing thread
      contab[i]->thread = make_thread (contab[i]->socket);
      contab[i]->condemned = false;
      count++;
      /// - invoke initconn function
      initconn (contab[i]->socket, contab[i]->thread);
      contab_lock.leave ();
    }
    else
    {
      //check if we've been signaled by close_connection
      contab_lock.enter ();
      for (size_t i=0; i<alloc; i++)
        if (contab[i] && contab[i]->condemned)
        {
          TRACE9 ("tcpserver::run - terminating condemned connection %d", i);
          termconn (contab[i]->socket, contab[i]->thread);
          delete contab[i];
          contab[i] = NULL;
          count--;
          evt.signal ();
          break;
        }
       contab_lock.leave ();
    }
    TRACE9 ("tcpserver::run %d connections active", count);
  }

  //End of run loop. Terminate all active connections
  contab_lock.enter ();
  for (size_t i = 0; i < alloc; i++)
    if (contab[i])
    {
      TRACE9 ("tcpserver::run at end - terminating connection %d", i);
      termconn (contab[i]->socket, contab[i]->thread);
      delete contab[i];
      contab[i] = NULL;
      count--;
    }
  contab_lock.leave ();
}

/*!
  Delete connection from connections table
*/
void tcpserver::close_connection (sock& s)
{
  size_t i;
  for (i = 0; i < alloc; i++)
  {
    if ((contab[i] != NULL) && (s == contab[i]->socket))
    {
#ifdef MLIB_TRACE
      if (contab[i]->socket.is_open ())
      {
        inaddr peer = contab[i]->socket.peer ();
        TRACE9 ("tcpserver::close_connection closing contab[%d] to %s:%d",
          i, peer.ntoa (), peer.port ());
      }
      else
        TRACE9 ("tcpserver::close_connection closing contab[%d] - socket closed", i);
#endif
      contab[i]->condemned = true;
      evt.signal ();
      break;
    }
  }
}

/*!
  Initializes a connection.

  If the connection has associated a servicing thread 
  (returned by make_thread), this thread is started now.
*/
void tcpserver::initconn (sock& socket, thread* th)
{
  TRACE8 ("TCP server %s - Accepted connection with %s:%d",
    thread::name ().c_str (), socket.peer().ntoa (), socket.peer().port ());
  if (th)
    th->start ();
}

/*!
  Set function or lambda expression that becomes the body of the thread
  serving a new connection.
*/
void tcpserver::set_connfunc (std::function<int (sock& conn)> f)
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
    auto f = /*std::bind (connfunc, connection);*/
      [&]()->int {
      int ret = connfunc (connection);
      close_connection (connection);
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
  if (socket.handle () != INVALID_SOCKET)
  {
    try {
      /* Throughout this sequence we might get slapped with a WSAECONNRESET error
      if the client has already closed the connection but we don't care: the
      catch clause will take care of it. */
      TRACE8 ("TCP server %s - Closed connection with %s:%d",
        thread::name ().c_str (), socket.peer ().ntoa (), socket.peer ().port ());
      /* Set linger option with a short timeout to avoid socket hanging around
        after close. */
      socket.linger (true, 1);
      socket.shutdown (sock::shut_write);

      //read and discard any pending data
      char buf[256];
      while (socket.is_readready (0) && socket.recv (buf, sizeof (buf)) != EOF)
        ;
    }
    catch (erc& x) {
      if (x != WSAECONNRESET)
        TRACE ("tcpserver::termconn caught %d", x);
    }
    socket.close ();
  }
}

/*!
  Return the thread servicing a connection
*/
thread *tcpserver::get_connection_thread(sock &connection)
{
  lock l(contab_lock);
  for (size_t i=0; i<alloc; i++)
    if (contab[i] != NULL && !contab[i]->condemned && contab[i]->socket == connection)
      return contab[i]->thread;

  return NULL;
}

/*!
  Terminate the tcp server.

*/
void tcpserver::terminate ()
{
  close ();
  end_req = true;
  TRACE2 ("TCP server %s stopped", thread::name ().c_str ());
  if (is_running())
  {
    TRACE2 ("tcpserver::terminate - stopping running thread");
    evt.signal ();
    if (current_thread().id () == id ()) 
      TRACE2 ("WARNING - terminate called from own thread");
    else
      wait ();
  }
}

/*!
  Invoke an iteration function for each active connection
*/
void tcpserver::foreach (conn_iter_func f, void *param)
{
  for (size_t i=0; i<alloc; i++)
    if (contab[i] != NULL && !contab[i]->condemned)
      f (contab[i]->socket, param);
}

/*!
  Set max number of accepted connections
*/
void tcpserver::maxconn (unsigned int new_max)
{
  limit = new_max;
}

}
