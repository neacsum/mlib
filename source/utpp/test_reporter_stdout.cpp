/*!
  \file test_reporter_stdout.cpp - Implementation of TestReporterStdout class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/test_reporter_stdout.h>
#include <utpp/failure.h>
#include <utpp/test.h>
#include <iostream>
#include <iomanip>
using namespace std;


namespace UnitTest {

/// Return the default reporter object. 
TestReporter& GetDefaultReporter ()
{
  static TestReporterStdout the_default_reporter;
  return the_default_reporter;
}

/*!
  Output to stdout a failure message. If a test is in progress (the normal case)
  the message includes the name of the test and suite.

  \param failure - the failure information (filename, line number and message)
*/
void TestReporterStdout::ReportFailure (const Failure& failure)
{
  cout << "Failure in ";
  if (CurrentTest)
  {
    if (CurrentSuite != DEFAULT_SUITE)
      cout << " suite " << CurrentSuite;
    cout << " test " << CurrentTest->test_name ();
  }
  cout << endl << failure.filename << "(" << failure.line_number << "):"
    << failure.message << endl;
  TestReporter::ReportFailure (failure);
}

/*!
  Prints a test run summary including number of tests, number of failures,
  running time, etc.
*/
int TestReporterStdout::Summary ()
{
  if (total_failed_count > 0)
  {
    cout << "FAILURE: " << total_failed_count << " out of "
      << total_test_count << " tests failed (" << total_failures_count
      << " failures)." << endl;
  }
  else
  {
    cout << "Success: " << total_test_count << " tests passed." << endl;
  }
  cout.setf (ios::fixed);
  cout << "Run time: " << setprecision (2) << total_time_msec / 1000. << endl;
  return TestReporter::Summary ();
}

}
