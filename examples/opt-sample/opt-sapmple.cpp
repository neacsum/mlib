#include <mlib/options.h>
#include <iostream>

using namespace std;
using namespace mlib;


int main(int argc, char** argv)
{
  //See what we've got
  cout << "argc=" << argc << endl;
  for (int i = 0; i < argc; i++)
    cout << "argv[" << i << "]=" << argv[i] << endl;
  cout << endl << endl;

  OptParser parser{
    "h|help \t show help message",
    "y| \t boolean flag",
    "n| \t another boolean flag",
    "p+param parameters \t one or more parameters",
    "o:option value \t optional value",
    "*stuff things \t option with zero or more arguments"
  };

  int nonopt;
  if (parser.parse(argc, argv, &nonopt) != 0)
  {
    cout << "Syntax error. Valid options are:" << endl;
    cout << parser.synopsis() << endl;
  }
  if (argc == 1)
  {
    cout << "Usage:" << endl;
    cout << parser.synopsis() << endl;
    cout << "Where:" << endl
      << parser.description() << endl;
    exit(0);
  }
  
  string optarg;
  if (parser.getopt('p', optarg, ','))
    cout << "Parameters are: " << optarg << endl;

  cout << "'y' flag is " << (parser.hasopt('y') ? "set" : "reset") << endl;
  cout << "'n' flag is " << (parser.hasopt('n') ? "set" : "reset") << endl;

  if (parser.getopt("stuff", optarg, ','))
    cout << "Stuff: " << optarg << endl;

  if (nonopt < argc)
    cout << "First non option argument is " << argv[nonopt] << endl;

  cout << "Synopsis:" << endl << parser.synopsis () << endl;
  cout << "Description:" << endl << parser.description () << endl;
  return 0;
}