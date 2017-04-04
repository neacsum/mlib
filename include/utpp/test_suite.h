#pragma once
/*!
  \file test_suite.h - Definition of TestSuite class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <string>
#include <deque>
#include "test_reporter.h"
#include "time_helpers.h"

#define DEFAULT_SUITE "DefaultSuite"

namespace UnitTest {

class Test;

/// Function pointer to a function that creates a test object
typedef UnitTest::Test* (*Testmaker)();

/// @brief A set of test cases that are run together
class TestSuite
{
public:
  struct maker_info {
    std::string name;
    std::string file;
    int line;
    Testmaker func;
  };

  TestSuite (const char *name);
  void Add (maker_info& inf);
  int RunTests (TestReporter& reporter, int max_runtime_ms);
  std::string name;

private:
  std::deque <maker_info> test_list;
  int max_runtime;

  bool SetupCurrentTest (maker_info& inf);
  void RunCurrentTest (maker_info& inf);
  void TearDownCurrentTest (maker_info& inf);
};

/// Constructor of this objects inserts the test in suite
class SuiteAdder
{
public:
  SuiteAdder (const char *suite_name, 
              const std::string& test_name, 
              const std::string& file, 
              int line,
              Testmaker func);
};

/// Run all tests from all test suites
int RunAllTests (TestReporter& rpt = GetDefaultReporter (), int max_time_ms = 0);

/// Run all tests from one suite
int RunSuite (const char *suite_name, TestReporter& rpt = GetDefaultReporter (), int max_time_ms = 0);
}

/// Return current suite name
inline const char* GetSuiteName ()
{
  return DEFAULT_SUITE;
}

