#pragma once
/*!
  \file test_macros.h - Macro-definitions for test generation.

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include "assert_exception.h"
#include "test.h"
#include <sstream>


#ifdef TEST
  #error UnitTest++ redefines TEST
#endif

#ifdef TEST_EX
  #error UnitTest++ redefines TEST_EX
#endif

#ifdef TEST_FIXTURE_EX
  #error UnitTest++ redefines TEST_FIXTURE_EX
#endif

/// Declares the beginning of a new test suite
#define SUITE(Name)                                                           \
  namespace Suite##Name                                                       \
  {                                                                           \
      inline char const* GetSuiteName () { return #Name ; }                   \
  }                                                                           \
  namespace Suite##Name

/// Defines a test case
#define TEST(Name)                                                            \
  class Test##Name : public UnitTest::Test                                    \
  {                                                                           \
  public:                                                                     \
    Test##Name() : Test(#Name) {}                                             \
  private:                                                                    \
    void RunImpl();                                                           \
  };                                                                          \
  UnitTest::Test* Name##maker() {return new Test##Name; };                    \
  UnitTest::SuiteAdder Name##adder (GetSuiteName(), #Name, __FILE__, __LINE__,\
    Name##maker);                                                             \
  void Test##Name::RunImpl()

/// Defines a test case with an associated fixture
#define TEST_FIXTURE(Fixture, Name)                                           \
  class Test##Fixture##Name : public Fixture, public UnitTest::Test           \
  {                                                                           \
  public:                                                                     \
    Test##Fixture##Name() : Fixture (), Test(#Fixture #Name) {}               \
  private:                                                                    \
    void RunImpl();                                                           \
  };                                                                          \
  UnitTest::Test* Fixture##Name##maker() {return new Test##Fixture##Name;};   \
  UnitTest::SuiteAdder Fixture##Name##adder (GetSuiteName(), #Fixture #Name,  \
    __FILE__, __LINE__, Fixture##Name##maker);                                \
  void Test##Fixture##Name::RunImpl()

