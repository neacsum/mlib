#include <utpp/utpp.h>
#include <fstream>

int main (int argc, char **argv)
{
  std::ofstream os ("geo_tests.xml");
  UnitTest::XmlTestReporter xml (os);
  UnitTest::RunAllTests (xml);
  return UnitTest::RunAllTests ();
}

