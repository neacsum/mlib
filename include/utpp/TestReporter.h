#pragma once

#include <string>

namespace UnitTest {

class TestDetails;

class TestReporter
{
public:
  virtual ~TestReporter () {};

  /// Invoked at the beginning of a test
  virtual void ReportTestStart (const TestDetails& test) {};

  /// Called when a test has failed
  virtual void ReportFailure (const TestDetails& test, const std::string& failure) {};

  //// Invoked at the end of a test
  virtual void ReportTestFinish (const TestDetails& test, float secondsElapsed) {};

  /// Generate results report
  virtual void ReportSummary (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) {};
};

extern TestReporter& default_reporter;
}
