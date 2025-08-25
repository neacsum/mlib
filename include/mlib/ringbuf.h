/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file ringbuf.h Definition of mlib::ring_buffer - a circular buffer class

#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <assert.h>

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

/// Circular buffer
template <class T>
class ring_buffer
{
public:
  /// Iterator through the circular buffer
  class const_iterator
  {
  public:
    // Following typedefs must exist to allow instantiation of std::iterator_traits
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference =  T const&;
    using pointer = T const*;
    using iterator_category = std::bidirectional_iterator_tag;

    /// Default constructor
    const_iterator ()
      : ring (nullptr)
      , pos (0)
    {}

    /// Dereference operator
    reference operator* ()
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return ring->buf[pos];
    }

    /// Dereference operator (const version)
    const reference operator* () const
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return ring->buf[pos];
    }

    /// Object pointer
    pointer operator->()
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return &ring->buf[pos];
    }

    /// Object pointer (const version)
    const pointer operator->() const
    {
      if (pos == (size_t)(-1))
        throw std::range_error ("Ring buffer iterator at end!");
      return &ring->buf[pos];
    }

    /// Increment operator (postfix)
    const_iterator operator++ (int)
    {
      const_iterator tmp = *this;
      pos = ring->increment (pos);
      return tmp;
    }

    /// Increment operator (prefix)
    const_iterator& operator++ ()
    {
      pos = ring->increment (pos);
      return *this;
    }

    /// Decrement operator (postfix)
    const_iterator operator-- (int)
    {
      const_iterator tmp = *this;
      pos = ring->decrement (pos);
      return tmp;
    }

    /// Decrement operator (prefix)
    const_iterator& operator-- ()
    {
      pos = ring->decrement (pos);
      return *this;
    }

    /// Equality comparison
    bool operator== (const const_iterator& it) const
    {
      return (ring == it.ring) && (pos == it.pos);
    }

    /// Inequality comparison
    bool operator!= (const const_iterator& it) const
    {
      return !operator== (it);
    }

    /// Assignment operator
    const_iterator& operator= (const const_iterator& rhs)
    {
      if (this != &rhs)
      {
        ring = rhs.ring;
        pos = rhs.pos;
      }
      return *this;
    }

    /// Addition operator
    const_iterator operator+ (size_t inc) const
    {
      const_iterator tmp = *this;
      tmp.pos = ring->add (pos, inc);
      return tmp;
    }

    /// Addition assignment operator
    const_iterator& operator+= (size_t inc)
    {
      pos = ring->add (pos, inc);
      return *this;
    }

    /// Subtraction operator
    const_iterator operator- (size_t dec) const
    {
      const_iterator tmp = *this;
      tmp.pos = ring->subtract (pos, dec);
      return tmp;
    }

    /// Subtraction assignment operator
    const_iterator& operator-= (size_t dec)
    {
      pos = ring->subtract (pos, dec);
      return *this;
    }

    /// Difference operator
    ptrdiff_t operator- (const const_iterator& other) const
    {
      assert (ring == other.ring);
      size_t p1 = (pos != -1)                           ? pos
                  : (ring->back_idx == ring->front_idx) ? ring->sz
                                                        : ring->back_idx;
      size_t p2 = (other.pos != -1)                     ? other.pos
                  : (ring->back_idx == ring->front_idx) ? ring->sz
                                                        : ring->back_idx;
      if (p1 >= p2)
        return p1 - p2;
      else
        return ring->cap - p2 + p1;
    }

  protected:
    const_iterator (const ring_buffer* ring_, size_t pos_)
      : ring (ring_)
      , pos (ring->sz?pos_: -1)
    {
    }

    const ring_buffer* ring;
    size_t pos;
    friend class ring_buffer;
  };

  class iterator final : public const_iterator
  {
  public:
    // Following typedefs must exist to allow instantiation of std::iterator_traits
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using iterator_category = std::bidirectional_iterator_tag;
    using my_base = const_iterator;

    /// Default constructor
    iterator ()
      : const_iterator ()
    {}

    iterator (const const_iterator& ci)
      : const_iterator (ci)
    {}

    /// Dereference operator
    reference operator* ()
    {
      return const_cast<reference> (my_base::operator* ());
    }

    /// Object pointer
    pointer operator->()
    {
      return const_cast<pointer> (my_base::operator-> ());
    }

    /// Increment operator (postfix)
    iterator operator++ (int x)
    {
      return my_base::operator++ (x);
    }

    /// Increment operator (prefix)
    iterator& operator++ ()
    {
      my_base::operator++ ();
      return *this;
    }

    /// Decrement operator (postfix)
    iterator operator-- (int x)
    {
      return my_base::operator-- (x);
    }

    /// Decrement operator (prefix)
    iterator& operator-- ()
    {
      my_base::operator-- ();
      return *this;
    }

    /// Assignment operator
    iterator& operator= (const const_iterator& rhs)
    {
      my_base::operator= (rhs);
      return *this;
    }

    /// Addition operator
    iterator operator+ (size_t inc) const
    {
      return my_base::operator+ (inc);
    }

    /// Addition assignment operator
    iterator& operator+= (size_t inc)
    {
      my_base::operator+= (inc);
      return *this;
    }

    /// Subtraction operator
    iterator operator- (size_t dec) const
    {
      return my_base::operator- (dec);
    }

    /// Subtraction assignment operator
    iterator& operator-= (size_t dec)
    {
      my_base::operator-= (dec);
      return *this;
    }

    /// Difference operator
    ptrdiff_t operator- (const const_iterator& other) const
    {
      return my_base::operator- (other);
    }

  private:
    iterator (const ring_buffer* ring_, size_t pos_)
      : const_iterator (ring_, pos_)
    {}

    friend class ring_buffer;
  };

  /// Constructor
  ring_buffer (size_t size)
    : buf (std::unique_ptr<T[]> (new T[size]))
    , cap (size)
    , front_idx (0)
    , back_idx (0)
    , sz (0)
  {}

  /// Default constructor
  ring_buffer ()
    : cap (0)
    , front_idx (0)
    , back_idx (0)
    , sz (0)
  {}

  /// Copy constructor
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

  /// Initializer list constructor
  ring_buffer (std::initializer_list<T> il)
    : buf (std::unique_ptr<T[]> (new T[il.size ()]))
    , cap (il.size ())
    , front_idx (0)
    , back_idx (0)
    , sz (il.size ())
  {
    size_t i = 0;
    for (const auto v : il)
      buf[i++] = std::move (v);
  }

  /// Assignment operator
  ring_buffer& operator= (const ring_buffer& rhs)
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

  /// Equality operator
  bool operator== (const ring_buffer<T>& other) const
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

  /// Inequality operator
  bool operator!= (const ring_buffer<T>& other) const
  {
    return !operator== (other);
  }

  /// Vector conversion operator
  operator std::vector<T> () const
  {
    std::vector<T> v (sz);
    for (size_t i = 0; i < sz; i++)
      v[i] = buf[(front_idx + i) % cap];

    return v;
  }

  /// Inserts new element in buffer
  void push_back (const T& item)
  {
    if (!cap)
      return; // container not allocated

    if (sz && back_idx == front_idx)
      front_idx = (front_idx + 1) % cap;
    else
      sz++;
    buf[back_idx] = item;
    back_idx = (back_idx + 1) % cap;
  }

  /// Return an iterator pointing to first (oldest) element in buffer
  iterator begin ()
  {
    return iterator (this, front_idx);
  }

  /// Return a const iterator pointing to first (oldest) element in buffer
  const_iterator begin () const
  {
    return const_iterator (this, front_idx);
  }

  /// Return a const iterator pointing to first (oldest) element in buffer
  const_iterator cbegin ()
  {
    return const_iterator (this, front_idx);
  }

  /// Return an iterator pointing past the last (newest) element in buffer
  iterator end ()
  {
    return iterator (this, (size_t)(-1));
  }

  /// Return a const iterator pointing past the last (newest) element in buffer
  const_iterator end () const
  {
    return const_iterator (this, (size_t)(-1));
  }

  /// Return an iterator pointing past the last (newest) element in buffer
  const_iterator cend ()
  {
    return const_iterator (this, (size_t)(-1));
  }

  /// Remove oldest element from buffer
  void pop_front ()
  {
  #ifdef _DEBUG
    if (!sz)
      throw std::range_error ("ring_buffer::pop_front - empty container");
  #endif
    front_idx = (front_idx + 1) % cap;
    sz--;
  }

  /// Return a reference to first (oldest) element in buffer.
  T& front ()
  {
#ifdef _DEBUG
    if (!sz)
      throw std::range_error ("ring_buffer::front - empty container");
#endif
    return buf[front_idx];
  }

  /// Return a reference to first (oldest) element in buffer.
  const T& front () const
  {
#ifdef _DEBUG
    if (!sz)
      throw std::range_error ("ring_buffer::front - empty container");
#endif
    return buf[front_idx];
  }

  /// Return reference to last (newest) element in buffer.
  T& back ()
  {
#ifdef _DEBUG
    if (!sz)
      throw std::range_error ("ring_buffer::back - empty container");
#endif
    return buf[(back_idx + cap - 1) % cap];
  }

  /// Return reference to last (newest) element in buffer.
  const T& back () const
  {
#ifdef _DEBUG
    if (!sz)
      throw std::range_error ("ring_buffer::back - empty container");
#endif
    return buf[(back_idx + cap - 1) % cap];
  }

  /// Erase element at iterator position. SHiftrs all elements between front
  /// (oldest) and iterator position. 
  iterator erase (const_iterator& ptr)
  {
#ifdef _DEBUG
    if (ptr.ring != this)
      throw std::runtime_error ("ring_buffer<>::erase - Invalid iterator");
#endif
    if (sz == 0)
      return end (); //erasing from an empty buffer is a no-op
    size_t pos = ptr.pos == (size_t)-1 ? back_idx : ptr.pos; 
#ifdef _DEBUG
    if ((front_idx < back_idx && (pos < front_idx || pos >= back_idx)) 
     || (front_idx > back_idx && (pos < back_idx || pos >= front_idx)))
      throw std::range_error ("ring_buffer<>::erase - Bad iterator position");
#endif
    for (auto p = pos; p != front_idx; )
    {
      auto pp = decrement (p);
      assert (pp != (size_t)-1);
      buf[p] = buf[pp];
      p = pp;
    }
    --sz;
    if (pos == front_idx)
      pos = increment (pos);
    front_idx = (front_idx + 1) % cap;
    return iterator(this, pos);
  }

  /// Remove all elements from buffer
  void clear (void)
  {
    front_idx = back_idx;
    sz = 0;
  }

  /// Return true if buffer is empty
  bool empty (void)
  {
    return (sz == 0);
  }

  /// Return true if buffer is full
  bool full (void) const
  {
    return (sz == cap);
  }

  /// Return maximum buffer size
  size_t capacity (void) const
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
    buf.swap (newbuf);
    front_idx = 0;
    back_idx = cap ? (front_idx + sz) % cap : 0;
  }

  /// Return number of elements in buffer
  size_t size () const
  {
    return sz;
  }

private:
  /// increment an iterator index
  size_t increment (size_t pos) const
  {
    if (cap && pos != (size_t)-1)
      pos = (pos + 1) % cap;
    if (pos == back_idx)
      pos = (size_t)-1;
    return pos;
  }

  /// decrement an iterator index
  size_t decrement (size_t pos) const
  {
    if (cap)
    {
      if (pos == (size_t)-1)
        pos = (back_idx + cap - 1) % cap;
      else if (pos != front_idx)
        pos = (pos + cap - 1) % cap;
    }
    return pos;
  }

  /// add a value to an iterator index
  size_t add (size_t oldpos, size_t delta) const
  {
    if (cap && oldpos != -1)
    {
      size_t np = oldpos + (delta % cap);
      if (np >= cap && np >= back_idx + cap)
        np = (size_t)(-1);
      else
        np %= cap;
      return np;
    }
    return oldpos;
  }

  /// subtract a value from an iterator index
  size_t subtract (size_t oldpos, size_t delta) const
  {
    if (cap)
    {
      size_t np = (oldpos == (size_t)-1 ? back_idx : oldpos) - (delta % cap) + cap;
      if (np < front_idx)
        np = front_idx;
      else
        np %= cap;
      return np;
    }
    return oldpos;
  }

  std::unique_ptr<T[]> buf; //the ring buffer
  size_t front_idx;         // start (oldest) element
  size_t back_idx;          // end (newest) element
  size_t cap;               // capacity
  size_t sz;                // size
};

} // namespace mlib
