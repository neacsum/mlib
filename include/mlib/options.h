/*!
  \file options.h Command line parser class

  (c) Mircea Neacsu 2017-2022

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

#if __has_include ("defs.h")
#include "defs.h"
#endif

namespace mlib {

class Options
{
public:
  Options ();
  Options (std::vector<const char*>& list);
  Options (std::initializer_list<const char*> list);
  Options (const char** list);

  void set_options (std::vector<const char*>& list);
  void add_option (const char* option);

  int parse (int argc, const char* const *argv, int *stop=0);
  bool next (std::string& opt, std::string& optarg, char sep='|');
  bool next (std::string& opt, std::vector<std::string>& optarg);
  bool getopt (const std::string& option, std::string& optarg, char sep = '|') const;
  bool getopt (const std::string& option, std::vector<std::string>& optarg) const;
  bool getopt (char option, std::string& optarg, char sep = '|') const;
  bool getopt (char option, std::vector<std::string>& optarg) const;
  bool hasopt (const std::string& option) const;
  bool hasopt (char option) const;
  const std::string synopsis () const;
  const std::string description (size_t indent_size = 2) const;
  const std::string& appname () const;

private:
  struct opt {
    char oshort;                    //short form
    std::string olong;              //long form
    char flag;                      //argument type
    std::string arg_descr;          //argument description
    std::vector<std::string> arg;   //actual argument(s)
  };

  std::vector<opt>::const_iterator find_option(const std::string& opt) const;
  std::vector<opt>::const_iterator find_option(char opt) const;
  void format_arg (std::string& str, const opt& option, char sep) const;

  std::vector<opt> optlist;
  std::vector<opt> cmd; 
  std::vector<opt>::iterator nextop;
  std::string app;
};

/*!
  Return program name
  
  This is the content of `argv[0]` without any path or extension.
*/
inline
const std::string& Options::appname () const
{
  return app;
}

///@{
/*!
  Check if command line has an option
  \param  option  long or short form of the option
*/
inline
bool Options::hasopt (const std::string& option) const
{
  return find_option (option) != cmd.end ();
}

/*!
  Check if command line has an option
  \param  option  short form of the option
*/
inline
bool Options::hasopt (char option) const
{
  return find_option (option) != cmd.end ();
}
///@}

///@{
/*!
  Return a specific option from the command

  \param  option  the requested option
  \param  optarg  option argument(s)

  \return   `true` if successful, `false` otherwise
*/

inline
bool Options::getopt(const std::string& option, std::vector<std::string>& optarg) const
{
  optarg.clear();
  auto p = find_option(option);
  if (p == cmd.end())
    return false;

  optarg = p->arg;
  return true;
}

inline
bool Options::getopt(char option, std::vector<std::string>& optarg) const
{
  optarg.clear();
  auto p = find_option(option);
  if (p == cmd.end())
    return false;

  optarg = p->arg;
  return true;
}
///@}

}
