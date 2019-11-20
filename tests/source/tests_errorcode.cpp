#include <utpp/utpp.h>
#include <mlib/errorcode.h>

#include <iostream>

using namespace MLIBSPACE;
using namespace std;

SUITE (errorcode)
{
  erc f (int i) {return i;}
  erc g (int i) {return erc (i, ERROR_PRI_WARNING);}
  erc ff () 
  {
    erc fret;
    fret = f (2);
    return fret;
  }

  // Default erc objects are thrown
  TEST (erc_trhow)
  {
    CHECK_THROW (erc, f (2));
  }

  // Assigned erc object is thrown
  TEST (erc_assign)
  {
    CHECK_THROW (erc, ff ());
  }

  // Integer conversion operator deactivates the erc
  TEST (erc_to_int)
  {
    int i = f (2);
    CHECK_EQUAL (2, i);
    cout << "Size of erc is " << sizeof (erc) << endl;
  }

  //Reactivated erc objects are thrown
  TEST (erc_reactivate)
  {
    int caught = 0;
    erc r;
    try {
      erc rr;
      rr = ff ();
      CHECK_EQUAL (2, rr);
      rr.reactivate ();
    }
    catch (erc& x)
    {
      caught = 1;
      r = x;
    }
    CHECK_EQUAL (1, caught);
    CHECK_EQUAL (2, r);
  }

  //Assign to active erc throws
  TEST (assign_to_active)
  {
    int caught = 0;
    try {
      erc r = ff (); //r is 2 now
      r = f (3); //should throw a 2
    }
    catch (erc& x)
    {
      caught = 1;
      CHECK_EQUAL (2, x);
    }
    CHECK_EQUAL (1, caught);
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


