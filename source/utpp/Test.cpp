#include <utpp/Config.h>
#include <utpp/Test.h>
#include <utpp/TestList.h>
#include <utpp/TestResults.h>
#include <utpp/AssertException.h>
#include <utpp/MemoryOutStream.h>
#include <utpp/ExecuteTest.h>


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
  ExecuteTest (*this, m_details);
}

void Test::RunImpl ()
{
}

}
