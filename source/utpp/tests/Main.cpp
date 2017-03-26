#include <utpp/utpp.h>
#include <utpp/TestReporterStdout.h>

#include <utpp/XmlTestReporter.h>
#include <iostream>
#include <fstream>

int main (int, char const *[])
{
  std::ofstream os ("test_results.xml");
  UnitTest::XmlTestReporter xml (os);
  int ret = UnitTest::RunAllTests ();
  UnitTest::RunAllTests (xml);

  return ret;
}
