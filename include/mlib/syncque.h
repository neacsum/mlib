/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file syncque.h Definition of classes for FIFO queues

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
  A template class that implements a "synchronous queue".
  
  This is, in effect, a mailbox that can store one "message", created by a
  producer thread, until consumed by a consumer thread.

  If the mailbox is full (a message already there), the producer is blocked
  until the previous message is consumed. If the mailbox is empty, the consumer
  is blocked until a new message arrives.

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
template <class M, class C = std::queue<M>>
class async_queue : protected C
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
  /// \return `true` if the element was appended or `false` if a timeout occurred.
  virtual bool produce (const M& obj, DWORD timeout = INFINITE)
  {
    update.enter ();
    if (limit < INFINITE)
    {
      // Bounded queue. See if there is enough space to produce
      if (timeout == 0)
      {
        //Don't wait if there is no space
        if (C::size () >= limit)
        {
          update.leave ();
          return false;
        }
      }
      else if (timeout == INFINITE)
      {
        //wait forever to get space
        while (C::size () >= limit)
        {
          update.leave ();
          prod_sema.wait ();
          update.enter ();
        }
      }
      else
      {
        stopwatch t; // need a stopwatch to count cumulated wait time
        t.start ();
        while (C::size () >= limit)
        {
          update.leave ();
          int t_wait = timeout - (int)t.lap_msec ();
          if (t_wait < 0 || prod_sema.wait (t_wait) == WAIT_TIMEOUT)
            return false;
          update.enter ();
        }
      }
    }
    C::push (obj);       // put a copy of the object at the end
    cons_sema.signal (); // and signal the semaphore
    update.leave ();

    return true;
  }

  /// Extract the first element in queue
  /// Return _true_ if an element was extracted or _false_ if a timeout occurred.
  virtual bool consume (M& result, int timeout = INFINITE)
  {
    update.enter ();
    if (timeout == 0)
    {
      //no wait
      if (C::empty ())
      {
        update.leave ();
        return false;
      }
    }
    else if (timeout == INFINITE)
    {
      //wait forever
      while (C::empty ())
      {
        update.leave ();
        cons_sema.wait (); // wait for a producer
        update.enter ();
      }
    }
    else
    {
      stopwatch t;
      int t_wait;
      t.start ();
      while (C::empty ())
      {
        update.leave ();
        t_wait = timeout - (int)t.lap_msec ();
        // give up or wait for a producer
        if (t_wait <= 0 || cons_sema.wait (t_wait) == WAIT_TIMEOUT)
          return false;
        update.enter ();
      }
    }
    result = std::move(C::front ()); // get the message
    C::pop ();
    if (limit != INFINITE)
      prod_sema.signal (); // signal producers there is space available

    update.leave ();
    return true;
  }

  /// Return _true_ if queue is empty
  bool empty ()
  {
    lock l (update);
    return C::empty ();
  }

  /// Return _true_ if queue is at capacity
  bool full ()
  {
    lock l (this->update);
    return (C::size () == limit);
  }

  /// Return queue size
  size_t size ()
  {
    lock l (update);
    return C::size ();
  }

protected:
  size_t limit;           ///< maximum queue size
  semaphore prod_sema;    ///< producers' semaphore counts down until queue is full
  semaphore cons_sema;    ///< consumers' semaphore counts down until queue is empty
  criticalsection update; ///< critical section protects queue's integrity
};

/*!
  A template class that implements "asynchronous ring buffer".
  These are producer/consumer buffers which can hold "messages" and deliver
  them in FIFO order. Attempting to consume from an empty buffer will block
  the calling thread until a message arrives. Producing in a full buffer replaces
  the "oldest" element with the new one.
*/
template <class M>
class async_buffer : protected mlib::ring_buffer<M>
{
public:
  /// Crates a buffer with the given maximum size
  async_buffer (size_t limit_)
    : ring_buffer<M> (limit_)
  {
  }

  /// Append an element to buffer
  virtual void produce (const M& obj)
  {
    lock l (update);
    auto prev_sz = ring_buffer<M>::size ();
    ring_buffer<M>::push_back (obj);
    if (ring_buffer<M>::size () != prev_sz)
      sema.signal ();
  }

  /// Extract the first element in queue
  /// Return _true_ if an element was extracted or _false_ if a timeout occurred.
  virtual bool consume (M& result, int timeout = INFINITE)
  {
    bool ret = false;
    if (timeout == 0)
    {
      update.enter ();
      {
        if (!ring_buffer<M>::empty ())
        {
          result = ring_buffer<M>::front ();
          ring_buffer<M>::pop_front ();
          ret = true;
        }
        update.leave ();
      }
      return ret;
    }
    if (timeout != INFINITE)
    {
      stopwatch t;
      int t_wait;
      t.start ();
      update.enter ();
      while (ring_buffer<M>::empty ())
      {
        update.leave ();
        t_wait = timeout - (int)t.lap_msec ();
        // give up or wait for a producer
        if (t_wait <= 0 || sema.wait (t_wait) == WAIT_TIMEOUT)
          return false;
        update.enter ();
      }
    }
    else
    {
      update.enter ();
      if (ring_buffer<M>::empty ())
      {
        while (1)
        {
          update.leave ();
          sema.wait (); // wait for a producer
          update.enter ();
          if (!ring_buffer<M>::empty ())
            break;
        }
      }
    }
    result = std::move (ring_buffer<M>::front ()); // get the message
    ring_buffer<M>::pop_front ();

    update.leave ();
    return true;
  }

protected:
  semaphore sema;         ///< semaphore counts buffer size
  criticalsection update; ///< critical section protects buffer's integrity
};


} // namespace mlib
