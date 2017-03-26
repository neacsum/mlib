#pragma once

namespace UnitTest {

class TestDetails;

class TestReporter
{
public:
  virtual ~TestReporter ();

  /// Invoked at the beginning of a test
  virtual void ReportTestStart (TestDetails const& test);

  /// Called when a test has failed
  virtual void ReportFailure (TestDetails const& test, char const* failure) = 0;

  //// Invoked at the end of a test
  virtual void ReportTestFinish (TestDetails const& test, float secondsElapsed);

  /// Generate results report
  virtual void ReportSummary (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) = 0;
};

}
