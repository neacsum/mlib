#include <utpp/Config.h>
#include <utpp/Test.h>
#include <utpp/TestList.h>
#include <utpp/TestResults.h>
#include <utpp/AssertException.h>
#include <utpp/CurrentTest.h>
#include <sstream>

namespace UnitTest {

TestList& Test::GetTestList ()
{
  static TestList s_list;
  return s_list;
}

Test::Test (char const* testName, char const* suiteName, char const* filename, int lineNumber)
  : m_details (testName, suiteName, filename, lineNumber)
  , next (0)
  , m_timeConstraintExempt (false)
{
}

Test::~Test ()
{
}

void Test::Run ()
{
//  ExecuteTest (*this, m_details);
  CurrentTest.Details = &m_details;

  try
  {
    RunImpl ();
  }
  catch (const AssertException& e)
  {
    m_details.filename = e.Filename ();
    m_details.lineNumber = e.LineNumber ();
    CurrentTest.Results->OnTestFailure (m_details, e.what ());
  }
  catch (const std::exception& e)
  {
    std::stringstream stream;
    stream << "Unhandled exception: " << e.what ();
    CurrentTest.Results->OnTestFailure (m_details, stream.str ());
  }
  catch (...)
  {
    CurrentTest.Results->OnTestFailure (m_details, "Unhandled exception: Crash!");
  }

}

/*
void Test::RunImpl ()
{
}
*/
}
