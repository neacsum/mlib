#pragma once
/*!
  \file errorcode.h  Definition of erc and erfac classes

  Copyright (c) Mircea Neacsu 2000
  Based on an idea from Marc Guillermont (CUJ 05/2000)

  \defgroup errors Error Handling
  \brief Unified error handling.

  erc objects are a cross between exceptions and return values. A function
  can return an erc object and the caller can check it just like a regular
  return value as in this example:

```CPP
  erc func ()
  {
    return erc(1, erc::error);
  }

  main () {
    if (func() != 1) {
      ...
    }
  }
```

  However with ercs, if the return result is not checked, it might be
  thrown as an exception as in the following example.

```CPP
  erc func ()
  {
    return erc(1, erc::error);
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
```

  This dual behavior is obtained by having erc objects throwing an exception
  in their destructor if the object is "active". An object is marked as "inactive"
  every time the integer conversion operator is invoked like in the first example.
*/

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>

#if (defined(_MSVC_LANG) && _MSVC_LANG < 201703L)                                                  \
  || (!defined(_MSVC_LANG) && (__cplusplus < 201703L))
#error "errorcode requires c++17"
#endif

namespace mlib {

class erc
{
  friend class errfac;

public:
  /// Error levels (borrowed from BSD Unix)
  enum level
  {
    none = 0, //!< always    not logged,   not thrown
    info,     //!< default   not logged,   not thrown
    notice,   //!< default   not logged,   not thrown
    warning,  //!< default   logged,       not thrown
    error,    //!< default   logged,       thrown
    critical, //!< default   logged,       thrown
    alert,    //!< default   logged,       thrown
    emerg     //!< always    logged,       thrown
  };
  erc ();
  explicit erc (int value, level priority = level::error, const errfac* f = nullptr);
  erc (const erc& other);
  erc (erc&& other);

  ~erc () noexcept (false);
  erc& operator= (const erc& rhs);
  erc& operator= (erc&& rhs);
  operator int () const;

  /// Return priority value
  level priority () const;

  /// Return activity flag
  bool is_active () const;

  /// Return reference to facility
  const errfac& facility () const;

  bool operator== (const erc& other) const;
  bool operator!= (const erc& other) const;

  void raise () const;

  erc& reactivate ();
  erc& deactivate ();

  int code () const;

  /// Get logging message
  std::string message () const;
  void message (const std::string& m);

  static erc success;

private:
  // bit fields
  int value : 24;
  int priority_ : 4;
  mutable int active : 1;

  const errfac* facility_;
  std::string msg;

  friend class errfac;
};

/*!
  An error facility routes a group of errors handled in a
  similar manner.
*/

class errfac
{
public:
  // constructors/destructor
  errfac (const std::string& name = "Error");

  /// set throw priority
  void throw_priority (erc::level pri);

  /// get throw priority
  erc::level throw_priority () const;

  /// set log priority
  void log_priority (erc::level pri);

  /// get log priority
  erc::level log_priority () const;

  /// return message to be logged
  virtual std::string message (const erc& e) const;

  /// get name
  const std::string& name () const;

  /// set default facility
  static void Default (errfac* f);

  /// get default facility
  static errfac& Default ();

  virtual void raise (const erc& e) const;
  virtual void log (const erc& e) const;

private:
  erc::level log_level;
  erc::level throw_level;
  std::string name_;
  static errfac* default_facility;
};

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)                                                 \
  || (!defined(_MSVC_LANG) && (__cplusplus >= 202002L))
template <class T>
concept checkable = !std::is_convertible_v<T, int>;

/*!
  Provides a mechanism similar to [expected](https://en.cppreference.com/w/cpp/utility/expected)
  for creating objects associated with error codes.

  `checked<T>` objects are derived  from `mlib::erc`, so they can be treated as
  regular `erc` objects, in particular they can be compared with an integer to
  check if they contain an error. To access the included `T` object, use the
  '->' or '*' operators.

  \note To avoid conflicts with the erc integer conversion operator, the template
  argument T should **NOT** be convertible to `int`.

  \tparam T - the type of the included object

  \ingroup errors
*/
template <checkable T>
class checked : public erc
{
public:
  /// Default constructor. Invoke T's default constructor and set the default
  /// error code value (0).
  checked ()
    : erc ()
    , obj ()
  {}

  /// Constructor using a T and an error code. Both are copy-constructed
  checked (const T& obj_, const erc& err)
    : erc (err)
    , obj (obj_)
  {}

  ///
  checked (const T& obj_, int value = 0, erc::level pri_ = erc::level::error,
           const errfac* fac_ = nullptr)
    : erc (value, pri_, fac_)
    , obj (obj_)
  {}

  checked (T&& obj_, erc&& err)
    : erc (std::move (err))
    , obj (std::move (obj_))
  {}
  checked (T&& obj_, const erc& err)
    : erc (err)
    , obj (std::move (obj_))
  {}
  checked (T&& obj_, int value = 0, erc::level pri_ = erc::level::error,
           const errfac* fac_ = nullptr)
    : erc (value, pri_, fac_)
    , obj (obj_)
  {}

  /// Copy constructor
  checked (const checked<T>& other)
    : erc (other)
    , obj (other.obj)
  {}

  /// Move constructor
  checked (checked<T>&& other)
    : erc (other)
    , obj (other.obj)
  {}

  ~checked () noexcept (false) = default;

  /// Assignment operator
  checked<T>& operator= (const checked<T>& rhs)
  {
    if (&rhs != this)
    {
      erc::operator= (rhs);
      obj = rhs.obj;
    }
    return *this;
  }

  /// Move assignment operator
  checked<T>& operator= (checked<T>&& rhs)
  {
    if (&rhs != this)
    {
      *(erc*)this = std::move (rhs);
      obj = std::move (rhs.obj);
    }
    return *this;
  }

  /// Set error value
  checked<T>& operator= (const erc& rhs)
  {
    erc::operator= (rhs);
    return *this;
  }

  ///@{
  /// Access the included T object
  T& operator* ()
  {
    if (code () && is_active () && priority ())
      raise ();
    return obj;
  }

  const T& operator* () const
  {
    if (code () && is_active () && priority ())
      raise ();
    return obj;
  }

  T* operator->()
  {
    if (code () && is_active () && priority ())
      raise ();
    return &obj;
  }
  const T* operator->() const
  {
    if (value && active && priority ())
      raise ();
    return &obj;
  }
  ///@}

protected:
  T obj;
};
#endif

//-----------------------  errfac inlines -------------------------------------

/// The default facility
inline errfac deffac;

/// Pointer to default facility
inline errfac* errfac::default_facility = &deffac;

/*!
  \class errfac
  \ingroup errors

  To group handling of erc objects, each erc has associated a _facility_.
  Instead of throwing an exception directly, the erc calls the
  facility's #raise function. In turn, it is this function that decides what
  should happen erc based on facility's log level and throw level.

  There is also a default facility that is used when the erc doesn't have
  an explicit facility.
*/

///  Set defaults for log and throw levels.
inline errfac::errfac (const std::string& name)
  : throw_level (erc::level::error)
  , log_level (erc::level::warning)
  , name_ (name)
{}

inline void errfac::throw_priority (erc::level pri)
{
  throw_level = (pri < erc::info) ? erc::info : (pri > erc::emerg) ? erc::emerg : pri;
}

inline erc::level errfac::throw_priority () const
{
  return throw_level;
}

inline void errfac::log_priority (erc::level pri)
{
  log_level = (pri < erc::info) ? erc::info : (pri > erc::emerg) ? erc::emerg : pri;
}

inline erc::level errfac::log_priority () const
{
  return log_level;
}

inline const std::string& errfac::name () const
{
  return name_;
}

/// Default message is "error <code>"
inline std::string errfac::message (const erc& e) const
{
  return std::string ("error ") + std::to_string (e.value);
}

inline errfac& errfac::Default ()
{
  return *default_facility;
}

/*!
  Change the default error facility.
  If called with a NULL argument reverts to generic error facility
*/
inline void errfac::Default (errfac* facility)
{
  default_facility = facility ? facility : &deffac;
}

/*!
  Check if error must be logged or thrown.
  This function is called by an active error (in destructor of erc objects
  or assignment operator).

  The typical action chain is:
  erc destructor -> errfac::raise -> erc is thrown
*/
inline void errfac::raise (const erc& e) const
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

/// Logging action. Default is to use stderr
/// Message is "<facility name> - <erc message>\n"
inline void errfac::log (const erc& e) const
{
  fprintf (stderr, "%s - %s\n", name ().c_str (), e.message ().c_str ());
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
inline erc::erc ()
  : value{0}
  , priority_{none}
  , active{false}
  , facility_{&errfac::Default ()}
{}

///  Ctor for a real erc
inline erc::erc (int v, level l, const errfac* f)
  : value{v}
  , priority_{(unsigned short)l}
  , facility_{f ? f : &errfac::Default ()}
  , active{true}
{}
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
inline erc::erc (const erc& other)
  : value{other.value}
  , priority_{other.priority_}
  , active{other.active}
  , facility_{other.facility_}
  , msg{other.msg}
{
  // we are the active error now, the other is deactivated
  other.active = 0;
}

///  Move constructor removes the activity flag of the original object
inline erc::erc (erc&& other)
  : value{other.value}
  , priority_{other.priority_}
  , active{other.active}
  , facility_{other.facility_}
  , msg{other.msg}
{
  // we are the active error now, the other is deactivated
  other.active = 0;
}

///  Destructor. Call raise() function to see if the error should get logged or thrown.
inline erc::~erc () noexcept (false)
{
  raise ();
}

/*!
  Principal assignment operator.

  If we were active before, call the facility to log or throw.
  Anyhow copy new values from the assigned object and take away it's active
  flag.

  \note It is rather bad practice to assign to an active `erc` object. Here we
  take the view that, since the left side object was already active, we have
  to deal with it first.
*/
inline erc& erc::operator= (const erc& rhs)
{
  if (&rhs != this)
  {
    int rhs_active = rhs.active;
    rhs.active = 0; // prevent rhs from throwing if we throw
    if (active && priority_)
      facility_->raise (*this);
    value = rhs.value;
    priority_ = rhs.priority_;
    facility_ = rhs.facility_;
    msg = rhs.msg;
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
inline erc& erc::operator= (erc&& rhs)
{
  if (&rhs != this)
  {
    bool rhs_active = rhs.active;
    rhs.active = 0; // prevent rhs from throwing if we throw
    if (active && value && priority_)
      facility_->raise (*this);
    value = rhs.value;
    priority_ = rhs.priority_;
    facility_ = rhs.facility_;
    msg = rhs.msg;
    active = rhs_active;
  }
  return *this;
}

inline erc::level erc::priority () const
{
  return (erc::level)priority_;
}

inline bool erc::is_active () const
{
  return active;
}

inline const errfac& erc::facility () const
{
  return *facility_;
}

/*!
  Equality comparison operator

  All success codes are equal. Other codes are equal only if their value, level
  and facility are equal.

  Resets the activity flag.
*/
inline bool erc::operator== (const erc& other) const
{
  active = false;
  if ((!priority_ || !value) && (!other.priority_ || !other.value))
    return true; // success values are the same
  if (facility_ == other.facility_ && priority_ == other.priority_ && value == other.value)
    return true;

  return false;
}

/*!
  Inequality comparison operator

  All success codes are equal. Other codes are equal only if their value, level
  and facility are equal.

  Resets the activity flag.
*/
inline bool erc::operator!= (const erc& other) const
{
  return !operator== (other);
}

/// Invoke facility's raise function (errfac::raise) to determine if error code
/// should be logged or thrown
inline void erc::raise () const
{
  if (value && active && priority_)
    facility_->raise (*this);
}

/*!
  Return numerical value.

  As opposed to the integer conversion operator, this function doesn't
  change the activity flag.
*/
inline int erc::code () const
{
  return value;
}

/*!
  Return message string associated this error.

  If no message string has been attached to this object, it calls
  errfac::message() function to generate the message string.
*/
inline std::string erc::message () const
{
  return msg.empty () ? facility_->message (*this) : msg;
}

/// Set the message for this error.
inline void erc::message (const std::string& m)
{
  msg = m;
}

/*!
  Integer conversion operator.

  Assume the error has been dealt with and reset the active flag.
*/
inline erc::operator int () const
{
  active = 0;
  return value;
}

///  Similar to re-throwing an exception.
inline erc& erc::reactivate ()
{
  active = 1;
  return *this;
}

/*!
  Marks error code as inactive.

  Useful in catch clauses when we don't really care what the code value is.
*/
inline erc& erc::deactivate ()
{
  active = 0;
  return *this;
}

/// The SUCCESS indicator
inline erc erc::success{0, erc::none};

} // namespace mlib
