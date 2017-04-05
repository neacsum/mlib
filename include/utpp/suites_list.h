#pragma once
/*!
  \file suites_list.h - Definition of SuitesList class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <deque>
#include "test_suite.h"

namespace UnitTest {
class TestReporter;

class SuitesList {
public:
  SuitesList ();
  ~SuitesList ();
  void Add (const char *suite, TestSuite::maker_info& inf);
  int Run (const char *suite, TestReporter& reporter, int max_time_ms);
  int RunAll (TestReporter& reporter, int max_time_ms);
  static SuitesList& GetSuitesList ();

private:
  std::deque <TestSuite *> suites;
};

}
