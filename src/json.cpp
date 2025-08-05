/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <istream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <utf8/utf8.h>

using namespace std;

namespace mlib::json {

errfac default_json_errors ("JSON Error");
errfac* json_errors = &default_json_errors;

errfac& Errors ()
{
  return *json_errors;
}

void Errors (mlib::errfac& facility)
{
  json_errors = &facility;
}

node null_node;

/// Constructor for an empty node
node::node (type t_)
  : t (t_)
{
  switch (t)
  {
  case type::object:
    new (&obj) nodes_map ();
    break;
  case type::array:
    new (&arr) nodes_array ();
    break;
  case type::string:
    new (&str) std::string ();
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
    for (auto& n : other.obj)
    {
      auto v = n.second.get ();
      obj.emplace (n.first, make_unique<node> (*v));
    }
    break;
  case type::array: {
    new (&arr) nodes_array (other.arr.size ());
    for (size_t i = 0; i < other.arr.size (); ++i)
    {
      const auto& n = *other.arr[i].get ();
      arr[i] = make_unique<node> (n);
    }
    break;
  }
  case type::numeric:
    num = other.num;
    break;
  case type::string:
    new (&str) std::string ();
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
node& node::operator= (const node& rhs)
{
  if (&rhs != this)
  {
    clear (rhs.t);

    switch (t)
    {
    case type::object:
      for (auto& n : rhs.obj)
        obj.emplace (n.first, make_unique<node> (*n.second));
      break;
    case type::array:
      new (&arr) nodes_array (rhs.arr.size ());
      for (size_t i = 0; i < rhs.arr.size (); ++i)
        arr[i] = make_unique<node> (*rhs.arr[i]);
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
node& node::operator= (node&& rhs)
{
  if (&rhs != this)
  {
    memmove (this, &rhs, sizeof (node));
    rhs.t = type::null;
  }
  return *this;
}

/*!
  Return value of an object node element.

  If \p name doesn't exist, it is appended to node.
*/
node& node::operator[] (const std::string& name)
{
  if (t == type::null)
  {
    t = type::object;
    new (&obj) nodes_map ();
  }
  else if (t != type::object)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  auto p = obj.find (name);
  if (p == obj.end ())
  {
    if (obj.size () < max_object_names)
      p = obj.emplace (std::make_pair (name, std::make_unique<node> ())).first;
    else
      mlib::erc (ERR_JSON_TOOMANY, Errors()).raise ();
  }
  return *p->second;
}

/*!
  Return value of an object node element (const version).

  If element doesn't exist it throws an ERR_JSON_MISSING exception.
*/
const node& node::operator[] (const std::string& name) const
{
  if (t != type::object)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  auto p = obj.find (name);
  if (p == obj.end ())
    mlib::erc (ERR_JSON_MISSING, Errors()).raise ();
  return *p->second;
}

/*!
  Return value of an object node element.

  If element doesn't exist throws an ERR_JSON_MISSING exception.
*/
const node& node::at (const std::string& name) const
{
  if (t != type::object)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  try
  {
    return *obj.at (name);
  }
  catch (std::out_of_range)
  {
    mlib::erc (ERR_JSON_MISSING, Errors(), mlib::erc::critical).raise ();
    return null_node;
  }
}

/*!
  Return value of an array node element.

  If element doesn't exist the array is extended with null elements.
*/
node& node::operator[] (size_t index)
{
  if (t == type::null)
  {
    t = type::array;
    new (&arr) nodes_array ();
  }
  else if (t != type::array)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  if (index >= arr.size ())
  {
    // extend array
    auto old_size = arr.size ();
    if (index < max_array_size - 1)
      arr.resize (index + 1);
    else
      mlib::erc (ERR_JSON_TOOMANY, Errors()).raise ();

    // add null nodes
    for (auto i = old_size; i <= index; i++)
      arr[i] = std::make_unique<node> ();
  }
  return *arr[index];
}

/*!
  Return value of an array node element (const version).

  If element doesn't exist throws an ERR_JSON_MISSING exception.
*/
const node& node::operator[] (size_t index) const
{
  if (t != type::array)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  if (index >= arr.size ())
    mlib::erc (ERR_JSON_MISSING, Errors()).raise ();

  return *arr[index];
}

/*!
  Return value of an array node element (const version).

  If element doesn't exist throws an ERR_JSON_MISSING exception.
*/
const node& node::at (size_t index) const
{
  if (t != type::array)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();

  try
  {
    return *arr.at (index);
  }
  catch (std::out_of_range)
  {
    mlib::erc (ERR_JSON_MISSING, Errors(), mlib::erc::critical).raise ();
    return null_node;
  }
}

/// Equality operator
bool node::operator== (const node& other) const
{
  if (t == other.t)
  {
    switch (t)
    {
    case type::object:
      return (obj.size () == other.obj.size ())
             && std::equal (obj.begin (), obj.end (), other.obj.begin (), other.obj.end (),
                            [] (auto const& n, auto const& m) -> bool {
                              return (n.first == m.first) && (*n.second == *m.second);
                            });

    case type::array:
      return (arr.size () == other.arr.size ())
             && std::equal (arr.begin (), arr.end (), other.arr.begin (), other.arr.end (),
                            [] (auto const& n, auto const& m) -> bool { return *n == *m; });

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

/*!
   If node is an object and has a child with the given name, returns 
   a pointer to child node. Otherwise returns `nullptr`
*/
const node* node::find (const std::string& name) const
{
  if (t != type::object)
    return nullptr;
  auto n = obj.find (name);
  if (n != obj.end ())
    return &*n->second;
  else
    return nullptr;
}

/*!
   If node is an object and has a child with the given name, returns
   a pointer to child node. Otherwise returns `nullptr`
*/
node* node::find (const std::string& name)
{
  if (t != type::object)
    return nullptr;
  auto n = obj.find (name);
  if (n != obj.end ())
    return &*n->second;
  else
    return nullptr;
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

  while (i++ < 10 && isalpha (is.peek ()))
    tok.push_back (is.get ());

  return tok;
}

/*
  Parse a JSON number
*/
static erc parse_num (istream& is, double& num)
{
  num = 0;
  double dec = 1.;
  int exp = 0, expsign = 1;

  char c = skipws (is);
  int sign = (c == '-') ? -1 : 1;
  if (c == '-')
    c = is.get ();

  // integer part
  if ('1' <= c && c <= '9')
  {
    do
    {
      num = num * 10 + (c - '0');
      c = is.get ();
    } while (isdigit (c));
  }
  else if (c == '0')
    c = is.get ();
  else
    return erc (ERR_JSON_INPUT, Errors());

  // fraction
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
    return erc::success;
  }

  // exponent
  c = is.get ();
  if (c == '+')
    c = is.get ();
  else if (c == '-')
  {
    expsign = -1;
    c = is.get ();
  }
  if (!isdigit (c))
    return erc (ERR_JSON_INPUT, Errors());

  do
  {
    exp = exp * 10 + (c - '0');
    c = is.get ();
  } while (isdigit (c));

  num *= sign * pow (10, exp * expsign);
  is.putback (c);
  return erc::success;
}

/*
  Parse a JSON string
*/
static erc parse_string (istream& is, std::string& s)
{
  s.clear ();
  int len = 0;
  int high_surrogate = 0;
  while (++len < max_string_length)
  {
    int c = is.get ();
    if (high_surrogate && c != '\\')
      return erc (ERR_JSON_INPUT, Errors ()); // non terminated surrogate sequence

    if (c == '"')
      return erc::success;
    else if (c < 0x20 || c == char_traits<char>::eof ())
      return erc (ERR_JSON_INPUT, Errors());
    else if (c == '\\')
    {
      c = is.get ();
      if (high_surrogate && c != 'u')
        return erc (ERR_JSON_INPUT, Errors ()); // non terminated surrogate sequence

      switch (c)
      {
      case '"':
      case '\\':
      case '/':
        s.push_back (c);
        break;
      case 'b':
        s.push_back ('\b');
        break;
      case 'f':
        s.push_back ('\f');
        break;
      case 'n':
        s.push_back ('\n');
        break;
      case 'r':
        s.push_back ('\r');
        break;
      case 't':
        s.push_back ('\t');
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
            return erc (ERR_JSON_INPUT, Errors());
        }
        if (0xd800 <= c && c <= 0xdbff)
        {
          if (!high_surrogate)
            high_surrogate = c;
          else
            return erc (ERR_JSON_INPUT, Errors ()); //two consecutive high surrogates
        }
        else if (0xdc00 <= c && c <= 0xdfff)
        {
          if (!high_surrogate)
            return erc (ERR_JSON_INPUT, Errors ()); // low surrogate without high
          c = 0x10000 + ((high_surrogate - 0xd800) << 10) + (c - 0xdc00);
          high_surrogate = 0;
          s += utf8::narrow ((char32_t)c);
        }
        else if (high_surrogate)
          return erc (ERR_JSON_INPUT, Errors ()); // high surrogate without low
        else
          s += utf8::narrow((char32_t)c);
        break;

      default:
        return erc (ERR_JSON_INPUT, Errors());
        break;
      }
    }
    else
      s.push_back (c);
  }
  return erc (ERR_JSON_SIZE, Errors());
}

/*!
  Deserialize the node from an input stream
*/
erc node::read (std::istream& is)
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
      return erc::success;
    }

    for (int i = 0; i < max_array_size; i++)
    {
      arr.push_back (std::make_unique<node> ());
      ret = arr[i]->read (is);
      if (ret.code () != 0)
        return ret;

      if ((c = skipws (is)) == ']')
        return erc::success;
      else if (c != ',')
        return erc (ERR_JSON_INPUT, Errors());
    }
    return erc (ERR_JSON_TOOMANY, Errors());

  case '{':
    is.get ();
    clear (type::object);
    if ((c = skipws (is)) == '}')
      return erc::success;

    for (int i = 0; i < max_object_names; i++)
    {
      if (c != '"')
        return erc (ERR_JSON_INPUT, Errors());
      ret = parse_string (is, sval);
      if (ret.code () != 0)
        return ret;
      c = skipws (is);
      if (c != ':')
        return erc (ERR_JSON_INPUT, Errors());
      auto n = obj.emplace (sval, std::make_unique<node> ()).first;
      ret = n->second->read (is);
      if (ret.code () != 0)
        return ret;
      c = skipws (is);
      if (c == '}')
        return erc::success;
      else if (c != ',')
        return erc (ERR_JSON_INPUT, Errors());
      c = skipws (is);
    }
    return erc (ERR_JSON_TOOMANY, Errors());

  case 't':
    if (token (is) != "true")
      return erc (ERR_JSON_INPUT, Errors());
    clear (type::boolean);
    logic = true;
    return erc::success;

  case 'f':
    if (token (is) != "false")
      return erc (ERR_JSON_INPUT, Errors());
    clear (type::boolean);
    logic = false;
    return erc::success;

  case 'n':
    if (token (is) != "null")
      return erc (ERR_JSON_INPUT, Errors());
    clear ();
    return erc::success;

  default:
    if (c == '-' || ('0' <= c && c <= '9'))
    {
      ret = parse_num (is, numval);
      if (ret.code () != 0)
        return ret;
      clear (type::numeric);
      num = numval;
      return erc::success;
    }
    return erc (ERR_JSON_INPUT, Errors());
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
    return erc (ERR_JSON_INPUT, Errors());

  return erc::success;
}

// Quotes all characters that need to be quotes in a string
static void quote (ostream& os, const std::string& s, int flags)
{
  std::u32string r = utf8::runes (s);
  for (auto chr : r)
  {
    switch (chr)
    {
    case '\b':
      os << "\\b";
      break;
    case '\f':
      os << "\\f";
      break;
    case '\n':
      os << "\\n";
      break;
    case '\r':
      os << "\\r";
      break;
    case '\t':
      os << "\\t";
      break;
    case '\\':
      os << "\\\\";
      break;
    case '"':
      os << "\\\"";
      break;
    case '/':
      if (flags & JSON_FMT_QUOTESLASH)
        os << "\\/";
      else
        os << '/';
      break;

    default:
      if (chr >= ' ' && chr <= 0x7f)
        os << (char)chr;
      else if (flags & JSON_FMT_UTF8)
        os << utf8::narrow (chr);
      else if (chr < 0xffff)
      {
        // basic multilingual plane
        char buf[8];
        sprintf (buf, "\\u%04x", (unsigned int)chr);
        os << buf;
      }
      else
      {
        // supplemental multilingual planes
        wstring ws = utf8::widen (chr);
        char buf[16];
        sprintf (buf, "\\u%04x\\u%04x", ws[0], ws[1]);
        os << buf;
      }
      break;
    }
  }
}

// formatting flags
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
    fill.resize (spaces * level, ' ');

  switch (t)
  {
  case type::object:
    os << "{";
    if ((flags & JSON_FMT_INDENT) != 0)
    {
      os << endl;
      os << fill;
    }
    for (auto ptr = obj.begin (); ptr != obj.end ();)
    {
      os << '"';
      quote (os, ptr->first, flags);
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
    for (size_t i = 0; i < arr.size (); ++i)
    {
      if (i)
        os << ',';
      if ((flags & JSON_FMT_INDENT) != 0)
        os << endl << fill;
      arr[i]->write (os, flags, spaces, level);
    }
    os << ']';
    break;

  case type::string:
    os << '"';
    quote (os, str, flags);
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
  return erc::success;
}

/// Write node to a string
mlib::erc node::write (std::string& s, int flags, int spaces) const
{
  ostringstream os;
  auto ret = write (os, flags, spaces, 0);
  if (ret == erc::success)
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
  case type::string: {
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

  throw mlib::erc (ERR_JSON_INVTYPE, Errors());
}

/*!
  String conversion function
*/
string node::to_str () const
{
  if (t == type::string)
    return str;
  else if (t == type::numeric)
    return std::to_string (num);
  else if (t == type::boolean)
    return logic ? "true" : "false";

  throw mlib::erc (ERR_JSON_INVTYPE, Errors());
}

/*!
  Conversion to boolean value
*/
bool node::to_bool () const
{
  if (t == type::boolean)
    return logic;
  if (t == type::string)
  {
    auto s = utf8::tolower (str);
    if (s == "false" || s == "0" || s == "off")
      return false;
    if (s == "true" || s == "1" || s == "on")
      return true;
  }
  else if (t == type::numeric)
  {
    return (num != 0);
  }
  throw mlib::erc (ERR_JSON_INVTYPE, Errors());
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

std::ostream& utf8 (std::ostream& os)
{
  os.iword (ostream_flags) |= (JSON_FMT_UTF8 << 8);
  return os;
}

std::ostream& noutf8 (std::ostream& os)
{
  os.iword (ostream_flags) &= ~(JSON_FMT_UTF8 << 8);
  return os;
}

  /// Write a JSON object to a stream
std::ostream& operator<< (std::ostream& os, const node& n)
{
  long fmt = os.iword (json::ostream_flags);
  n.write (os, fmt >> 8, fmt & 0xff);
  return os;
}

/// Read a JSON node from a stream
std::istream& operator>> (std::istream& is, node& n)
{
  n.clear ();
  erc ret = n.read (is);
  if (ret.code () != 0)
    throw ret;
  if (n.kind () != json::type::array && n.kind () != json::type::object)
    throw erc (ERR_JSON_INPUT, json::Errors());

  return is;
}

} // namespace json
