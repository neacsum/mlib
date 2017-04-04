#pragma once
/*!
  \file test_reporter_stdout.h - Definition of TestReporterStdout class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include "test_reporter.h"

namespace UnitTest {

/// A TestReporter that sends messages to stdout
class TestReporterStdout : public TestReporter
{
private:
  virtual void ReportFailure (const Failure& failure);
  virtual int ReportSummary ();
};

}
