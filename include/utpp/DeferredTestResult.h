#pragma once

#include <string>
#include <vector>

namespace UnitTest
{

struct DeferredTestResult
{
  DeferredTestResult ();
  DeferredTestResult (const std::string& suite, const std::string& test);

  std::string suiteName;
  std::string testName;
  std::string failureFile;

  typedef std::pair< int, std::string > Failure;
  typedef std::vector< Failure > FailureVec;
  FailureVec failures;

  float timeElapsed;
  bool failed;
};

}
