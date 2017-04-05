/*!
  \file suites_list.cpp - Implementation of SuitesList class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <utpp/suites_list.h>

namespace UnitTest {

SuitesList::SuitesList ()
{
}

SuitesList::~SuitesList ()
{
  for (auto p = suites.begin (); p != suites.end (); p++)
    delete *p;
}

void SuitesList::Add (const char *suite_name, TestSuite::maker_info& inf)
{
  TestSuite *suite;
  auto p = suites.cbegin ();
  while (p != suites.cend ())
  {
    if ((*p)->name == suite_name)
      break;
    p++;
  }
  if (p == suites.cend ())
  {
    suite = new TestSuite (suite_name);
    suites.push_back (suite);
  }
  else
    suite = *p;
  suite->Add (inf);
}

int SuitesList::Run (const char *suite_name, TestReporter& reporter, int max_time_ms)
{
  for (auto p = suites.cbegin (); p != suites.cend (); p++)
  {
    if ((*p)->name == suite_name)
      return (*p)->RunTests (reporter, max_time_ms);
  }
  return -1;
}

int SuitesList::RunAll (TestReporter& reporter, int max_time_ms)
{
  int ret = -1;
  for (auto p = suites.cbegin (); p != suites.cend (); p++)
    ret += (*p)->RunTests (reporter, max_time_ms);

  return ret;
}

SuitesList& SuitesList::GetSuitesList ()
{
  static SuitesList all_suites;
  return all_suites;
}

//////////////////////////// SuiteAdder ///////////////////////////////////////
SuiteAdder::SuiteAdder (const char *suite_name,
                        const std::string& test_name,
                        const std::string& file,
                        int line,
                        Testmaker func)
{
  TestSuite::maker_info inf{ test_name, file, line, func };
  SuitesList::GetSuitesList ().Add (suite_name, inf);
}

}