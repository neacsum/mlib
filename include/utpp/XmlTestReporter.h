#pragma once

#include "DeferredTestReporter.h"

#include <iosfwd>

namespace UnitTest
{

class XmlTestReporter : public DeferredTestReporter
{
public:
  explicit XmlTestReporter (std::ostream& ostream);

  virtual void ReportSummary (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);

private:
  XmlTestReporter (XmlTestReporter const&);
  XmlTestReporter& operator=(XmlTestReporter const&);

  void AddXmlElement (char const* encoding);
  void BeginResults (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed);
  void EndResults ();
  void BeginTest (DeferredTestResult const& result);
  void AddFailure (DeferredTestResult const& result);
  void EndTest (DeferredTestResult const& result);

  std::ostream& os;
};

}
