/*!
  \file options.h Command line parser class

  (c) Mircea Neacsu 2017
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
  ~Options ();

  /*
   Sample option list entries:
      const char *optlist[] {
        "a? optional_arg",        // option -a can have an argument
                                  // example: -a 1 or -a xyz
        "b: required_arg",        // option -b must be followed by an argument
                                  // example: -b mmm
        "c+ one_or_more_args",    // option -c can be followed by one or more arguments
                                  // example: -c 12 ab cd.
                                  // The arguments finish at the next option
        "d* 0_or_more_args",      // option -d can have zero or more arguments
        "e|",                     // option -e doesn't have any arguments
        "f?longorshort optional", // option -f can be also written as --longorshort
                                  // and can have an argument
        ":longopt required",      // option --longopt must have an argument
    0 };

  */
  void set_options (std::vector<const char*>& list);
  void add_option (const char* option);

  int parse (int argc, const char* const *argv, int *stop=0);
  bool next (std::string& opt, std::string& optarg, char sep='|');
  bool getopt (const std::string& option, std::string& optarg, char sep = '|') const;
  bool getopt (char option, std::string& optarg, char sep = '|') const;
  bool hasopt (const std::string& option) const;
  bool hasopt (char option) const;
  const std::string& usage (char sep=' ');
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
  std::string usage_;
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


}
