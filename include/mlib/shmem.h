#pragma once
/*!
    \file SHMEM.H Shared memory object with support for single-writer multiple-readers.

    (c) Mircea Neacsu 2004-2009
*/

#include "defs.h"
#include <stdexcept>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


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
  bool    is_opened ()          {return mem != NULL;};
  size_t  size () const         {return sz;};
  
  //get/set function for read/write timeout
  void    wtmo (DWORD msec)     {wtmo_ = msec;};
  DWORD   wtmo () const         {return wtmo_;};
  void    rtmo (DWORD msec)     {rtmo_ = msec;};
  DWORD   rtmo () const         {return rtmo_;};

#ifdef SHMEM_INTERFACE_TEST
public:
#else
protected:
#endif

  //lock control
  bool    rdlock ();
  void    rdunlock ();
  bool    wrlock ();
  void    wrunlock ();

  //data access function
  bool    write (const void * data);
  bool    read (void * data);

protected:

#pragma pack (push, 1)
  /// variables used for access syncronization
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

  virtual void get (void *data);
  virtual void put (const void *data);
  void *dataptr () const        {return data;};
  syncblk *syncptr () const     {return syn;};

private:
  char* name;
  LONG in_rdlock;
  LONG in_wrlock;
  bool mem_created;
  HANDLE file;
  HANDLE rdgate, wrex;
  DWORD rtmo_, wtmo_;
  size_t sz;
  void *data;
  struct syncblk *syn;
  void *mem;
  friend class lockr_base;
};

template<class S> class shmem : public shmem_base
{
public:
  shmem ();
  shmem (const char *name, size_t sz = sizeof(S));
  bool open (const char *name, size_t sz = sizeof(S));
  void operator >>(S& data);
  void operator <<(const S& data);

protected:
  S* dataptr() const {return (S*)shmem_base::dataptr();};

  template<class S> friend class lockr;
  template<class S> friend class lockw;
};

template<class S>
shmem<S>::shmem () : shmem_base () {};

template<class S>
shmem<S>::shmem (const char *name, size_t sz) : shmem_base (name, sz) {};

template<class S>
bool shmem<S>::open(const char *name, size_t sz) {return shmem_base::open (name, sz);};

template<class S>
void shmem<S>::operator >>(S& data) {read (&data);};

template<class S>
void shmem<S>::operator <<(const S& data) {write (&data);};

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

template<class S> class lockr
{
public:
  lockr (shmem<S>& mem_);
  ~lockr () {mem.rdunlock ();};

  const S* operator ->() {return const_cast <const S*>(mem.dataptr());};
  operator const S*() {return const_cast <const S*>(mem.dataptr());};
private:
  shmem<S>& mem;
};

template<class S>
lockr<S>::lockr (shmem<S>& mem_) : mem(mem_) 
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

template<class S> class lockw
{
public:
  lockw (shmem<S>& mem_);
  ~lockw () {mem.wrunlock ();};

  S* operator ->() {return mem.dataptr();};
  operator S*() {return mem.dataptr();};
private:
  shmem<S>& mem;
};

template<class S>
lockw<S>::lockw (shmem<S>& mem_) : mem(mem_) 
{
  if (!mem.wrlock ())
    throw std::runtime_error ("wrlock failed");
};

#ifdef MLIBSPACE
}
#endif
