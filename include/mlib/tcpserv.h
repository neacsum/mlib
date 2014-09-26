#pragma once
/*!
  \file TCPSERV.H Definition of tcpserv class

  (c) Mircea Neacsu 2003
*/

#include "wsockstream.h"
#include "inaddr.h"
#include "thread.h"
#include "event.h"
#include "critsect.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

///Connections iteration function
typedef void (*conn_iter_func)(sock& conn, void *param);

class tcpserver : public virtual sock, public thread
{
public:
  tcpserver (unsigned int max_conn=0, DWORD idle_time = INFINITE);
  ~tcpserver ();
  
  void foreach (conn_iter_func f, void *param);
  thread* get_connection_thread (sock& connection);
  void close_connection (sock& connection);
  void terminate ();

  ///Return number of active connections
  unsigned int numconn () const {return count;};

  void maxconn (unsigned int new_max);

  ///Return maxiumum number of conenctions accepted
  unsigned int maxconn () const {return limit;};

protected:
  bool init ();
  void run ();

  virtual bool idle_action ();
  virtual void initconn (sock& socket, thread* thread);
  virtual void termconn (sock& socket, thread* thread);
  virtual thread* make_thread(sock& connection);

private:
  /// Entry in connections table
  struct conndata
  {
    sock socket;
    thread *thread;
    bool condemned;
  } **contab;

  criticalsection contab_lock;
  unsigned int count;
  unsigned int limit;
  size_t alloc;
  event evt;
  bool end_req;
  DWORD idle;
};

/*!
  Return a servicing thread for each connection.

  If the connection doesn't need a servicing thread (single-threaded TCP server)
  return NULL.

  The return thread should not be started. It will be started from the initconn
  function.
*/
inline
thread* tcpserver::make_thread (sock& connection)
{
  return NULL;
};

/*!
  Called periodically from run loop.

  If it returns false the run loop terminates and all connections are
  closed.
*/
inline
bool tcpserver::idle_action () 
{ 
  return true;
};

#ifdef MLIBSPACE
};
#endif
