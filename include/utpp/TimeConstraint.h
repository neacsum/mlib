#pragma once

#include "TimeHelpers.h"

namespace UnitTest {

class TestResults;
class TestDetails;

class TimeConstraint
{
public:
  TimeConstraint (int ms, TestDetails const& details);
  ~TimeConstraint ();

private:
  void operator=(TimeConstraint const&);
  TimeConstraint (TimeConstraint const&);

  Timer timer;
  TestDetails const& details;
  int const maxMs;
};

#define UNITTEST_TIME_CONSTRAINT(ms) \
  UnitTest::TimeConstraint unitTest__timeConstraint__(ms, UnitTest::TestDetails(m_details, __LINE__))

#define UNITTEST_TIME_CONSTRAINT_EXEMPT() do { m_timeConstraintExempt = true; } while (0)

}
