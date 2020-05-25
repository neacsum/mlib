#include <utpp/utpp.h>
#include <mlib/tvops.h>

using namespace MLIBSPACE;

SUITE (tvops)
{
  //simple arithmetic operations
  TEST (artih)
  {
    timeval tv1 {100, 100};
    timeval tv2 {200, 200};
    timeval sum {300, 300};
    CHECK_EQUAL (sum, tv1 + tv2);
    CHECK_EQUAL (tv1, sum - tv2);

    tv2 += tv1;
    CHECK_EQUAL (sum, tv2);
    
    CHECK_EQUAL (sum, 3 * tv1);
    CHECK_EQUAL (sum, tv1 * 3);
  }

  TEST (overflow)
  {
    timeval tv1 {100, 999999};

    tv1 += timeval {0, 1};
    CHECK_EQUAL ((timeval{101, 0}), tv1);
    tv1 -= timeval {0,1};
    CHECK_EQUAL ((timeval {100, 999999}), tv1);
  }

  TEST (systemtime)
  {
    SYSTEMTIME st1, st2;
    GetSystemTime (&st1);
    GetLocalTime (&st2);

    timeval tv1;

    tv1 = fromsystime (st1);
    tolocaltime (tv1, &st1);
  }
}
