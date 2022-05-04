/*!
  \file json.cpp Implementation of json::node class

  (c) Mircea Neacsu 2022. All rights reserved.
*/

#include <mlib/json.h>
#include <mlib/trace.h>
#include <istream>
#include <ostream>
#include <sstream>
#include <utf8/utf8.h>

using namespace std;
using namespace mlib;

namespace json {

errfac json_errors ("JSON Error");
errfac* errors = &json_errors;


/// Constructor for an empty node
node::node (type t_)
  : t (t_)
{
  switch (t) {
  case type::object:
    new (&obj) nodes_map ();
    break;
  case type::array:
    new (&arr) nodes_array ();
    break;
  case type::string:
    new (&str)std::string ();
    break;
  case type::numeric:
    num = 0.;
    break;
  case type::boolean:
    logic = false;
    break;
  }
}

/// Copy constructor
node::node (const node& other)
  : t (other.t)
{
  switch (t)
  {
  case type::object:
    new (&obj) nodes_map ();
    for (auto n = other.obj.begin(); n != other.obj.end (); ++n)
      obj.emplace (n->first, make_unique<node> (n->second.get()));
    break;
  case type::array:
    new (&arr) nodes_array (other.arr.size ());
    for (auto n = other.arr.begin (); n != other.arr.end (); ++n)
      arr.emplace_back (make_unique<node> (n->get()));
    break;
  case type::numeric:
    num = other.num;
    break;
  case type::string:
    new (&str)std::string ();
    str = other.str;
    break;
  case type::boolean:
    logic = other.logic;
    break;
  }
}

/// Move constructor
node::node (node&& other)
{
  size_t sz = sizeof (node);
  memmove (this, &other, sz);
  other.t = type::null;
}

// Destructor
node::~node ()
{
  clear ();
}

/// Principal assignment operator
node& node::operator=(const node& rhs)
{
  if (&rhs != this)
  {
    clear (rhs.t);

    switch (t)
    {
    case type::object:
      for (auto n = rhs.obj.begin (); n != rhs.obj.end (); ++n)
        obj.emplace (n->first, make_unique<node> (*n->second));
      break;
    case type::array:
      for (size_t i = 0; i < rhs.arr.size(); i++)
        arr.emplace_back (make_unique<node>(*rhs.arr[i]));
      break;
    case type::string:
      str = rhs.str;
      break;
    case type::numeric:
      num = rhs.num;
      break;
    case type::boolean:
      logic = rhs.logic;
      break;
    }
  }
  return *this;
}

/// Move assignment operator
node& node::operator =(node&& rhs)
{
  if (&rhs != this)
  {
    memmove (this, &rhs, sizeof (node));
    rhs.t = type::null;
  }
  return *this;
}

/// Return value of an object node element
node& node::operator[](const std::string& name)
{
  if (t == type::null)
  {
    t = type::object;
    new (&obj) nodes_map ();
  }
  else if (t != type::object)
    throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);

  auto p = obj.find (name);
  if (p == obj.end ())
  {
    if (obj.size () < max_object_names)
      p = obj.emplace (std::make_pair (name, std::make_unique<node> ())).first;
    else
      throw mlib::erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);
  }
  return *p->second;
}

/*!
  Return value of an object node element (const version)
  If element doesn't exist it throws an ERR_JSON_MISSING exception.
*/
const node& node::operator[](const std::string& name) const
{
  if (t != type::object)
    throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);

  auto p = obj.find (name);
  if (p == obj.end ())
    throw mlib::erc (ERR_JSON_MISSING, ERROR_PRI_ERROR, errors);
  return *p->second;
}

/// Return value of an array node element
node& node::operator[](size_t index)
{
  if (t == type::null)
  {
    t = type::array;
    new (&arr) nodes_array ();
  }
  else if (t != type::array)
    throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);

  if (index >= arr.size ())
  {
    //extend array
    auto old_size = arr.size ();
    if (index < max_array_size - 1)
      arr.resize (index + 1);
    else
      throw mlib::erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);

    //add null nodes
    for (auto i=old_size; i<=index; i++)
      arr[i] = std::make_unique<node> ();
  }
  return *arr[index];
}

/*!
  Return value of an array node element (const version)
  If element doesn't exist it throws an ERR_JSON_MISSING exception.
*/
const node& node::operator[](size_t index) const
{
  if (t != type::array)
    throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);

  if (index >= arr.size ())
    throw mlib::erc (ERR_JSON_MISSING, ERROR_PRI_ERROR, errors);

  return *arr[index];
}

/// Equality operator
bool node::operator==(const node& other) const
{
  if (t == other.t)
  {
    switch (t)
    {
    case type::object:
      return (obj.size () == other.obj.size ())
        && std::equal (obj.begin (), obj.end (), other.obj.begin (), other.obj.end (),
          [](auto const& n, auto const& m) ->bool {
            return (n.first == m.first) && (*n.second == *m.second);
          });

    case type::array:
      return (arr.size() == other.arr.size())
        && std::equal (arr.begin (), arr.end (), other.arr.begin (), other.arr.end (),
          [](auto const& n, auto const& m) ->bool {
            return *n == *m;
          });

    case type::numeric:
      return num == other.num;
    case type::string:
      return str == other.str;
    case type::boolean:
      return logic == other.logic;
    case type::null:
      return true;
    }
  }
  return false;
}

/// Return `true` if node is an object and has a child with that name
bool node::has (const std::string& name) const
{
  if (t == type::object)
    return obj.find (name) != obj.end ();
  return false;
}

/// Erase the child with that name, if it exists
void node::erase (const std::string& name)
{
  if (t == type::object)
  {
    auto n = obj.find (name);
    if (n != obj.end ())
      obj.erase (n);
  }
}




// Parsing helper functions ---------------------------------------------------
// Check if character is whitespace
static bool is_ws (char c)
{
  const char ws[] = " \t\r\n";
  return (strchr (ws, c) != nullptr);
}

// Skip over whitespace characters
// Returns next peeked character
static int peekws (istream& is)
{
  char c;
  while (is_ws (c = is.peek ()))
    is.get ();
  return c;
}

// Skip over whitespace characters
// Return next non-space character extracted from stream
static int skipws (istream& is)
{
  char c;
  while (is_ws (c = is.get ()))
    ;
  return c;
}

/*
  Collect all characters until next non-alpha.
  As we use this function only to look for predefined tokens (true, false, null),
  the limit for token length is really small.
*/
static std::string token (istream& is)
{
  std::string tok;
  int i = 0;
  tok.reserve (10);

  while (i++ < 10 && isalpha(is.peek ()))
    tok.push_back (is.get());

  return tok;
}

/*
  Parse a JSON number
*/
static erc parse_num (istream& is, double& num)
{
  num = 0;
  double dec = 1.;
  int exp=0, expsign = 1;

  char c = skipws (is);
  int sign = (c == '-') ? -1 : 1;
  if (c == '-')
    c = is.get ();

  // integer part
  if ('1' <= c && c <= '9')
  {
    do {
      num = num * 10 + (c - '0');
      c = is.get ();
    } while (isdigit (c));
  }
  else if (c == '0')
    c = is.get ();
  else
    return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);

  //fraction
  if (c == '.')
  {
    c = is.get ();
    while (isdigit (c))
    {
      dec /= 10;
      num += (c - '0') * dec;
      c = is.get ();
    }
  }

  if (c != 'e' && c != 'E')
  {
    num *= sign;
    is.putback (c);
    return ERR_SUCCESS;
  }

  //exponent
  c = is.get ();
  if (c == '+')
    c = is.get ();
  else if (c == '-')
  {
    expsign = -1;
    c = is.get ();
  }
  if (!isdigit (c))
    return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);

  do {
    exp = exp * 10 + (c - '0');
    c = is.get ();
  } while (isdigit (c));

  num *= sign * pow (10, exp * expsign);
  is.putback (c);
  return ERR_SUCCESS;
}

/*
  Parse a JSON string
*/
static erc parse_string (istream& is, std::string& s)
{
  s.clear ();
  wstring ws;
  int len = 0;
  while (++len < max_string_length)
  {
    int c = is.get ();
    if (c == L'"')
    {
      s = utf8::narrow (ws);
      return ERR_SUCCESS;
    }
    else if (c < 0x20 || c == char_traits<char>::eof ())
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    else if (c == '\\')
    {
      c = is.get ();
      switch (c)
      {
      case '"':
      case '\\':
      case '/':
        ws.push_back (c);
        break;
      case 'b':
        ws.push_back (L'\b');
        break;
      case 'f':
        ws.push_back (L'\f');
        break;
      case 'n':
        ws.push_back (L'\n');
        break;
      case 'r':
        ws.push_back (L'\r');
        break;
      case 't':
        ws.push_back (L'\t');
        break;
      case 'u':
        c = 0;
        for (int i = 0; i < 4; i++)
        {
          int x = is.get ();
          if ('0' <= x && x <= '9')
            c = (c << 4) + (x - '0');
          else if ('A' <= x && x <= 'F')
            c = (c << 4) + (x - 'A' + 10);
          else if ('a' <= x && x <= 'f')
            c = (c << 4) + (x - 'a' + 10);
          else
            return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
        }
        ws.push_back (c);
        break;

      default:
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
        break;
      }
    }
    else
      ws.push_back (c);
  }
  return erc (ERR_JSON_SIZE, ERROR_PRI_ERROR, errors);
}

erc node::read (istream& is)
{
  std::string sval;
  double numval;
  erc ret;

  char c = peekws (is);
  switch (c)
  {
  case '"':
    is.get ();
    ret = parse_string (is, sval);
    if (ret.code () == 0)
    {
      clear (type::string);
      str = sval;
    }
    return ret;

  case '[':
    is.get ();
    clear (type::array);
    if (peekws (is) == ']')
    {
      is.get ();
      return ERR_SUCCESS;
    }

    for (int i = 0; i < max_array_size; i++)
    {
      arr.push_back (std::make_unique<node> ());
      ret = arr[i]->read (is);
      if (ret.code () != 0)
        return ret;

      if ((c= skipws (is)) == ']')
        return ERR_SUCCESS;
      else if (c != ',')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    }
    return erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);

  case '{':
    is.get ();
    clear (type::object);
    if ((c = skipws (is)) == '}')
      return ERR_SUCCESS;

    for (int i = 0; i < max_object_names; i++)
    {
      if (c != '"')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      ret = parse_string (is, sval);
      if (ret.code () != 0)
        return ret;
      c = skipws (is);
      if (c != ':')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      auto n = obj.emplace (sval, std::make_unique<node> ()).first;
      ret = n->second->read (is);
      if (ret.code () != 0)
        return ret;
      c = skipws (is);
      if (c == '}')
        return ERR_SUCCESS;
      else if (c != ',')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      c = skipws (is);
    }
    return erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);

  case 't':
    if (token (is) != "true")
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    clear (type::boolean);
    logic = true;
    return ERR_SUCCESS;

  case 'f':
    if (token (is) != "false")
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    clear (type::boolean);
    logic = false;
    return ERR_SUCCESS;

  case 'n':
    if (token (is) != "null")
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    clear ();
    return ERR_SUCCESS;

  default:
    if (c == '-' || ('0' <= c && c <= '9'))
    {
      ret = parse_num (is, numval);
      if (ret.code () != 0)
        return ret;
      clear (type::numeric);
      num = numval;
      return ERR_SUCCESS;
    }
    return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
  }
}

/*!
  Parse a string to a JSON node.
  Returns an error if there are any extra characters after the JSON text.
*/
erc node::read (const std::string& s)
{
  stringstream ss (s);
  
  erc ret = read (ss);
  if (ret.code ())
    return ret;
  if (peekws (ss) != char_traits<char>::eof ())
    return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    
  return ERR_SUCCESS;
}

// Quotes all characters that need to be quotes in a string
static void quote (ostream& os, const std::string& s, bool quote_slash)
{
  std::u32string r = utf8::runes (s);
  for (auto chr : r)
  {
    switch (chr)
    {
    case '\b':  os << "\\b"; break;
    case '\f':  os << "\\f"; break;
    case '\n':  os << "\\n"; break;
    case '\r':  os << "\\r"; break;
    case '\t':  os << "\\t"; break;
    case '\\':  os << "\\\\"; break;
    case '"':   os << "\\\""; break;
    case '/':
      if (quote_slash)
        os << "\\/";
      else
        os << '/';
      break;

    default:
      if (chr >= ' ' && chr <= 0x7f)
        os << (char)chr;
      else if (chr < ' ' && chr < 0xffff)
      {
        //controls & basic multilingual plane
        char buf[8];
        sprintf (buf, "\\u%04x", (unsigned int)chr);
        os << buf;
      }
      else
      {
        //supplemental multilingual planes
        wstring ws = utf8::widen (utf8::narrow (&chr, 1));
        char buf[16];
        sprintf (buf, "\\u%04x\\u%04x", ws[0], ws[1]);
        os << buf;
      }
      break;
    }
  }
}

//formatting flags
int ostream_flags = ios_base::xalloc ();
/*
  Structure of formatting flags word (long):
  -----------------+----------------+
  | flags (8bits)  | spaces (8bits) |
  +----------------+----------------+
*/

/*!
  Write node to a stream.
  \param os stream to write to
  \param flags formatting flags
  \param spaces number of spaces to indent. If 0 uses tabs instead of spaces
  \param level starting indentation level
*/ 
erc node::write (std::ostream& os, int flags, int spaces, int level) const
{
  string fill;
  ++level;
  if (spaces == 0)
    fill.resize (level, '\t');
  else
    fill.resize (spaces*level, ' ');

  switch (t)
  {
  case type::object:
    os << "{";
    if ((flags & JSON_FMT_INDENT) != 0)
    {
      os << endl;
      os << fill;
    }
    for (auto ptr = obj.begin (); ptr != obj.end (); )
    {
      os << '"';
      quote (os, ptr->first, (flags & JSON_FMT_QUOTESLASH) != 0);
      os << "\":";
      ptr->second->write (os, flags, spaces, level);
      ++ptr;
      if (ptr != obj.end ())
        os << ',';
      if ((flags & JSON_FMT_INDENT) != 0)
      {
        os << endl;
        os << fill;
      }
    }
    os << '}';
    break;

  case type::array:
    os << "[";
    if ((flags & JSON_FMT_INDENT) != 0)
    {
      os << endl;
      os << fill;
    }
    for (auto ptr = arr.begin (); ptr != arr.end (); )
    {
      (*ptr++)->write (os, flags, spaces, level);
      if (ptr != arr.end ())
        os << ',';
      if ((flags & JSON_FMT_INDENT) != 0)
      {
        os << endl;
        os << fill;
      }
    }
    os << ']';
    break;

  case type::string:
    os << '"';
    quote (os, str, (flags & JSON_FMT_QUOTESLASH) != 0);
    os << '"';
    break;

  case type::numeric:
    if (num == floor (num))
      os << (long long)num;
    else
      os << num;
    break;

  case type::boolean:
    os << (logic ? "true" : "false");
    break;
  case type::null:
    os << "null";
    break;
  }
  --level;
  return ERR_SUCCESS;
}

/// Write node to a string
mlib::erc node::write (std::string& s, int flags, int spaces) const
{
  ostringstream os;
  auto ret = write (os, flags, spaces, 0);
  if (ret.code () == ERR_SUCCESS)
    s = os.str ();
  return ret;
}

/*
  Number conversion function
*/
double node::to_num () const
{
  switch (t)
  {
  case type::numeric:
    return num;
  case type::string:
    {
      char* end;
      double d = strtod (str.c_str (), &end);
      if (!*end)
        return d;
    }
  case type::null:
      return 0;
  case type::boolean:
    return logic ? 1. : 0.;
  }

  throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);
}

string node::to_str () const
{
  if (t == type::string)
    return str;
  else if (t == type::numeric)
    return std::to_string (num);
  else if (t == type::boolean)
    return logic ? "true" : "false";

  throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);
}
void indenter (std::ios_base& os, int spaces)
{
  os.iword (ostream_flags) &= ~0xff;
  os.iword (ostream_flags) |= (JSON_FMT_INDENT << 8) | spaces;
}


std::ostream& noindent (std::ostream& os)
{
  os.iword (ostream_flags) &= ~(JSON_FMT_INDENT << 8);
  return os;
}

/// Write a JSON object to a stream
std::ostream& operator << (std::ostream& os, const node& n)
{
  long fmt = os.iword (json::ostream_flags);
  n.write (os, fmt >> 8, fmt & 0xff);
  return os;
}

/// Read a JSON node from a stream
std::istream& operator >> (std::istream& is, node& n)
{
  n.clear ();
  erc ret = n.read (is);
  if (ret.code () != 0)
    throw ret;
  if (n.kind () != json::type::array && n.kind () != json::type::object)
    throw erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, json::errors);

  return is;
}


} // end json namespace


