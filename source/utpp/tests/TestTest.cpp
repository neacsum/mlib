#include <utpp/utpp.h>
#include <utpp/TestReporter.h>
#include <utpp/TimeHelpers.h>

#include "ScopedCurrentTest.h"
#include "RecordingReporter.h"

using namespace UnitTest;
extern TestReporter null_reporter;

TEST (PassingTestHasNoFailures)
{
  class PassingTest : public Test
  {
  public:
    PassingTest () : Test ("passing") {}
    void RunImpl ()
    {
      CHECK (true);
    }
  };

  TestResults results (null_reporter);
  {
    ScopedCurrentTest scopedResults (results);
    PassingTest ().Run ();
  }

  CHECK_EQUAL (0, results.GetFailureCount ());
}


TEST (FailingTestHasFailures)
{
  class FailingTest : public Test
  {
  public:
    FailingTest () : Test ("failing") {}
    void RunImpl ()
    {
      CHECK (false);
    }
  };

  TestResults results (null_reporter);
  {
    ScopedCurrentTest scopedResults (results);
    FailingTest ().Run ();
  }

  CHECK_EQUAL (1, results.GetFailureCount ());
}


TEST (ThrowingTestsAreReportedAsFailures)
{
  class CrashingTest : public Test
  {
  public:
    CrashingTest () : Test ("throwing") {}
    void RunImpl ()
    {
      throw "Blah";
    }
  };

  TestResults results (null_reporter);
  {
    ScopedCurrentTest scopedResult (results);
    CrashingTest ().Run ();
  }

  CHECK_EQUAL (1, results.GetFailureCount ());
}


#if _MSC_VER < 1800
TEST(CrashingTestsAreReportedAsFailures)
{
  class CrashingTest : public Test
  {
  public:
    CrashingTest() : Test("crashing") {}
    virtual void RunImpl() const
    {
      reinterpret_cast< void (*)() >(0)();
    }
  };

  TestResults results;
  {
    ScopedCurrentTest scopedResult(results);
    CrashingTest().Run();
  }

  CHECK_EQUAL(1, results.GetFailureCount());
}
#endif

TEST (TestWithUnspecifiedSuiteGetsDefaultSuite)
{
  EmptyTest test ("test");
  CHECK_EQUAL ("DefaultSuite", test.m_details.suiteName);
}

TEST (TestReflectsSpecifiedSuiteName)
{
  EmptyTest test ("test", "testSuite");
  CHECK_EQUAL ("testSuite", test.m_details.suiteName);
}

void Fail ()
{
  CHECK (false);
}

TEST (OutOfCoreCHECKMacrosCanFailTests)
{
  TestResults results (null_reporter);
  {
    ScopedCurrentTest scopedResult (results);
    Fail ();
  }

  CHECK_EQUAL (1, results.GetFailureCount ());
}

