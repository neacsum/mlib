#pragma once

#include "TestReporter.h"

namespace UnitTest {

class TestReporterStdout : public TestReporter
{
private:
  virtual void ReportFailure (const TestDetails& test, const std::string& failure);
  virtual void ReportSummary (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);
};

}
