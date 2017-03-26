#pragma once
#include "TestDetails.h"
#include "TestResults.h"

namespace UnitTest {


struct struct_CurrentTest
{
  TestResults* Results;
  TestDetails* Details;
};

extern struct_CurrentTest CurrentTest;
}
