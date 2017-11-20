#include <utpp/utpp.h>
#include <fstream>

int main (int argc, char **argv)
{
  std::ofstream os ("mlib_tests.xml");
  UnitTest::ReporterXml xml (os);
  UnitTest::RunAllTests (xml);
  UnitTest::ReporterDbgout dbg;
  return UnitTest::RunAllTests (dbg);
}

