/*!
  OPT-SAMPLE.CPP - This program shows how to use the parser for command line
  options included in MLIB library.

  (c) Mircea Neacsu 2017-2025

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <iostream>
#include <mlib/mlib.h>

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
    cout << '\t' << parser.appname () << " [options] <other arguments>" << endl
         << endl <<"Valid options are:" << endl 
         << "\t" << parser.synopsis () << endl
         << endl << "Where:" << endl
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