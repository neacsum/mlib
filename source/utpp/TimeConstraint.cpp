#include <utpp/TimeConstraint.h>
#include <utpp/TestResults.h>
#include <utpp/CurrentTest.h>
#include <sstream>

namespace UnitTest {


TimeConstraint::TimeConstraint (int ms, const TestDetails& details_)
  : details (details_)
  , maxMs (ms)
{
  timer.Start ();
}

TimeConstraint::~TimeConstraint ()
{
  int const totalTimeInMs = timer.GetTimeInMs ();
  if (totalTimeInMs > maxMs)
  {
    std::stringstream stream;
    stream << "Time constraint failed. Expected to run test under " << maxMs <<
      "ms but took " << totalTimeInMs << "ms.";

    UnitTest::CurrentTest.Results->OnTestFailure (details, stream.str ());
  }
}

}
