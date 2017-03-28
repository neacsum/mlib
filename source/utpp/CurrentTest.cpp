#include <utpp/CurrentTest.h>
#include <cstddef>

namespace UnitTest {

struct_CurrentTest CurrentTest;

void struct_CurrentTest::OnTestFailure (int where, const std::string& what)
{
  Details->lineNumber = where;
  Results->OnTestFailure (*Details, what);
}

#if 0
TestResults*& CurrentTest::Results ()
{
  static TestResults* testResults = NULL;
  return testResults;
}

const TestDetails*& CurrentTest::Details ()
{
  static const TestDetails* testDetails = NULL;
  return testDetails;
}
#endif
}
