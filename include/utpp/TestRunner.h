#pragma once

#include "Test.h"
#include "TestList.h"
#include "CurrentTest.h"
#include "TestReporterStdout.h"

namespace UnitTest {

class TestResults;
class Timer;

extern TestReporterStdout stdout_reporter;

int RunAllTests (TestReporter& rpt = stdout_reporter);

class TestRunner
{
public:
  TestRunner (TestReporter& reporter);
  ~TestRunner ();
  int RunTests (TestList const& list, char const* suiteName, int maxTestTimeInMs) const;

private:
  TestReporter* reporter;
  TestResults* result;
  Timer* timer;

  int Finish () const;
  bool IsTestInSuite (const Test* const curTest, char const* suiteName) const;
  void RunTest (Test* const curTest, int const maxTestTimeInMs) const;
};

}
