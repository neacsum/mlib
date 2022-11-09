#include <utpp/utpp.h>
#include <fstream>
#include <mlib/mlib.h>

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