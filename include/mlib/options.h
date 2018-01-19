#pragma once

/*  \file OPTIONS.H - Command line parser
*/
#include <string>
#include <deque>

#include "defs.h"

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class Options
{
public:
  Options ();
  Options (const char* const *list);
  ~Options ();

  void set_optlist (const char* const *list);

  int parse (int argc, const char* const *argv, int *stop=0);
  int next (string& opt, string& optarg);

  int getopt (const string& opt, string& optarg);
  int getopt (char opt, string& optarg);
  bool hasopt (const string& opt);
  bool hasopt (char opt);
  const string& usage (char sep=' ');
  const string& appname () {return app;};

private:
  struct opt {
    char oshort;
    string olong;
    char flag;
    string arg;
  };
  
  deque<opt> optlist;
  deque<opt> cmd; 
  deque<opt>::iterator nextop;
  string app;
  string usage_;
};

#ifdef MLIBSPACE
}
#endif
