/*!
  \file RINGBUF.H - Simple circular (ring) buffer class
  (c) Mircea Neacsu 2018

*/
#pragma once

#include <memory>
#include <vector>

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


/// Circular buffer
template <class T>
class ring_buffer 
{
public:
  /// Iterator through the circular buffer
  class iterator
  {
  public:
    //Following typedefs must exist to allow instantiation of std::iterator_traits
    typedef ptrdiff_t difference_type;
    typedef typename T value_type;
    typedef typename T& reference;
    typedef typename T* pointer;
    typedef std::bidirectional_iterator_tag iterator_category;

    ///Default constructor
    iterator () : ring(nullptr), pos(0) {}

    ///Dereference operator
    reference operator *()
    {
      return ring->buf[pos];
    }

    ///Dereference operator (const version)
    const reference operator *() const
    {
      return ring->buf[pos];
    }

    ///Object pointer
    pointer operator ->()
    {
      return &ring->buf[pos];
    }

    ///Object pointer (const version)
    const pointer operator ->() const
    {
      return &ring->buf[pos];
    }

    ///Increment operator (postfix)
    iterator operator ++ (int)
    {
      iterator tmp = *this;
      inc_func ();
      return tmp;
    }

    ///Increment operator (prefix)
    iterator& operator ++ ()
    {
      inc_func ();
      return *this;
    }

    ///Decrement operator (postfix)
    iterator operator -- (int)
    {
      iterator tmp = *this;
      dec_func ();
      return tmp;
    }

    ///Decrement operator (prefix)
    iterator& operator -- ()
    {
      dec_func ();
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

    ///Addition operator
    iterator operator +(size_t inc) const
    {
      iterator tmp = *this;
      add_func (tmp, inc);
      return tmp;
    }

    ///Addition assignment operator
    iterator& operator +=(size_t inc)
    {
      add_func (*this, inc);
      return *this;
    }

    ///Subtraction operator
    iterator operator -(size_t dec) const
    {
      iterator tmp = *this;
      sub_func (tmp, dec);
      return tmp;
    }

    ///Subtraction assignment operator
    iterator& operator -=(size_t dec)
    {
      sub_func (*this, dec);
      return *this;
    }

  private:
    iterator (const ring_buffer* ring_, size_t pos_)
      : ring (ring_)
      , pos (pos_)
    {
    }

    void inc_func ()
    {
      if (ring->cap && pos != -1)
        pos = (pos + 1) % ring->cap;
      if (pos == ring->back_idx)
        pos = -1;
    }

    void dec_func ()
    {
      if (ring->cap)
      {
        if (pos == -1)
          pos = (ring->back_idx + ring->cap - 1) % ring->cap;
        else if (pos != ring->front_idx)
          pos = (pos + ring->cap - 1) % ring->cap;
      }
    }

    void add_func (iterator& it, size_t inc) const
    {
      if (ring->cap && pos != -1)
      {
        size_t np = it.pos + (inc % ring->cap);
        if (np >= ring->cap && np >= ring->back_idx + ring->cap)
          it.pos = -1;
        else
          it.pos = np % ring->cap;
      }
    }

    void sub_func (iterator& it, size_t dec) const
    {
      if (ring->cap)
      {
        size_t np = (it.pos == -1 ? ring->back_idx : it.pos) - (dec % ring->cap) + ring->cap;
        if (np < ring->front_idx)
          it.pos = ring->front_idx;
        else
          it.pos = np % ring->cap;
      }
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

  ///Return an iterator pointing past the last (newest) element in buffer
  iterator end () const
  {
    return iterator (this, (size_t)(-1));
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

#ifdef MLIBSPACE
}
#endif
