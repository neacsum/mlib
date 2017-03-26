#include <utpp/TestResults.h>
#include <utpp/TestReporter.h>

#include <utpp/TestDetails.h>

namespace UnitTest {

TestResults::TestResults (TestReporter* testReporter)
  : testReporter (testReporter)
  , totalTestCount (0)
  , failedTestCount (0)
  , failureCount (0)
  , currentTestFailed (false)
{
}

void TestResults::OnTestStart (TestDetails const& test)
{
  ++totalTestCount;
  currentTestFailed = false;
  if (testReporter)
    testReporter->ReportTestStart (test);
}

void TestResults::OnTestFailure (TestDetails const& test, char const* failure)
{
  ++failureCount;
  if (!currentTestFailed)
  {
    ++failedTestCount;
    currentTestFailed = true;
  }

  if (testReporter)
    testReporter->ReportFailure (test, failure);
}

void TestResults::OnTestFinish (TestDetails const& test, float secondsElapsed)
{
  if (testReporter)
    testReporter->ReportTestFinish (test, secondsElapsed);
}





}
