#pragma once

#include "TestDetails.h"

#define DEFAULT_SUITE "DefaultSuite"

namespace UnitTest {

class TestResults;
class TestList;

class Test
{
public:
  Test (const char* testName, const char* suiteName = DEFAULT_SUITE, const char* filename = "", int lineNumber = 0);
  virtual ~Test ();
  void Run ();

  TestDetails m_details;
  Test* next;
  mutable bool m_timeConstraintExempt;

  static TestList& GetTestList ();

  virtual void RunImpl () = 0;

private:
  Test (Test const&);
  Test& operator =(Test const&);
};


}
