/*!
  \file syncque.h Definition of sync_queue and bounded_queue classes

  (c) Mircea Neacsu 1999-2014. All rights reserved.
*/
#pragma once

#include <queue>
#include "semaphore.h"
#include "critsect.h"
#include "trace.h"

namespace mlib 
{

/*!
  A template class that implements "synchronized queues".
  These are producer/consumer queues which can hold "messages" and deliver
  them in FIFO order. Attempting to consume from an empty queue will block
  the calling thread until a message arrives.

  \note sync_queue is not bounded and it can grow up to the available memory.
*/
template <class M> 
class sync_queue : protected std::queue< M, std::deque<M> >
{
public:
  sync_queue () {};
  virtual void produce (const M&);
  virtual M consume ();
  bool empty();
  size_t size ();

protected:
  semaphore con_sema;       ///< consumers' semaphore counts down until queue is empty
  criticalsection update;   ///< critical section protects queue's integrity
};

/*!
  A synchronized queue with a limited size.

  bounded_queue objects can grow only up to the set limit. If the queue is full,
  producers have to wait until space becomes available.
*/
template <class M>
class bounded_queue : public sync_queue<M>
{
public:
  bounded_queue (size_t limit);
  void produce (const M&);
  M consume ();
  bool full ();

private:
  size_t limit;
  semaphore pro_sema;   ///< producers' semaphore counts down until queue is full
};

//-------------------- sync_queue methods -------------------------------------
/// Append an element to queue
template <class M>
void sync_queue<M>::produce (const M& obj)
{
  mlib::lock l (update);  //take control of the queue
  push (obj);             //put a copy of the object at the end
  con_sema.signal ();     //and signal the semaphore
}

/// Extract and return first element in queue
template <class M> 
M sync_queue<M>::consume ()
{
  M result;
  update.enter ();
  if (empty ())
  {
    while (1)
    {
      update.leave ();
      con_sema.wait ();        //wait for a producer
      update.enter ();
      if (!empty ())
        break;
    }
  }
  result = front ();  //get the message
  pop ();
  update.leave ();
  return result;
}

/// Return _true_ if queue is empty
template <class M>
bool sync_queue<M>::empty ()
{
  lock l (update);
  return queue<M>::empty ();
}

/// Return queue size 
template <class M>
size_t sync_queue<M>::size ()
{
  lock l (update);
  return queue<M>::size ();
}

//-------------------- bounded_queue methods ------------------------------------
template <class M>
bounded_queue<M>::bounded_queue (size_t limit_)
  : limit (limit_)
{
  pro_sema.signal ((int)limit);
}

/// Append an element to queue. If queue is already full, waits until there is
/// space available.
template <class M>
void bounded_queue<M>::produce (const M& obj)
{
  update.enter ();
  while (std::queue<M>::size () > limit)
  {
    update.leave ();
    pro_sema.wait ();
    update.enter ();
  }
  push (obj);
  con_sema.signal ();
  update.leave ();
}

template <class M>
M bounded_queue<M>::consume ()
{
  M result;
  update.enter ();
  if (std::queue<M>::empty ())
  {
    while (1)
    {
      update.leave ();
      con_sema.wait ();        //wait for a producer
      update.enter ();
      if (!empty ())
        break;
    }
  }
  result = front ();  //get the message
  pop ();
  pro_sema.signal ();  //signal producers there is space available
  update.leave ();
  return result;
}

template <class M>
bool bounded_queue<M>::full ()
{
  lock l (update);
  return (std::queue<M>.size () == limit);
}

}  //end namespace
