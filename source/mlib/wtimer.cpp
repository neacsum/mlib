/*!
  \file wtimer.cpp Implementation of timer class

	(c) Mircea Neacsu 1999

*/
#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/wtimer.h>
#include <assert.h>
#include <mlib/utf8.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \class timer
  \brief Waitable %timer.
  \ingroup syncro

  A waitable %timer object is set to signaled when the specified due time
  arrives. There are two types of waitable timers that can be created: 
  manual and automatic (called synchronization timers in Windows documentation).
  Automatic timers are reset to thier non-signaled state when a %thread 
  completes a wait operation on the object (similar to automatic events).

  Manual timers remain signaled until restarted.
  
  A %timer of either type can also be a periodic %timer.

  When signaled, waitable timers can call the #at_timer function as an APC 
  (Asyncronous Procedure Call) to be executed in the context of the %thread
  that created the %timer. To use this feature you need to derive another class
  that reimplements the #at_timer function.
*/

/*!
  \param  m       timer type (manual or automatic)
  \param  name    object's name
  \param  use_apc \b true if object's at_timer function should be called
*/
timer::timer(mode m, const char *name, bool use_apc) :
  syncbase( name ),
  apc_ (use_apc)
{
  HANDLE h=CreateWaitableTimer( NULL, (m==manual), name?widen(name).c_str():0 );

  assert (h);
  set_handle( h );
}

/*!
  Set the start time of the %timer and it's period.
  \param  interval time in milliseconds from now when object should be signaled
  \param  period  timer's period (0 if it's a one-shot)
*/
void timer::start( DWORD interval, DWORD period )
{
  LARGE_INTEGER time;
  interval = -abs((int)interval);    //make sure it's a negative value;
  time.QuadPart = Int32x32To64( interval, 10000 );
  SetWaitableTimer( handle(), &time, period, 
    apc_?(PTIMERAPCROUTINE)timerProc:0, this, FALSE );
}

/*!
  Set the timer at an absolute time.
  \param  utctime UTC time when object should become signaled
  \param  period  timer's period (0 if it's a one-shot)
*/
void timer::at (FILETIME& utctime, DWORD period)
{
  SetWaitableTimer( handle(), (LARGE_INTEGER *)&utctime, period, 
    apc_?(PTIMERAPCROUTINE)timerProc:0, this, FALSE );
}

/*!
  Stop the %timer.
*/
void timer::stop ()
{
  CancelWaitableTimer (handle());
}

/*!
  glue function that calls the at_timer function.
*/
void CALLBACK timer::timerProc( timer *obj, DWORD loval, DWORD hival )
{
  obj->at_timer( loval, hival );
}

#ifdef MLIBSPACE
};
#endif
