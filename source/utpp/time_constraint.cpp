#include <utpp/time_constraint.h>
#include <utpp/test.h>

#include <sstream>
#include <cassert>

namespace UnitTest {


TimeConstraint::TimeConstraint (int ms, const char* file, int line)
  : filename (file)
  , line_number (line)
  , max_ms (ms)
{
  timer.Start ();
}

TimeConstraint::~TimeConstraint ()
{
  int t = timer.GetTimeInMs ();
  if (t > max_ms)
  {
    assert (CurrentTest);
    std::stringstream stream;
    stream << "Time constraint failed. Expected to run test under " << max_ms <<
      "ms but took " << t << "ms.";

    CurrentTest->ReportFailure (filename, line_number, stream.str ());
  }
}

}
