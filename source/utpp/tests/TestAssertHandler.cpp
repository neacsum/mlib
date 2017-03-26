#include <utpp/utpp.h>
#include <utpp/AssertException.h>
#include <utpp/ReportAssert.h>

using namespace UnitTest;

TEST (ReportAssertThrowsAssertException)
{
  bool caught = false;

  try
  {
    ReportAssert ("", "", 0);
  }
  catch (AssertException const&)
  {
    caught = true;
  }

  CHECK (true == caught);
}

TEST (ReportAssertSetsCorrectInfoInException)
{
  const int lineNumber = 12345;
  const char* description = "description";
  const char* filename = "filename";

  try
  {
    ReportAssert (description, filename, lineNumber);
  }
  catch (AssertException const& e)
  {
    CHECK_EQUAL (description, e.what ());
    CHECK_EQUAL (filename, e.Filename ());
    CHECK_EQUAL (lineNumber, e.LineNumber ());
  }
}

