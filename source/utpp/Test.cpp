/*!
  \file test.cpp - Implementation of Test class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/test.h>
#include <utpp/test_suite.h>
#include <utpp/test_reporter.h>
#include <utpp/failure.h>

#include <sstream>

namespace UnitTest {

/// Constructor
Test::Test (const char* testName)
  : name (testName)
  , failures (0)
  , time (0)
{
}

Test::~Test ()
{
}

/// Start a timer and call RunImpl to execute test code
void Test::run ()
{
  Timer test_timer;
  if (time >= 0)
    test_timer.Start ();

  RunImpl ();

  if (time >= 0)
    time = test_timer.GetTimeInMs ();
}


void Test::failure ()
{
  failures++;
}


void ReportFailure (const std::string& filename, int line, const std::string& message)
{
  assert (CurrentReporter);

  if (CurrentTest)
    CurrentTest->failure ();
  CurrentReporter->ReportFailure (Failure (filename, line, message));
}


}
