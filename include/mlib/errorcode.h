#pragma once
/*!
  \file errorcode.h  Definition of erc and erfac classes

  Copyright (c) Mircea Neacsu 2000
  Based on an idea from Marc Guillermont (CUJ 05/2000)

*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <mlib/trace.h>

#include <string>

namespace mlib {

class erc;

/*! 
  \anchor ERROR_PRI
  \name Error priorities (borrowed from BSD Unix)
 \{
*/
#define ERROR_PRI_SUCCESS     0  //!< always    not logged,   not thrown
#define ERROR_PRI_INFO        1  //!< default   not logged,   not thrown
#define ERROR_PRI_NOTICE      2  //!< default   not logged,   not thrown
#define ERROR_PRI_WARNING     3  //!< default   logged,       not thrown
#define ERROR_PRI_ERROR       4  //!< default   logged,       thrown
#define ERROR_PRI_CRITICAL    5  //!< default   logged,       thrown
#define ERROR_PRI_ALERT       6  //!< default   logged,       thrown
#define ERROR_PRI_EMERG       7  //!< always    logged,       thrown
/// \}

/*! 
  An error facility routes a group of errors handled in a
  similar manner.
*/

class errfac
{
  friend class erc;

public:
  // constructors/destructor
  errfac (const std::string& name = "Error");
  
  /// set throw priority
  void throw_priority (unsigned int pri);

  /// get throw priority
  unsigned int throw_priority () const;

  /// set log priority
  void log_priority (unsigned int pri);

  /// get log priority
  unsigned int log_priority () const;

  /// return message to be logged
  virtual std::string message (const erc& e) const;

  /// get name
  const std::string& name () const;

  /// set default facility
  static void Default (errfac *f);

  /// get default facility
  static errfac& Default ();

  virtual void raise (const erc& e) const;
  virtual void log (const erc& e) const;

private:
  unsigned int    log_level;
  unsigned int    throw_level;
  std::string     name_;
  static errfac*  default_facility;
};

class erc
{
public:
  erc ();
  erc (int value, short int priority=ERROR_PRI_ERROR, const errfac* f = 0);
  erc (const erc& other);
  erc (erc&& other);

  ~erc () noexcept(false);
  erc& operator= (const erc& rhs);
  erc& operator= (erc&& rhs);
  operator int () const;
  
  ///Return priority value
  unsigned int priority () const;

  ///Return reference to facility
  const errfac& facility () const;

  erc& reactivate ();
  erc& deactivate ();

  int code () const;

  ///Get logging message
  std::string  message () const;

private:
  //bit fields
  int             value : 24;
  unsigned int    priority_ : 4;
  mutable unsigned int active : 1;

  const errfac*   facility_;

friend class errfac;
};

//-----------------------  errfac inlines -------------------------------------

/*!
  \class errfac
  \ingroup errors

  To centralize and facilitate handling of erc objects, each erc has associated
  a _facility_. Instead of throwing an exception directly, the erc calls the
  facility's #raise function. In turn, it is this function that decides what
  should happen erc based on facility's log level and throw level.

  There is also a default facility that is used when the erc doesn't have
  an explicit facility.
*/

///  Set defaults for log and throw levels.
inline
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
inline
void errfac::throw_priority (unsigned int level)
{
  throw_level = (level < ERROR_PRI_INFO) ? ERROR_PRI_INFO :
    (level > ERROR_PRI_EMERG) ? ERROR_PRI_EMERG :
    level;
}

inline
unsigned int  errfac::throw_priority () const
{
  return throw_level;
}

/*!
  Logging priority must be between ERROR_PRI_SUCCESS and ERROR_PRI_EMERG
  \ref ERROR_PRI "see Error Priorities"
*/
inline
void errfac::log_priority (unsigned int level)
{
  log_level = (level < ERROR_PRI_INFO) ? ERROR_PRI_INFO :
    (level > ERROR_PRI_EMERG) ? ERROR_PRI_EMERG :
    level;
}

inline
unsigned int errfac::log_priority () const
{
  return log_level;
}

inline
const std::string& errfac::name () const
{
  return name_;
}

/// Default message is facility name followed by the error code value
inline
std::string errfac::message (const erc& e) const
{
  return name_ + ' ' + std::to_string (e.value);
}

inline
errfac& errfac::Default ()
{
  return *default_facility;
}

/*!
  Check if error must be logged or thrown.
  This function is called by an active error (in destructor of erc objects
  or assignment operator).

  The typical action chain is:
  erc destructor -> errfac::raise -> erc is thrown
*/
inline
void errfac::raise (const erc& e) const
{
  if (e.priority_ >= log_level)
    log (e);
  if (e.priority_ >= throw_level)
  {
    // Make sure this erc is not thrown again
    e.active = 0;
    throw e;
  }
}

/// Logging action. Default is to use dprintf
inline
void errfac::log (const erc& e) const
{
  dprintf (message (e).c_str ());
}


//--------------------------- erc inlines -------------------------------------

/*!
  \class erc
  \ingroup errors
  \brief objects returned as a function result or thrown directly.

  If not tested and their level is above facility's logging level they will
  be logged (by calling errfac::log() ). If the level is above facility's throwing
  level they will be thrown as exceptions.
*/

///  Default ctor for erc objects creates an inactive error
inline
erc::erc () :
  value (0),
  priority_ (ERROR_PRI_SUCCESS),
  active (0),
  facility_ (&errfac::Default ())
{
}

///  Ctor for a real erc
inline
erc::erc (int v, short int l, const errfac* f) :
  value (v),
  priority_ (l),
  facility_ (f ? f : &errfac::Default ()),
  active (1)
{
}
/*!
  Copy constructor removes the activity flag of the original object.

  Having two erc's active at the same time is a big no-no: when one of them
  is thrown, the stack unwinding process invokes the destructor of the other
  one, which in turn might throw again. Throwing an exception during stack
  unwinding will terminate your application. For more details see:
  https://isocpp.org/wiki/faq/exceptions#dtors-shouldnt-throw

  However, the whole concept of erc's is based upon destructors throwing
  exceptions. Here we carefully navigate between a rock and a hard place.
*/
inline
erc::erc (const erc& other) :
  value (other.value),
  priority_ (other.priority_),
  active (other.active),
  facility_ (other.facility_)
{
  TRACE9 ("erc copy ctor");
  //we are the active error now, the other is deactivated
  other.active = 0;
}

///  Move constructor removes the activity flag of the original object
inline
erc::erc (erc&& other) :
  value (other.value),
  priority_ (other.priority_),
  active (other.active),
  facility_ (other.facility_)
{
  TRACE9 ("erc move ctor");
  //we are the active error now, the other is deactivated
  other.active = 0;
}

///  Destructor. If we are active, call our facility to see if we get logged or thrown.
inline
erc::~erc () noexcept(false)
{
  if (value && active && priority_)
    facility_->raise (*this);
}

/*!
  Principal assignment operator.

  If we were active before, call the facility to log or throw.
  Anyhow copy new values from the assigned object and take away it's active
  flag.

  \note It is rather bad practice to assign to an active erc object. Here we
  take the view that, since the left side object was already active, we have
  to deal with it first.
*/
inline
erc& erc::operator= (const erc& rhs)
{
  if (&rhs != this)
  {
    TRACE9 ("erc assignment");
    int rhs_active = rhs.active;
    rhs.active = 0; //prevent rhs from throwing if we throw
    if (active && priority_)
      facility_->raise (*this);
    value = rhs.value;
    priority_ = rhs.priority_;
    facility_ = rhs.facility_;
    active = rhs_active;
  }
  return *this;
}

/*!
  Move assignment operator.

  If we were active before, call the facility to log or throw.
  Anyhow copy new values from the assigned object and take away it's active
  flag.
*/
inline
erc& erc::operator= (erc&& rhs)
{
  if (&rhs != this)
  {
    TRACE9 ("erc move assignment");
    bool rhs_active = rhs.active;
    rhs.active = 0; //prevent rhs from throwing if we throw
    if (active && value && priority_)
      facility_->raise (*this);
    value = rhs.value;
    priority_ = rhs.priority_;
    facility_ = rhs.facility_;
    active = rhs_active;
  }
  return *this;
}

inline
unsigned int erc::priority () const
{
  return priority_;
}

inline
const errfac& erc::facility () const 
{
  return *facility_;
}

/*!
  Return numerical value.

  As opposed to the integer conversion operator, this function doesn't
  change the activity flag.
*/
inline
int erc::code () const
{
  return value;
}

inline
std::string erc::message () const
{
  return facility_->message (*this);
}

/*!
  Integer conversion operator.

  Assume the error has been dealt with and reset the active flag.
*/
inline
erc::operator int () const
{
  active = 0;
  return value;
}

///  Similar to re-throwing an exception.
inline
erc& erc::reactivate ()
{
  active = 1;
  return *this;
}

/*!
  Marks error code as inactive.

  Useful in catch clauses when we don't really care what the code value is.
*/
inline
erc& erc::deactivate ()
{
  active = 0;
  return *this;
}


}
/// The SUCCESS indicator
#define ERR_SUCCESS (mlib::erc (0, ERROR_PRI_SUCCESS))



