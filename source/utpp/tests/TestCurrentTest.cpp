#include <utpp/utpp.h>
#include <utpp/CurrentTest.h>

#include "ScopedCurrentTest.h"

using namespace UnitTest;

TEST (CanSetandGetDetails)
{
  bool ok = false;
  {
    ScopedCurrentTest scopedTest;

    TestDetails* details = reinterpret_cast<TestDetails*>(12345);
    CurrentTest.Details = details;

    ok = (CurrentTest.Details == details);
  }

  CHECK (ok);
}

TEST (CanSetAndGetResults)
{
  bool ok = false;
  {
    ScopedCurrentTest scopedTest;

    TestResults results;
    CurrentTest.Results = &results;

    ok = (CurrentTest.Results == &results);
  }

  CHECK (ok);
}
