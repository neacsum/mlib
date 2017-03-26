#include <utpp/XmlTestReporter.h>
#include <utpp/Config.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

static void ReplaceChar (string& str, char c, string const& replacement)
{
  for (size_t pos = str.find (c); pos != string::npos; pos = str.find (c, pos + 1))
    str.replace (pos, 1, replacement);
}

static string XmlEscape (string const& value)
{
  string escaped = value;

  ReplaceChar (escaped, '&', "&amp;");
  ReplaceChar (escaped, '<', "&lt;");
  ReplaceChar (escaped, '>', "&gt;");
  ReplaceChar (escaped, '\'', "&apos;");
  ReplaceChar (escaped, '\"', "&quot;");

  return escaped;
}

static string BuildFailureMessage (string const& file, int line, string const& message)
{
  ostringstream failureMessage;
  failureMessage << file << "(" << line << ") : " << message;
  return failureMessage.str ();
}


namespace UnitTest {

XmlTestReporter::XmlTestReporter (ostream& ostream)
  : os (ostream)
{
}

void XmlTestReporter::ReportSummary (int totalTestCount, int failedTestCount,
                                     int failureCount, float secondsElapsed)
{
  AddXmlElement (NULL);

  BeginResults (totalTestCount, failedTestCount, failureCount, secondsElapsed);

  DeferredTestResultList const& results = GetResults ();
  for (DeferredTestResultList::const_iterator i = results.begin (); i != results.end (); ++i)
  {
    BeginTest (*i);

    if (i->failed)
      AddFailure (*i);

    EndTest (*i);
  }

  EndResults ();
}

void XmlTestReporter::AddXmlElement (char const* encoding)
{
  os << "<?xml version=\"1.0\"";

  if (encoding != NULL)
    os << " encoding=\"" << encoding << "\"";

  os << "?>" <<endl;
}

void XmlTestReporter::BeginResults (int totalTestCount, int failedTestCount,
                                    int failureCount, float secondsElapsed)
{
  os << "<unittest-results"
     << " tests=\"" << totalTestCount << "\""
     << " failedtests=\"" << failedTestCount << "\""
     << " failures=\"" << failureCount << "\""
     << " time=\"" << secondsElapsed << "\""
     << ">" << endl;
}

void XmlTestReporter::EndResults ()
{
  os << "</unittest-results>" << endl;
}

void XmlTestReporter::BeginTest (DeferredTestResult const& result)
{
  os << "<test"
     << " suite=\"" << result.suiteName << "\""
     << " name=\"" << result.testName << "\""
     << " time=\"" << result.timeElapsed << "\"";
}

void XmlTestReporter::EndTest (DeferredTestResult const& result)
{
  if (result.failed)
    os << "</test>";
  else
    os << "/>";
  os << endl;
}

void XmlTestReporter::AddFailure (DeferredTestResult const& result)
{
  os << ">" << endl; // close <test> element

  for (DeferredTestResult::FailureVec::const_iterator it = result.failures.begin ();
       it != result.failures.end ();
       ++it)
  {
    string const escapedMessage = XmlEscape (it->second);
    string const message = BuildFailureMessage (result.failureFile, it->first, escapedMessage);

    os << "<failure" << " message=\"" << message << "\"" << "/>" << endl;
  }
}

}
