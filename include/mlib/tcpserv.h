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

class tcpserver : public thread
{
public:
  tcpserver (unsigned  short port, const std::string &name = std::string (), unsigned int max_conn = 0);
  ~tcpserver ();
  sock &socket ()
    {return srv_sock;}
  void foreach (conn_iter_func f, void *param);
  thread* get_connection_thread (const sock& connection);
  void close_connection (const sock& connection);
  void terminate ();

  ///Return number of active connections
  size_t numconn () const {return contab.size();};

  /// Return max interval to wait for an incoming connection (in milliseconds)
  unsigned int timeout () const;

  ///Set maximum timeout interval (in milliseconds)
  void timeout (DWORD msec);

  void maxconn (unsigned int new_max);

  ///Return maximum number of connections accepted
  unsigned int maxconn () const {return limit;};

  void set_connfunc (std::function<int (const sock&)>f);

protected:
  bool init () override;
  void run () override;

  virtual bool idle_action ();
  virtual void initconn (sock& socket, thread* thread);
  virtual void termconn (sock& socket, thread* thread);
  virtual thread* make_thread(sock& connection);

private:
  sock srv_sock;  //listening socket
  inaddr addr;    //listening address

  /// Entry in connections table
  struct conndata
  {
    sock socket;
    thread *thread;
    bool condemned;
  };
  std::vector<conndata> contab;
  criticalsection contab_lock;
  unsigned int limit;     //max number of active connections
  auto_event evt;         // main event
  bool end_req;           //true when server must exit
  DWORD idle;
  std::function<int (const sock& s)> connfunc;

};

/*!
  \return timeout value in milliseconds

  If timeout interval is 0 or `INFINITE` the server waits indefinitely for
  incoming connections.
*/
inline
unsigned int mlib::tcpserver::timeout () const
{
  return (idle == INFINITE) ? 0 : idle;
}

/*!
  \param msec timeout interval in milliseconds.

  If timeout interval is 0 or `INFINITE` the server waits indefinitely for
  incoming connections.
*/
inline
void tcpserver::timeout (DWORD msec)
{
  idle = msec ? msec : INFINITE;
  if (is_running ())
    evt.signal ();
}

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
