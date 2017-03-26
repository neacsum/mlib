#pragma once
#include "Config.h"
#include "ExecuteTest.h"
#include "AssertException.h"
#include "TestDetails.h"
#include "MemoryOutStream.h"

#ifdef TEST
  #error UnitTest++ redefines TEST
#endif

#ifdef TEST_EX
  #error UnitTest++ redefines TEST_EX
#endif

#ifdef TEST_FIXTURE_EX
  #error UnitTest++ redefines TEST_FIXTURE_EX
#endif

//
#define SUITE(Name)                                                           \
  namespace Suite##Name                                                       \
  {                                                                           \
      inline char const* GetSuiteName () { return #Name ; }                   \
  }                                                                           \
  namespace Suite##Name

#define TEST_EX(Name, List)                                                   \
  class Test##Name : public UnitTest::Test                                    \
  {                                                                           \
  public:                                                                     \
    Test##Name() : Test(#Name, GetSuiteName(), __FILE__, __LINE__) {}         \
  private:                                                                    \
    virtual void RunImpl();                                                   \
  } test##Name##Instance;                                                     \
                                                                              \
  UnitTest::ListAdder adder##Name (List, &test##Name##Instance);              \
                                                                              \
  void Test##Name::RunImpl()


#define TEST(Name) TEST_EX(Name, UnitTest::Test::GetTestList())


#define TEST_FIXTURE_EX(Fixture, Name, List)                                  \
  class Fixture##Name##Helper : public Fixture                                \
  {                                                                           \
  public:                                                                     \
    Fixture##Name##Helper()  {}                                               \
    void RunImpl();                                                           \
  };                                                                          \
                                                                              \
  class Test##Fixture##Name : public UnitTest::Test                           \
  {                                                                           \
  public:                                                                     \
    Test##Fixture##Name() : Test(#Name, GetSuiteName (), __FILE__, __LINE__) {}\
  private:                                                                    \
    virtual void RunImpl();                                                   \
  } test##Fixture##Name##Instance;                                            \
                                                                              \
  UnitTest::ListAdder adder##Fixture##Name (List, &test##Fixture##Name##Instance); \
                                                                              \
  void Test##Fixture##Name::RunImpl()                                         \
  {                                                                           \
    bool ctorOk = false;                                                      \
    try {                                                                     \
      Fixture##Name##Helper fixtureHelper;                                    \
      ctorOk = true;                                                          \
      UnitTest::ExecuteTest(fixtureHelper, m_details);                        \
    }                                                                         \
    catch (const UnitTest::AssertException& e)                                \
    {                                                                         \
      UnitTest::CurrentTest.Results->OnTestFailure(UnitTest::TestDetails(m_details.testName, m_details.suiteName, e.Filename(), e.LineNumber()), e.what()); \
    }                                                                         \
    catch (const std::exception& e)                                           \
    {                                                                         \
      UnitTest::MemoryOutStream stream;                                       \
      stream << "Unhandled exception: " << e.what();                          \
      UnitTest::CurrentTest.Results->OnTestFailure(m_details, stream.GetText()); \
    }                                                                         \
    catch (...) {                                                             \
      if (ctorOk)                                                             \
      {                                                                       \
        UnitTest::CurrentTest.Results->OnTestFailure(UnitTest::TestDetails(m_details, __LINE__), \
        "Unhandled exception while destroying fixture " #Fixture);            \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        UnitTest::CurrentTest.Results->OnTestFailure(UnitTest::TestDetails(m_details, __LINE__), \
          "Unhandled exception while constructing fixture " #Fixture);        \
      }                                                                       \
    }                                                                         \
  }                                                                           \
  void Fixture##Name##Helper::RunImpl()

#define TEST_FIXTURE(Fixture,Name) TEST_FIXTURE_EX(Fixture, Name, UnitTest::Test::GetTestList())

