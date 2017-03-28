#pragma once

#include "TestReporter.h"
#include "DeferredTestResult.h"

#include <vector>

namespace UnitTest
{

/*!
  A DeferredTestReporter is TestReporter that keeps a list of test results.
*/
class DeferredTestReporter : public TestReporter
{
public:
  virtual void ReportTestStart (const TestDetails& details);
  virtual void ReportFailure (const TestDetails& details, const std::string& failure);
  virtual void ReportTestFinish (const TestDetails& details, float secondsElapsed);

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
