#include <utpp/utpp.h>
#include <fstream>

int main (int argc, char **argv)
{
  std::ofstream os ("mlib_tests.xml");
  UnitTest::XmlTestReporter xml (os);
  UnitTest::RunAllTests (xml);
  UnitTest::TestReporterDbgout dbg;
  return UnitTest::RunAllTests (dbg);
}

