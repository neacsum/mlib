#include <utpp/test.h>
#include <utpp/test_suite.h>
#include <utpp/test_reporter.h>
#include <utpp/failure.h>

#include <sstream>

namespace UnitTest {

Test::Test (const char* testName)
  : name (testName)
  , suite (0)
  , failures (0)
  , time (0)
{
}

Test::~Test ()
{
}

void Test::run ()
{
  Timer test_timer;
  if (time >= 0)
    test_timer.Start ();

  RunImpl ();

  if (time >= 0)
    time = test_timer.GetTimeInMs ();
}

void Test::ReportFailure (const std::string& filename, int line, const std::string& message)
{
  failures++;
  CurrentReporter->ReportFailure (Failure(filename, line, message));
}

}
