#include <utpp/checks.h>

using namespace std;

namespace UnitTest {

static bool CheckStringsEqual (const char* expected, const char* actual, std::string& msg)
{
  if (strcmp(expected, actual))
  {
    std::stringstream stream;
    stream << "Expected \'" << expected << "\' but was \'" << actual << "\'";
    msg = stream.str ();
    return false;
  }
  return true;
}

bool CheckEqual (const std::string& expected, const std::string& actual, std::string& msg)
{
  return CheckStringsEqual (expected.c_str (), actual.c_str (), msg);
}

bool CheckEqual (const char* expected, const std::string& actual, std::string& msg)
{
  return CheckStringsEqual (expected, actual.c_str (), msg);
}

bool CheckEqual (const char* expected, char const* actual, string& msg)
{
  return CheckStringsEqual (expected, actual, msg);
}

bool CheckEqual (char* expected, char* actual, string& msg)
{
  return CheckStringsEqual (expected, actual, msg);
}

bool CheckEqual (char* expected, const char* actual, string& msg)
{
  return CheckStringsEqual (expected, actual, msg);
}

bool CheckEqual (char const* expected, char* actual, string& msg)
{
  return CheckStringsEqual (expected, actual, msg);
}

}
