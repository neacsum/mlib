#include <utpp/utpp.h>
#include <mlib/mlib.h>

using namespace mlib;

SUITE (statpars)
{
  TEST (avg)
  {
    statpars p;
    for (int i = 0; i <= 10; i++)
      p.add (i);
    CHECK_CLOSE (5., p.average (), 1e-9);
  }

  TEST (std)
  {
    statpars p;
    double v[] = { 4, 9, 11, 12, 17, 5, 8, 12, 14 };
    p.add (v, _countof (v));
    CHECK_CLOSE (4.17665469538, p.stdev (), 1e-11); //according to my HP48
  }

  //add a whole vector to a statpars object
  TEST (vector)
  {
    statpars p;
    p.add ({ 4, 9, 11, 12, 17, 5, 8, 12, 14 });
    CHECK_CLOSE (17.4444444444, p.variance (), 1e-10);
    CHECK_CLOSE (-0.042210009, p.skewness (), 1e-9); //according to Excel
    CHECK_CLOSE (-0.519235785, p.kurtosis (), 1e-9); //according to Excel
    CHECK_CLOSE (3.308641975, p.mad (), 1e-9);
  }

  TEST (vector_constructor)
  {
    statpars p ({ 4, 9, 11, 12, 17, 5, 8, 12, 14 });
    CHECK_CLOSE (17.4444444444, p.variance (), 1e-10);
    CHECK_CLOSE (-0.042210009, p.skewness (), 1e-9); //according to Excel
    CHECK_CLOSE (-0.519235785, p.kurtosis (), 1e-9); //according to Excel
    CHECK_CLOSE (3.308641975, p.mad (), 1e-9);
  }

  TEST (moving_vals)
  {
    statpars p (3);
    p.add ({ 4, 9, 11, 12, 17, 5, 8, 12, 14 });
    CHECK_CLOSE (11.3333, p.average (), 1e-4);
    CHECK_CLOSE (3.0550, p.stdev (), 1e-4);
    CHECK_CLOSE (9.3333, p.variance (), 1e-4);
  }
}
