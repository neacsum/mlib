/*!
  \file ringbuf.h Simple circular (ring) buffer class

  (c) Mircea Neacsu 2018
*/
#pragma once

#include <memory>
#include <vector>
#include <assert.h>

#if __has_include ("defs.h")
#include "defs.h"
#endif

namespace mlib {


/// Circular buffer
template <class T>
class ring_buffer 
{
public:

  /// Iterator through the circular buffer
  template <bool C_>
  class iterator_type
  {
  public:
    //Following typedefs must exist to allow instantiation of std::iterator_traits
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = typename std::conditional_t< C_, T const&, T& >;
    using pointer = typename std::conditional_t< C_, T const*, T* >;
    using iterator_category = std::bidirectional_iterator_tag;

    ///Default constructor
    iterator_type<C_> () : ring (nullptr), pos (0) {}

    ///Dereference operator
    reference operator *()
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return ring->buf[pos];
    }

    ///Dereference operator (const version)
    const reference operator *() const
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return ring->buf[pos];
    }

    ///Object pointer
    pointer operator ->()
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return &ring->buf[pos];
    }

    ///Object pointer (const version)
    const pointer operator ->() const
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return &ring->buf[pos];
    }

    ///Increment operator (postfix)
    iterator_type<C_> operator ++ (int)
    {
      iterator_type<C_> tmp = *this;
      add (1);
      return tmp;
    }

    ///Increment operator (prefix)
    iterator_type<C_>& operator ++ ()
    {
      add (1);
      return *this;
    }

    ///Decrement operator (postfix)
    iterator_type<C_> operator -- (int)
    {
      iterator_type<C_> tmp = *this;
      sub (1);
      return tmp;
    }

    ///Decrement operator (prefix)
    iterator_type<C_>& operator -- ()
    {
      sub (1);
      return *this;
    }

    ///Equality comparison
    bool operator == (const iterator_type<C_>& it) const
    {
      return (ring == it.ring) && (pos == it.pos);
    }

    ///Inequality comparison
    bool operator != (const iterator_type<C_>& it) const
    {
      return !operator == (it);
    }
    ///Assignment operator
    iterator_type<C_>& operator = (const iterator_type<C_>& rhs)
    {
      if (this != &rhs)
      {
        ring = rhs.ring;
        pos = rhs.pos;
      }
      return *this;
    }

    ///Addition operator
    iterator_type<C_> operator +(size_t inc) const
    {
      iterator_type<C_> tmp = *this;
      tmp.add (inc);
      return tmp;
    }

    ///Addition assignment operator
    iterator_type<C_>& operator +=(size_t inc)
    {
      add (inc);
      return *this;
    }

    ///Subtraction operator
    iterator_type<C_> operator -(size_t dec) const
    {
      iterator_type<C_> tmp = *this;
      tmp.sub (dec);
      return tmp;
    }

    ///Subtraction assignment operator
    iterator_type<C_>& operator -=(size_t dec)
    {
      sub (dec);
      return *this;
    }

    ///Difference operator
    ptrdiff_t operator -(const iterator_type<C_>& other) const
    {
      assert (ring == other.ring);
      size_t p1 = (pos != -1) ? pos : (ring->back_idx == ring->front_idx) ? ring->sz : ring->back_idx;
      size_t p2 = (other.pos != -1) ? other.pos : (ring->back_idx == ring->front_idx) ? ring->sz : ring->back_idx;
      if (p1 >= p2)
        return p1 - p2;
      else
        return ring->cap - p2 + p1;
    }

  private:
    iterator_type<C_> (const ring_buffer* ring_, size_t pos_)
      : ring (ring_)
      , pos (pos_)
    {
    }

    // addition and increment helper function
    void add (size_t inc)
    {
      if (!ring->cap || pos == -1 || !inc)
        return;

      inc %= ring->cap;
      pos = (pos + inc) % ring->cap;
      if ((pos >= ring->back_idx) && 
          ((ring->back_idx >= ring->front_idx) || (pos < ring->front_idx)))
        pos = -1;
    }

    // subtraction and decrement helper function
    void sub (size_t dec)
    {
      if (!ring->cap || pos == ring->front_idx || !dec)
        return;
      if (pos == (size_t)(-1))
        pos = ring->back_idx;

      pos = pos - (dec % ring->cap);
      if (pos < 0)
        pos += ring->cap;

      if (pos >= ring->back_idx && pos < ring->front_idx)
        pos = ring->front_idx;
    }

    const ring_buffer* ring;
    size_t pos;
    friend class ring_buffer;

  };

  typedef iterator_type<false> iterator;
  typedef iterator_type<true> const_iterator;

  ///Constructor
  ring_buffer (size_t size) 
    : buf (std::unique_ptr<T[]> (new T[size]))
    , cap (size)
    , front_idx (0)
    , back_idx (0)
    , sz (0)
  {
  }

  ///Default constructor
  ring_buffer ()
    : cap (0)
    , front_idx (0)
    , back_idx (0)
    , sz (0)
  {
  }

  ///Copy constructor
  ring_buffer (const ring_buffer& other)
    : front_idx (other.front_idx)
    , back_idx (other.back_idx)
    , cap (other.cap)
    , sz (other.sz)
  {
    if (other.cap)
    {
      buf = std::unique_ptr<T[]> (new T[other.cap]);
      for (size_t i = 0; i < sz; i++)
      {
        size_t idx = (front_idx + i) % cap;
        buf[idx] = other.buf[idx];
      }
    }
  }

  ///Initializer list constructor
  ring_buffer (std::initializer_list<T> il)
    : buf (std::unique_ptr<T[]> (new T[il.size ()]))
    , cap (il.size ())
    , front_idx (0)
    , back_idx (0)
    , sz (il.size())
  {
    size_t i = 0;
    for (const auto v : il)
      buf[i++] = std::move(v);
  }

  ///Assignment operator
  ring_buffer& operator = (const ring_buffer& rhs)
  {
    if (&rhs != this)
    {
      front_idx = rhs.front_idx;
      back_idx = rhs.back_idx;
      cap = rhs.cap;
      sz = rhs.sz;
      if (rhs.cap)
      {
        buf.reset (new T[rhs.cap]);
        for (size_t i = 0; i < sz; i++)
        {
          size_t idx = (front_idx + i) % cap;
          buf[idx] = rhs.buf[idx];
        }
      }
      else
        buf.reset ();
    }
    return *this;
  }

  ///Equality operator
  bool operator == (const ring_buffer<T>& other) const
  {
    if (cap == other.cap && sz == other.sz)
    {
      for (size_t i = 0; i < sz; i++)
      {
        size_t idx1 = (front_idx + i) % cap;
        size_t idx2 = (other.front_idx + i) % other.cap;
        if (buf[idx1] != other.buf[idx2])
          return false;
      }
      return true;
    }
    return false;
  }

  ///Inequality operator
  bool operator != (const ring_buffer<T>& other) const
  {
    return !operator ==(other);
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
    if (!cap)
      return; //container not allocated

    if (sz && back_idx == front_idx)
      front_idx = (front_idx + 1) % cap;
    else
      sz++;
    buf[back_idx] = item;
    back_idx = (back_idx + 1) % cap;
  }

  ///Return an iterator pointing to first (oldest) element in buffer
  iterator begin () const
  {
    return iterator (this, front_idx);
  }

  ///Return a const iterator pointing to first (oldest) element in buffer
  const_iterator cbegin () const
  {
    return const_iterator (this, front_idx);
  }

  ///Return an iterator pointing past the last (newest) element in buffer
  iterator end () const
  {
    return iterator (this, (size_t)(-1));
  }

  ///Return a const iterator pointing past the last (newest) element in buffer
  const_iterator cend () const
  {
    return const_iterator (this, (size_t)(-1));
  }

  ///Remove oldest element from buffer
  void pop_front ()
  {
    if (sz)
    {
      front_idx = (front_idx + 1) % cap;
      sz--;
    }
  }

  /// Return a reference to first (oldest) element in buffer.
  T& front ()
  {
    return buf[front_idx];
  }

  /// Return reference to last (newest) element in buffer.
  T& back ()
  {
    return buf[(back_idx + cap - 1) % cap];
  }

  ///Remove all elements from buffer
  void clear (void)
  {
    front_idx = back_idx;
    sz = 0;
  }

  ///Return true if buffer is empty
  bool empty (void) 
  {
    return (sz == 0);
  }

  ///Return true if buffer is full
  bool full (void)
  {
    return (sz == cap);
  }

  ///Return maximum buffer size
  size_t capacity (void)
  {
    return cap;
  };

  ///(Re)allocate buffer with a different capacity
  void resize (size_t new_cap)
  {
    std::unique_ptr<T[]> newbuf (new_cap ? new T[new_cap] : nullptr);
    if (new_cap)
    {
      if (sz >= new_cap)
        sz = new_cap;
      for (size_t i = 0; i < sz; i++)
        newbuf[i] = buf[(front_idx + i) % cap];
    }
    else
      sz = 0;
    cap = new_cap;
    buf.swap(newbuf);
    front_idx = 0;
    back_idx = cap ? (front_idx + sz) % cap : 0;
  }

  ///Return number of elements in buffer
  size_t size () const
  {
    return sz;
  }

private:
  std::unique_ptr<T[]> buf;
  size_t front_idx, back_idx, cap, sz;

};

}
