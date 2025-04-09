/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file tcpserver.h Definition of mlib::tcpserver class

#pragma once

#include "wsockstream.h"
#include "inaddr.h"
#include "thread.h"
#include "event.h"
#include "critsect.h"

namespace mlib {

/// Connections iteration function
typedef void (*conn_iter_func) (sock& conn, void* param);

class tcpserver : public thread
{
public:
  tcpserver (unsigned short port, const std::string& name = std::string (),
             unsigned int max_conn = 0);
  ~tcpserver ();

  /// Provides access to server listening socket
  sock& socket ();

  void foreach (conn_iter_func f, void* param);
  thread* get_connection_thread (const sock& conn_sock);
  void close_connection (const sock& conn_sock);
  void terminate ();

  /// Return number of active connections
  size_t numconn () const;

  /// Return max interval to wait for an incoming connection (in milliseconds)
  unsigned int timeout () const;

  /// Set maximum timeout interval (in milliseconds)
  void timeout (DWORD msec);

  /// Set maximum number of connections accepted
  void maxconn (unsigned int new_max);

  /// Return maximum number of connections accepted
  unsigned int maxconn () const;

  void set_connfunc (std::function<int (const sock&)> f);

protected:
  bool init () override;
  void run () override;

  virtual bool idle_action ();
  virtual void initconn (sock& conn_sock, thread* thread);
  virtual void termconn (sock& conn_sock, thread* thread);
  virtual thread* make_thread (sock& conn_sock);

private:
  sock srv_sock; // listening socket
  inaddr addr;   // listening address

  /// Entry in connections table
  struct conndata
  {
    sock socket;
    thread* thread;
    bool condemned;
  };
  std::vector<conndata> contab;
  criticalsection contab_lock;
  unsigned int limit; // max number of active connections
  auto_event evt;     // main event
  bool end_req;       // true when server must exit
  DWORD idle;
  std::function<int (const sock& s)> connfunc;
};

/*==================== INLINE FUNCTIONS ===========================*/

inline
sock& tcpserver::socket ()
{
  return srv_sock;
}

inline
size_t tcpserver::numconn () const
{
  return contab.size ();
}

/*!
  \return timeout value in milliseconds

  If timeout interval is 0 or `INFINITE` the server waits indefinitely for
  incoming connections.
*/
inline
unsigned int tcpserver::timeout () const
{
  return (idle == INFINITE) ? 0 : idle;
}

/*!
  \param msec timeout interval in milliseconds.

  If timeout interval is 0 or `INFINITE` the server waits indefinitely for
  incoming connections.
*/
inline void tcpserver::timeout (DWORD msec)
{
  idle = msec ? msec : INFINITE;
  if (is_running ())
    evt.signal ();
}

inline
unsigned int tcpserver::maxconn () const
{
  return limit;
}


/*!
  Called periodically from run loop.

  If it returns false the run loop terminates and all connections are
  closed.
*/
inline bool tcpserver::idle_action ()
{
  return true;
};

} // namespace mlib
