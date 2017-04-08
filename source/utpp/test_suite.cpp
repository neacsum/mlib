/*!
  \file test_suite.cpp - Implementation of TestSuite class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include <utpp/test_suite.h>
#include <utpp/test.h>
#include <utpp/failure.h>
#include <utpp/assert_exception.h>
#include <utpp/suites_list.h>

#include <cassert>
#include <sstream>
#include <deque>

using namespace std;

namespace UnitTest {

//The global CurrentTest object
Test* CurrentTest;

//The global CurrentSuite name
string CurrentSuite = DEFAULT_SUITE;

/*!
  \class UnitTest::TestSuite
  A suite maintains a container with information about all tests included in
  the suite.
*/

///Constructor
TestSuite::TestSuite (const char *name_)
  : name (name_)
{
  CurrentTest = nullptr;
  CurrentSuite = name;
}

/// Add a new test information to test_list
void TestSuite::Add (maker_info& inf)
{
  test_list.push_back (inf);
}

/*!
  Run all tests in suite

  \param rep Reporter object to be used
  \param maxtime maximum run time for each test

  Iterate through all test information objects doing the following:
*/
int TestSuite::RunTests (TestReporter& rep, int maxtime)
{
  /// Establish reporter as CurrentReporter
  CurrentReporter = &rep;
  auto listp = test_list.begin ();
  max_runtime = maxtime;

  while (listp != test_list.end())
  {
    /// Setup the test context
    if (SetupCurrentTest (*listp))
    {
      RunCurrentTest (*listp); /// Run test
      TearDownCurrentTest (*listp);  /// Tear down test context
    }
    /// Repeat for all tests
    listp++;
  }

  ///At the end invoke reporter summary
  return CurrentReporter->ReportSummary ();
}

/*!
  Invoke the maker function to create the test object instance and
  assign it to this suite.

  The actual test object might be derived also from a fixture. When maker 
  function is called, it triggers the construction of the fixture and this
  might fail. That is why the construction is wrapped in a try...catch block.

  \return true if constructor was successful
*/
bool TestSuite::SetupCurrentTest (maker_info& inf)
{
  bool ok = false;
  try {
    CurrentTest = (*inf.func)();
    ok = true;
  }
  catch (const std::exception& e)
  {
    std::stringstream stream;
    stream << "Unhandled exception: " << e.what ()
      << " while setting up test " << inf.name;
    ReportFailure (inf.file, inf.line, stream.str ());
  }
  catch (...)
  {
    ReportFailure (inf.file, inf.line, "Setup unhandled exception: Crash!");
  }
  return ok;
}

/// Run the test
void TestSuite::RunCurrentTest (maker_info& inf)
{
  assert (CurrentTest);
  CurrentReporter->ReportTestStart (*CurrentTest);


  try {
    CurrentTest->run ();
  }
  catch (const AssertException& e)
  {
    ReportFailure (e.filename (), e.line_number (), e.what ());
  }
  catch (const std::exception& e)
  {
    std::stringstream stream;
    stream << "Unhandled exception: " << e.what ();
    ReportFailure (inf.file, inf.line, stream.str ());
  }
  catch (...)
  {
    ReportFailure (inf.file, inf.line, "Unhandled exception: Crash!");
  }

  int actual_time = CurrentTest->test_time_ms ();
  if (actual_time >= 0 && max_runtime && actual_time > max_runtime)
  {
    std::stringstream stream;
    stream << "Global time constraint failed. Expected under " << max_runtime <<
      "ms but took " << actual_time << "ms.";

    ReportFailure (inf.file, inf.line, stream.str ());
  }
  CurrentReporter->ReportTestFinish (*CurrentTest);
}

/// Delete current test instance
void TestSuite::TearDownCurrentTest (maker_info& inf)
{
  try {
    delete CurrentTest;
    CurrentTest = 0;
  }
  catch (const std::exception& e)
  {
    std::stringstream stream;
    stream << "Unhandled exception: " << e.what ()
      << " while tearing down test " << inf.name;
    ReportFailure (inf.file, inf.line, stream.str ());
  }
  catch (...)
  {
    ReportFailure (inf.file, inf.line, "TearDown unhandled exception: Crash!");
  }
}


//////////////////////////// RunAll functions /////////////////////////////////

int RunAllTests (TestReporter& rpt, int max_time_ms)
{
  return SuitesList::GetSuitesList ().RunAll (rpt, max_time_ms);
}

int RunSuite (const char *suite_name, TestReporter& rpt, int max_time_ms)
{
  return SuitesList::GetSuitesList ().Run (suite_name, rpt, max_time_ms);
}

}
