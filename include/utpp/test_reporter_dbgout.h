#pragma once
/*!
  \file test_reporter_dbgout.h - Definition of TestReporterDbgout class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include "test_reporter.h"

namespace UnitTest {

/// A TestReporter that sends messages to debug output
class TestReporterDbgout : public TestReporter
{
private:
  virtual void ReportFailure (const Failure& failure);
  virtual int Summary ();
};

}
