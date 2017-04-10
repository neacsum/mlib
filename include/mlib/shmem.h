#pragma once
/*!
    \file SHMEM.H Shared memory object with support for single-writer multiple-readers.

    (c) Mircea Neacsu 2004-2017
*/

#include "defs.h"
#include <stdexcept>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

///Base class for shared memory objects
class shmem_base
{
public:
  //constructors/destructor
  shmem_base ();
  shmem_base (const char * name, size_t size);
  virtual ~shmem_base();

  //open/close
  virtual bool open (const char *name, size_t size);
  virtual bool close ();

  bool    created () const      {return mem_created;};
  bool    is_opened () const    {return mem != NULL;};
  size_t  size () const         {return sz;};
  const char *name () const     { return name_; };
  
  //get/set function for read/write timeout
  void    wtmo (DWORD msec)     {wtmo_ = msec;};
  DWORD   wtmo () const         {return wtmo_;};
  void    rtmo (DWORD msec)     {rtmo_ = msec;};
  DWORD   rtmo () const         {return rtmo_;};

  //data access function
  virtual bool    write (const void * data);
  virtual bool    read (void * data);

protected:
  //Lock control
  bool    rdlock ();
  void    rdunlock ();
  bool    wrlock ();
  void    wrunlock ();

  //Unprotected data access functions. Use always together with lock control functions
  virtual void get (void *data);
  virtual void put (const void *data);
  void *dataptr () const        {return mem;};

private:
#pragma pack (push, 1)
  /// variables used for access synchronization
  struct syncblk
  {
    HANDLE wrex;            ///< writers exclusion mutex
    HANDLE rdgate;          ///< readers blocking event
    DWORD wrid;             ///< thread id of writer
    //    long rdreq;             ///< requesting readers 
    LONG rc;                ///< active readers
    LONG wc;                ///< active writers
  };
#pragma pack (pop)

  char* name_;
  LONG in_rdlock;
  LONG in_wrlock;
  bool mem_created;
  HANDLE file;
  HANDLE rdgate, wrex;
  DWORD rtmo_, wtmo_;
  size_t sz;
  struct syncblk *syn;
  void *mem;
};

///Shared memory area with support for single-writer multiple-readers
template<class S, class B=shmem_base> class shmem : public B
{
public:
  shmem ();
  shmem (const char *name);
  bool open (const char *name, size_t sz = sizeof(S));
  void operator >>(S& data);
  void operator <<(const S& data);

protected:
  S* dataptr() const {return (S*)B::dataptr();};

  template<class S, class B> friend class lockr;
  template<class S, class B> friend class lockw;
};

/// Default constructor
template<class S, class B>
shmem<S, B>::shmem () : B () {};

/// Creates and opens a shared memory area with the given name
template<class S, class B>
shmem<S, B>::shmem (const char *name) : B (name, sizeof (S)) {};

/// Opens a shared memory area
template<class S, class B>
bool shmem<S, B>::open(const char *name) {return B::open (name, sizeof(S));};

/// Read the content of shared memory area
template<class S, class B>
void shmem<S, B>::operator >>(S& data) {read (&data);};

/// Update (write) the content of a shared memory area
template<class S, class B>
void shmem<S, B>::operator <<(const S& data) {write (&data);};

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

template<class S, class B=shmem_base> class lockr
{
public:
  lockr (shmem<S, B>& mem_);
  ~lockr () {mem.rdunlock ();};

  const S* operator ->() {return const_cast <const S*>(mem.dataptr());};
  operator const S*() {return const_cast <const S*>(mem.dataptr());};
private:

  shmem<S, B>& mem;
};

template<class S, class B>
lockr<S, B>::lockr (shmem<S, B>& mem_) : mem(mem_) 
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

template<class S, class B=shmem_base> class lockw
{
public:
  lockw (shmem<S, B>& mem_);
  ~lockw () {mem.wrunlock ();};

  S* operator ->() {return mem.dataptr();};
  operator S*() {return mem.dataptr();};
private:
  shmem<S, B>& mem;
};

template<class S, class B>
lockw<S, B>::lockw (shmem<S, B>& mem_) : mem(mem_) 
{
  if (!mem.wrlock ())
    throw std::runtime_error ("wrlock failed");
};

#ifdef MLIBSPACE
}
#endif
