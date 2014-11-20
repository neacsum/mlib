#include <utpp/TestReporterStdout.h>
#include <cstdio>

#include <utpp/TestDetails.h>
#include <utpp/Test.h>

namespace UnitTest {

void TestReporterStdout::ReportFailure(TestDetails const& details, char const* failure)
{
  if (details.suiteName == DEFAULT_SUITE)
    printf ("Failure in test %s (file %s,line %d)\n  %s\n",
      details.testName, details.filename, details.lineNumber, failure);
  else
    printf ("Failure in suite %s test %s (file %s, line %d)\n  %s\n",
      details.suiteName, details.testName, details.filename, details.lineNumber, failure);
}

void TestReporterStdout::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float secondsElapsed)
{
  if (failureCount > 0)
    printf ("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
  else
    printf ("Success: %d tests passed.\n", totalTestCount);

  printf ("Test time: %.2f seconds.\n", secondsElapsed);
}

}
