/*!
  \file xml_test_reporter.cpp - Implementation of XmlTestReporter class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include <utpp/xml_test_reporter.h>
#include <utpp/test_suite.h>

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

//XML char encoding
const string encoding;

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

/// Constructor
XmlTestReporter::XmlTestReporter (std::ostream& ostream)
  : os (ostream)
{
}

/// Generate XML report
int XmlTestReporter::Summary ()
{
  string suite;

  os << "<?xml version=\"1.0\"";
  if (!encoding.empty())
    os << " encoding=\"" << encoding << "\"";
  os << "?>" << endl;

  os << "<unittest-results"
    << " tests=\"" << total_test_count << "\""
    << " failedtests=\"" << total_failed_count << "\""
    << " failures=\"" << total_failures_count << "\""
    << " time_sec=\"" << fixed << setprecision (3) << total_time_msec / 1000. << "\""
    << ">" << endl;

  for (auto i = results.begin (); i != results.end (); ++i)
  {
    if (i->suite_name != suite)
    {
      if (!suite.empty ())
        os << " </suite>" << endl;
      suite = i->suite_name;
      os << " <suite name=\"" << suite << "\">" << endl;
    }

    BeginTest (*i);

    if (!i->failures.empty())
      AddFailure (*i);

    EndTest (*i);
  }

  os << " </suite>" << endl;
  os << "</unittest-results>" << endl;
  return DeferredTestReporter::Summary ();
}

void XmlTestReporter::BeginTest (const DeferredTestReporter::TestResult& result)
{
  os << "  <test"
     << " name=\"" << result.test_name << "\""
     << " time_ms=\"" << result.test_time_ms << "\"";
}

void XmlTestReporter::EndTest (const DeferredTestReporter::TestResult& result)
{
  if (!result.failures.empty())
    os << "  </test>";
  else
    os << "/>";
  os << endl;
}

void XmlTestReporter::AddFailure (const DeferredTestReporter::TestResult& result)
{
  os << ">" << endl; // close <test> element

  for (auto it = result.failures.begin ();
       it != result.failures.end ();
       ++it)
  {
    string const escapedMessage = XmlEscape (it->message);
    string const message = BuildFailureMessage (it->filename, it->line_number, escapedMessage);

    os << "   <failure" << " message=\"" << message << "\"" << "/>" << endl;
  }
}

}
