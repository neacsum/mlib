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
  Options (std::vector<const char*> l);
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
  void set_optlist (std::vector<const char*> list);

  int parse (int argc, const char* const *argv, int *stop=0);
  bool next (std::string& opt, std::string& optarg, char sep='|');
  bool getopt (const std::string& opt, std::string& optarg, char sep = '|');
  bool getopt (char opt, std::string& optarg, char sep = '|');
  bool hasopt (const std::string& opt);
  bool hasopt (char opt);
  const std::string& usage (char sep=' ');
  const std::string& appname () {return app;};

private:
  struct opt {
    char oshort;                    //short form
    std::string olong;              //long form
    char flag;                      //argument type
    std::string arg_descr;          //argument description
    std::vector<std::string> arg;   //actual argument(s)
  };

  std::vector<opt>::iterator find_option(const std::string& opt);
  std::vector<opt>::iterator find_option(char opt);
  void format_arg (std::string& str, opt& option, char sep);
  std::vector<opt> optlist;
  std::vector<opt> cmd; 
  std::vector<opt>::iterator nextop;
  std::string app;
  std::string usage_;
};

}
