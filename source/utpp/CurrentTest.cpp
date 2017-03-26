#include <utpp/CurrentTest.h>
#include <cstddef>

namespace UnitTest {

struct_CurrentTest CurrentTest;

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
