#include <mlib/json.h>
#include <istream>
#include <ostream>
#include <utf8/utf8.h>

using namespace std;

namespace mlib
{

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
    clear ();

    t = rhs.t;
    switch (t)
    {
    case type::object:
      new (&obj) nodes_map ();
      for (auto n = rhs.obj.begin (); n != rhs.obj.end (); ++n)
        obj.emplace (n->first, make_unique<node> (*n->second));
      break;
    case type::array:
      new (&arr) nodes_array (rhs.arr.size ());
      for (int i = 0; i < rhs.arr.size(); i++)
        arr[i] = make_unique<node> (*rhs.arr[i]);
      break;
    case type::string:
      new (&str)std::string ();
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

/// Assign a boolean value to node
node& node::operator=(bool b)
{
  clear (type::boolean);
  logic = b;
  return *this;
}

/// Assign an integer value to node
node& node::operator=(int n)
{
  clear (type::numeric);
  num = n;
  return *this;
}

/// Assign a float value to node
node& node::operator=(double d)
{
  clear (type::numeric);
  num = d;
  return *this;
}

/// Assign a string value to node
node& node::operator=(const std::string& s)
{
  clear (type::string);
  str = s;
  return *this;
}

/// Assign a string value to node
node& node::operator=(const char* s)
{
  clear (type::string);
  str = s;
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

/// Return value of an array node element
node& node::operator[](int index)
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
    int old_size = (int)arr.size ();
    if (index < max_array_size - 1)
      arr.resize (index + 1);
    else
      throw mlib::erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);

    //add null nodes
    for (int i=old_size; i<=index; i++)
      arr[i] = std::make_unique<node> ();
  }
  return *arr[index];
}

// Parsing helper functions ---------------------------------------------------
// Check if caharcter is whitespace
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
  Collect all characters until next whitespace.
  As we use this function only to look for predefined tokens (true, false, null),
  the limit for token length is really small.
*/
static std::string token (istream& is)
{
  std::string tok;
  char c;
  int i = 0;
  tok.reserve (10);

  while (i++ < 10 && !is_ws (c = is.get ()))
    tok.push_back (c);

  return tok;
}

/*
  Parse a JSON number
*/
static erc parse_num (istream& is, double& num)
{
  num = 0;
  int dec = 1, exp, expsign = 1;
  enum { first_digit, digit, fraction, first_exp, exponent } state = first_digit;

  int c = skipws (is);
  int sign = (c == '-') ? -1 : 1;
  for (int len = 0; len <max_num_digits; len++)
  {
    switch (state)
    {
    case first_digit: //first digit
      if (c == 0)
        state = fraction;
      else if ('1' <= c && c <= '9')
      {
        num = c - '0';
        state = digit;
      }
      else
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      break;

    case digit: //integer part
      if ('0' <= c && c <= '9')
        num = num * 10 + (c - '0');
      else if (c == '.')
        state = fraction;
      else if (c == 'e' || c == 'E')
        state = first_exp;
      else
      {
        num *= sign;
        is.putback (c);
        return ERR_SUCCESS;
      }
      break;

    case fraction: //fractional part
      if (0 <= 'c' && c <= '9')
      {
        dec /= 10;
        num += (c - '0') * dec;
      }
      else if ('c' == 'e' || 'c' == 'E')
        state = first_exp;
      else
      {
        num *= sign;
        is.putback (c);
        return ERR_SUCCESS;
      }
    case first_exp: //exponent
      if ('c' == '-')
        expsign = -1;
      else if ('0' <= c && c <= '9')
        exp = c - '0';
      else if ('c' != '+')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      state = exponent;
      break;

    case exponent: //exponent digit
      if ('0' <= c && c <= '9')
        exp = exp * 10 + (c - '0');
      else
      {
        num *= sign * pow (10, exp*expsign);
        is.putback (c);
        return true;
      }
      break;
    }
    c = is.get ();
  }
  return erc (ERR_JSON_SIZE, ERROR_PRI_ERROR, errors);
}

/*
  Parse a JSON string
*/
static erc parse_string (istream& is, std::string& s)
{
  s.clear ();
  char32_t u;
  int len = 0;
  while (++len < max_string_length)
  {
    int c = is.get ();
    if (c == '"')
      return ERR_SUCCESS;
    else if (c < 0x20 || c == char_traits<char>::eof ())
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    else if (c == '\\')
    {
      c = is.get ();
      switch (c)
      {
      case '"':
      case '\\':
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
        u = 0;
        for (int i = 0; i < 4; i++)
        {
          int x = is.get ();
          if ('0' <= x && x <= '9')
            u = (u << 4) + (x - '0');
          else if ('A' <= x && x <= 'F')
            u = (u << 4) + (x - 'A' + 10);
          else if ('a' <= x && x <= 'f')
            u = (u << 4) + (x - 'a' + 10);
          else
            return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
        }
        s += utf8::narrow (&u, 1);
      default:
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
        break;
      }
    }
    else
      s.push_back (c);
  }
  return erc (ERR_JSON_SIZE, ERROR_PRI_ERROR, errors);
}

erc read (istream& is, node& n)
{
  std::string str;
  double num;
  erc ret;

  char c = peekws (is);
  switch (c)
  {
  case '"':
    is.get ();
    ret = parse_string (is, str);
    if (ret.code () == 0)
      n = str;
    return ret;

  case '[':
    is.get ();
    n.clear (type::array);
    if (peekws (is) == ']')
    {
      is.get ();
      return ERR_SUCCESS;
    }

    for (int i = 0; i < max_array_size; i++)
    {
      ret = read (is, n[i]);
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
    n.clear (type::object);
    if ((c = skipws (is)) == '}')
      return ERR_SUCCESS;

    for (int i = 0; i < max_object_names; i++)
    {
      if (c != '"')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      ret = parse_string (is, str);
      if (ret.code () != 0)
        return ret;
      c = skipws (is);
      if (c != ':')
        return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
      ret = read (is, n[str]);
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
    n = true;
    return ERR_SUCCESS;

  case 'f':
    if (token (is) != "false")
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    n = false;
    return ERR_SUCCESS;

  case 'n':
    if (token (is) != "null")
      return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
    n.clear ();
    return true;

  default:
    if (c == '-' || ('0' <= c && c <= '9'))
    {
      ret = parse_num (is, num);
      if (ret.code () != 0)
        return ret;
      n = num;
      return ERR_SUCCESS;
    }
    return erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, errors);
  }
}

/// Wite a JSON node to a stream
erc write (std::ostream& os, const node& n)
{
  switch (n.kind ())
  {
  case type::object:
    os << " {";
    for (auto ptr = n.begin (); ptr != n.end (); )
    {
      os << '"' << ptr.name () << "\":" << *ptr;
      ++ptr;
      if (ptr != n.end ())
        os << ", ";
    }
    os << '}';
    break;

  case type::array:
    os << " [";
    for (auto ptr = n.begin (); ptr != n.end (); )
    {
      os << *ptr++;
      if (ptr != n.end ())
        os << ", ";
    }
    os << ']';
    break;

  case type::string:
    os << " \"" << n.to_string () << '"';
    break;
  case type::numeric:
    os << ' ' << n.to_number ();
    break;
  case type::boolean:
    os << (n.to_bool () ? " true" : " false");
    break;
  case type::null:
    os << " null";
    break;
  }
  return ERR_SUCCESS;
}


} // end json namespace

/// Write a JSON object to a stream
std::ostream& operator << (std::ostream& os, const json::node& n)
{
  write (os, n);
  return os;
}

/// Read a JSON node from a stream
std::istream& operator >> (std::istream& is, json::node& n)
{
  n.clear ();
  erc ret = read (is, n);
  if (ret.code () != 0)
    throw ret;
  if (n.kind () != json::type::array && n.kind () != json::type::object)
    throw erc (ERR_JSON_INPUT, ERROR_PRI_ERROR, json::errors);

  return is;
}

} //end mlib namespace