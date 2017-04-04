#pragma once
#include <string>

namespace UnitTest {

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