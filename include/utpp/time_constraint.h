#pragma once

#include <string>
#include "time_helpers.h"

namespace UnitTest {

class TestResults;

class TimeConstraint
{
public:
  TimeConstraint (int ms, const char* file, int line);
  ~TimeConstraint ();

private:
  void operator=(TimeConstraint const&);
  TimeConstraint (TimeConstraint const&);

  Timer timer;
  int const max_ms;
  std::string filename;
  int line_number;
};

/// Defines a local time constraint
#define UNITTEST_TIME_CONSTRAINT(ms) \
  UnitTest::TimeConstraint unitTest__timeConstraint__(ms, __FILE__, __LINE__)

/// Flags a test as not subject to the global time constraint
#define UNITTEST_TIME_CONSTRAINT_EXEMPT() no_time_constraint ()

}
