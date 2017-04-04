#pragma once

#include "deferred_test_reporter.h"

#include <iosfwd>

namespace UnitTest
{

/// XmlTestReporte is a DeferredTestReporter that prints results in an XML file.
class XmlTestReporter : public DeferredTestReporter
{
public:
  XmlTestReporter (std::ostream& ostream);

  int ReportSummary ();

private:
  XmlTestReporter (XmlTestReporter const&);
  XmlTestReporter& operator=(XmlTestReporter const&);

  void AddXmlElement (char const* encoding);
  void BeginResults ();
  void EndResults ();
  void BeginSuite (const std::string& suite);
  void BeginTest (const DeferredTestReporter::TestResult& result);
  void AddFailure (const DeferredTestReporter::TestResult& result);
  void EndTest (const DeferredTestReporter::TestResult& result);

  std::ostream& os;
  std::string suite;
};

}
