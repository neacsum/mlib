#pragma once
/*!
  \file EVENT.H event class definition.

	(c) Mircea Neacsu 1999

*/

#include "syncbase.h"

namespace mlib {

class event : public syncbase
{
public:
  ///Event modes
  enum mode { manual, automatic };

  event   (mode m=automatic, bool signaled=false, const char *name=NULL);
  event& operator =(const event& rhs);

  ///Set event to signaled state
  void   signal  ()        {SetEvent (handle ());};

  ///Pulse event so that only one waiting thread is released
  void   pulse   ()        {PulseEvent (handle ());};

  ///Set event to non-signaled state
  void   reset   ()        {ResetEvent (handle ());};

  operator bool ();
private:
  mode m;     ///< event mode (manual or automatic)
};

};

