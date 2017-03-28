#include <utpp/TestReporterStdout.h>
#include <iostream>

#include <utpp/TestDetails.h>
#include <utpp/Test.h>
using namespace std;

namespace UnitTest {

static TestReporterStdout the_default_reporter;
TestReporter& default_reporter = the_default_reporter;

void TestReporterStdout::ReportFailure (TestDetails const& details, const string& failure)
{
  if (details.suiteName == DEFAULT_SUITE)
    cout << "Failure in test " << details.testName
    << "(file " << details.filename << ", line " << details.lineNumber << ")\n"
    << failure << endl;
  else
    cout << "Failure in suite " << details.suiteName
    << " test " << details.testName
    << "(file " << details.filename << ", line " << details.lineNumber << ")\n"
    << failure << endl;
}

void TestReporterStdout::ReportSummary (int const totalTestCount, int const failedTestCount,
                                        int const failureCount, float secondsElapsed)
{
  if (failureCount > 0)
    printf ("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
  else
    printf ("Success: %d tests passed.\n", totalTestCount);

  printf ("Test time: %.2f seconds.\n", secondsElapsed);
}

}
