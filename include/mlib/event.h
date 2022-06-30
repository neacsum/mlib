#pragma once
/*!
  \file event.h event class definition.

	(c) Mircea Neacsu 1999

*/

#include "syncbase.h"

namespace mlib {

class event : public syncbase
{
public:
  ///Event modes
  enum mode { manual, automatic };

  event (mode m = automatic, bool signaled = false, const std::string& name = std::string());
  event& operator =(const event& rhs);

  ///Set event to signaled state
  void   signal  ()        {SetEvent (handle ());};

  ///Pulse event so that only one waiting thread is released
  void   pulse   ()        {PulseEvent (handle ());};

  ///Set event to non-signaled state
  void   reset   ()        {ResetEvent (handle ());};

  ///Check if event is signaled
  bool is_signaled ();

private:
  mode m;     ///< event mode (manual or automatic)
};

};

