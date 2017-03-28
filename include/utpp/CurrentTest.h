#pragma once
#include "TestDetails.h"
#include "TestResults.h"
#include <string>

namespace UnitTest {


struct struct_CurrentTest
{
  TestResults* Results;
  TestDetails* Details;
  void OnTestFailure (int where, const std::string& what);
};

extern struct_CurrentTest CurrentTest;
}
