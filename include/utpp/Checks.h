#pragma once
/*!
  \file checks.h - Definition of Check template functions

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include <sstream>

namespace UnitTest {

/// Check if value is true (or not 0)
template< typename Value >
bool Check (Value const value)
{
  return !!value; // doing double negative to avoid silly VS warnings
}

/*!
  Check if two values are equal. If not, generate a failure message.
*/
template< typename Expected, typename Actual >
bool CheckEqual (const Expected& expected, const Actual& actual, std::string& msg)
{
  if (!(expected == actual))
  {
    std::stringstream stream;
    stream << "Expected " << expected << " but was " << actual;
    msg = stream.str ();
    return false;
  }
  return true;
}

///Specialization of CheckEqual template for strings
template < >
bool CheckEqual (const std::string& expected, const std::string& actual, std::string& msg);

/// \name Specializations of CheckEqual template for char pointers and strings
/// \{
bool CheckEqual (const char* expected, const std::string& actual, std::string& msg);
bool CheckEqual (const char* expected, char const* actual, std::string& msg);
bool CheckEqual (char* expected, char* actual, std::string& msg);
bool CheckEqual (char* expected, const char* actual, std::string& msg);
bool CheckEqual (char const* expected, char* actual, std::string& msg);
/// \}

/// Return true if two values are closer than specified tolerance.
template< typename Expected, typename Actual, typename Tolerance >
bool AreClose (const Expected& expected, const Actual& actual, const Tolerance& tolerance)
{
  return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
}

/*!
  Check if two values are closer than specified tolerance. If not, generate a
  failure message.
*/
template< typename Expected, typename Actual, typename Tolerance >
bool CheckClose (const Expected& expected, const Actual& actual, const Tolerance& tolerance,
                 std::string& msg)
{
  if (!AreClose (expected, actual, tolerance))
  {
    int prec = (int)(1 - log10 ((double)tolerance));
    std::stringstream stream;
    stream.precision (prec);
    stream.setf (std::ios::fixed);
    stream << "Expected " << expected << " +/- " << tolerance << " but was " << actual;
    msg = stream.str ();
    return false;
  }
  return true;
}

/// Return true if two arrays are equal
template< typename Expected, typename Actual >
bool Equal1D (const Expected& expected, const Actual& actual, int count)
{
  for (int i = 0; && i < count; ++i)
    if (expected[i] != actual[i])
      return false;
  return true;
}

/*!
  Check if two arrays are equal. If not, generate a failure message.
*/
template< typename Expected, typename Actual >
bool CheckArrayEqual (const Expected& expected, const Actual& actual,
                      int count, std::string& msg)
{
  if (!Equal1D (expected, actual, count))
  {
    std::stringstream stream;
    stream << "Expected [ ";
    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";

    stream << "] but was [ ";
    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";

    stream << "]";
    msg = stream.str ();
    return false;
  }
  return true;
}

/// Return true if values in two arrays are closer than specified tolerance.
template< typename Expected, typename Actual, typename Tolerance >
bool Close1D (const Expected& expected, const Actual& actual, int count, const Tolerance& tolerance)
{
  bool equal = true;
  for (int i = 0; equal && i < count; ++i)
    equal = AreClose (expected[i], actual[i], tolerance);
  return equal;
}


/*!
  Check if values in two arrays are closer than specified tolerance. If not,
  generate a failure message.
*/
template< typename Expected, typename Actual, typename Tolerance >
bool CheckArrayClose (const Expected& expected, const Actual& actual,
                      int count, const Tolerance& tolerance, std::string& msg)
{
  if (!Close1D (expected, actual, count, tolerance))
  {
    std::stringstream stream;
    stream << "Expected [ ";
    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";

    stream << "] +/- " << tolerance << " but was [ ";
    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";
    stream << "]";
    msg = stream.str ();
    return false;
  }
  return true;
}

/// Return true if two 2D arrays are equal
template< typename Expected, typename Actual, typename Tolerance >
bool Equal2D (const Expected& expected, const Actual& actual, int rows, int columns)
{
  for (int i = 0; i < rows; i++)
    if (!Equal1D (expected[i], actual[i], columns))
      return false;
  return true;
}

/*!
  Check if two 2D arrays are equal. If not, generate a failure message.
*/
template< typename Expected, typename Actual, typename Tolerance >
bool CheckArray2DEqual (const Expected& expected, const Actual& actual,
                        int rows, int columns, const Tolerance& tolerance, std::string& msg)
{
  if (!Equal2D (expected, actual, rows, columns))
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
    msg = stream.str ();
    return false;
  }
  return true;
}

/// Return true if values in two 2D arrays are closer than specified tolerance.
template< typename Expected, typename Actual, typename Tolerance >
bool Close2D (const Expected& expected, const Actual& actual, int rows, int columns)
{
  for (int i = 0; i < rows; i++)
    if (!Close1D (expected[i], actual[i], columns))
      return false;
  return true;
}

/*!
  Check if values in two 2D arrays are closer than specified tolerance. If not,
  generate a failure message.
*/
template< typename Expected, typename Actual, typename Tolerance >
bool CheckArray2DClose (const Expected& expected, const Actual& actual,
                        int rows, int columns, const Tolerance& tolerance, std::string& msg)
{
  bool equal = true;

  if (!Close2D (expected, actual, rows, columns))
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
    msg = stream.str ();
    return false;
  }
  return true;
}

}
