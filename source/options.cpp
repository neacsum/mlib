/*!
  \file options.cpp Implementation of command line parsing class.

  \class mlib::Options 
  \brief Command Line Parsing class

  The parser matches approximately the POSIX specifications from:
  http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
  and GNU Standard for Command line Interface at
  http://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html

  ## Usage Instructions ##
  1. Construct an options parsing object with it's list of valid options.
  2. Call the Options::parse() function to process the command line arguments.
  3. Call Options::getopt() function to retrieve the status of the varios options

  ## Example ##
  The following example shows the different formats available for options.
````
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
  }

  if (optparser.hasopt ('e'))
  {
    // option -e is present
  }

````
*/
#include <mlib/options.h>
#include <ctype.h>

namespace mlib {
  
/*!
  Initialize parser
*/
Options::Options ()
{
}

/*!
  Initializes parser and sets the list of valid options.

  \param  list array of option definitions strings

  See Options::set_optlist for details
*/
Options::Options (const char* const *list)
{
  set_optlist (list);
}

/*!
  Set list of valid options
  \param list  - array of option definition strings

  Each valid option is described by a string in the list array. Each string is
  composed of an option syntax part followed by a space and the option argument
  description

  Example:
  ` "l?list option_argument" `

  In this example the option syntax part is \c"l?list" and the argument description
  is \c"option_argument"

  The option syntax string has the following syntax:
  `[short_form][arg_char][long_form]`

  \e short_form is a letter giving the short form of the option ('l' in the example
  above).

  arg_char is one of:
  - ? option has one optional parameter
  - : option requires one argument
  - + option has one or more arguments
  - * option has 0 or more arguments
  - | option doesn't have any arguments

  \e long_form is the long form of the option ('list' in the example above).

  The array is terminated by a NULL string.
*/
void Options::set_optlist (const char* const *list)
{
  char tmp[256];

  //remove previous optlist and parsed command
  optlist.clear ();
  cmd.clear ();
  nextop = cmd.begin ();
  while (*list)
  {
    strcpy (tmp, *list);
    opt option;
    char *ptr = tmp;

    option.oshort = (isalnum (*ptr))? *ptr++ : 0;
    option.flag = *ptr++;
    char *p1 = strchr (ptr, ' ');
    if (p1)
    {
      *p1++ = 0;
      while (*p1 && isspace (*p1))
        p1++;
      option.arg = p1;
    }
    option.olong = ptr;
    optlist.push_back (option);
    list++;
  }
}

Options::~Options()
{
}

/*!
  Parse a command line
  \param argc   number of arguments
  \param argv   array of arguments
  \param stop   pointer to an integer that receives the number of the first non-option
  argument

  \return   0   success
  \return   1   unknown option found
  \return   2   required argument is missing
*/
int Options::parse(int argc, const char* const *argv, int *stop)
{
  int i=1;
  int ret = 0;
  const char *ptr = strrchr (argv[0], '\\');
  if (ptr)
    ptr++;
  else
    ptr = argv[0];
  const char *ptr1 = strchr(ptr, '.');
  size_t sz;
  if (ptr1)
    sz = ptr1 - ptr;
  else
    sz = strlen(ptr);
  app = string (ptr, sz);
  cmd.clear ();

  while (i<argc)
  {
    ptr = argv[i];
    if (*ptr++ == '-')
    {
      deque<opt>::iterator op = optlist.begin ();
      opt option;
      option.oshort = 0;
      if (*ptr == '-')
      {
        //long option
        ptr++;
        while (op != optlist.end () && strcmpi (ptr, op->olong.c_str()))
          op++;
        option.olong = ptr;
        if (op != optlist.end ())
          option.oshort = op->oshort;
      }
      else
      {
        //short option
        while (op != optlist.end () && op->oshort != *ptr)
          op++;
        option.oshort = *ptr;
        if (op != optlist.end ())
          option.olong = op->olong;
      }
      if (op == optlist.end ())
      {
        ret = 1;  //unknown option
        goto done;
      }

      i++;
      switch (op->flag)
      {
      case '?':   //optional arg
        if (i<argc && *argv[i] != '-')
          option.arg = argv[i++];
        break;

      case ':': //required arg
        if (i<argc && *argv[i] != '-')
          option.arg = argv[i++];
        else
        {
          ret = 2;   //required arg missing
          goto done;
        }
        break;

      case '+':   //one or more
        if (i<argc && *argv[i] != '-')
        {
          option.arg = argv[i++];
          while (i<argc && *argv[i] != '-')
          {
            option.arg += '|';
            option.arg += argv[i++];
          }
        }
        else
        {
          ret = 2;   //required arg missing
          goto done;
        }
        break;

      case '*':     //zero or more
        while (i<argc && *argv[i] != '-')
        {
          if (!option.arg.empty ())
            option.arg += '|';
          option.arg += argv[i++];
        }
        break;

      case '|':   //no argument
        break;
      }
      cmd.push_back (option);
    }
    else
      break;  //end of options
  }
  nextop = cmd.begin ();

done:
  if (stop)
    *stop = i;
  return ret;
}

/*!
  Return next option in the command

  \param  opt   option
  \param  optarg  option argument

  \return   0   success
  \return   -1  no more options

  If the next option has a long form, returns the long form. Otherwise return
  the short form.

  If the option has multiple arguments, optarg contains the arguments separated
  by '|'.
 */
int Options::next (string& opt, string& optarg)
{
  if (cmd.empty () || nextop == cmd.end ())
    return -1;

  if (!nextop->olong.empty ())
    opt = nextop->olong;
  else
    opt = nextop->oshort;
  optarg = nextop->arg;

  nextop++;
  return 0;
}


/*!
  Return a specific option from the command

  \param  option  the requested option
  \param  optarg  option argument

  \return   0   success
  \return   -1  option not found

  If the option has multiple arguments, optarg contains the arguments separated
  by '|'.
*/
int Options::getopt(const string &option, string& optarg)
{
  deque<opt>::iterator op = cmd.begin ();
  
  while (op != cmd.end ())
  {
    if ((option.length () > 1 && op->olong == option)
     || (option.length () == 1 && op->oshort == option[0]))
    {
      optarg = op->arg;
      return 0;
    }
    op++;
  }
  return -1;
}

int Options::getopt (char option, string& optarg)
{
  deque<opt>::iterator op = cmd.begin ();
  
  while (op != cmd.end ())
  {
    if (op->oshort == option)
    {
      optarg = op->arg;
      return 0;
    }
    op++;
  }
  return -1;
}

/*!
  Check if command line has an option
  \param  option  long or short form of the option
*/
bool Options::hasopt (const string& option)
{
  deque<opt>::iterator op = cmd.begin ();
  
  while (op != cmd.end ())
  {
    if ((option.length () > 1 && op->olong == option)
     || (option.length () == 1 && op->oshort == option[0]))
      return true;
    op++;
  }
  return false;
}

/*!
  Check if command line has an option
  \param  option  short form of the option
*/
bool Options::hasopt (char option)
{
  deque<opt>::iterator op = cmd.begin ();
  
  while (op != cmd.end ())
  {
    if (op->oshort == option)
      return true;
    op++;
  }
  return false;
}

/*!
  Generate a nicely formatted usage string.

  \param  sep separator to be inserted between option descriptions
*/
const string& Options::usage(char sep)
{
  if (!usage_.empty())
    return usage_;

  string term;
  deque<opt>::iterator op = optlist.begin ();

  usage_ = app + sep;
  while (op != optlist.end ())
  {
    if (op->oshort)
    {
      usage_ += '-';
      usage_ += op->oshort;
      if (!op->olong.empty())
        usage_ += '|';
    }
    if (!op->olong.empty())
    {
      usage_ += "--";
      usage_ += op->olong;
    }

    switch (op->flag)
    {
    case '?':
      usage_ += " [";
      term = "]";
      break;
    case ':':
      usage_ += " <";
      term = ">";
      break;
    case '*':
      usage_ += " [";
      term = " ...]";
      break;
    case '+':
      usage_ += " <";
      term = ">...";
      break;
    case '|':
      term = "";
      break;
    }
    usage_ += op->arg;
    usage_ += term;
    usage_ += sep;

    op++;
  }
  return usage_;
}

}
