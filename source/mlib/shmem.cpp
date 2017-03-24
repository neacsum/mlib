/*!
  \file SHMEM.CPP - Implementation of shared memory areas with support for
  single-writer multiple-readers.

    (c) Mircea Neacsu. 2004-2017
*/
#ifndef UNICODE
#define UNICODE
#endif

//comment this line if you want debug messages from this module
#undef _TRACE

#include <mlib/shmem.h>
#include <mlib/trace.h>
#include <assert.h>
#include <mlib/utf8.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


shmem_base::shmem_base () :
 name_ (NULL),
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
 name_ (NULL),
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
  close ();
  TRACE2 ("shmem_base destructor done");
}

bool shmem_base::open (const char * nam, size_t sz_)
{
  
  close ();
  assert (nam);
  name_ = new char[strlen (nam) + 1];
  strcpy (name_, nam);

  std::wstring wname = utf8::widen(name_);
  std::wstring tmp;
  try {
    tmp = wname + L".MEM";
    sz = sz_;
    file = CreateFileMapping (INVALID_HANDLE_VALUE,         //memory based
                              NULL,                         //security
                              PAGE_READWRITE,               //protection
                              0, (DWORD)(sz+sizeof(syncblk)), //size MSW and LSW
                              tmp.c_str());                   //name
    DWORD ret = GetLastError ();
    if (!file)
    {
      TRACE ("shmem_base::open(%s) CreateFileMapping failed!", name_);
      throw ret;
    }
  
    mem_created = (ret != ERROR_ALREADY_EXISTS);

    syn = (syncblk*)MapViewOfFile (file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!syn)
    {
      TRACE ("shmem_base::open(%s) MapViewOfFile failed!", name_);
      throw GetLastError();
    }

    /* data is after the sync block */
    mem = syn + 1;
    tmp = wname + L".EVT";
    rdgate = CreateEvent (NULL, TRUE, TRUE, tmp.c_str());
    if (mem_created)
      syn->rdgate = rdgate;
    tmp = wname + L".MUT";
    wrex = CreateMutex (NULL, FALSE, tmp.c_str());
    if (mem_created)
    {
      TRACE ("shmem_base::open(%s) Created shared memory", name_);
      syn->wrex = wrex;
      syn->rc = syn->wc = 0;
      syn->wrid = 0;
      memset (mem, 0, sz);
    }
  } catch (DWORD& x) {
    TRACE ("shmem_base::open(%s) Error %ld", name_, x);
    close ();
    return false;
  }
  TRACE ("shmem_base::open(%s) size %d done", name_, sz);
  return true;
}

bool shmem_base::close ()
{
  assert (!in_rdlock && !in_wrlock);
  if (name_)
    TRACE ("shmem_base::close (%s)", name_);
  if (syn)
    UnmapViewOfFile (syn);
  if (file)
    CloseHandle (file);
  file = NULL;
  mem = syn = NULL;
  if (wrex != INVALID_HANDLE_VALUE)
    CloseHandle (wrex);
  if (rdgate != INVALID_HANDLE_VALUE)
    CloseHandle (rdgate);
  rdgate = wrex = INVALID_HANDLE_VALUE;
  mem_created = false;
  delete name_;
  name_ = 0;
  return true;
}

bool shmem_base::rdlock ()
{
  assert (syn);

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
  assert (syn);
  assert (in_rdlock);
  assert (syn->rc > 0);

  InterlockedDecrement (&syn->rc);
  in_rdlock--;
  TRACE ("shmem_base::rdunlock (%d)- done", in_rdlock);
}

bool shmem_base::wrlock ()
{
  assert (syn);
  DWORD res;

  InterlockedIncrement (&syn->wc);
  in_wrlock++;
  if ((res=WaitForSingleObject (wrex, wtmo_)) == WAIT_OBJECT_0)
  {
    DWORD endt;
    TRACE ("shmem_base::wrlock - Acquired wrex");
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
  assert (syn);
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
  assert (syn);

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
  assert (syn);

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

