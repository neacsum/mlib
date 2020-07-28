#include <utpp/utpp.h>
#include <mlib/ipow.h>
#include <mlib/poly.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

using namespace mlib;
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
    t.Start ();
    for (int i = 0; i < NMAX; i++)
      dx = pow (i, exp);
    long long dt5 = t.GetTimeInUs ();

    // Check timer
    t.Start ();
    Sleep (1000);
    long long dt0 = t.GetTimeInUs ();
    cout << endl << "One second has " << dt0 << " usec" << endl;

    cout << "Pow " << exp << " results (usec): " << endl <<
      " ipow - integer base, integer power - " << dt1 << endl <<
      " ipow - double base, integer power - " << dt2 << endl <<
      " pow  - double base, integer power - " << dt3 << endl <<
      " pow  - double base, double power - " << dt4 << endl <<
      " pow  - integer base, integer power - " << dt5 << endl;
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

SUITE (poly)
{
  TEST (int_poly)
  {
    //coeffs for (X+1)^3
    int cube[] { 1,3,3,1 };

    int v = poly (2, cube, _countof(cube));
    CHECK_EQUAL (27, v);

    // f(x) = x^4 + 2x^3 + 3x^2 + 4x + 5
    // f(2) = 57
    v = poly (2, std::array<int, 5>{ 5, 4, 3, 2, 1 });
    CHECK_EQUAL (57, v);
  }

  TEST (dbl_poly)
  {
    std::vector<double> a(8);

    // Coefficients for sine approximation using Taylor series
    // sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
    double fact = -1;
    for (int i = 1; i <= 7; i++)
    {
      if (i % 2)
      {
        fact *= -i;
        a[i] = 1 / fact;
      }
      else
        fact *= i;
    }

    double vv = poly (M_PI/2, a);
    CHECK_CLOSE (1.0, vv, 0.001);
  }
}
