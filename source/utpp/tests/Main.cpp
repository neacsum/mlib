#include <utpp/utpp.h>
#include <utpp/TestReporterStdout.h>

#include <utpp/XmlTestReporter.h>
#include <iostream>
#include <fstream>

int main (int, char const *[])
{
  /*The default argument to RunAllTests is a TestReporterStdout that sends 
  all results to console...*/
  int ret = UnitTest::RunAllTests ();

  /*...but it can be replaced with a different reporter like the XmlTestReporter
  in the example below.*/
  std::ofstream os ("test_results.xml");
  UnitTest::XmlTestReporter xml (os);
  UnitTest::RunAllTests (xml);

  return ret;
}
