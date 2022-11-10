#include <utpp/utpp.h>
#include <mlib/tvops.h>


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

  TEST(compare)
  {
    timeval t1{ 100, 100 };
    timeval t2{ 101, 100 };
    timeval t3{ 100,101 };
    CHECK(t1 < t2);
    CHECK(t1 < t3);
    CHECK(t2 >= t1);
    CHECK(t1 <= t2);

    timeval t4{ 200,200 };
    timeval t5{ 200,200 };
    CHECK(t4 == t5);
    CHECK(t4 <= t5);
    CHECK(t4 >= t5);

    CHECK(t1 != t2);
    CHECK(t1 != t3);
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
