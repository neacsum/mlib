#pragma once
/*!
  \file errorcode.h  Definition of erc and erfac classes

  Copyright (c) Mircea Neacsu 2000
  Based on an idea from Marc Guillermont (CUJ 05/2000)

*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


/*! 
  \anchor ERROR_PRI
  \name Error priorities (borrowed from BSD Unix)
 \{
*/
#define ERROR_PRI_SUCCESS     0  ///< always    not logged,   not thrown
#define ERROR_PRI_INFO        1  ///< default   not logged,   not thrown
#define ERROR_PRI_NOTICE      2  ///< default   not logged,   not thrown
#define ERROR_PRI_WARNING     3  ///< default   logged,       not thrown
#define ERROR_PRI_ERROR       4  ///< default   logged,       thrown
#define ERROR_PRI_CRITICAL    5  ///< default   logged,       thrown
#define ERROR_PRI_ALERT       6  ///< default   logged,       thrown
#define ERROR_PRI_EMERG       7  ///< always    logged,       thrown
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
  static errfac* Default ();

protected:
  virtual void raise (const erc& e);
  virtual void log (const erc& e);

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
  erc (int value , short int priority=ERROR_PRI_ERROR, errfac* f = 0);
  erc (const erc& other) = default;
  erc (erc&& other);

  ~erc () noexcept(false);
  erc& operator= (const erc& rhs) = default;
  erc& operator= (erc&& rhs);
  operator int () const;
  
  ///Return priority value
  unsigned int priority () const;

  ///Return reference to facility
  errfac& facility () const;

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

  errfac*         facility_;

friend class errfac;
};

inline
unsigned int  errfac::throw_priority () const 
{
  return throw_level;
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

inline
errfac* errfac::Default ()
{
  return default_facility;
}

inline
unsigned int erc::priority () const
{
  return priority_;
}

inline
errfac& erc::facility () const 
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


#ifdef MLIBSPACE
}
#endif

/// The SUCCESS indicator
#define ERR_SUCCESS (MLIBSPACE::erc (0, ERROR_PRI_SUCCESS))


