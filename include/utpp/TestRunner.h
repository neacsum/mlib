#pragma once

#include "Test.h"
#include "TestList.h"
#include "CurrentTest.h"
#include "TestReporterStdout.h"
#include "timehelpers.h"

namespace UnitTest {

class TestResults;

int RunAllTests (TestReporter& rpt = default_reporter);

class TestRunner
{
public:
  TestRunner (TestReporter& reporter);
  int RunTests (TestList const& list, char const* suiteName, int maxTestTimeInMs);

private:
  TestReporter& reporter;
  TestResults result;
  Timer timer;

  int Finish () const;
  bool IsTestInSuite (const Test* const curTest, char const* suiteName) const;
  void RunTest (Test* const curTest, int const maxTestTimeInMs);
};

}
