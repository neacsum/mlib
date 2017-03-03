#pragma once
/*!
  \file SYNCBASE.H syncbase class definition.

  (c) Mircea Neacsu 1999
*/

#include "defs.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Base class for all named synchronization objects
class syncbase
{
public:
  syncbase ();
  syncbase (const syncbase& e);
  virtual ~syncbase ();

  syncbase& operator = (const syncbase& rhs);
  int operator == (const syncbase& rhs) const;

  virtual DWORD wait (DWORD time_limit=INFINITE);
  virtual DWORD wait_alertable (DWORD time_limit=INFINITE);
  virtual DWORD wait_msg (DWORD time_limit=INFINITE, DWORD mask=QS_ALLINPUT);
  virtual operator bool ();
  virtual bool try_wait ();

  /// Return OS handle of this object
  HANDLE handle () const { return hl->handle_; };

  /// Return object's name
  const char *name () const { return name_; };


protected:
  syncbase (const char *name);        //protected constructor
  void set_handle (HANDLE h);
  void set_name (const char *name);

private:
  struct handle_life {
    HANDLE handle_;
    unsigned int life;
  } *hl;
  char *name_;
};

/*!
  Try to wait on the object.

  Returns \b true if the object was in a signaled state.
*/
inline
bool syncbase::try_wait ()
{
  return (wait(0) == WAIT_OBJECT_0);
}


DWORD multiwait (bool all, int count, syncbase** array, DWORD time_limit=INFINITE);
DWORD multiwait_msg (bool all, int count, syncbase** array, DWORD time_limit=INFINITE, DWORD mask=QS_ALLINPUT);
void udelay (unsigned short usec);

#ifdef MLIBSPACE
};  //namespace
#endif
