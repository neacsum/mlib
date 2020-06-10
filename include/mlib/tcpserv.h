/*!
  \file tcpserv.h Definition of tcpserv class

  (c) Mircea Neacsu 2003
*/
#pragma once

#include "wsockstream.h"
#include "inaddr.h"
#include "thread.h"
#include "event.h"
#include "critsect.h"

namespace mlib {

///Connections iteration function
typedef void (*conn_iter_func)(sock& conn, void *param);

class tcpserver : public sock, public thread
{
public:
  tcpserver (unsigned int max_conn=0, DWORD idle_time = INFINITE, const char *name = 0);
  ~tcpserver ();
  
  void foreach (conn_iter_func f, void *param);
  thread* get_connection_thread (sock& connection);
  void close_connection (sock& connection);
  void terminate ();

  ///Return number of active connections
  unsigned int numconn () const {return count;};

  void maxconn (unsigned int new_max);

  ///Return maximum number of connections accepted
  unsigned int maxconn () const {return limit;};

  void set_connfunc (std::function<int (sock&)>f);

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
  unsigned int count;     //number of active connections
  unsigned int limit;     //max number of active connections
  size_t alloc;           //number of allocated entries in conntab
  event evt;              //main event
  bool end_req;           //true when server must exit
  DWORD idle;
  std::function<int (sock& s)> connfunc;

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

}
