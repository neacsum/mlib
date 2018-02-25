/*!
  \file SYNCBASE.CPP Member functions of syncbase class

  (c) Mircea Neacsu 1999-2017

*/
#include <assert.h>
#include <mlib/syncbase.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif
/*!
  \defgroup syncro  Synchronization Objects
  \brief Wrapper classes for Windows synchronization mechanisms.
*/

/*!
  \class syncbase
  \ingroup syncro

  The Windows handle has an instance counter so copying and assignment are
  safe and rather cheap (doesn't call DuplicateHandle).

  The virtual bool conversion operator allows testing the signaled state of the
  object (it is true if object is signaled).
*/

/// Default constructor
syncbase::syncbase () :
hl (NULL),
name_ (NULL)
{
}

/// Protected constructor
syncbase::syncbase (const char *a_name) :
hl (new handle_life),
name_ (a_name?strcpy (new char[strlen(a_name)+1], a_name):NULL)
{
  hl->handle_ = NULL;
  hl->life = 1;
}

/// Copy constructor
syncbase::syncbase (const syncbase& other) :
hl (other.hl),
name_ (other.name_?strcpy (new char[strlen(other.name_)+1],other.name_):NULL)
{
  hl->life++;
}


/// Destructor
syncbase::~syncbase ()
{
  if (hl && --hl->life == 0)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
    delete hl;
    hl = 0;
  }
  delete []name_;
}

/// Assignment operator
syncbase& syncbase::operator =(const syncbase& rhs)
{
  if (&rhs == this)
    return *this;       //trivial assignment

  if (hl && --hl->life == 0)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
    delete hl;
  }
  delete name_;
  hl = rhs.hl;
  if (hl)
    hl->life++;
  name_ = rhs.name_?strcpy (new char[strlen(rhs.name_)+1], rhs.name_):NULL;
  return *this;
}

/// Equality operator
int syncbase::operator ==(const syncbase& rhs) const
{
  return (!hl && !rhs.hl) || (hl->handle_ == rhs.hl->handle_);
}

/// Wait for the object to become signaled
DWORD syncbase::wait (DWORD time_limit)
{
  assert (hl->handle_);
  return WaitForSingleObject (hl->handle_, time_limit);
}

/// Wait for object to become signaled or an APC or IO completion routine 
/// to occur
DWORD syncbase::wait_alertable (DWORD time_limit)
{
  assert (hl->handle_);
  return WaitForSingleObjectEx (hl->handle_, time_limit, TRUE);
}

/// Change object's handle. Closes the previous one.
void syncbase::set_handle (HANDLE h)
{
  if (hl)
  {
    if (hl->handle_)
      CloseHandle (hl->handle_);
  }
  else
  {
    hl = new handle_life;
    hl->life = 1;
  }
  hl->handle_ = h;
}

/// Change object's name
void syncbase::set_name(const char *name)
{
  delete []name_;
  name_ = name?strcpy (new char[strlen(name)+1],name):NULL;
}

/// Check if object is signaled
syncbase::operator bool ()
{
  return (WaitForSingleObject (hl->handle_, 0) == WAIT_OBJECT_0);
}

/// Wait for object to become signaled or a message to be queued
DWORD syncbase::wait_msg (DWORD time_limit, DWORD mask)
{
  return MsgWaitForMultipleObjects (1, &hl->handle_, FALSE, time_limit, mask);
}

/*!
  \ingroup syncro

  Wait for multiple objects
  \param  all     if true wait for all objects to become signaled
  \param  count   number of objects in \p array 
  \param  array   array of objects to wait for
  \param  time_limit  time limit in milliseconds 
*/
DWORD multiwait (bool all, int count, syncbase** array, DWORD time_limit)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i=0; i<count; i++)
    harr[i] = array[i]->handle();

  DWORD result = WaitForMultipleObjects (count, harr, all, time_limit);
  return result;
}

/*!
  \ingroup syncro
  
  Wait for multiple objects or a message to be queued
  \param  all     if true wait for all objects to become signaled
  \param  count   number of objects in \p array 
  \param  array   array of objects to wait for
  \param  time_limit  time limit in milliseconds
  \param  mask    message mask (combination of QS_... constants)
*/
DWORD multiwait_msg (bool all, int count, syncbase** array, DWORD time_limit, DWORD mask)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i=0; i<count; i++)
    harr[i] = array[i]->handle();

  DWORD result = MsgWaitForMultipleObjects (count, harr, all, time_limit, mask);
  return result;
}

/// Busy waiting for a number of microseconds
void udelay (unsigned short usec)
{
  LARGE_INTEGER interval, crt;
  static LARGE_INTEGER freq;
  if (!freq.QuadPart)
    QueryPerformanceFrequency (&freq);
  QueryPerformanceCounter (&interval);
  interval.QuadPart += usec * 1000000L / freq.QuadPart;
  do {
    QueryPerformanceCounter (&crt);
  } while (crt.QuadPart < interval.QuadPart);
}

#ifdef MLIBSPACE
};
#endif