#pragma once
/*!
  \file deferred_test_reporter.h - Definition of DeferredTestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include "test_reporter.h"
#include "failure.h"

#include <deque>

namespace UnitTest
{

/// A TestReporter that keeps a list of test results.
class DeferredTestReporter : public TestReporter
{
public:
  DeferredTestReporter () {};
  virtual void ReportTestStart (const Test& test);
  virtual void ReportFailure (const Failure& failure);
  virtual void ReportTestFinish (const Test& test);

protected:
  ///@brief %Test results including all failure messages
  struct TestResult
  {
    TestResult ();
    TestResult (const std::string& suite, const std::string& test);

    std::string suite_name;
    std::string test_name;
    int test_time_ms;

    ///@brief All failures of a test
    typedef std::deque< Failure > FailureVec;
    FailureVec failures;
  };

  ///@brief All test results
  typedef std::deque< TestResult > DeferredTestResultList;

  DeferredTestResultList results;   ///< These are the results of all tests
};

/// Default constructor needed container inclusion
inline 
DeferredTestReporter::TestResult::TestResult ()
  : test_time_ms (0)
{
}

/// Constructor
inline 
DeferredTestReporter::TestResult::TestResult (const std::string& suite, const std::string& test)
  : suite_name (suite)
  , test_name (test)
  , test_time_ms (0)
{
}

}
