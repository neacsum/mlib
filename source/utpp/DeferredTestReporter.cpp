#include <utpp/DeferredTestReporter.h>
#include <utpp/TestDetails.h>
#include <utpp/Config.h>

using namespace UnitTest;

void DeferredTestReporter::ReportTestStart (TestDetails const& details)
{
  results.push_back (DeferredTestResult (details.suiteName, details.testName));
}

void DeferredTestReporter::ReportFailure (TestDetails const& details, char const* failure)
{
  DeferredTestResult& r = results.back ();
  r.failed = true;
  r.failures.push_back (DeferredTestResult::Failure (details.lineNumber, failure));
  r.failureFile = details.filename;
}

void DeferredTestReporter::ReportTestFinish (TestDetails const&, float secondsElapsed)
{
  DeferredTestResult& r = results.back ();
  r.timeElapsed = secondsElapsed;
}

