#include <utpp/utpp.h>
#include <mlib/options.h>
#include <iostream>

using namespace mlib;

SUITE (Options)
{

char *optlist[]  = {"a? optional_arg",
                    "b: required_arg",
                    "c+ one_or_more_args",
                    "d* 0_or_more_args",
                    "e|",
                    "f?longorshort optional",
                    ":onlylong required",
                    0};

TEST (CostructorWithOptlist)
{
  Options o1;
  Options o2 (optlist);

  o1.set_optlist (optlist);

  string usage1 = o1.usage ();
  string usage2 = o2.usage ();

  CHECK (usage1 == usage2);
}

TEST (CopyConstructor)
{
  Options o1 (optlist);
  Options o2 (o1);


  string usage1 = o1.usage ();
  string usage2 = o2.usage ();

  CHECK (usage1 == usage2);
}


TEST (Usage)
{
  char *cmd[] ={"program"};
  Options o (optlist);

  CHECK_EQUAL (0, o.parse (1, cmd));
  cout << o.usage () << endl;
}

TEST (UnknownOpt)
{
  char *cmd[] ={"programname", "-x"};

  Options o (optlist);
  CHECK_EQUAL (1, o.parse (2, cmd));
}

TEST (GetMissingOpt)
{
  string argval;
  char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (-1, o.getopt ('b', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArgNoArg)
{
  string argval;
  char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('a', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArg)
{
  string argval;
  char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('a', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgValue)
{
  string argval;
  char *cmd[] ={"programname", "-b", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('b', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgMissing)
{
  char *cmd[] ={"programname", "-b"};

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (2, cmd));
}

TEST (OneOrMoreWithOne)
{
  string argval;
  char *cmd[] ={"programname", "-c", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (OneOrMoreWithMore)
{
  string argval;
  char *cmd[] ={"programname", "-c", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (5, cmd));

  CHECK_EQUAL (0, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (OneOrMoreWithNone)
{
  string argval;
  char *cmd[] ={"programname", "-c"};

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (2, cmd));
}

TEST (ZeroOrMoreWithOne)
{
  string argval;
  char *cmd[] ={"programname", "-d", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (ZeroOrMoreWithMore)
{
  string argval;
  char *cmd[] ={"programname", "-d", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (5, cmd));

  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (ZeroOrMoreWithNone)
{
  string argval;
  char *cmd[] ={"programname", "-d"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK (argval.empty ());
}

TEST (NoArg)
{
  string argval;
  char *cmd[] ={"programname", "-e"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('e', argval));
  CHECK (argval.empty ());
}

TEST (LongOptShortForm)
{
  string argval;
  char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptShortFormAsString)
{
  string argval;
  char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("f", argval));
  CHECK_EQUAL ("abcd", argval);
}


TEST (LongOptLongForm)
{
  string argval;
  char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptGetByLongName)
{
  string argval;
  char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("longorshort", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptNoShortForm)
{
  string argval;
  char *cmd[] ={"programname", "--onlylong", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("onlylong", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (NonOptionParam)
{
  int nextarg;
  string argval;
  char *cmd[] ={"programname", "-a", "abcd", "nonopt"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (4, cmd, &nextarg));

  CHECK_EQUAL ("nonopt", cmd[nextarg]);
}

TEST (EndOfParams)
{
  int nextarg;
  string argval;
  char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd, &nextarg));

  CHECK_EQUAL (3, nextarg);
}

TEST (NextOnEmpytParser)
{
  Options o;
  string argval;
  string opt;
  CHECK_EQUAL (-1, o.next (opt, argval));
  CHECK_EQUAL (-1, o.next (opt, argval));
}

TEST (Next)
{
  int nextarg;
  string argval, argopt;
  char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd, &nextarg));
  CHECK_EQUAL (0, o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval);
}

TEST (NextGetsLongForm)
{
  int nextarg;
  string argval, argopt;
  char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd, &nextarg));
  CHECK_EQUAL (0, o.next (argopt, argval));

  CHECK_EQUAL ("longorshort", argopt);
  CHECK_EQUAL ("abcd", argval);
}

TEST (NextAdvances)
{
  int nextarg;
  string argval, argopt;
  char *cmd[] ={"programname", "-a", "abcd", "-b", "efgh"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (5, cmd, &nextarg));
  CHECK_EQUAL (0, o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval);

  CHECK_EQUAL (0, o.next (argopt, argval));
  CHECK_EQUAL ("b", argopt);
  CHECK_EQUAL ("efgh", argval);
}

TEST (SampleOptionsCode)
{
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

  Options optparser (optlist);

  //sample command line
  const char *samp_argv[]{ "program", "-a", "1", "-e", "--longopt", "par" };

  optparser.parse (6, samp_argv);
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


