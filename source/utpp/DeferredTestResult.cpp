#include <utpp/DeferredTestResult.h>

namespace UnitTest
{

DeferredTestResult::DeferredTestResult ()
  : suiteName ("")
  , testName ("")
  , failureFile ("")
  , timeElapsed (0.0f)
  , failed (false)
{
}

DeferredTestResult::DeferredTestResult (const std::string& suite, const std::string& test)
  : suiteName (suite)
  , testName (test)
  , failureFile ("")
  , timeElapsed (0.0f)
  , failed (false)
{
}

}
