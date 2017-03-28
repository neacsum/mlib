#pragma once

#include "TimeHelpers.h"
#include "TestDetails.h"

namespace UnitTest {

class TestResults;

class TimeConstraint
{
public:
  TimeConstraint (int ms, TestDetails const& details);
  ~TimeConstraint ();

private:
  void operator=(TimeConstraint const&);
  TimeConstraint (TimeConstraint const&);

  Timer timer;
  TestDetails details;
  int const maxMs;
};

#define UNITTEST_TIME_CONSTRAINT(ms) \
  UnitTest::TimeConstraint unitTest__timeConstraint__(ms, (m_details.lineNumber = __LINE__, m_details))

#define UNITTEST_TIME_CONSTRAINT_EXEMPT() do { m_timeConstraintExempt = true; } while (0)

}
