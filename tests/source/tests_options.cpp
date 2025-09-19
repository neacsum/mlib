#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <iostream>

using namespace mlib;
using std::string;

SUITE (Options)
{

std::vector<const char*> optvec
  {"a? optional_arg",
   "b: required_arg",
   "c+ one_or_more_args",
   "d* 0_or_more_args",
   "e|", "g|", "h|",
   "f?longorshort optional",
   ":onlylong required"};

const char* optlist[] =
{ "a? optional_arg",
 "b: required_arg",
 "c+ one_or_more_args",
 "d* 0_or_more_args",
 "e|", "g|", "h|",
 "f?longorshort optional",
 ":onlylong required", 0};

TEST (CostructorWithOptlist)
{
  OptParser o1;
  OptParser o2 (optlist);

  o1.set_options (optvec);

  string usage1 = o1.synopsis ();
  string usage2 = o2.synopsis ();

  CHECK (usage1 == usage2);
}

TEST (CopyConstructor)
{
  OptParser o1 (optlist);
  OptParser o2 (o1);


  string usage1 = o1.synopsis ();
  string usage2 = o2.synopsis ();

  CHECK (usage1 == usage2);
}

TEST (UnknownOpt)
{
  const char *cmd[] ={"programname", "-a", "-x", "-e"};

  OptParser o (optlist);
  int stop;
  CHECK_EQUAL (1, o.parse (std::size(cmd), cmd, &stop));
  CHECK_EQUAL (2, stop);
}

TEST (GetMissingOpt)
{
  string argval="something";
  const char *cmd[] ={"programname", "-a"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK(!o.getopt ('b', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArgNoArg)
{
  string argval = "something";
  const char *cmd[] ={"programname", "-a"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK (o.getopt ('a', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArg)
{
  string argval;
  const char *cmd[] ={"programname", "-a", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('a', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgValue)
{
  string argval;
  const char *cmd[] ={"programname", "-b", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size(cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('b', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgMissing)
{
  const char *cmd[] ={"programname", "-b"};
  int stop;

  OptParser o (optlist);
  CHECK_EQUAL (2, o.parse (std::size (cmd), cmd, &stop));
  CHECK_EQUAL (1, stop);
}

TEST (OneOrMoreWithOne)
{
  string argval;
  const char *cmd[] ={"programname", "-c", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (OneOrMoreWithMore)
{
  string argval;
  const char *cmd[] ={"programname", "-c", "abcd", "efgh", "ijkl"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (OneOrMoreWithNone)
{
  string argval;
  const char *cmd[] ={"programname", "-c"};

  OptParser o (optlist);
  CHECK_EQUAL (2, o.parse (std::size (cmd), cmd));
}

TEST (ZeroOrMoreWithOne)
{
  string argval;
  const char *cmd[] ={"programname", "-d", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (ZeroOrMoreWithMore)
{
  string argval;
  const char *cmd[] ={"programname", "-d", "abcd", "efgh", "ijkl"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (ZeroOrMoreWithNone)
{
  string argval;
  const char *cmd[] ={"programname", "-d"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK_EQUAL (1, o.getopt ('d', argval));
  CHECK (argval.empty ());
}

TEST (NoArg)
{
  string argval="something";
  const char *cmd[] ={"programname", "-e"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK_EQUAL (1, o.getopt ('e', argval));
  CHECK (argval.empty ());
}

TEST (LongOptShortForm)
{
  string argval;
  const char *cmd[] ={"programname", "-f", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptShortFormAsString)
{
  string argval;
  const char *cmd[] ={"programname", "-f", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ("f", argval));
  CHECK_EQUAL ("abcd", argval);
}


TEST (LongOptLongForm)
{
  string argval;
  const char *cmd[] ={"programname", "--longorshort", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptGetByLongName)
{
  string argval;
  const char *cmd[] ={"programname", "--longorshort", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK_EQUAL (1, o.getopt ("longorshort", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptNoShortForm)
{
  string argval;
  const char *cmd[] ={"programname", "--onlylong", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));

  CHECK_EQUAL (1, o.getopt ("onlylong", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (NonOptionParam)
{
  int nextarg;
  const char *cmd[] ={"programname", "-a", "abcd", "nonopt"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd, &nextarg));

  CHECK_EQUAL ("nonopt", cmd[nextarg]);
}

TEST (EndOfParams)
{
  int nextarg;
  string argval;
  const char *cmd[] ={"programname", "-a", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd, &nextarg));

  CHECK_EQUAL (std::size (cmd), nextarg);
}

TEST(EndOfOptions)
{
  int nextarg;
  string argval;
  const char* cmd[] = { "programname", "-d", "abcd", "--", "--not_an_option" };

  OptParser o(optlist);
  CHECK_EQUAL(0, o.parse(std::size(cmd), cmd, &nextarg));

  CHECK_EQUAL("--not_an_option", cmd[nextarg]);
}

TEST(HyphenHyphenAtEnd)
{
  int nextarg;
  string argval;
  const char* cmd[] = { "programname", "-d", "abcd", "--" };

  OptParser o(optlist);
  CHECK_EQUAL(0, o.parse(std::size(cmd), cmd, &nextarg));

  CHECK_EQUAL(std::size(cmd), nextarg);
}

TEST (NextOnEmpytParser)
{
  OptParser o;
  string argval;
  string opt;
  CHECK (!o.next (opt, argval));
  CHECK (!o.next (opt, argval));
}

TEST (Next)
{
  int nextarg;
  string argval, argopt;
  const char *cmd[] ={"programname", "-a", "abcd", "-b", "efgh"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval);

  CHECK(o.next(argopt, argval));

  CHECK_EQUAL("b", argopt);
  CHECK_EQUAL("efgh", argval);
}

TEST (NextGetsLongForm)
{
  int nextarg;
  string argval, argopt;
  const char *cmd[] ={"programname", "-f", "abcd"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("longorshort", argopt);
  CHECK_EQUAL ("abcd", argval);
}

TEST (NextWithStringArray)
{
  int nextarg;
  string argopt;
  std::vector<string> argval;
  const char *cmd[] ={"programname", "-a", "abcd", "-c", "efgh", "ijkl"};

  OptParser o (optlist);
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval[0]);

  CHECK (o.next (argopt, argval));
  CHECK_EQUAL ("c", argopt);
  CHECK_EQUAL ("efgh", argval[0]);
  CHECK_EQUAL ("ijkl", argval[1]);
}

TEST (MultiOptionOK)
{
  OptParser o (optlist);
  const char* cmd[] = { "programname", "-egh" };

  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK (o.hasopt ('e'));
  CHECK (o.hasopt ('g'));
  CHECK (o.hasopt ('h'));
}

TEST (MultiOptionLastArg)
{
  OptParser o (optlist);
  const char* cmd[] = { "programname", "-egf", "f_arg"};

  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK (o.hasopt ('e'));
  CHECK (o.hasopt ('g'));
  CHECK (o.hasopt ('f'));
  string v;
  CHECK_EQUAL (1, o.getopt ('f', v));
  CHECK_EQUAL ("f_arg", v);
}

TEST (MultiOptionArgInMiddle)
{
  OptParser o (optlist);
  //incorrect command. Option with argument not last
  const char* cmd[] = { "programname", "-efg", "f_arg" };

  CHECK_EQUAL (3, o.parse (std::size (cmd), cmd));
}

TEST (AccumulatedArgs)
{
  OptParser o (optlist);
  const char *cmd[] = {"programname", "-a", "arg1", "-b", "arg_b", "-a", "arg2"};
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  string str;
  CHECK_EQUAL (2, o.getopt ('a', str));
  CHECK_EQUAL ("arg1|arg2", str);
}

TEST (RepeatedOption)
{
  OptParser o (optlist);
  string str;
  const char *cmd[] = {"programname", "-e", "-e", "-e"};
  CHECK_EQUAL (0, o.parse (std::size (cmd), cmd));
  CHECK_EQUAL (3, o.getopt ('e', str));
}

TEST (SampleOptionsCode)
{
  OptParser optparser ({
    "a? optional_arg \t -a can have an argument example: -a 1 or -a xyz",
    "b: required_arg \t -b must be followed by an argument example: -b mmm",
    "c+ one_or_more_args \t -c can be followed by one or more arguments example: -c 12 ab cd",
    "d* 0_or_more_args \t -d can have zero or more arguments",
    "e|\t-e doesn't have any arguments",
    "f?longorshort optional \t -f can be also written as --longorshort",
    ":longopt required \t --longopt must have an argument"});

  //sample command line
  const char *samp_argv[]{ "c:\\path\\to\\file\\program.exe", "-a", "1", "-e", "--longopt", "par" };

  optparser.parse (std::size (samp_argv), samp_argv);
  string lo;
  if (optparser.getopt ("longopt", lo))
  {
    // lo should be "par"
    CHECK_EQUAL ("par", lo);
  }

  if (optparser.hasopt ('e'))
  {
    // option -e is present
  }
  else
    CHECK ("Missing option");

  std::cout << optparser.synopsis () << std::endl
            << "Where:" << std::endl
            << optparser.description () << std::endl;
}

TEST (Sample_args_as_vector)
{
  OptParser optparser (
    {"a? optional_arg \t -a can have an argument example: -a 1 or -a xyz",
     "b: required_arg \t -b must be followed by an argument example: -b mmm",
     "c+ one_or_more_args \t -c can be followed by one or more arguments example: -c 12 ab cd",
     "d* 0_or_more_args \t -d can have zero or more arguments", "e|\t-e doesn't have any arguments",
     "f?longorshort optional \t -f can be also written as --longorshort",
     ":longopt required \t --longopt must have an argument"});

  // sample command line
  const std::vector <std::string> args {"c:\\path\\to\\file\\program.exe", "-a", "1", "-e", "--longopt", "par"};

  optparser.parse (args);
  string lo;
  if (optparser.getopt ("longopt", lo))
  {
    // lo should be "par"
    CHECK_EQUAL ("par", lo);
  }

  if (optparser.hasopt ('e'))
  {
    // option -e is present
  }
  else
    CHECK ("Missing option");
}

}


