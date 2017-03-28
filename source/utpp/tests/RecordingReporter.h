#ifndef UNITTEST_RECORDINGREPORTER_H
#define UNITTEST_RECORDINGREPORTER_H

#include <utpp/TestReporter.h>
#include <cstring>

#include <utpp/TestDetails.h>

struct RecordingReporter : public UnitTest::TestReporter
{
private:
  enum { kMaxStringLength = 256 };

public:
  RecordingReporter ()
    : testRunCount (0)
    , testFailedCount (0)
    , lastFailedLine (0)
    , testFinishedCount (0)
    , lastFinishedTestTime (0)
    , summaryTotalTestCount (0)
    , summaryFailedTestCount (0)
    , summaryFailureCount (0)
    , summarySecondsElapsed (0)
  {
    lastStartedSuite[0] = '\0';
    lastStartedTest[0] = '\0';
    lastFailedFile[0] = '\0';
    lastFailedSuite[0] = '\0';
    lastFailedTest[0] = '\0';
    lastFailedMessage[0] = '\0';
    lastFinishedSuite[0] = '\0';
    lastFinishedTest[0] = '\0';
  }

  virtual void ReportTestStart (const UnitTest::TestDetails& test)
  {
    using namespace std;

    ++testRunCount;
    strcpy (lastStartedSuite, test.suiteName.c_str ());
    strcpy (lastStartedTest, test.testName.c_str ());
  }

  virtual void ReportFailure (const UnitTest::TestDetails& test, const std::string& failure)
  {
    using namespace std;

    ++testFailedCount;
    strcpy (lastFailedFile, test.filename.c_str ());
    lastFailedLine = test.lineNumber;
    strcpy (lastFailedSuite, test.suiteName.c_str ());
    strcpy (lastFailedTest, test.testName.c_str ());
    strcpy (lastFailedMessage, failure.c_str());
  }

  virtual void ReportTestFinish (const UnitTest::TestDetails& test, float testDuration)
  {
    using namespace std;

    ++testFinishedCount;
    strcpy (lastFinishedSuite, test.suiteName.c_str ());
    strcpy (lastFinishedTest, test.testName.c_str ());
    lastFinishedTestTime = testDuration;
  }

  virtual void ReportSummary (int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed)
  {
    summaryTotalTestCount = totalTestCount;
    summaryFailedTestCount = failedTestCount;
    summaryFailureCount = failureCount;
    summarySecondsElapsed = secondsElapsed;
  }

  int testRunCount;
  char lastStartedSuite[kMaxStringLength];
  char lastStartedTest[kMaxStringLength];

  int testFailedCount;
  char lastFailedFile[kMaxStringLength];
  int lastFailedLine;
  char lastFailedSuite[kMaxStringLength];
  char lastFailedTest[kMaxStringLength];
  char lastFailedMessage[kMaxStringLength];

  int testFinishedCount;
  char lastFinishedSuite[kMaxStringLength];
  char lastFinishedTest[kMaxStringLength];
  float lastFinishedTestTime;

  int summaryTotalTestCount;
  int summaryFailedTestCount;
  int summaryFailureCount;
  float summarySecondsElapsed;
};


struct EmptyTest : public UnitTest::Test
{
  EmptyTest (const char* testName, const char* suiteName = DEFAULT_SUITE)
    : Test (testName, suiteName) {};
  void RunImpl () {};
};


#endif
