#include <utpp/utpp.h>
#include <mlib/options.h>
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
  Options o1;
  Options o2 (optlist);

  o1.set_options (optvec);

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
  const char *cmd[] ={"program"};
  Options o (optlist);

  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  std::cout << o.usage ('\n') << std::endl;
}

TEST (UnknownOpt)
{
  const char *cmd[] ={"programname", "-a", "-x", "-e"};

  Options o (optlist);
  int stop;
  CHECK_EQUAL (1, o.parse (_countof(cmd), cmd, &stop));
  CHECK_EQUAL (2, stop);
}

TEST (GetMissingOpt)
{
  string argval="something";
  const char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK(!o.getopt ('b', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArgNoArg)
{
  string argval = "something";
  const char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.getopt ('a', argval));
  CHECK (argval.empty ());
}

TEST (OptionalArg)
{
  string argval;
  const char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK(o.getopt ('a', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgValue)
{
  string argval;
  const char *cmd[] ={"programname", "-b", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof(cmd), cmd));

  CHECK(o.getopt ('b', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (RequiredArgMissing)
{
  const char *cmd[] ={"programname", "-b"};
  int stop;

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (_countof (cmd), cmd, &stop));
  CHECK_EQUAL (1, stop);
}

TEST (OneOrMoreWithOne)
{
  string argval;
  const char *cmd[] ={"programname", "-c", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK(o.getopt ('c', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (OneOrMoreWithMore)
{
  string argval;
  const char *cmd[] ={"programname", "-c", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ('c', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (OneOrMoreWithNone)
{
  string argval;
  const char *cmd[] ={"programname", "-c"};

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (_countof (cmd), cmd));
}

TEST (ZeroOrMoreWithOne)
{
  string argval;
  const char *cmd[] ={"programname", "-d", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ('d', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (ZeroOrMoreWithMore)
{
  string argval;
  const char *cmd[] ={"programname", "-d", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ('d', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (ZeroOrMoreWithNone)
{
  string argval;
  const char *cmd[] ={"programname", "-d"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.getopt ('d', argval));
  CHECK (argval.empty ());
}

TEST (NoArg)
{
  string argval="something";
  const char *cmd[] ={"programname", "-e"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.getopt ('e', argval));
  CHECK (argval.empty ());
}

TEST (LongOptShortForm)
{
  string argval;
  const char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptShortFormAsString)
{
  string argval;
  const char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ("f", argval));
  CHECK_EQUAL ("abcd", argval);
}


TEST (LongOptLongForm)
{
  string argval;
  const char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptGetByLongName)
{
  string argval;
  const char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.getopt ("longorshort", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (LongOptNoShortForm)
{
  string argval;
  const char *cmd[] ={"programname", "--onlylong", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));

  CHECK (o.getopt ("onlylong", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (NonOptionParam)
{
  int nextarg;
  const char *cmd[] ={"programname", "-a", "abcd", "nonopt"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd, &nextarg));

  CHECK_EQUAL ("nonopt", cmd[nextarg]);
}

TEST (EndOfParams)
{
  int nextarg;
  string argval;
  const char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd, &nextarg));

  CHECK_EQUAL (_countof (cmd), nextarg);
}

TEST (NextOnEmpytParser)
{
  Options o;
  string argval;
  string opt;
  CHECK (!o.next (opt, argval));
  CHECK (!o.next (opt, argval));
}

TEST (Next)
{
  int nextarg;
  string argval, argopt;
  const char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval);
}

TEST (NextGetsLongForm)
{
  int nextarg;
  string argval, argopt;
  const char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("longorshort", argopt);
  CHECK_EQUAL ("abcd", argval);
}

TEST (NextAdvances)
{
  int nextarg;
  string argval, argopt;
  const char *cmd[] ={"programname", "-a", "abcd", "-b", "efgh"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd, &nextarg));
  CHECK (o.next (argopt, argval));

  CHECK_EQUAL ("a", argopt);
  CHECK_EQUAL ("abcd", argval);

  CHECK (o.next (argopt, argval));
  CHECK_EQUAL ("b", argopt);
  CHECK_EQUAL ("efgh", argval);
}

TEST (MultiOptionOK)
{
  Options o (optlist);
  const char* cmd[] = { "programname", "-egh" };

  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.hasopt ('e'));
  CHECK (o.hasopt ('g'));
  CHECK (o.hasopt ('h'));
}

TEST (MultiOptionLastArg)
{
  Options o (optlist);
  const char* cmd[] = { "programname", "-egf", "f_arg"};

  CHECK_EQUAL (0, o.parse (_countof (cmd), cmd));
  CHECK (o.hasopt ('e'));
  CHECK (o.hasopt ('g'));
  CHECK (o.hasopt ('f'));
  string v;
  CHECK (o.getopt ('f', v));
  CHECK_EQUAL ("f_arg", v);
}

TEST (MultiOptionArgInMiddle)
{
  Options o (optlist);
  //incorrect command. Option with argument not last
  const char* cmd[] = { "programname", "-efg", "f_arg" };

  CHECK_EQUAL (3, o.parse (_countof (cmd), cmd));
}



TEST (SampleOptionsCode)
{
  Options optparser ( {
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
    }
  );

  //sample command line
  const char *samp_argv[]{ "program", "-a", "1", "-e", "--longopt", "par" };

  optparser.parse (_countof (samp_argv), samp_argv);
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


