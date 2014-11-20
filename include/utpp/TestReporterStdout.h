#pragma once

#include "TestReporter.h"

namespace UnitTest {

class TestReporterStdout : public TestReporter
{
private:
  virtual void ReportFailure(TestDetails const& test, char const* failure);
  virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);
};

}
