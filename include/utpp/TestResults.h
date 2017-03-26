#pragma once

namespace UnitTest {

class TestReporter;
class TestDetails;

class TestResults
{
public:
  TestResults (TestReporter* reporter = 0);

  void OnTestStart (TestDetails const& test);
  void OnTestFailure (TestDetails const& test, char const* failure);
  void OnTestFinish (TestDetails const& test, float secondsElapsed);

  int GetTotalTestCount () const;
  int GetFailedTestCount () const;
  int GetFailureCount () const;

private:
  TestReporter* testReporter;
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
