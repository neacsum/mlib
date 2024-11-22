/*!
  \file shmem.cpp Implementation of shared memory areas with support for
  single-writer multiple-readers.

    (c) Mircea Neacsu. 2004-2017
*/
#include <mlib/mlib.h>
#pragma hdrstop

#include <assert.h>
#include <utf8/utf8.h>

namespace mlib {

/*!
  Default constructor.

  The shared memory area (SMA) must be opened before use.
*/
shmem_base::shmem_base ()
  : file (NULL) /* Initialized to NULL not INVALID_HANDLE_VALUE because
                CreateFileMapping return NULL on failure */
  , mem (NULL)
  , wrex (INVALID_HANDLE_VALUE)
  , rdgate (INVALID_HANDLE_VALUE)
  , in_rdlock (0)
  , in_wrlock (0)
  , mem_created (false)
  , rtmo_ (INFINITE)
  , wtmo_ (INFINITE)
  , sz (0)
  , syn (0)
{}

/*!
  Create and open a SMA

  \param nam    name of SMA
  \param sz_    size of SMA (not including size of synchronization stuff)
*/
shmem_base::shmem_base (const std::string& nam, size_t sz_)
  : file (NULL)
  , mem (NULL)
  , wrex (INVALID_HANDLE_VALUE)
  , rdgate (INVALID_HANDLE_VALUE)
  , in_rdlock (0)
  , in_wrlock (0)
  , mem_created (false)
  , rtmo_ (INFINITE)
  , wtmo_ (INFINITE)
  , sz (sz_)
  , syn (0)
{
  open (nam, sz_);
}

/*!
  Destructor.

  If SMA was opened, close it now.
*/
shmem_base::~shmem_base ()
{
  close ();
  TRACE9 ("shmem_base destructor done");
}

/*!
  Open a SMA
  \param  nam   name of SMA
  \param  sz_   size of SMA (not including size of synchronization stuff)

  The SMA is called `<nam>.MEM`. Additionally, the following objects are
  created:
  - event flag `<nam>.EVT`
  - Mutex `<nam>.MUT`

*/
bool shmem_base::open (const std::string& nam, size_t sz_)
{
  close ();
  assert (!nam.empty ());
  name_ = nam;

  std::wstring wname = utf8::widen (name_);
  std::wstring tmp;
  try
  {
    tmp = wname + L".MEM";
    sz = sz_;
    file = CreateFileMapping (INVALID_HANDLE_VALUE,              // memory based
                              NULL,                              // security
                              PAGE_READWRITE,                    // protection
                              0, (DWORD)(sz + sizeof (syncblk)), // size MSW and LSW
                              tmp.c_str ());                     // name
    DWORD ret = GetLastError ();
    if (!file)
    {
      TRACE2 ("shmem_base::open(%s) CreateFileMapping failed!", name_.c_str ());
      throw ret;
    }

    mem_created = (ret != ERROR_ALREADY_EXISTS);

    syn = (syncblk*)MapViewOfFile (file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!syn)
    {
      TRACE2 ("shmem_base::open(%s) MapViewOfFile failed!", name_.c_str ());
      throw GetLastError ();
    }

    /* data is after the sync block */
    mem = syn + 1;
    tmp = wname + L".EVT";
    rdgate = CreateEvent (NULL, TRUE, TRUE, tmp.c_str ());
    if (mem_created)
      syn->rdgate = rdgate;
    tmp = wname + L".MUT";
    wrex = CreateMutex (NULL, FALSE, tmp.c_str ());
    if (mem_created)
    {
      syn->wrex = wrex;
      syn->rc = syn->wc = 0;
      syn->wrid = 0;
      memset (mem, 0, sz);
    }
  }
  catch (DWORD& x)
  {
    TRACE2 ("shmem_base::open(%s) Error %ld", name_.c_str (), x);
    close ();
    return false;
  }
  return true;
}

/*!
  Close a SMA
*/
bool shmem_base::close ()
{
  assert (!in_rdlock && !in_wrlock);
  if (!name_.empty ())
    TRACE9 ("shmem_base::close (%s)", name_.c_str ());
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
  name_.clear ();
  return true;
}

/*!
  Obtain a reader lock on SMA.

  \return `true` if successful, `false` otherwise

  If a write operation is in progress, the function waits for it to complete
  before acquiring read lock. If it cannot obtain the lock in read timeout set,
  the function returns `false`.
*/
bool shmem_base::rdlock ()
{
  assert (syn);

  if (in_rdlock || in_wrlock) // write lock has also read semantics
  {
    TRACE9 ("shmem_base::rdlock - in_rdlock=%d in_wrlock=%d", in_rdlock, in_wrlock);
    InterlockedIncrement (&syn->rc);
  }
  else
  {
    while (syn->wc > 0)
    {
      TRACE9 ("shmem_base::rdlock - wc=%d", syn->wc);
      if (WaitForSingleObject (rdgate, rtmo_) == WAIT_TIMEOUT)
        return false;

      TRACE9 ("shmem_base::rdlock - passed rdgate");
    }
    InterlockedIncrement (&syn->rc);
  }
  in_rdlock++;
  return true;
}

/*!
  Release a previously obtained reader lock
*/
void shmem_base::rdunlock ()
{
  assert (syn);
  assert (in_rdlock);
  assert (syn->rc > 0);

  InterlockedDecrement (&syn->rc);
  in_rdlock--;
  TRACE9 ("shmem_base::rdunlock (%d)- done", in_rdlock);
}

/*!
  Obtain a writer lock on SMA.

  \return true if successful, false otherwise

  If another operation is in progress, the function waits for it to complete
  before acquiring write lock. If it cannot obtain the lock in write timeout set,
  the function returns false.
*/
bool shmem_base::wrlock ()
{
  assert (syn);
  DWORD res;

  InterlockedIncrement (&syn->wc);
  in_wrlock++;
  if ((res = WaitForSingleObject (wrex, wtmo_)) == WAIT_OBJECT_0)
  {
    DWORD endt;
    TRACE9 ("shmem_base::wrlock - Acquired wrex");
    syn->wrid = GetCurrentThreadId ();
    ResetEvent (rdgate);
    if (wtmo_ != INFINITE)
      endt = GetTickCount () + wtmo_;
    else
      endt = INFINITE;
    while ((syn->rc > in_rdlock) && (GetTickCount () < endt))
      Sleep (0);
    if (syn->rc == in_rdlock)
    {
      TRACE9 ("shmem_base::wrlock - done");
      return true;
    }
    else
    {
      TRACE2 ("shmem_base::wrlock - Readers still in");
      if (syn->wc == 1)
      {
        TRACE9 ("shmem_base::wrlock - rdgate opened");
        SetEvent (rdgate);
      }
      syn->wrid = 0;
      ReleaseMutex (wrex);
      InterlockedDecrement (&syn->wc);
      in_wrlock--;
    }
  }
  else if (res == WAIT_TIMEOUT)
  {
    assert (syn->wc > 1);
    InterlockedDecrement (&syn->wc);
    in_wrlock--;
  }
  return false;
}

/*!
  Release a previously obtained writer lock
*/
void shmem_base::wrunlock ()
{
  assert (syn);
  assert (in_wrlock);
  assert (syn->wc > 0);
  syn->wrid = 0;
  ReleaseMutex (wrex);
  if (syn->wc == 1)
  {
    TRACE9 ("shmem_base::wrunlock - rdgate opened");
    SetEvent (rdgate);
  }
  InterlockedDecrement (&syn->wc);
  in_wrlock--;
  TRACE9 ("shmem_base::wrunlock (%d) - done", in_wrlock);
}

/// Write new data into the SMA
void shmem_base::put (const void* data)
{
  memcpy (dataptr (), data, size ());
}

/// Retrieve current content of SMA
void shmem_base::get (void* data)
{
  memcpy (data, dataptr (), size ());
}

/*!
  Obtain writer lock and update SMA
  \param pData pointer to data to write in SMA
*/
bool shmem_base::write (const void* pData)
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

/*!
  Obtain a reader lock and retrieve SMA content
  \param pData pointer to data
*/
bool shmem_base::read (void* pData)
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

} // namespace mlib
