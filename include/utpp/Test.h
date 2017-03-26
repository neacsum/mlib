#pragma once

#include "TestDetails.h"

#define DEFAULT_SUITE "DefaultSuite"

namespace UnitTest {

class TestResults;
class TestList;

class Test
{
public:
  Test (char const* testName, char const* suiteName = DEFAULT_SUITE, char const* filename = "", int lineNumber = 0);
  virtual ~Test ();
  void Run ();

  TestDetails m_details;
  Test* next;
  mutable bool m_timeConstraintExempt;

  static TestList& GetTestList ();

  virtual void RunImpl ();

private:
  Test (Test const&);
  Test& operator =(Test const&);
};


}
