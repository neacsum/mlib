#include <mlib/mlib.h>
#include <utpp/utpp.h>
#pragma hdrstop

#include <iostream>

using namespace mlib;
using namespace std;

SUITE (errorcode)
{

erc f (int i) {return erc(i);}
erc g (int i) {return erc (i, erc::warning);}
erc ff () 
{
  erc fret;
  fret = f (2);
  return fret;
}

erc gg (int i)
{
  return erc (i, erc::error); // return a throwing error code
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

  // erc stuff is packed in one integer (plus one pointer to facility)
  // let's verify the object size
#if 0
  //TODO - find why doesn't work in 32-bit 
  struct S { int i; void* p; };
  size_t sz = sizeof (erc);
  CHECK_EQUAL (sizeof (S), sz);
#endif
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
  auto p = errfac::Default ().throw_priority ();
  errfac::Default ().throw_priority (erc::warning);
  CHECK_THROW (erc, g (1));
  errfac::Default ().throw_priority (p); //restore previous throw priority
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
  return erc (i, erc::error, &other);
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

TEST (copy_elision)
{
  /* copy elision rules (https://en.cppreference.com/w/cpp/language/copy_elision)
  require only one constructor to be invoked*/
  erc r = gg (2);
  CHECK_EQUAL (2, r);
  r = gg (2); //move assignment invoked here. r takes activity flag from 
              //object returned by gg.
  r.deactivate ();
}

TEST (erc_assignment)
{
  erc rf, rg;
  rf = ff (3);
  rg = gg (2);

  // assigning to an active erc throws
  CHECK_THROW (erc, (rf = rg));
}

TEST (erc_copy)
{
  erc rf = ff (3);

  erc rf1 (rf);
  CHECK (rf1 == rf);

  rf1.deactivate ();
}

TEST (erc_equal)
{
  erc s0 (0, erc::none), s1 (1, erc::none);
  CHECK_EQUAL (s0, s1);

  erc w0 (0, erc::warning);
  CHECK_EQUAL (s0, w0);
  CHECK_EQUAL (s1, w0);

  erc w1 (1, erc::warning), w2 (2, erc::warning);
  CHECK (w1 != w2);
}

checked<string> cc (const string& s, int v)
{
  return {s, v};
}

TEST (checked_basic)
{
  checked<string> c1; //Default constructed
  CHECK (c1->empty ()); //verify also that it's not throwing
  CHECK_EQUAL (0, c1); // erc value is 0

  const checked<string> c2{"stuff", 1, erc::info};
  CHECK_EQUAL ("stuff", *c2); //also not throwing
  CHECK_EQUAL (1, c2);

  checked<string> c3{"stuff", 1}; //this will throw
  bool thrown = false;
  try {
    string s = *c3;
  }
  catch (erc& x) {
    CHECK_EQUAL (1, x);
    thrown = true;
  }
  CHECK (thrown);

  const checked<string> c4{"stuff"};
  string s = *c4 + "abc";
  CHECK_EQUAL ("stuffabc", s);
}

TEST (checked_copy)
{
  auto c1 = cc ("stuff", 1);
  checked<string> c2(c1);
  CHECK_EQUAL (1, c2); //also reset the error code
  CHECK_EQUAL ("stuff", c2->c_str ());
}

TEST (checked_assignment)
{
  auto c1 = cc ("stuff", 1);
  checked<string> c2;
  c2 = c1;
  CHECK_EQUAL (1, c2); // also reset the error code
  CHECK_EQUAL ("stuff", c2->c_str ());
    
  checked<string> c3;
  c3 = cc ("stuff", 1);
  CHECK_EQUAL (1, c3);
  CHECK_EQUAL ("stuff", c3->c_str ());
}

checked<string> seterr (const string &s, int v)
{
  checked<string> r;
  erc er{v, erc::error};
  *r = s;
  r = er;
  return r;
}

TEST (checked_set_error)
{
  auto c = seterr ("stuff", 1);
  checked<string> d;
  d = c;
  CHECK_EQUAL (1, d);
  CHECK_EQUAL ("stuff", *d);
}

} //end suite
