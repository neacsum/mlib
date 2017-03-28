#include <utpp/Checks.h>

namespace UnitTest {

static void CheckStringsEqual (TestResults& results, char const* expected, char const* actual,
                        TestDetails const& details)
{
  using namespace std;

  if (strcmp (expected, actual))
  {
    std::stringstream stream;
    stream << "Expected " << expected << " but was " << actual;

    results.OnTestFailure (details, stream.str ());
  }
}


void CheckEqual (TestResults& results, char const* expected, char const* actual,
                 TestDetails const& details)
{
  CheckStringsEqual (results, expected, actual, details);
}

void CheckEqual (TestResults& results, char* expected, char* actual,
                 TestDetails const& details)
{
  CheckStringsEqual (results, expected, actual, details);
}

void CheckEqual (TestResults& results, char* expected, char const* actual,
                 TestDetails const& details)
{
  CheckStringsEqual (results, expected, actual, details);
}

void CheckEqual (TestResults& results, char const* expected, char* actual,
                 TestDetails const& details)
{
  CheckStringsEqual (results, expected, actual, details);
}


}
