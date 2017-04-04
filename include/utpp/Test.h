#pragma once
#include <string>
#include <cassert>
#include "test_suite.h"

namespace UnitTest {

class Test
{
public:
  Test (const char* testName);
  virtual ~Test ();
  void assign_suite (TestSuite* suite);
  void no_time_constraint ();

  const std::string& suite_name () const;
  int failure_count () const;
  int test_time_ms () const;
  const std::string& test_name () const;

  void ReportFailure (const std::string& filename, int line, const std::string& message);
  void run ();
  virtual void RunImpl () = 0;

protected:
  std::string name;
  TestSuite *suite;
  int failures;
  int time;

private:
  Test (Test const&);
  Test& operator =(Test const&);
  friend class TestSuite;
};

inline
void Test::assign_suite (TestSuite *suite_)
{
  suite = suite_;
}

inline
const std::string& Test::suite_name () const
{
  assert (suite);
  return suite->name;
}

inline 
int Test::failure_count () const
{
  return failures;
}

inline
int Test::test_time_ms () const
{
  return time;
}

inline
const std::string& Test::test_name () const
{
  return name;
}

inline
void Test::no_time_constraint ()
{
  time = -1;
}

//Currently executing test
extern Test* CurrentTest;
}
