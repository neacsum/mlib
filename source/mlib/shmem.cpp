/*!
  \file SHMEM.CPP - Impemenation of shared memory areas with support for
  single-writer multiple-readers.

    (c) Mircea Neacsu. 2004-2009
*/
#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/shmem.h>
#include <mlib/trace.h>
#include <assert.h>
#include <mlib/utf8.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


shmem_base::shmem_base () :
 name (NULL),
 mem (NULL),
 file (NULL),
 wrex (INVALID_HANDLE_VALUE),
 rdgate (INVALID_HANDLE_VALUE),
 in_rdlock (0),
 in_wrlock (0),
 mem_created (false),
 rtmo_ (INFINITE),
 wtmo_ (INFINITE),
 sz (0),
 syn (0)
{
}

shmem_base::shmem_base (const char * nam, size_t sz_) :
 name (NULL),
 file (NULL),
 mem (NULL),
 wrex (INVALID_HANDLE_VALUE),
 rdgate (INVALID_HANDLE_VALUE),
 in_rdlock (0),
 in_wrlock (0),
 mem_created (false),
 rtmo_ (INFINITE),
 wtmo_ (INFINITE),
 sz (sz_),
 syn (0)
{
  open (nam, sz_);
}

shmem_base::~shmem_base()
{
  TRACE2 ("shmem_base (%s) destructor", name);
  close ();
  delete name;
  TRACE2 ("shmem_base destructor done");
}

bool shmem_base::open (const char * nam, size_t sz_)
{
  
  close ();
  assert (nam);
  std::wstring name = widen(nam);
  std::wstring tmp;
  try {
    tmp = name + L".MEM";
    sz = sz_;
    file = CreateFileMapping (INVALID_HANDLE_VALUE,         //memory based
                              NULL,                         //security
                              PAGE_READWRITE,               //protection
                              0, (DWORD)(sz+sizeof(syncblk)), //size MSW and LSW
                              tmp.c_str());                   //name
    if (!file)
    {
      TRACE ("shmem_base::open CreateFileMapping failed!");
      throw GetLastError ();
    }
  
    if (GetLastError () != ERROR_ALREADY_EXISTS)
      mem_created = true;
    mem = MapViewOfFile (file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!mem)
    {
      TRACE ("shmem_base::open MapViewOfFile failed!");
      throw GetLastError();
    }

    /* for compatibility with old shared memory protocol, synchronization
    block is AFTER the data */
    data = mem;
    syn = (syncblk*)((char*)mem + sz);
    tmp = name + L".EVT";
    rdgate = CreateEvent (NULL, TRUE, TRUE, tmp.c_str());
    if (mem_created)
      syn->rdgate = rdgate;
    tmp = name + L".MUT";
    wrex = CreateMutex (NULL, FALSE, tmp.c_str());
    if (mem_created)
    {
      syn->wrex = wrex;
      syn->rc = syn->wc = 0;
      syn->wrid = 0;
      memset (data, 0, sz);
    }
  } catch (DWORD& x) {
    TRACE ("Error %ld", x);
    close ();
    return false;
  }

  return true;
}

bool shmem_base::close ()
{
  assert (!in_rdlock && !in_wrlock);
  if (mem)
    UnmapViewOfFile (mem);
  if (file)
    CloseHandle (file);
  file = NULL;
  mem = NULL;
  if (wrex != INVALID_HANDLE_VALUE)
    CloseHandle (wrex);
  if (rdgate != INVALID_HANDLE_VALUE)
    CloseHandle (rdgate);
  rdgate = wrex = INVALID_HANDLE_VALUE;
  mem_created = false;
  return true;
}

bool shmem_base::rdlock ()
{
  assert (mem);

  if (in_rdlock || in_wrlock) //write lock has also read semantics
  {
    TRACE ("shmem_base::rdlock - in_rdlock=%d in_wrlock=%d", in_rdlock, in_wrlock);
    InterlockedIncrement (&syn->rc);
  }
  else
  {
    while (syn->wc > 0)
    {
      TRACE ("shmem_base::rdlock - wc=%d", syn->wc);
      if (WaitForSingleObject (rdgate, rtmo_) == WAIT_TIMEOUT)
      {
        TRACE ("shmem_base::rdlock - rdgate timeout");
        return false;
      }
      TRACE ("shmem_base::rdlock - passed rdgate");
    }
    InterlockedIncrement (&syn->rc);
  }
  in_rdlock++;
  return true;
}

void shmem_base::rdunlock ()
{
  assert (mem);
  assert (in_rdlock);
  assert (syn->rc > 0);

  InterlockedDecrement (&syn->rc);
  in_rdlock--;
  TRACE ("shmem_base::rdunlock (%d)- done", in_rdlock);
}

bool shmem_base::wrlock ()
{
  assert (mem);
  DWORD res;

  InterlockedIncrement (&syn->wc);
  in_wrlock++;
  if ((res=WaitForSingleObject (wrex, wtmo_)) == WAIT_OBJECT_0)
  {
    DWORD endt;
    TRACE ("shmem_base::wrlock - Aquired wrex");
    syn->wrid = GetCurrentThreadId ();
    ResetEvent (rdgate);
    if (wtmo_ != INFINITE)
      endt = GetTickCount () + wtmo_;
    else
      endt = INFINITE;
    while ((syn->rc > in_rdlock) && (GetTickCount() < endt))
      Sleep (0);
    if (syn->rc == in_rdlock)
    {
      TRACE ("shmem_base::wrlock - done");
      return true;
    }
    else
    {
      TRACE ("shmem_base::wrlock - Readers still in");
      if (syn->wc == 1)
      {
        TRACE ("shmem_base::wrlock - rdgate opened");
        SetEvent (rdgate);
      }
      syn->wrid = 0;
      ReleaseMutex (wrex);
      InterlockedDecrement (&syn->wc);
      in_wrlock--;
      return false;
    }
  }
  else if (res == WAIT_TIMEOUT)
  {
    TRACE ("shmem_base::wrlock - failed wc=%d", syn->wc);
    assert (syn->wc > 1); 
    InterlockedDecrement (&syn->wc);
    in_wrlock--;
    return false;
  }
  TRACE ("Unexpected wait result 0x%x", res);
  return false;
}

void shmem_base::wrunlock()
{
  assert (mem);
  assert (in_wrlock);
  assert (syn->wc > 0);
  syn->wrid = 0;
  ReleaseMutex (wrex);
  if (syn->wc == 1)
  {
    TRACE ("shmem_base::wrunlock - rdgate opened");
    SetEvent (rdgate);
  }
  InterlockedDecrement (&syn->wc);
  in_wrlock--;
  TRACE ("shmem_base::wrunlock (%d) - done", in_wrlock);
}

void shmem_base::put (const void *data)
{
  memcpy (dataptr(), data, size() );
}

void shmem_base::get (void *data)
{
  memcpy (data, dataptr(), size());
}

bool shmem_base::write (const void *pData)
{
  assert (mem);

  if (wrlock ())
  {
    put (pData);
    wrunlock ();
    return true;
  }
  return false;
}

bool shmem_base::read (void * pData)
{
  assert (mem);

  if (rdlock ())
  {
    get (pData);
    rdunlock ();
    return true;
  }
  return false;
}

#ifdef MLIBSPACE
}
#endif

