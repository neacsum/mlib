/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file shmem.h Shared memory object with support for single-writer multiple-readers.

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include <stdexcept>

namespace mlib {

/// Base class for shared memory objects
class shmem_base
{
public:
  // constructors/destructor
  shmem_base ();
  shmem_base (const std::string& name, size_t size);
  virtual ~shmem_base ();

  // open/close
  virtual bool open (const std::string& name, size_t size);
  virtual bool close ();

  bool created () const;
  bool is_opened () const;
  size_t size () const;

  const std::string& name () const;

  // get/set function for read/write timeout
  void wtmo (DWORD msec);
  DWORD wtmo () const;
  void rtmo (DWORD msec);
  DWORD rtmo () const;

  // data access function
  virtual bool write (const void* data);
  virtual bool read (void* data);

protected:
  // Lock control
  virtual bool rdlock ();
  virtual void rdunlock ();
  virtual bool wrlock ();
  virtual void wrunlock ();

  /// \name Naked data access functions
  /// Use always together with lock control functions
  /// @{
  void get (void* data) const;
  void put (const void* data);
  const void* dataptr () const;
  void* dataptr ();
  /// @}
private:
#pragma pack(push, 1)
  /// variables used for access synchronization
  struct syncblk
  {
    HANDLE wrex;   ///< writers exclusion mutex
    HANDLE rdgate; ///< readers blocking event
    DWORD wrid;    ///< thread id of writer
    //    long rdreq;             ///< requesting readers
    LONG rc; ///< active readers
    LONG wc; ///< active writers
  };
#pragma pack(pop)

  std::string name_;
  LONG in_rdlock;
  LONG in_wrlock;
  bool mem_created;
  HANDLE file;
  HANDLE rdgate, wrex;
  DWORD rtmo_, wtmo_;
  size_t sz;
  struct syncblk* syn;
  void* mem;
};

/// Shared memory area with support for single-writer multiple-readers
template <class S, class B = shmem_base>
class shmem : public B
{
public:
  shmem ();
  shmem (const char* name);
  bool open (const char* name, size_t sz = sizeof (S));
  void operator>> (S& data);
  void operator<< (const S& data);

protected:
  const S* dataptr () const;
  S* dataptr ();

  template <class S, class B>
  friend class lockr;
  template <class S, class B>
  friend class lockw;
};

/// Default constructor
template <class S, class B>
shmem<S, B>::shmem ()
  : B (){};

/// Creates and opens a shared memory area with the given name
template <class S, class B>
shmem<S, B>::shmem (const char* name)
  : B (name, sizeof (S)){};

/// Opens a shared memory area
template <class S, class B>
bool shmem<S, B>::open (const char* name, size_t sz)
{
  return B::open (name, sz);
};

/// Read the content of shared memory area
template <class S, class B>
void shmem<S, B>::operator>> (S& data)
{
  this->read (&data);
};

/// Update (write) the content of a shared memory area
template <class S, class B>
void shmem<S, B>::operator<< (const S& data)
{
  this->write (&data);
};

/*!
  Smart read pointer for a shared memory area.

  lockr objects can be used in code like this:
\verbatim
      struct DATA {
        int i;
      };

      shmem<DATA> mem("SHARED_DATA");
      int li;
      try {
        lockr<DATA> rl(mem);
        li = rl->i;
      }
      catch (runtime_error& err) {
        printf ("read lock failed\n");
      }
\endverbatim
  When created, the object places a read lock on the shared memory area that
  stays in place until the object is destroyed. The object can than be used as
  a const pointer to the structure in the shared memory area.
*/

template <class S, class B = shmem_base>
class lockr
{
public:
  lockr (shmem<S, B>& mem_);
  ~lockr ()
  {
    mem.rdunlock ();
  };

  const S* operator->()
  {
    return mem.dataptr ();
  };
  operator const S* ()
  {
    return const_cast<const S*> (mem.dataptr ());
  };

private:
  shmem<S, B>& mem;
};

template <class S, class B>
lockr<S, B>::lockr (shmem<S, B>& mem_)
  : mem (mem_)
{
  if (!mem.rdlock ())
    throw std::runtime_error ("rdlock failure");
}

/*!
  Smart write pointer for a shared memory area.

  lockw objects can be used in code like this:
\verbatim
      struct DATA {
        int i;
      };

      shmem<DATA> mem("SHARED_DATA");
      try {
        lockw<DATA> wl(mem);
        wl->i = 2;
      }
      catch (runtime_error& err) {
        printf ("write lock failed\n");
      }
\endverbatim
  When created, the object places a write lock on the shared memory area that
  stays in place until the object is destroyed. The object can than be used as
  a (non-const) pointer to the structure in the shared memory area.
*/

template <class S, class B = shmem_base>
class lockw
{
public:
  lockw (shmem<S, B>& mem_);
  ~lockw ()
  {
    mem.wrunlock ();
  };

  ///@{ Dereferences pointer to shared memory area
  S* operator -> ()
  {
    return mem.dataptr ();
  };
  operator S* ()
  {
    return mem.dataptr ();
  };
  /// @}
private:
  shmem<S, B>& mem;
};

template <class S, class B>
lockw<S, B>::lockw (shmem<S, B>& mem_)
  : mem (mem_)
{
  if (!mem.wrlock ())
    throw std::runtime_error ("wrlock failed");
};

//---------------------------------------------------------------------------

/// Return `true` if this instance has created the shared memory area or `false`
/// if shared memory existed already.
inline 
bool shmem_base::created () const
{
  return mem_created;
}

/// Return `true` if shared memory is opened
inline 
bool shmem_base::is_opened () const
{
  return mem != NULL;
}

/// Return size of shared memory area
inline 
size_t shmem_base::size () const
{
  return sz;
}

/// Return the name of the shared memory area
inline 
const std::string& shmem_base::name () const
{
  return name_;
}

/// Set timeout value for write operations
inline 
void shmem_base::wtmo (DWORD msec)
{
  wtmo_ = msec;
}

/// Return current write timeout value (in milliseconds)
inline
DWORD shmem_base::wtmo () const
{
  return wtmo_;
}

/// Set timeout value for read operations
inline
void shmem_base::rtmo (DWORD msec)
{
  rtmo_ = msec;
}

/// Return current timeout value for read operations (in milliseconds)
inline
DWORD shmem_base::rtmo () const
{
  return rtmo_;
}

/// Write new data into the SMA
inline
void shmem_base::put (const void* data)
{
  memcpy (dataptr (), data, size ());
}

/// Retrieve current content of SMA
inline
void shmem_base::get (void* data) const
{
  memcpy (data, dataptr (), size ());
}

/// Return a pointer to content of SMA (const variant)
inline
const void* shmem_base::dataptr () const
{
  return mem;
}

/// Return a pointer to content of SMA (non-const variant)
inline
void* shmem_base::dataptr ()
{
  return mem;
}

/// Return a pointer to content of SMA (const variant)
template <class S, class B>
const S* shmem<S, B>::dataptr () const
{
  return (const S*)B::dataptr ();
}

/// Return a pointer to content of SMA (const variant)
template <class S, class B>
S* shmem<S, B>::dataptr ()
{
  return (S*)B::dataptr ();
}

} // namespace mlib
