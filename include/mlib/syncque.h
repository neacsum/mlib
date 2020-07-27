/*!
  \file syncque.h Definition of sync_queue and bounded_queue classes

  (c) Mircea Neacsu 1999-2020. All rights reserved.
*/
#pragma once

#include <queue>
#include "semaphore.h"
#include "critsect.h"

namespace mlib 
{

/*!
  A template class that implements "synchronized queues".
  These are producer/consumer queues which can hold "messages" and deliver
  them in FIFO order. Attempting to consume from an empty queue will block
  the calling thread until a message arrives.

  \note sync_queue is not bounded and it can grow up to the available memory.
*/
template <class M, class C=std::deque<M>> 
class sync_queue : protected std::queue<M, C>
{
public:
  sync_queue () {};

  /// Append an element to queue
  virtual void produce (const M& obj)
  {
    lock l (update);        //take control of the queue
    this->push (obj);       //put a copy of the object at the end
  con_sema.signal ();     //and signal the semaphore
  }

  /// Extract and return first element in queue
  virtual M consume ()
  {
  M result;
  update.enter ();
    while (std::queue<M, C>::empty ())
    {
      update.leave ();
      con_sema.wait ();        //wait for a producer
      update.enter ();
    }
    result = this->front ();  //get the message
    this->pop ();
  update.leave ();
  return result;
  }

  /// Return _true_ if queue is empty
  bool empty()
  {
  lock l (update);
    return std::queue<M, C>::empty ();
  }

  /// Return queue size 
  size_t size ()
  {
  lock l (update);
    return std::queue<M, C>::size ();
  }

protected:
  semaphore con_sema;       ///< consumers' semaphore counts down until queue is empty
  criticalsection update;   ///< critical section protects queue's integrity
};

/*!
  A synchronized queue with a limited size.

  bounded_queue objects can grow only up to the set limit. If the queue is full,
  producers have to wait until space becomes available.
*/
template< class M, class C = std::deque<M> >
class bounded_queue : public sync_queue<M, C>
{
public:
  bounded_queue (size_t limit_) : limit (limit_)
  {
  pro_sema.signal ((int)limit);
  }

  /// Append an element to queue. If queue is full, waits until space
  /// becomes available.
  void produce (const M& obj)
  {
    this->update.enter ();
    while (std::queue<M, C>::size () > limit)
    {
      this->update.leave ();
    pro_sema.wait ();
      this->update.enter ();
    }
    this->push (obj);
    this->con_sema.signal ();
    this->update.leave ();
  }

  /// Extract and return first element in queue
  M consume ()
  {
  M result;
    this->update.enter ();
    if (std::queue<M, C>::empty ())
  {
    while (1)
    {
        this->update.leave ();
        this->con_sema.wait ();        //wait for a producer
        this->update.enter ();
        if (!std::queue<M, C>::empty ())
        break;
    }
  }
    result = this->front ();  //get the message
    this->pop ();
  pro_sema.signal ();  //signal producers there is space available
    this->update.leave ();
  return result;
  }

  /// Return _true_ if queue is at capacity
  bool full ()
  {
    lock l (this->update);
    return (std::queue<M, C>.size () == limit);
  }


protected:
  size_t limit;
  semaphore pro_sema;   ///< producers' semaphore counts down until queue is full
};


}  //end namespace
