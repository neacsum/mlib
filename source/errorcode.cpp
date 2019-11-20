/*!
    \file errorcode.cpp  Implementation of erc and errfac classes

    Copyright (c) Mircea Neacsu 2000
    Based on an idea from Marc Guillermont (CUJ 05/2000)
*/
#include <mlib/errorcode.h>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <mlib/trace.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


/*!
  \defgroup errors Error Error Handling
  \brief Unified error handling.

  erc objects are a cross between exceptions and return values. A function
  can return an erc object and the caller can check it just like a regular
  return value as in this example:
\code
  erc func ()
  {
    return erc(1);
  }

  main () {
    if (func() != 1) {
      ...
    }
  }
\endcode

  However with ercs, if the return result is not checked, it might be
  thrown as an exception as in the following example.
\code
  erc func ()
  {
    return erc(1);
  }
  main () {
    try {
      func ();
      ...
    }
    catch (erc& err) {
     printf ("func result %d", (int)err);
    }
  }
\endcode

  This dual behavior is obtained by having erc objects throwing an exception
  in their destructor if the object is "active". An object is marked as "inactive"
  every time the integer conversion operator is invoked like in the first example.
*/


///The default facility
static errfac deffac;

///Pointer to default facility
errfac* errfac::default_facility = &deffac;


/*!
  \class errfac
  \ingroup errors

  To facilitate handling of ercs, each erc has associated a "facility".
  Instead of throwing an exception directly the erc calls the facility's
  #raise function. In turn, it is this function that decides what should happen
  with the erc based on facility's log level and throw level.

  There is also a default facility that is used when the erc doesn't have
  an explicit facility.
*/

/*!
  Set defaults for log and throw levels.
*/
errfac::errfac (const std::string& name)
  : throw_level (ERROR_PRI_ERROR)
  , log_level (ERROR_PRI_WARNING)
  , name_ (name)
{
}


/*!
  Throw priority must be between ERROR_PRI_SUCCESS and ERROR_PRI_EMERG
  \ref ERROR_PRI "see Error Priorities"
*/
void errfac::throw_priority (unsigned int level)
{
  throw_level = (level < ERROR_PRI_INFO)  ? ERROR_PRI_INFO :
                (level > ERROR_PRI_EMERG) ? ERROR_PRI_EMERG :
                                            level;
}

/*!
  Logging priority must be between ERROR_PRI_SUCCESS and ERROR_PRI_EMERG
  \ref ERROR_PRI "see Error Priorities"
*/
void errfac::log_priority (unsigned int level)
{
  log_level = (level < ERROR_PRI_INFO)  ? ERROR_PRI_INFO :
              (level > ERROR_PRI_EMERG) ? ERROR_PRI_EMERG :
                                          level;
}

/*!
  Check if error must be logged or thrown.
  This function is called by an active error (in destructor of erc objects
  or assignment operator).

  The typical action chain is:
  erc destructor -> errfac::raise -> erc is thrown
*/
void errfac::raise (const erc& e)
{
  if (e.priority_ >= log_level)
    log (e);
  if (e.priority_ >= throw_level)
  {
    // Make sure this erc is not thrown again
    e.active = false;
    throw e;
  }
}

/// Default message the facility name followed by the error code value
std::string errfac::message (const erc& e) const
{
  return name_ + ' ' + std::to_string (e.value);
}


/// Logging action. Default is to use dprintf
void errfac::log( const erc& e )
{
  std::string msg{ "Log -- " + message (e) + "\n" };
  dprintf (msg.c_str());
}

/*!
  Change the default error facility.
  If called with a NULL argument reverts to generic error facility
*/
void errfac::Default (errfac *facility)
{
  default_facility = facility? facility : &deffac;
}


/*!
  \class erc
  \ingroup errors
  \brief objects returned as a function result or thrown directly. 
  
  If not tested and their level is above facility's logging level they will 
  be logged ( errfac::log() ). If the level is above facility's throwing
  level they will be thrown as exceptions.
*/

/*!
  Default ctor for erc objects creates an inactive error
*/
erc::erc() :
  value (0),
  priority_ (ERROR_PRI_SUCCESS),
  active (false),
  facility_ (errfac::Default ())
{
}

/*!
  Ctor for a real erc
*/
erc::erc (int v, short int l, errfac* f) :
  value (v),
  priority_ (l),
  facility_ (f? f : errfac::Default ()),
  active (true)
{
}

erc::erc (erc&& other) :
  value (other.value),
  priority_ (other.priority_),
  active (other.active),
  facility_ (other.facility_)
{
  TRACE ("erc move ctor");
  //we become the active error, the other is deactivated
  other.active = false;
}

/*! 
  Dtor. If we are active, call our facility to see if we get logged
  or thrown.
*/
erc::~erc () noexcept(false)
{
  if (value && active && priority_)
    facility_->raise (*this);
}

/*!
  Move assignment operator.
  
  If we were active before, call the facility to log or throw. 
  Anyhow copy new values from the assigned object and take away it's active
  flag.
*/
erc& erc::operator= (erc&& rhs)
{
  TRACE ("erc move assignment");
  bool rhs_active = rhs.active;
  rhs.active = false; //prevent rhs from throwing if we throw
  if (active && priority_)
    facility_->raise (*this);
  value = rhs.value;
  priority_ = rhs.priority_;
  facility_ = rhs.facility_;
  active = rhs_active;
  return *this;
}

/*!
  Integer conversion operator. 
  
  Assume the error has been dealt with and reset the active flag. 
*/
erc::operator int () const
{
  active = false;
  return value;
}

/*!
  Similar to rethrowing an exception.
*/
erc& erc::reactivate ()
{
  active = true;
  return *this;
}

/*!
  Marks error code as inactive.

  Useful in catch clauses when we don't really care what the code value is.
*/
erc& erc::deactivate ()
{
  active = false;
  return *this;
}

#ifdef MLIBSPACE
}
#endif

