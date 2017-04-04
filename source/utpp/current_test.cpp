#include <utpp/current_test.h>
#include <cassert>

namespace UnitTest {

struct_CurrentTest CurrentTest;
static TestResults* saved_results = 0;

void struct_CurrentTest::OnTestFailure (const std::string& file, int location, const std::string& what)
{
  Details->lineNumber = location;
  Details->filename = file;
  Results->OnTestFailure (*Details, what);
}

void struct_CurrentTest::PushResults (TestResults* new_results)
{
  assert (!saved_results);
  saved_results = Results;
  Results = new_results;
}

void struct_CurrentTest::PopResults ()
{
  assert (!saved_results);
  Results = saved_results;
}


}
