#include <mlib/json.h>

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
  if (t == type::object)
    (&obj)->~nodes_map ();
  else if (t == type::array)
    (&arr)->~nodes_array ();
  else if (t == type::string)
    (&str)->~basic_string ();
}

/// Principal assignment operator
node& node::operator=(const node& rhs)
{
  if (&rhs != this)
  {
    //clear previous content
    if (t == type::object)
      (&obj)->~nodes_map ();
    else if (t == type::array)
      (&arr)->~nodes_array ();
    else if (t == type::string)
      (&str)->~basic_string ();

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
      for (auto n = rhs.arr.begin (); n != rhs.arr.end (); ++n)
        arr.emplace_back (make_unique<node> (**n));
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

} // end json namespace

std::ostream& operator << (std::ostream& os, const json::node& n)
{
  switch (n.kind ())
  {
  case json::type::object:
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

  case json::type::array:
    os << " [";
    for (auto ptr = n.begin (); ptr != n.end (); )
    {
      os << *ptr++;
      if (ptr != n.end ())
        os << ", ";
    }
    os << ']';
    break;

  case json::type::string:
    os << " \"" << n.to_string () << '"';
    break;
  case json::type::numeric:
    os << ' ' << n.to_number ();
    break;
  case json::type::boolean:
    os << (n.to_bool () ? " true" : " false");
    break;
  case json::type::null:
    os << " null";
    break;
  }
  return os;
}


} //end mlib namespace