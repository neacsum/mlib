#pragma once
/*!
  \file xml_test_reporter.h - Definition of XmlTestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include "deferred_test_reporter.h"

#include <iosfwd>

namespace UnitTest
{

/// A DeferredTestReporter that sends results to an XML file.
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
