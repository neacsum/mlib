#include <utpp/TestRunner.h>
#include <utpp/TestResults.h>
#include <utpp/TestReporter.h>
#include <utpp/TestReporterStdout.h>
#include <utpp/TimeHelpers.h>

#include <sstream>

namespace UnitTest {

int RunAllTests (TestReporter& reporter)
{
  TestRunner runner (reporter);
  return runner.RunTests (Test::GetTestList (), NULL, 0);
}


TestRunner::TestRunner (TestReporter& reporter_)
  : reporter (reporter_)
  , result (reporter)
{
  timer.Start ();
}

int TestRunner::RunTests (TestList const& list, char const* suiteName, int maxTestTimeInMs)
{
  Test* curTest = list.GetHead ();

  while (curTest != 0)
  {
    if (IsTestInSuite (curTest, suiteName))
    {
      RunTest (curTest, maxTestTimeInMs);
    }

    curTest = curTest->next;
  }

  return Finish ();
}

int TestRunner::Finish () const
{
  float const secondsElapsed = timer.GetTimeInMs () / 1000.0f;
  reporter.ReportSummary (result.GetTotalTestCount (),
                             result.GetFailedTestCount (),
                             result.GetFailureCount (),
                             secondsElapsed);

  return result.GetFailureCount ();
}

bool TestRunner::IsTestInSuite (const Test* const curTest, const char* suiteName) const
{
  using namespace std;
  return (suiteName == NULL) || curTest->m_details.suiteName != suiteName;
}

void TestRunner::RunTest (Test* const curTest, int const maxTestTimeInMs)
{
  CurrentTest.Results = &result;
  CurrentTest.Details = &curTest->m_details;
  Timer testTimer;
  testTimer.Start ();

  result.OnTestStart (curTest->m_details);

  curTest->Run ();

  int const testTimeInMs = testTimer.GetTimeInMs ();
  if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !curTest->m_timeConstraintExempt)
  {
    std::stringstream stream;
    stream << "Global time constraint failed. Expected under " << maxTestTimeInMs <<
      "ms but took " << testTimeInMs << "ms.";

    result.OnTestFailure (curTest->m_details, stream.str ());
  }

  result.OnTestFinish (curTest->m_details, testTimeInMs / 1000.0f);
}

}
