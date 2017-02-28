#pragma once
/*!
  \file errorcode.h  Definition of erc and errorfacility classes

  Copyright (c) Mircea Neacsu 2000
  Based on an idea from Marc Guillermont (CUJ 05/2000)

*/

#include <mlib/defs.h>

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
  errfac (const char *name=0);
  errfac (const errfac& other);
  virtual       ~errfac();

  /// assignment operator
  errfac&  operator = (const errfac& other);
  
  /// comparison
  virtual int operator == (const errfac& other) const;
  virtual int operator != (const errfac& other) const;

  /// set throw priority
  void          throw_priority (short int pri);

  /// get throw priority
  short int     throw_priority () const        { return throw_level; };

  /// set log priority
  void          log_priority (short int pri);

  /// get log priority
  short int     log_priority () const          { return log_level; };

  /// return message to be logged
  virtual void  message (const erc& e, char *msg, size_t sz) const;

  /// get name
  const char*   name () const                  { return name_; };

  /// set default facility
  static void Default (errfac *f);

  /// get default facility
  static errfac* Default () { return default_facility; };

protected:
  virtual void raise (const erc& e);
  virtual void log (const erc& e);

private:
  short int  log_level;
  short int  throw_level;
  char*      name_;
  static errfac* default_facility;
};

class erc
{
public:
  erc ();
  erc (int value , short int priority=ERROR_PRI_ERROR, errfac* f = 0);
  erc (const erc& other);
  ~erc ();

  erc&           operator= (const erc& other);

                 operator int () const;
  ///return priority value
  short int      priority () const      { return priority_; };

  ///return pointer to facility
  errfac&        facility () const      { return *facility_;};

  erc&           reactivate ();
  erc&           deactivate ();

  /*! Return numerical value.
  As opposed to the integer conversion operator this function doesn't
  change the activity flag*/
  int             code () const          { return value;};

  ///Get logging message
  void            message (char *msg, size_t sz);

private:
  int             value;
  short int       priority_;
  mutable bool    thrown;
  mutable bool    active;
  errfac*         facility_;

friend class errfac;
};

inline
void erc::message(char *msg, size_t sz)
{
  facility_->message (*this, msg, sz);
}

inline
int errfac::operator == (const errfac& other) const
{
  return !strcmp (name_, other.name_);
}

inline
int errfac::operator != (const errfac& other) const
{
  return !operator == (other);
}


namespace MLIBSPACE {

typedef erc errc;
typedef errfac errfacility;

};

/// The SUCCESS indicator
#define ERR_SUCCESS (erc (0, ERROR_PRI_SUCCESS))


