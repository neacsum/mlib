#pragma once
/*!
  \file test.h - Definition of Test class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <string>
#include <cassert>
#include "test_suite.h"

namespace UnitTest {

/// @brief %Test case
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

  void failure ();
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

/// Assign the test to a TestSuite
inline
void Test::assign_suite (TestSuite *suite_)
{
  suite = suite_;
}

/// Return the name of the currently assigned suite 
inline
const std::string& Test::suite_name () const
{
  static const std::string nil ("");
  return suite?suite->name : nil;
}

/// Return the number of failures in this test
inline 
int Test::failure_count () const
{
  return failures;
}

/// Return test running time in milliseconds
inline
int Test::test_time_ms () const
{
  return time;
}

/// Return test name
inline
const std::string& Test::test_name () const
{
  return name;
}

/// Flags the test as exempt from global time constraint
inline
void Test::no_time_constraint ()
{
  time = -1;
}

///Currently executing test
extern Test* CurrentTest;

/// Main error reporting function
void ReportFailure (const std::string& filename, int line, const std::string& message);
}
