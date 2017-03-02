/*!
  \file TCPSERV.CPP Implementation of tcpserver class

  (c) Mircea Neacsu 2003


*/

//comment this line if you want debug messages from this module
#undef _TRACE

#include <mlib/defs.h>
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
tcpserver::tcpserver (unsigned int max_conn, DWORD idle_timeout) : 
limit (max_conn),
count (0),
alloc (ALLOC_INCR),
contab (new conndata* [ALLOC_INCR]),
end_req (false),
idle (idle_timeout)
{
  memset (contab, 0, alloc*sizeof (conndata*));
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
      TRACE ("tcpserver::run - idle timeout");
      end_req = !idle_action ();
    }
    TRACE ("tcpserver::run - loop again end_req = %d", end_req);    
    if (end_req)        //bail out?
      continue;

    if (is_readready(0))
    {
      /// - check if there is space in connections table
      if (limit && count >= limit)
      {
        TRACE ("Max number of connections (%d) reached", limit);
        accept().close ();
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
        TRACE ("tcpserver::run - increased connection table size to %d", alloc);
      }

      inaddr claddr;
      contab[i] = new conndata;
      contab[i]->socket = accept (claddr);

      /* clear inherited attributes (nonblocking mode and event mask)*/
      contab[i]->socket.setevent (0, 0);
      contab[i]->socket.blocking (true);
      
      TRACE ("tcpserver::run contab[%d] - request from %s:%d",i, claddr.ntoa(), claddr.port());

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
          TRACE ("tcpserver::run - terminating condemned connection %d", i);
          termconn (contab[i]->socket, contab[i]->thread);
          delete contab[i];
          contab[i] = NULL;
          count--;
          evt.signal ();
          break;
        }
       contab_lock.leave ();
    }
    TRACE ("tcpserver::run %d connections active", count);
  }
}

/*!
  Delete connection from connections table
*/
void tcpserver::close_connection (sock& s)
{
  size_t i;
  for (i=0; i<alloc; i++)
    if ((contab[i] != NULL) && (s == contab[i]->socket))
    {
#ifdef _TRACE
    if (contab[i]->socket.is_open ())
    {
      inaddr peer = contab[i]->socket.peer ();
      TRACE ("tcpserver::close_connection closing contab[%d] to %s:%d",
        i, peer.ntoa (), peer.port ());
    }
    else
      TRACE ("tcpserver::close_connection closing contab[%d] - socket closed", i);
#endif
      contab[i]->condemned = true;
      evt.signal ();
      break;
    }
  if (i == alloc)
    TRACE ("tcpserver::close_connection cannot find contab entry");
}

/*!
  Initializes a connection.

  If the connection has associated a servicing thread 
  (returned by make_thread), this thread is started now.
*/
void tcpserver::initconn (sock& /*socket*/, thread* th)
{
  TRACE ("tcpserver::initconn");
  if (th)
    th->start ();
}

/*!
  Finalizes a connection.

  Wait for the servicing thread to terminate (if there is one) than close
  the socket
*/
void tcpserver::termconn (sock& socket, thread *th)
{
  TRACE ("tcpserver::termconn");
  if (th)
  {
    th->wait (10);
    delete th;
  }
  try {
    if (socket.handle() != INVALID_SOCKET)
    {
      //set linger option to avoid socket hanging around after close
      socket.linger (true, 1);
      socket.shutdown (sock::shut_write);

      //read and discard any pending data
      char buf [256];
      while (socket.recv (buf, sizeof(buf)) != EOF)
        ;
      socket.close ();
    }
  } catch (erc& x) {
    x.deactivate ();
    TRACE ("tcpserver::termconn caught %d", x);
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
  TRACE ("tcpserver::terminate");
  close ();
  end_req = true;
  if (is_running())
  {
    TRACE ("tcpserver::terminate - stopping running thread");
    evt.signal ();
    if (current_thread().id () == id ()) 
      TRACE ("WARNING - terminate called from own thread");
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
void tcpserver::maxconn(unsigned int new_max)
{
  limit = new_max;
}

} //namespace
