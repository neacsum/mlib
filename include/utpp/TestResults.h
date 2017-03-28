#pragma once

#include "TestReporter.h"
#include <string>

namespace UnitTest {

class TestDetails;

class TestResults
{
public:
  TestResults (TestReporter& reporter = default_reporter);

  void OnTestStart (const TestDetails& test);
  void OnTestFailure (const TestDetails& test, const std::string& failure);
  void OnTestFinish (const TestDetails& test, float secondsElapsed);

  int GetTotalTestCount () const;
  int GetFailedTestCount () const;
  int GetFailureCount () const;

private:
  TestReporter& testReporter;
  int totalTestCount;
  int failedTestCount;
  int failureCount;

  bool currentTestFailed;

  TestResults (TestResults const&);
  TestResults& operator =(TestResults const&);
};


inline
int TestResults::GetTotalTestCount () const
{
  return totalTestCount;
}

inline
int TestResults::GetFailedTestCount () const
{
  return failedTestCount;
}

inline
int TestResults::GetFailureCount () const
{
  return failureCount;
}

}
