#pragma once

#include "Config.h"
#include "TestResults.h"
#include <sstream>

namespace UnitTest {


template< typename Value >
bool Check(Value const value)
{
    return !!value; // doing double negative to avoid silly VS warnings
}


template< typename Expected, typename Actual >
void CheckEqual (TestResults& results, const Expected& expected, const Actual& actual, const TestDetails& details)
{
  if (!(expected == actual))
  {
    std::stringstream stream;
    stream << "Expected " << expected << " but was " << actual;

    results.OnTestFailure (details, stream.str());
  }
}

void CheckEqual(TestResults& results, const char* expected, const char* actual, const TestDetails& details);

void CheckEqual(TestResults& results, char* expected, char* actual, const TestDetails& details);

void CheckEqual(TestResults& results, char* expected, const char* actual, const TestDetails& details);

void CheckEqual(TestResults& results, const char* expected, char* actual, const TestDetails& details);

template< typename Expected, typename Actual, typename Tolerance >
bool AreClose (const Expected& expected, const Actual& actual, const Tolerance& tolerance)
{
  return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckClose (TestResults& results, const Expected& expected, const Actual& actual, const Tolerance& tolerance,
                 const TestDetails& details)
{
  if (!AreClose (expected, actual, tolerance))
  {
    int prec = (int)(1 - log10 ((double)tolerance));
    std::stringstream stream;
    stream.precision (prec);
    stream.setf (std::ios::fixed);
    stream << "Expected " << expected << " +/- " << tolerance << " but was " << actual;
    results.OnTestFailure (details, stream.str ());
  }
}


template< typename Expected, typename Actual >
void CheckArrayEqual (TestResults& results, const Expected& expected, const Actual& actual,
                      int count, const TestDetails& details)
{
  bool equal = true;
  for (int i = 0; i < count; ++i)
    equal &= (expected[i] == actual[i]);

  if (!equal)
  {
    std::stringstream stream;

    stream << "Expected [ ";

    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";

    stream << "] but was [ ";

    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";

    stream << "]";

    results.OnTestFailure (details, stream.str());
  }
}

template< typename Expected, typename Actual, typename Tolerance >
bool ArrayAreClose (const Expected& expected, const Actual& actual, int count, const Tolerance& tolerance)
{
  bool equal = true;
  for (int i = 0; equal && i < count; ++i)
    equal = AreClose (expected[i], actual[i], tolerance);
  return equal;
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckArrayClose (TestResults& results, const Expected& expected, const Actual& actual,
                      int count, const Tolerance& tolerance, const TestDetails& details)
{
  bool equal = ArrayAreClose (expected, actual, count, tolerance);

  if (!equal)
  {
    std::stringstream stream;

    stream << "Expected [ ";
    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";
    stream << "] +/- " << tolerance << " but was [ ";

    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";
    stream << "]";

    results.OnTestFailure (details, stream.str());
  }
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckArray2DClose (TestResults& results, const Expected& expected, const Actual& actual,
                        int rows, int columns, const Tolerance& tolerance, const TestDetails& details)
{
  bool equal = true;
  for (int i = 0; equal && i < rows; ++i)
    equal = ArrayAreClose (expected[i], actual[i], columns, tolerance);

  if (!equal)
  {
    std::stringstream stream;

    stream << "Expected [ ";

    for (int expectedRow = 0; expectedRow < rows; ++expectedRow)
    {
      stream << "[ ";
      for (int expectedColumn = 0; expectedColumn < columns; ++expectedColumn)
        stream << expected[expectedRow][expectedColumn] << " ";
      stream << "] ";
    }

    stream << "] +/- " << tolerance << " but was [ ";

    for (int actualRow = 0; actualRow < rows; ++actualRow)
    {
      stream << "[ ";
      for (int actualColumn = 0; actualColumn < columns; ++actualColumn)
        stream << actual[actualRow][actualColumn] << " ";
      stream << "] ";
    }

    stream << "]";

    results.OnTestFailure (details, stream.str());
  }
}

}
