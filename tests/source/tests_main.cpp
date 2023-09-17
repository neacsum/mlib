#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <fstream>

using namespace std;

TEST_MAIN (int argc, char **argv)
{
  if (argc == 1)
  {
    string fname = argv[0];
    auto p = fname.rfind ('\\') + 1;
    fname.erase (p);
    fname.append ("mlib_tests.xml");
    cout << "Results file: " << fname << endl;
    std::ofstream os (fname);
    UnitTest::ReporterXml xml (os);
    return UnitTest::RunAllTests (xml);
  }
  else
    return UnitTest::RunSuite (argv[1]);
}

SUITE (rotations)
{
  TEST (rotmat_ctor)
  {
    mlib::RotMat r (30_deg, 20_deg, 40_deg);
    double expected[3][3] = {{0.719846, -0.425669, 0.548295},
                             {0.604023, 0.773337, -0.19263},
                             {-0.34202, 0.469846, 0.813798}};

    CHECK_ARRAY2D_CLOSE (expected, r.matrix (), 3, 3, 1e-6);
  }

  TEST (z90)
  {
    mlib::RotMat r;
    r.z_rotation (90_deg);
    double m[3][3] = {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}};
    CHECK_ARRAY2D_CLOSE (m, r.matrix (), 3, 3, 1e-10);

    double pt[3] = {1, 1, 1};
    r.rotate (pt);
    double expected[3] = {-1, 1, 1}; 
    CHECK_ARRAY_CLOSE (expected, pt, 3, 1e-10);
  }

  TEST (x90)
  {
    mlib::RotMat r;
    r.x_rotation (90_deg);
    double m[3][3] = {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}};
    CHECK_ARRAY2D_CLOSE (m, r.matrix (), 3, 3, 1e-10);

    double pt[3] = {1, 1, 1};
    r.rotate (pt);
    double expected[3] = {1, -1, 1};
    CHECK_ARRAY_CLOSE (expected, pt, 3, 1e-10);
  }

  TEST (y90)
  {
    mlib::RotMat r;
    r.y_rotation (90_deg);
    double m[3][3] = {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}};
    CHECK_ARRAY2D_CLOSE (m, r.matrix (), 3, 3, 1e-10);

    double pt[3] = {1, 1, 1};
    r.rotate (pt);
    double expected[3] = {1, 1, -1};
    CHECK_ARRAY_CLOSE (expected, pt, 3, 1e-10);
  }

}

TEST (dprintf_ok)
{
  const char *greek = u8"ελληνικό αλφάβητο";

  CHECK (dprintf ("This is OK"));
  CHECK (dprintf ("A Greek text sample: %s", greek));
}

TEST (dprintf_long)
{
  char superlong[MAX_DPRINTF_CHARS + 256];
  memset (superlong, 0, sizeof (superlong));

  for (int i = 0; i < MAX_DPRINTF_CHARS; i++)
    superlong[i] = (i % 100 != 0) ? ' ' : i / 100 % 10 + '0';
  CHECK (dprintf (superlong));
  for (int i = 0; i < MAX_DPRINTF_CHARS; i++)
    superlong[i] = (i % 10 != 0) ? ' ' : i / 10 % 10 + '0';
  CHECK (dprintf (superlong));
  for (int i = 0; i < MAX_DPRINTF_CHARS; i++)
    superlong[i] = i % 10 + '0';
  CHECK (dprintf (superlong));
}