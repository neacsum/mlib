/*!
  \file RINGBUF.H - Simple circular (ring) buffer class
  (c) Mircea Neacsu 2018

*/
#pragma once

#include <memory>
#include <vector>

/// Circular buffer
template <class T>
class ring_buffer 
{
public:
  /// Iterator through the circular buffer
  class iterator
  {
  public:
    ///Default constructor
    iterator () : ring(nullptr), pos(0) {}

    ///Dereference operator
    T& operator *()
    {
      return ring->buf[pos];
    }

    ///Object pointer
    T* operator ->()
    {
      return &ring->buf[pos];
    }

    ///Increment operator (postfix)
    iterator operator ++ (int)
    {
      iterator tmp = *this;
      auto tpos = (pos + 1) % ring->cap;
      if (tpos != ring->back_idx)
        pos = tpos;
      return tmp;
    }

    ///Increment operator (prefix)
    iterator& operator ++ ()
    {
      auto tpos = (pos + 1) % ring->cap;
      if (tpos != ring->back_idx)
        pos = tpos;
      return *this;
    }

    ///Decrement operator (postfix)
    iterator operator -- (int)
    {
      iterator tmp = *this;
      if (pos != ring->front_idx)
        pos = (pos + ring->cap - 1) % ring->cap;
      return tmp;
    }

    ///Decrement operator (prefix)
    iterator& operator -- ()
    {
      if (pos != ring->front_idx)
        pos = (pos + ring->cap - 1) % ring->cap;
      return *this;
    }

    ///Equality comparison
    bool operator == (const iterator& it) const
    {
      return (ring == it.ring) && (pos == it.pos);
    }

    ///Inequality comparison
    bool operator != (const iterator& it) const
    {
      return !operator == (it);
    }

    ///Assignment operator
    iterator& operator = (const iterator& rhs)
    {
      if (this != &rhs)
      {
        ring = rhs.ring;
        pos = rhs.pos;
      }
      return *this;
    }

  private:
    iterator (const ring_buffer* ring_, size_t pos_)
      : ring (ring_)
      , pos (pos_)
    {
    }
    const ring_buffer* ring;
    size_t pos;
    friend class ring_buffer;
  };

  ///Constructor
  ring_buffer (size_t size) 
    : buf (std::unique_ptr<T[]> (new T[size]))
    , cap (size)
    , front_idx (0)
    , back_idx (0)
    , sz (0)
  {
  }

  ///Copy constructor
  ring_buffer (const ring_buffer& other)
    : buf (std::unique_ptr<T[]> (new T[other.cap]))
    , front_idx (other.front_idx)
    , back_idx (other.back_idx)
    , cap (other.cap)
    , sz (other.sz)
  {
    for (size_t i = 0; i < sz; i++)
    {
      size_t idx = (front_idx + i) % cap;
      buf[idx] = other.buf[idx];
    }
  }

  ///Assignment operator
  ring_buffer& operator = (const ring_buffer& rhs)
  {
    if (&rhs != this)
    {
      buf.reset (new T[rhs.cap]);
      front_idx = rhs.front_idx;
      back_idx = rhs.back_idx;
      cap = rhs.cap;
      sz = rhs.sz;
      for (size_t i = 0; i < sz; i++)
      {
        size_t idx = (front_idx + i) % cap;
        buf[idx] = rhs.buf[idx];
      }
    }
    return *this;
  }

  /// Vector conversion operator
  operator std::vector<T> () const
  {
    std::vector<T> v(sz);
    for (size_t i = 0; i < sz; i++)
      v[i] = buf[(front_idx + i) % cap];

    return v;
  }

  ///Inserts new element in buffer
  void push_back (const T& item) 
  {
    if (sz && back_idx == front_idx)
      front_idx = (front_idx + 1) % cap;
    else
      sz++;
    buf[back_idx] = item;
    back_idx = (back_idx + 1) % cap;
  }

  ///Removes oldest element from buffer
  void pop_front ()
  {
    if (sz)
    {
      front_idx = (front_idx + 1) % cap;
      sz--;
    }
  }

  /*!
    Returns an iterator pointing to first (oldest) element in buffer.
    This iterator can only be incremented to access subsequent (newer) elements.
  */
  iterator front ()
  {
    return iterator (this, front_idx);
  }

  /*!
    Returns an iterator pointing to last (newest) element in buffer.
    This iterator can only be decremented to access previous (older) elements.
  */
  iterator back ()
  {
    return iterator (this, (back_idx + cap - 1) % cap);
  }

  ///Removes all elements from buffer
  void clear (void)
  {
    front_idx = back_idx;
    sz = 0;
  }

  ///Returns true if buffer is empty
  bool empty (void) 
  {
    return (sz == 0);
  }

  ///Returns true if buffer is full
  bool full (void)
  {
    return (sz == cap);
  }

  ///Returns buffer maximum size
  size_t capacity (void) 
  {
    return cap;
  };

  ///Returns current number of elements in buffer
  size_t size () const
  {
    return sz;
  }

private:
  std::unique_ptr<T[]> buf;
  size_t front_idx, back_idx, cap, sz;
};