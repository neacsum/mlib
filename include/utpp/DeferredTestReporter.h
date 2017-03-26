#pragma once

#include "TestReporter.h"
#include "DeferredTestResult.h"

#include <vector>

namespace UnitTest
{

/*
  A DeferredTestReporter is TestReporter that keeps a list of test results.
  It is an abstract base because it is still missing a ReportSummary function.
*/
class DeferredTestReporter : public TestReporter
{
public:
  virtual void ReportTestStart (TestDetails const& details);
  virtual void ReportFailure (TestDetails const& details, char const* failure);
  virtual void ReportTestFinish (TestDetails const& details, float secondsElapsed);

  typedef std::vector< DeferredTestResult > DeferredTestResultList;
  DeferredTestResultList& GetResults ();

private:
  DeferredTestResultList results;
};

inline
DeferredTestReporter::DeferredTestResultList& DeferredTestReporter::GetResults ()
{
  return results;
}

}
