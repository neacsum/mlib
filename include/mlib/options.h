/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/*!
  \file options.h Command line parser class

  Can parse a command line based on options descriptions like these:
```C
    const char *optlist[] {
      "a? optional_arg \t -a can have an argument example: -a 1 or -a xyz",
      "b: required_arg \t -b must be followed by an argument example: -b mmm",
      "c+ one_or_more_args \t -c can be followed by one or more arguments example: -c 12 ab cd",
      "d* 0_or_more_args \t -d can have zero or more arguments",
      "e| \t -e doesn't have any arguments",
      "f?longorshort optional \t -f can be also written as --longorshort",
      ":longopt required \t --longopt must have an argument",
      0 };
```
*/
#pragma once

#include <string>
#include <vector>

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

class OptParser
{
public:
  explicit OptParser ();

  /// Initializes parser and sets the list of valid options.
  OptParser (std::vector<const char*>& list);

  /// Initializes parser and sets the list of valid options.
  OptParser (std::initializer_list<const char*> list);

  /// Initializes parser and sets the list of options descriptors.
  OptParser (const char** list);

  /// Set list of valid options
  void set_options (std::vector<const char*>& list);

  /// Add a new option descriptor to the list of options.
  void add_option (const char* descr);

  /// Parse a command line
  int parse (int argc, const char* const* argv, int* stop = nullptr);

  /// Parse a vector of string arguments
  int parse (const std::vector<std::string>& args, int* stop = nullptr);

  ///@{
  /// Return next option in command line
  bool next (std::string& opt, std::string& optarg, char sep = '|');

  /// Return next option in command line
  bool next (std::string& opt, std::vector<std::string>& optarg);
  ///@}

  ///@{
  /// Return a specific option from the command line
  int getopt (const std::string& option, std::string& optarg, char sep = '|') const;

  /// Return a specific option from the command line
  int getopt (const std::string& option, std::vector<std::string>& optarg) const;

  /// Return a specific option from the command line
  int getopt (char option, std::string& optarg, char sep = '|') const;

  /// Return a specific option from the command line
  int getopt (char option, std::vector<std::string>& optarg) const;
  ///@}

  ///@{
  /// Check if command line has an option
  bool hasopt (const std::string& option) const;

  /// Check if command line has an option
  bool hasopt (char option) const;
  ///@}
  
  /// Return a nicely formatted syntax string containing all the options
  const std::string synopsis () const;

  /// Return options description
  const std::string description (size_t indent_size = 2) const;

  /// Return program name
  const std::string& appname () const;

private:
  struct opt
  {
    char oshort;                  // short form
    std::string olong;            // long form
    char flag;                    // argument type
    std::string arg_descr;        // argument description
    std::vector<std::string> arg; // actual argument(s)
    int count;                    // number of occurrences
  };

  std::vector<opt>::const_iterator find_option (const std::string& opt) const;
  std::vector<opt>::const_iterator find_option (char opt) const;
  void format_arg (std::string& str, const opt& option, char sep) const;

  std::vector<opt> optlist;
  std::vector<opt> cmd;
  std::vector<opt>::iterator nextop;
  std::string app;
};

/*!
  This is the content of `argv[0]` without any path or extension.
*/
inline const std::string& OptParser::appname () const
{
  return app;
}

/*!
  \param  option  long or short form of the option
*/
inline bool OptParser::hasopt (const std::string& option) const
{
  return find_option (option) != cmd.end ();
}

/*!
  Check if command line has an option
  \param  option  short form of the option
*/
inline bool OptParser::hasopt (char option) const
{
  return find_option (option) != cmd.end ();
}

/*!
  \param  option  the requested option
  \param  optarg  option argument(s)

  \return  number of occurrences on command line
*/

inline int OptParser::getopt (const std::string& option, std::vector<std::string>& optarg) const
{
  optarg.clear ();
  auto p = find_option (option);
  if (p == cmd.end ())
    return 0;

  optarg = p->arg;
  return p->count;
}

/*!
  \param  option  the requested option
  \param  optarg  option argument(s)

  \return  number of occurrences on command line
*/
inline int OptParser::getopt (char option, std::vector<std::string>& optarg) const
{
  optarg.clear ();
  auto p = find_option (option);
  if (p == cmd.end ())
    return 0;

  optarg = p->arg;
  return p->count;
}

} // namespace mlib
