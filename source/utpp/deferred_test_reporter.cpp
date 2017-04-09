/*!
  \file deferred_test_reporter.cpp - Implementation of DeferredTestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/deferred_test_reporter.h>
#include <utpp/test.h>

using namespace UnitTest;

/// Add a new test to the results container
void DeferredTestReporter::TestStart (const Test& test)
{
  TestReporter::TestStart (test);
  results.push_back (TestResult (CurrentSuite, test.test_name()));
}

/// Add a new failure to current test
void DeferredTestReporter::ReportFailure (const Failure& failure)
{
  TestReporter::ReportFailure (failure);
  results.back ().failures.push_back (failure);
}

/// Store test runtime when the test finishes
void DeferredTestReporter::TestFinish (const Test& test)
{
  TestReporter::TestFinish (test);
  results.back ().test_time_ms = test.test_time_ms();
}

