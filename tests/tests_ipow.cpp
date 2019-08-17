#include <utpp/utpp.h>
#include <mlib/ipow.h>

#include <iostream>

using namespace MLIBSPACE;
using namespace std;

#define NMAX 1000000
SUITE (ipow)
{
  volatile double dx;
  volatile int ix;

  void go (int exp)
  {
    UnitTest::Timer t;

    t.Start ();
    for (int i = 0; i < NMAX; i++)
      ix = ipow (i, exp);
    long long dt1 = t.GetTimeInUs ();
    t.Start ();
    for (int i = 0; i < NMAX; i++)
      dx = ipow ((double)i, exp);
    long long dt2 = t.GetTimeInUs ();
    t.Start ();
    for (int i = 0; i < NMAX; i++)
      dx = pow ((double)i, exp);
    long long dt3 = t.GetTimeInUs ();
    t.Start ();
    for (int i = 0; i < NMAX; i++)
      dx = pow ((double)i, (double)exp);
    long long dt4 = t.GetTimeInUs ();

    cout << "Pow " << exp << " results (usec): " << endl <<
      " ipow - integer base, integer power - " << dt1 << endl <<
      " ipow - double base, integer power - " << dt2 << endl <<
      " pow  - double base, integer power - " << dt3 << endl <<
      " pow  - double base, double power - " << dt4 << endl;
  }

  TEST (pow)
  {
    CHECK_EQUAL (pow (5, 6), ipow (5, 6));
    CHECK_EQUAL (pow (12, 13), ipow (12., 13));
    CHECK_EQUAL (pow (123, 45), ipow (123., 45));
  }

  TEST (timing)
  {
    go (2);
    go (32);
  }
}
