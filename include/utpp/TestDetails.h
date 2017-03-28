#pragma once
#include <string>

namespace UnitTest {

class TestDetails
{
public:
  TestDetails (const char* testName, const char* suiteName, const char* filename, int lineNumber);
//  TestDetails (const TestDetails& details, int lineNumber);

  std::string suiteName;
  std::string testName;
  std::string filename;
  int lineNumber;
};

}
