/*!
  \file options.cpp Implementation of command line parsing class.

  \class mlib::OptParser 
  \brief Command Line Parsing class

  The parser matches approximately the POSIX specifications from:
  http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
  and GNU Standard for Command line Interface at
  http://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html

  It accepts either short options composed of a dash ('-') followed by a letter,
  or long options composed of '--' followed by the option name. An option can have
  arguments that are either required or optional. If an option can have multiple
  arguments, all program arguments following the option are considered option
  arguments until the next option argument (argument preceded by '-' or '--')
  or until the last argument. The first '--' argument stops options' processing.
  All remaining arguments are considered non-option arguments (operands).

  It is possible to specify several short options after one '-', as long as all
  (except possibly the last) do not have required or optional arguments.

  An option can be repeated. In this case, if the option has arguments, they are
  concatenated. For instance, the following two command lines are equivalent:
```
  -d arg1 arg2 -e
```
and
```
  -d arg1 -e -d arg2
```

  ## Usage Instructions ##
  1. Construct an options parsing object with its list of valid options. See
     below for the syntax of an option descriptor.
  2. Call the OptParser::parse() function to process the command line arguments.
  3. Call OptParser::getopt() function to retrieve the status of the various options

  See OptParser::add_option() function for the syntax of the option descriptor.

  ## Example ##
  The following example shows how to use an OptParser object and the different
  formats available for options.
```CPP
  const char *optlist[] {
    "h|help",                 // option -h or --help
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

  OptParser optparser (optlist);

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

```
*/
#include <mlib/options.h>
#include <ctype.h>
#include <filesystem>

using namespace std;

namespace mlib {

///  Initialize parser
OptParser::OptParser ()
  : nextop (cmd.begin ())
{
}

/*!
  Initializes parser and sets the list of valid options.
  \param  list array of option descriptor strings

  \see OptParser::add_option for syntax of a descriptor string
*/
OptParser::OptParser (std::vector<const char*>& list)
  : nextop(cmd.begin())
{
  set_options (list);
}


/*!
  Initializes parser and sets the list of valid options.
  \param  list array of option descriptor strings

  \see OptParser::add_option for syntax of a descriptor string
*/
OptParser::OptParser (std::initializer_list<const char*> list)
  : nextop (cmd.begin ())
{
  for (auto &o : list)
    add_option (o);
}

/*!
  Initializes parser and sets the list of options descriptors.
  \param  list array of pointers to option descriptor strings

  \note The list must be terminated with a null pointer.
  \see OptParser::add_option for syntax of a descriptor string
*/
OptParser::OptParser (const char** list)
{
  while (*list)
    add_option (*list++);
}

/*!
  Set list of valid options
  \param list  - array of option descriptor strings

  Each valid option is described by a string in the list array.
*/
void OptParser::set_options (std::vector<const char*>& list)
{
  //remove previous optlist
  optlist.clear ();
  for (auto& t : list)
    add_option (t);
}

/*!
  Add a new option descriptor to the list of options.
  \param descr option descriptor string

  The descriptor string is has the following syntax:
```
  [<short_form>] <flag> [<long_form>] [<spaces><arg_description>][\t<opt_description>]
```
  `<short_form>` is a letter giving the short form of the option.

  `flag` is one of:
  - `?` option has one optional parameter
  - `:` option requires one argument
  - `+` option has one or more arguments
  - `*` option has 0 or more arguments
  - `|` option doesn't have any arguments

  `<long_form>` is the long form of the option.

  The `<arg_description>` is the argument description and is used to generate the synopsis string. 
  The `<opt_description>` is the option description and is used to generate the description string.
  The `<opt_description>` part is preceded by a TAB (`\t`) character

  Example:
  ` "l?list list_arg \t list something" `

  In this example the option syntax part is `l?list`, the argument description
  is `list_arg` and the option description is `list something`. The option
  can appear on the command line as:
```
  -l stuff
```
  or
```
  --list stuff
```
  or, simply:
```
  -l
```
  because the argument is optional.

  The synopsis string will include the fragment:
```
  -l|--list [list_arg]
```
  and the description string will include the line:
```
  -l|--list [list_arg]    list something
```
*/
void OptParser::add_option (const char* descr)
{
  char *tmp = strdup (descr);
  char* ptr = tmp;
  opt entry;

  entry.oshort = (isalnum (*ptr)) ? *ptr++ : 0;
  entry.flag = *ptr++;
  char* p1 = strchr (ptr, ' ');
  if (p1)
  {
    *p1++ = 0;
    while (*p1 && *p1 == ' ')
      p1++;
    entry.arg_descr = p1;
  }
  entry.olong = ptr;
  optlist.push_back (entry);
  free (tmp);
}

/*!
  Parse a command line
  \param argc   number of arguments
  \param argv   array of arguments
  \param stop   pointer to an integer that receives the index of the first non-option
                argument if function is successful, or the index of the invalid
                option in case of failure

  \return   0   success
  \return   1   unknown option found
  \return   2   required argument is missing
  \return   3   invalid multiple options string
*/
int OptParser::parse (int argc, const char* const* argv, int* stop)
{
  int ret = 0;
  string d, p, e; //unused
  const char* ptr;

  app = std::filesystem::path (argv[0]).stem ().string();

  cmd.clear ();

  int i = 1;
  while (i<argc)
  {
    ptr = argv[i];
    if (*ptr++ != '-')
      break; //end of options
    auto op = optlist.begin ();
    opt option;
    option.oshort = 0;
    if (*ptr == '-')
    {
      //long option
      ptr++;
      if (*ptr == 0)
      {
        //explicit end of options
        i++;
        break;
      }
      op = find_if (op, optlist.end (), [&ptr](auto& o) {return !strcmp (ptr, o.olong.c_str ()); });
      option.olong = ptr;
      if (op != optlist.end ())
        option.oshort = op->oshort;
      else
      {
        ret = 1;  //unknown option
        goto done;
      }
    }
    else
    {
      //short option(s)
      op = find_if (op, optlist.end (), [&ptr](auto& o) {return o.oshort == *ptr; });
      if (op == optlist.end ())
      {
        ret = 1;  //unknown option
        goto done;
      }
      option.oshort = *ptr;
      option.olong = op->olong;
      while (*++ptr)
      {
        //could be multiple short options. All of them must have no arguments
        if (op->flag != '|')
        {
          ret = 3; 
          goto done;
        }
        cmd.push_back (option);
        op = find_if (optlist.begin(), optlist.end (), [&ptr](auto& o) {return o.oshort == *ptr; });
        if (op == optlist.end ())
        {
          ret = 1;  //unknown option
          goto done;
        }
        option.oshort = *ptr;
        option.olong = op->olong;
      }
    }

    i++;
    switch (op->flag)
    {
    case '?':   //optional arg
      if (i < argc && *argv[i] != '-')
        option.arg.push_back (argv[i++]);
      break;

    case ':': //required arg
      if (i < argc && *argv[i] != '-')
        option.arg.push_back (argv[i++]);
      else
      {
        ret = 2;   //required arg missing
        --i;
        goto done;
      }
      break;

    case '+':   //one or more
      if (i < argc && *argv[i] != '-')
      {
        option.arg.push_back (argv[i++]);
        while (i < argc && *argv[i] != '-')
          option.arg.push_back (argv[i++]);
      }
      else
      {
        ret = 2;   //required arg missing
        --i;
        goto done;
      }
      break;

    case '*':     //zero or more
      while (i < argc && *argv[i] != '-')
        option.arg.push_back (argv[i++]);
      break;

    case '|':   //no argument
      break;
    }
    //check for a previous instance of this option
    auto prev = find_if(cmd.begin(), cmd.end(),
      [&option](auto& c) {return (c.oshort == option.oshort) && (c.olong == option.olong); });
    if (prev == cmd.end())
      cmd.push_back(option);
    else
      prev->arg.insert(prev->arg.end(), option.arg.begin(), option.arg.end());
  }
  nextop = cmd.begin (); //init options iterator

done:
  if (stop)
    *stop = i;
  return ret;
}

/*!
  Return next option in the command

  \param  opt   option
  \param  optarg  option argument(s)
  \param  sep     separator character to insert between arguments if option has multiple arguments

  \return   `true` if successful, `false` if there are no more options

  The internal position counter is initialized to the first option when command
  line is parsed and is incremented after each call to this function. Note that
  the internal counter is not thread-safe. Calls to `next()` function from any
  thread increment the same counter.

  If the next option has both a long form and a short form, the function returns
  the long form.

  If the option has multiple arguments, `optarg` contains the arguments separated
  by separator character.
 */
bool OptParser::next (std::string& opt, std::string& optarg, char sep)
{
  if (nextop == cmd.end ())
    return false;

  if (!nextop->olong.empty ())
    opt = nextop->olong;
  else
    opt = nextop->oshort;
  format_arg (optarg, *nextop++, sep);
  return true;
}


/*!
  Return next option in the command

  \param  opt   option
  \param  optarg  option argument(s)

  \return   `true` if successful, `false` if there are no more options

  The internal position counter is initialized to the first option when command
  line is parsed and is incremented after each call to next() function. Note that
  the internal counter is not thread-safe. Calls to `next()` function from any
  thread increment the same counter.

  If the next option has both a long form and a short form, the function returns
  the long form.
 */
bool OptParser::next (std::string& opt, std::vector<std::string>& optarg)
{
  optarg.clear ();
  if (cmd.empty () || nextop == cmd.end ())
    return false;
  if (!nextop->olong.empty ())
    opt = nextop->olong;
  else
    opt = nextop->oshort;
  optarg = nextop->arg;
  nextop++;
  return true;
}

///@{
/*!
  Return a specific option from the command

  \param  option  the requested option
  \param  optarg  option argument(s)
  \param  sep     separator character to insert between arguments if option has multiple arguments

  \return   `true` if successful, `false` otherwise

  If the option has multiple arguments, `optarg` contains the arguments separated
  by separator character.
*/
bool OptParser::getopt(const std::string &option, std::string& optarg, char sep) const
{
  optarg.clear();

  auto op = find_option(option);

  if (op == cmd.end())
    return false;
  format_arg(optarg, *op, sep);
  return true;
}

bool OptParser::getopt (char option, std::string& optarg, char sep) const
{
  optarg.clear();

  auto op = find_option (option);
  if (op == cmd.end())
    return false;

  format_arg(optarg, *op, sep);
  return true;
}
///@}


/*!
  Generate a nicely formatted syntax string.
*/
const string OptParser::synopsis () const
{
  string result, term;
  auto op = optlist.begin ();

  result = app;
  for (auto& op : optlist)
  {
    result += ' ';
    if (op.oshort)
    {
      result += '-';
      result += op.oshort;
      if (!op.olong.empty())
        result += '|';
    }
    if (!op.olong.empty())
    {
      result += "--";
      result += op.olong;
    }

    if (op.flag == '|')
      continue;

    switch (op.flag)
    {
    case '?':
      result += " [";
      term = "]";
      break;
    case ':':
      result += " <";
      term = ">";
      break;
    case '*':
      result += " [";
      term = " ...]";
      break;
    case '+':
      result += " <";
      term = ">...";
      break;
    }
    string param = op.arg_descr;
    auto tabpos = param.find('\t');
    if (tabpos != string::npos)
    {
      param.erase(tabpos);
      param.erase(
        find_if(param.rbegin(), param.rend(),
          [](char& ch) {return !isspace(ch); }).base(), param.end());
    }
    result += param;
    result += term;
  }

  return result;
}

/*!
  Generate options description
  \param indent_size number of spaces to indent each option description line

  Each option and its synopsis is shown on a separate line indented by 
  `indent_size` spaces, followed by the option description (if any).
*/
const std::string OptParser::description (size_t indent_size) const
{
  string descr;
  vector<string> lines;
  size_t maxlen = 0;
  for (auto& op : optlist)
  {
    string line (indent_size, ' '), term;
    if (op.oshort)
    {
      line += '-';
      line += op.oshort;
      if (!op.olong.empty())
        line += '|';
    }
    if (!op.olong.empty())
    {
      line += "--";
      line += op.olong;
    }

    switch (op.flag)
    {
    case '?':
      line += " [";
      term = "]";
      break;
    case ':':
      line += " <";
      term = ">";
      break;
    case '*':
      line += " [";
      term = " ...]";
      break;
    case '+':
      line += " <";
      term = ">...";
      break;
    }

    if (op.flag != '|')
    {
      auto tabpos = op.arg_descr.find ('\t');
      string param = op.arg_descr;
      if (tabpos != string::npos)
      {
        param.erase (tabpos);
        param.erase (
          find_if (param.rbegin (), param.rend (), [] (char &ch) { return !isspace (ch); }).base (),
          param.end ());
      }
      line += param;
      line += term;
    }
    if (line.size () > maxlen)
      maxlen = line.size ();
    lines.push_back (line);
  }

  maxlen += indent_size;
  int i = 0;
  for (auto &op : optlist)
  {
    descr += lines[i];
    auto tabpos = op.arg_descr.find ('\t');
    if (tabpos != string::npos)
    {
      // lineup all descriptions
      descr.append (maxlen - lines[i].size (), ' ');
      while (isspace (op.arg_descr[++tabpos]))
        ;
      descr += op.arg_descr.substr (tabpos);
    }
    descr += '\n';
    i++;
  }
  return descr;
}


// Find an option (can be long)
std::vector<OptParser::opt>::const_iterator OptParser::find_option(const std::string& option) const
{
  std::vector<OptParser::opt>::const_iterator ptr;
  if (option.length() > 1)
    ptr = std::find_if(cmd.begin(), cmd.end(),
      [&option](auto& op) {return  op.olong == option; });
  else
  { 
    char ch = option[0];
    ptr = std::find_if(cmd.begin(), cmd.end(),
      [&ch](auto& op) {return  op.oshort == ch; });
  }
  return ptr;
}

// Find a short option
std::vector<OptParser::opt>::const_iterator OptParser::find_option (char option) const
{
  auto ptr =
    std::find_if (cmd.begin (), cmd.end (), [&option] (auto &op) { return op.oshort == option; });

  return ptr;
}

//combine all option parameters in one string separated by 'sep'
void OptParser::format_arg (std::string& str, const opt& option, char sep) const
{
  str.clear ();
  auto p = option.arg.begin ();
  while (p != option.arg.end ())
  {
    str += *p++;
    if (p != option.arg.end ())
      str.push_back (sep);
  }
}

}
