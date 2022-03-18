#pragma once

#include <map>
#include <memory>
#include <mlib/errorcode.h>
#include <ostream>
#include <string>
#include <vector>

namespace mlib {

namespace json {

constexpr int max_array_size = 8192;
constexpr int max_object_names = 8192;

// Errors
#define ERR_JSON_INVTYPE    -1    //invalid node type
#define ERR_JSON_TOOMANY    -2    //too many descendants
#define ERR_JSON_ITERTYPE   -3    //invalid iterator type
#define ERR_JSON_ITERPOS    -4    //invalid iterator position

extern mlib::errfac* errors;

enum type { null, object, array, numeric, string, boolean };

class node {
public:
  using pnode = std::unique_ptr<node>;
  using nodes_map = std::map<const std::string, pnode>;
  using nodes_array = std::vector<pnode>;

  // Constructors
  node (type t = type::null);
  node (const std::string& s);
  node (const char* s);
  node (double d);
  node (int d);
  node (bool b);
  node (const node& other);
  ~node ();

  template <bool C_>
  class iterator_type {
  public:
    //Following typedefs must exist to allow instantiation of std::iterator_traits
    using difference_type = std::ptrdiff_t;
    using value_type = node;
    using reference = typename std::conditional_t< C_, node const&, node& >;
    using pointer = typename std::conditional_t< C_, node const*, node* >;
    using obj_iter = typename std::conditional_t< C_, nodes_map::const_iterator, nodes_map::iterator>;
    using arr_iter = typename std::conditional_t< C_, nodes_array::const_iterator, nodes_array::iterator>;
    using iterator_category = std::bidirectional_iterator_tag;

    // Copy constructor
    iterator_type<C_> (const iterator_type<C_>& other)
      : target (other.target)
    {
      if (target.t == type::object)
        new (&objit) obj_iter (other.objit);
      else if (target.t == type::array)
        new (&arrit) arr_iter (other.arrit);
      else
        at_end = other.at_end;
    }

    // Destructor
    ~iterator_type<C_> () 
    {
      if (target.t == type::object)
        (&objit)->~obj_iter ();
      else if (target.t == type::array)
        (&arrit)->~arr_iter ();
    }

    /// Dereference value
    reference operator *()
    {
      if (target.t == type::object)
      {
        if (objit == target.obj.end ())
          throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
        return *objit->second;
      }
      else if (target.t == type::array)
      {
        if (arrit == target.arr.end ())
          throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
        return **arrit;
      }
      else if (!at_end)
        return target;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
    }

    /// Value pointer
    pointer operator ->()
    {
      if (target.t == type::object)
      {
        if (objit == target.obj.end ())
          throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
        return objit->second.get ();
      }
      else if (target.t == type::array)
      {
        if (arrit == target.arr.end ())
          throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
        return arrit->get ();
      }
      else if (!at_end)
        return &target;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
    }

    /// Object name
    const std::string& name () const
    {
      if (target.t == type::object)
      {
        if (objit == target.obj.end ())
          throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
        return objit->first;
      }
      else
        throw mlib::erc (ERR_JSON_ITERTYPE, ERROR_PRI_ERROR, errors);
    }

    ///Increment operator (postfix)
    iterator_type<C_> operator ++ (int)
    {
      iterator_type<C_> tmp = *this;
      if (target.t == type::object)
        objit.operator++ ();
      else if (target.t == type::array)
        arrit.operator ++();
      else if (!at_end)
        at_end = true;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
      return tmp;
    }

    ///Increment operator (prefix)
    iterator_type<C_>& operator ++ ()
    {
      if (target.t == type::object)
        objit.operator++ ();
      else if (target.t == type::array)
        arrit.operator ++();
      else if (!at_end)
        at_end = true;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);

      return *this;
    }

    ///Decrement operator (postfix)
    iterator_type<C_> operator -- (int)
    {
      iterator_type<C_> tmp = *this;
      if (target.t == type::object)
        objit.operator-- ();
      else if (target.t == type::array)
        arrit.operator --();
      else if (at_end)
        at_end = false;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
      return tmp;
    }

    ///Decrement operator (prefix)
    iterator_type<C_> operator -- ()
    {
      if (target.t == type::object)
        objit.operator-- ();
      else if (target.t == type::array)
        arrit.operator --();
      else if (at_end)
        at_end = false;
      else
        throw mlib::erc (ERR_JSON_ITERPOS, ERROR_PRI_ERROR, errors);
      return tmp;
    }

    ///Equality operator
    bool operator == (const iterator_type<C_>& other) const
    {
      if (&target != &other.target)
        return false;
      else if (target.t == type::object)
        return (objit == other.objit);
      else if (target.t == type::array)
        return (arrit == other.arrit);
      else
        return (at_end == other.at_end);
    }

    ///Inequality operator
    bool operator != (const iterator_type<C_>& other) const
    {
      return !operator ==(other);
    }

  private:
    iterator_type<C_> (const node& n, bool end)
      : target (n)
      , at_end (end)
    {
    }

    iterator_type<C_> (const node& n, const arr_iter& it) 
      : target (n)
      , arrit (it)
    {

    }

    iterator_type (const node& n, const obj_iter& it)
      : target (n)
      , objit (it)
    {
    }

    union {
      obj_iter objit;
      arr_iter arrit;
      bool at_end;
    };

    const node& target;
    friend class node;
  };

  typedef iterator_type<false> iterator;
  typedef iterator_type<true> const_iterator;

  node& operator = (const node& rhs);

  node& operator [](const std::string& name);
  node& operator [](int index);

  const_iterator begin () const;
  iterator begin ();

  const_iterator end () const;
  iterator end ();

  /// String value
  std::string to_string () const
  {
    if (t == type::string)
      return str;
    else
      throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);
  }

  /// Numeric value
  double to_number () const
  {
    if (t == type::numeric)
      return num;
    else
      throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);
  }

  /// Boolean value
  bool to_bool () const
  {
    if (t == type::boolean)
      return logic;
    else
      throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);
  }

  void clear ();
  type kind () const;
  int descendants () const;

private:
  type t;
  union {
    nodes_map obj;
    nodes_array arr;
    std::string str;
    double num;
    bool logic;
    friend class node;
  };
};


/// Constructor for a string type node
inline node::node (const std::string& s)
  : t (type::string)
  , str (s)
{
}

/// Alternate constructor for a string node
inline node::node (const char* s)
  : t (type::string)
  , str (s)
{
}

/// Constructor for a numeric node
inline node::node (double d)
  : t (type::numeric)
  , num (d)
{
}

///Alternate constructor for a numeric node
inline node::node (int d)
  : t (type::numeric)
  , num (d)
{
}

/// Constructor for a boolean node
inline node::node (bool b)
  : t (type::boolean)
  , logic (b)
{
}

/// Return value of an object node element
inline node& node::operator[](const std::string& name)
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
inline node& node::operator[](int index)
{
  if (t == type::null)
    t = type::array;
  else if (t != type::array)
    throw mlib::erc (ERR_JSON_INVTYPE, ERROR_PRI_ERROR, errors);

  if (index >= arr.size ())
  {
    if (index < max_array_size - 1)
      arr.resize (index + 1);
    else
      throw mlib::erc (ERR_JSON_TOOMANY, ERROR_PRI_ERROR, errors);
  }
  return *arr[index];
}

// Begin iterator
inline node::const_iterator node::begin () const
{
  if (t == type::object)
    return const_iterator (*this, obj.begin ());
  else if (t == type::array)
    return const_iterator (*this, arr.begin ());
  else
    return const_iterator (*this, false);
}

inline node::iterator node::begin ()
{
  if (t == type::object)
    return iterator (*this, obj.begin ());
  else if (t == type::array)
    return iterator (*this, arr.begin ());
  else
    return iterator (*this, false);
}

/// End iterator
inline node::const_iterator node::end () const
{
  if (t == type::object)
    return const_iterator (*this, obj.end ());
  else if (t == type::array)
    return const_iterator (*this, arr.end ());
  else
    return const_iterator (*this, true);
}

inline node::iterator node::end ()
{
  if (t == type::object)
    return iterator (*this, obj.end ());
  else if (t == type::array)
    return iterator (*this, arr.end ());
  else
    return iterator (*this, true);
}

// Remove node content
inline void node::clear ()
{
  switch (t)
  {
  case type::object:
    obj.clear ();
    break;
  case type::array:
    arr.clear ();
    break;
  case type::string:
    str.clear ();
    break;
  case type::numeric:
    num = 0.;
    break;
  case type::boolean:
    logic = false;
    break;
  }
}

/// Return the type of node
inline type node::kind () const
{
  return t;
}

/// Return number of direct descendants
inline int node::descendants () const
{
  return (t == type::object) ? (int)obj.size () :
         (t == type::array) ? (int)arr.size () : 0;
}

} //namespace json

std::ostream& operator << (std::ostream& os, const json::node& n);


} //namespace mlib