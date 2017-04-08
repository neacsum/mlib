#pragma once
/*!
  \file utpp.h - Master include file

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include "test.h"
#include "test_suite.h"
#include "test_reporter.h"
#include "test_reporter_dbgout.h"
#include "xml_test_reporter.h"
#include "test_macros.h"

#include "check_macros.h"
#include "time_constraint.h"

namespace UnitTest {

/// Run all tests from all test suites
int RunAllTests (TestReporter& rpt = GetDefaultReporter (), int max_time_ms = 0);

/// Run all tests from one suite
int RunSuite (const char *suite_name, TestReporter& rpt = GetDefaultReporter (), int max_time_ms = 0);

}

#pragma comment (lib, "utpp.lib")
