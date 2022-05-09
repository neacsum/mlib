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


namespace mlib {

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
  Change the default error facility.
  If called with a NULL argument reverts to generic error facility
*/
void errfac::Default (errfac *facility)
{
  default_facility = facility? facility : &deffac;
}

}

