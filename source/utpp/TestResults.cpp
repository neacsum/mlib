#include <utpp/TestResults.h>
#include <utpp/TestReporter.h>

#include <utpp/TestDetails.h>

namespace UnitTest {

TestResults::TestResults (TestReporter& testReporter_)
  : testReporter (testReporter_)
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
  testReporter.ReportTestStart (test);
}

void TestResults::OnTestFailure (TestDetails const& test, const std::string& failure)
{
  ++failureCount;
  if (!currentTestFailed)
  {
    ++failedTestCount;
    currentTestFailed = true;
  }

  testReporter.ReportFailure (test, failure);
}

void TestResults::OnTestFinish (TestDetails const& test, float secondsElapsed)
{
  testReporter.ReportTestFinish (test, secondsElapsed);
}





}
