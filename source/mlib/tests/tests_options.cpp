#include <utpp/utpp.h>
#include <mlib/options.h>
#include <iostream>

char *optlist[]  = {"a? optional_arg",
                    "b: required_arg",
                    "c+ one_or_more_args",
                    "d* 0_or_more_args",
                    "e|",
                    "f?longorshort optional",
                    ":onlylong required",
                    0};

TEST (Options_CostructorWithOptlist)
{
  Options o1;
  Options o2 (optlist);

  o1.set_optlist (optlist);

  string usage1 = o1.usage ();
  string usage2 = o2.usage ();

  CHECK (usage1 == usage2);
}

TEST (Options_CopyConstructor)
{
  Options o1 (optlist);
  Options o2 (o1);


  string usage1 = o1.usage ();
  string usage2 = o2.usage ();

  CHECK (usage1 == usage2);
}


TEST (Options_Usage)
{
  char *cmd[] ={"program"};
  Options o (optlist);

  CHECK_EQUAL (0, o.parse (1, cmd));
  cout << o.usage () << endl;
}

TEST (Options_UnknownOpt)
{
  char *cmd[] ={"programname", "-x"};

  Options o (optlist);
  CHECK_EQUAL (1, o.parse (2, cmd));
}

TEST (Options_GetMissingOpt)
{
  string argval;
  char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (-1, o.getopt ('b', argval));
  CHECK (argval.empty ());
}

TEST (Options_OptionalArgNoArg)
{
  string argval;
  char *cmd[] ={"programname", "-a"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('a', argval));
  CHECK (argval.empty ());
}

TEST (Options_OptionalArg)
{
  string argval;
  char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('a', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_RequiredArgValue)
{
  string argval;
  char *cmd[] ={"programname", "-b", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('b', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_RequiredArgMissing)
{
  char *cmd[] ={"programname", "-b"};

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (2, cmd));
}

TEST (Options_OneOrMoreWithOne)
{
  string argval;
  char *cmd[] ={"programname", "-c", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_OneOrMoreWithMore)
{
  string argval;
  char *cmd[] ={"programname", "-c", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (5, cmd));

  CHECK_EQUAL (0, o.getopt ('c', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (Options_OneOrMoreWithNone)
{
  string argval;
  char *cmd[] ={"programname", "-c"};

  Options o (optlist);
  CHECK_EQUAL (2, o.parse (2, cmd));
}

TEST (Options_ZeroOrMoreWithOne)
{
  string argval;
  char *cmd[] ={"programname", "-d", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_ZeroOrMoreWithMore)
{
  string argval;
  char *cmd[] ={"programname", "-d", "abcd", "efgh", "ijkl"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (5, cmd));

  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK_EQUAL ("abcd|efgh|ijkl", argval);
}

TEST (Options_ZeroOrMoreWithNone)
{
  string argval;
  char *cmd[] ={"programname", "-d"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('d', argval));
  CHECK (argval.empty ());
}

TEST (Options_NoArg)
{
  string argval;
  char *cmd[] ={"programname", "-e"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (2, cmd));
  CHECK_EQUAL (0, o.getopt ('e', argval));
  CHECK (argval.empty ());
}

TEST (Options_LongOptShortForm)
{
  string argval;
  char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_LongOptShortFormAsString)
{
  string argval;
  char *cmd[] ={"programname", "-f", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("f", argval));
  CHECK_EQUAL ("abcd", argval);
}


TEST (Options_LongOptLongForm)
{
  string argval;
  char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ('f', argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_LongOptGetByLongName)
{
  string argval;
  char *cmd[] ={"programname", "--longorshort", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("longorshort", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_LongOptNoShortForm)
{
  string argval;
  char *cmd[] ={"programname", "--onlylong", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd));

  CHECK_EQUAL (0, o.getopt ("onlylong", argval));
  CHECK_EQUAL ("abcd", argval);
}

TEST (Options_NonOptionParam)
{
  int nextarg;
  string argval;
  char *cmd[] ={"programname", "-a", "abcd", "nonopt"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (4, cmd, &nextarg));

  CHECK_EQUAL ("nonopt", cmd[nextarg]);
}

TEST (Options_EndOfParams)
{
  int nextarg;
  string argval;
  char *cmd[] ={"programname", "-a", "abcd"};

  Options o (optlist);
  CHECK_EQUAL (0, o.parse (3, cmd, &nextarg));

  CHECK_EQUAL (3, nextarg);
}

TEST (Options_NextOnEmpytParser)
{
  Options o;
  string argval;
  string opt;
  CHECK_EQUAL (-1, o.next (opt, argval));
  CHECK_EQUAL (-1, o.next (opt, argval));
}

TEST (Options_Next)
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

TEST (Options_NextGetsLongForm)
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

TEST (Options_NextAdvances)
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


