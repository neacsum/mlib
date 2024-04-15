/*!
  \file syncque.h Definition of sync_queue and async_queue classes

  (c) Mircea Neacsu 1999-2021. All rights reserved.
*/
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <queue>
#include "semaphore.h"
#include "critsect.h"
#include "stopwatch.h"

namespace mlib {
/*!
  A template class that implements a "synchronous queue", in effect a mailbox
  that can store one "message", created by a producer thread, until consumed
  by a consumer thread.

  If the mailbox is full (a message already there), the producer is blocked
  until previous message is consumed. If the mailbox is empty, a consumer is
  blocked until a new message arrives.

  This is strictly equivalent with an async_queue with size 1 however it doesn't
  have the underlining std::queue structure and avoids all the back and forth
  copying of messages.
*/
template <class M>
class sync_queue
{
public:
  /// Create a synchronous queue
  sync_queue ()
    : message (nullptr)
  {}

  /// Put new element in queue
  void produce (const M& obj)
  {
    update.enter ();
    while (message)
    {
      update.leave ();
      prod_sema.wait ();
      update.enter ();
    }
    cons_sema.signal ();
    message = &obj;
    update.leave ();
  }

  void consume (M& obj)
  {
    update.enter ();
    while (!message)
    {
      update.leave ();
      cons_sema.wait ();
      update.enter ();
    }
    prod_sema.signal ();
    obj = *message;
    message = nullptr;
    update.leave ();
  }

private:
  criticalsection update; ///< critical section protects queue's integrity
  semaphore prod_sema;    ///< producers' semaphore counts waiting producers
  semaphore cons_sema;    ///< consumers' semaphore counts waiting consumers
  const M* message;
};

/*!
  A template class that implements "asynchronous queues".
  These are producer/consumer queues which can hold "messages" and deliver
  them in FIFO order. Attempting to consume from an empty queue will block
  the calling thread until a message arrives.

  \note The default size of async_queue is `INFINITE` and it can grow up to the
  available memory.
*/
template <class M, class C = std::deque<M>>
class async_queue : protected std::queue<M, C>
{
public:
  /// Crates a queue with the given maximum size
  async_queue (size_t limit_ = INFINITE)
    : limit (limit_)
  {
    if (limit > 0 && limit < INFINITE)
      prod_sema.signal ((int)limit);
  }

  /// Append an element to queue
  /// \return _true_ if the element was appended or _false_ if a timeout occurred.
  virtual bool produce (const M& obj, DWORD timeout = INFINITE)
  {
    if (limit < INFINITE)
    {
      // Bounded queue. See if there is enough space to produce
      stopwatch t; // need a stopwatch to count cumulated wait time
      if (timeout != INFINITE)
        t.start ();
      update.enter ();
      while (std::queue<M, C>::size () >= limit)
      {
        update.leave ();
        if (timeout != INFINITE)
        {
          int t_wait = timeout - (int)t.msecLap ();
          if (t_wait < 0 || prod_sema.wait (t_wait) == WAIT_TIMEOUT)
            return false;
        }
        else
          prod_sema.wait ();
        update.enter ();
      }
      std::queue<M, C>::push (obj);
      cons_sema.signal ();
      update.leave ();
    }
    else
    {
      // Unbounded queue. Producer will always be successful
      lock l (update);              // take control of the queue
      std::queue<M, C>::push (obj); // put a copy of the object at the end
      cons_sema.signal ();          // and signal the semaphore
    }
    return true;
  }

  /// Extract the first element in queue
  /// Return _true_ if an element was extracted or _false_ if a timeout occurred.
  virtual bool consume (M& result, int timeout = INFINITE)
  {
    if (timeout != INFINITE)
    {
      stopwatch t;
      int t_wait;
      t.start ();
      update.enter ();
      while (std::queue<M, C>::empty ())
      {
        update.leave ();
        t_wait = timeout - (int)t.msecLap ();
        // give up or wait for a producer
        if (t_wait <= 0 || cons_sema.wait (t_wait) == WAIT_TIMEOUT)
          return false;
        update.enter ();
      }
    }
    else
    {
      update.enter ();
      if (std::queue<M, C>::empty ())
      {
        while (1)
        {
          update.leave ();
          cons_sema.wait (); // wait for a producer
          update.enter ();
          if (!std::queue<M, C>::empty ())
            break;
        }
      }
    }
    result = std::queue<M, C>::front (); // get the message
    std::queue<M, C>::pop ();
    if (limit != INFINITE)
      prod_sema.signal (); // signal producers there is space available

    update.leave ();
    return true;
  }

  /// Return _true_ if queue is empty
  bool empty ()
  {
    lock l (update);
    return std::queue<M, C>::empty ();
  }

  /// Return _true_ if queue is at capacity
  bool full ()
  {
    lock l (this->update);
    return (std::queue<M, C>.size () == limit);
  }

  /// Return queue size
  size_t size ()
  {
    lock l (update);
    return std::queue<M, C>::size ();
  }

protected:
  size_t limit;
  semaphore prod_sema;    ///< producers' semaphore counts down until queue is full
  semaphore cons_sema;    ///< consumers' semaphore counts down until queue is empty
  criticalsection update; ///< critical section protects queue's integrity
};

} // namespace mlib
