#pragma once

namespace UnitTest {

class TestDetails;

class TestReporter
{
public:
    virtual ~TestReporter();

    virtual void ReportTestStart(TestDetails const& test);
    virtual void ReportFailure(TestDetails const& test, char const* failure) = 0;
    virtual void ReportTestFinish(TestDetails const& test, float secondsElapsed);
    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) = 0;
};

}
