#pragma once

#include "test_reporter.h"

namespace UnitTest {

/// TestReporterStdout is a TestReporter that sends messages to stdout
class TestReporterStdout : public TestReporter
{
private:
  virtual void ReportFailure (Failure& failure);
  virtual int ReportSummary ();
};

}
