#pragma once
/*!
  \file test_reporter.h - Definition of TestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

namespace UnitTest {

class Test;
struct Failure;
class TestSuite;

/// @brief Abstract base for all reporters
class TestReporter
{
public:
  TestReporter ();
  virtual ~TestReporter () {};

  /// Invoked at the beginning of a test suite
  virtual void SuiteStart (const TestSuite& suite);

  /// Invoked at the beginning of a test
  virtual void TestStart (const Test& test);

  /// Called when a test has failed
  virtual void ReportFailure (const Failure& failure);

  /// Invoked at the end of a test
  virtual void TestFinish (const Test& test);

  /// Invoked at the end of a test suite
  virtual int SuiteFinish (const TestSuite& suite);

  /// Generate results report
  virtual int Summary () { return total_failed_count; }

protected:
  int suite_test_count,     ///< number of tests in suite
    suite_failed_count,     ///< number of failed tests in suite
    suite_failures_count,   ///< number of failures in suite
    suite_time_msec;        ///< total suite running time in milliseconds

  int total_test_count,     ///< total number of tests
    total_failed_count,     ///< total number of failed tests
    total_failures_count,   ///< total number of failures
    total_time_msec;        ///< total running time in milliseconds
  
  int suites_count;         ///< number of suites ran
};

/// Return the default reporter object
TestReporter& GetDefaultReporter ();

/// Pointer to current reporter object
extern TestReporter* CurrentReporter;
}
