/*!
  \file test_reporter_dbgout.cpp - Implementation of TestReporterDbgout class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/test_reporter_dbgout.h>
#include <utpp/failure.h>
#include <utpp/test.h>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <mlib/utf8.h>

using namespace std;
#pragma comment (lib, "mlib.lib")


namespace UnitTest {

/*!
  Output to debug output a failure message. If a test is in progress (the normal case)
  the message includes the name of the test and suite.

  \param failure - the failure information (filename, line number and message)
*/
void TestReporterDbgout::ReportFailure (const Failure& failure)
{
  stringstream ss;
  ss << "Failure in ";
  if (CurrentTest)
  {
    if (CurrentSuite != DEFAULT_SUITE)
      ss << " suite " << CurrentSuite;
    ss << " test " << CurrentTest->test_name ();
  }
  ss << endl;
  OutputDebugString (utf8::widen(ss.str()).c_str());
  ss.clear ();
  ss.str ("");
  ss << failure.filename << "(" << failure.line_number << "):"
    << failure.message << endl;
  OutputDebugString (utf8::widen (ss.str ()).c_str ());
  TestReporter::ReportFailure (failure);
}

/*!
  Prints a test run summary including number of tests, number of failures,
  running time, etc.
*/
int TestReporterDbgout::Summary ()
{
  stringstream ss;
  if (total_failed_count > 0)
  {
    ss << "FAILURE: " << total_failed_count << " out of "
      << total_test_count << " tests failed (" << total_failures_count
      << " failures).";
  }
  else
  {
    ss << "Success: " << total_test_count << " tests passed.";
  }
  ss << endl;
  OutputDebugString (utf8::widen (ss.str ()).c_str ());
  ss.clear ();
  ss.str ("");
  ss.setf (ios::fixed);
  ss << "Run time: " << setprecision (2) << total_time_msec / 1000.;
  OutputDebugString (utf8::widen (ss.str ()).c_str ());

  return TestReporter::Summary ();
}

}
