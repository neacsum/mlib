#include <utpp/utpp.h>
#include <mlib/errorcode.h>

#include <iostream>

using namespace MLIBSPACE;
using namespace std;

SUITE (errorcode)
{
  erc f (int i) {return erc (i);}
  erc g (int i) {return erc (i, ERROR_PRI_WARNING);}

  // Default erc objects are thrown
  TEST (erc_trhow)
  {
    CHECK_THROW (erc, f (2));
  }

  // Integer conversion operator deactivates the erc
  TEST (erc_to_int)
  {
    int i = f (2);
    CHECK_EQUAL (2, i);
  }

  // Low priority erc objects are not thrown
  TEST (low_pri)
  {
    g (1);
  }

  // Can change facility's throw priority
  TEST (facility_pri)
  {
    short p = errfac::Default ()->throw_priority ();
    errfac::Default ()->throw_priority (ERROR_PRI_WARNING);
    CHECK_THROW (erc, g (1));
    errfac::Default ()->throw_priority (p); //restore previous throw priority
    g (1);
  }

  // erc message
  TEST (erc_message)
  {
    erc r = f (1);
    string s = r.message ();
    CHECK_EQUAL ("Error 1", s);
    r.deactivate ();
  }

  errfac other{ "Bad Stuff" };
  erc ff (int i)
  {
    return erc (i, ERROR_PRI_ERROR, &other);
  }

  // erc objects using another facility
  TEST (other_facility)
  {
    erc r = ff (3);
    string s = r.message ();
    CHECK_EQUAL ("Bad Stuff 3", s);
    CHECK_EQUAL (3, r); // comparison invokes integer conversion and deactivates
                        // the error code
  }
}


