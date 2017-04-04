#pragma once
/*!
  \file failure.h - Definition of Failure class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <string>

namespace UnitTest {

/// @brief The failure records the file name, the line number and a message
struct Failure {
  Failure (const std::string& filename, int line, const std::string& msg);
  std::string filename;
  std::string message;
  int line_number;
};

inline
Failure::Failure (const std::string& file, int line, const std::string& msg)
: filename (file)
, message (msg)
, line_number (line)
{
}

}