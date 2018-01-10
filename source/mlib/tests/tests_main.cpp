#include <utpp/utpp.h>
#include <fstream>

int main (int argc, char **argv)
{
  std::ofstream os ("mlib_tests.xml");
  UnitTest::ReporterXml xml (os);
  return UnitTest::RunAllTests (xml);
}

