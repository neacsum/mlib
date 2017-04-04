/*!
  \file test_reporter.cpp - Implementation of TestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/test_reporter.h>
#include <utpp/test.h>

namespace UnitTest {

TestReporter* CurrentReporter;

TestReporter::TestReporter ()
  : suite_test_count (0)
  , suite_failed_count (0)
  , suite_failures_count (0)
  , suite_time_msec (0)
  , total_test_count (0)
  , total_failed_count (0)
  , total_failures_count (0)
  , total_time_msec (0)
  , suites_count (0)
{
}

void TestReporter::ReportSuiteStart (const TestSuite&)
{
  suites_count++;
  suite_test_count = suite_failed_count = suite_failures_count = 0;
}

void TestReporter::ReportTestStart (const Test&)
{
  suite_test_count++;
  total_test_count++;
}

void TestReporter::ReportFailure (const Failure &f)
{
}

void TestReporter::ReportTestFinish (const Test& t)
{
  int f = t.failure_count ();
  if (f)
  {
    suite_failed_count++;
    suite_failures_count += f;
    total_failed_count++;
    total_failures_count += f;
  }
  int ms = t.test_time_ms ();
  suite_time_msec += ms;
  total_time_msec += ms;
}

void TestReporter::ReportSuiteFinish (const TestSuite&)
{
}

}
