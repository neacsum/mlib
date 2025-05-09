/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file json.h Definition of mlib::json::node class

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mlib/errorcode.h>
#include <iomanip>
#include <string>
#include <vector>

namespace mlib::json {

/// Maximum number of array elements in a JSON node
constexpr int max_array_size = 8192;

/// Maximum number of properties for a node
constexpr int max_object_names = 8192;

/// Maximum string length
constexpr int max_string_length = 8192;

// Formatting flags
#define JSON_FMT_INDENT     0x01 //!< Indent JSON string when writing
#define JSON_FMT_QUOTESLASH 0x02 //!< Quote all solidus ('/') characters
#define JSON_FMT_UTF8       0x04 //!< Do not encode UTF8 characters
// JSON error codes
#define ERR_JSON_INVTYPE  -1 //!< invalid node type
#define ERR_JSON_TOOMANY  -2 //!< too many descendants
#define ERR_JSON_ITERTYPE -3 //!< invalid iterator type
#define ERR_JSON_ITERPOS  -4 //!< invalid iterator position
#define ERR_JSON_INPUT    -5 //!< invalid character in input stream
#define ERR_JSON_SIZE     -7 //!< invalid element size
#define ERR_JSON_MISSING  -8 //!< invalid index or key on const node

/// Return error facility used for JSON errors
mlib::errfac& Errors ();

/// Set error facility for JSON errors
void Errors (mlib::errfac& facility);

/// JSON node types
enum class type
{
  null,
  object,
  array,
  numeric,
  string,
  boolean
};

/// Representation of a JSON node
class node
{
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

  template <typename T>
  node (const std::vector<T>& vec);

  template <class T>
  node (const T& t, decltype (&T::to_json)* = nullptr);

  node (const node& other);
  node (node&& other);
  ~node ();

  /// node iterator
  template <bool C_>
  class iterator_type
  {
  public:
    /// \cond not_documented
    // Following typedefs must exist to allow instantiation of std::iterator_traits
    using difference_type = std::ptrdiff_t;
    using value_type = node;
    using reference = typename std::conditional_t<C_, node const&, node&>;
    using pointer = typename std::conditional_t<C_, const node*, node*>;
    using obj_iter =
      typename std::conditional_t<C_, nodes_map::const_iterator, nodes_map::iterator>;
    using arr_iter =
      typename std::conditional_t<C_, nodes_array::const_iterator, nodes_array::iterator>;
    using iterator_category = std::bidirectional_iterator_tag;
    /// \endcond
    /// Copy constructor
    iterator_type (const iterator_type& other)
      : target (other.target)
    {
      if (target.t == type::object)
        new (&objit) obj_iter (other.objit);
      else if (target.t == type::array)
        new (&arrit) arr_iter (other.arrit);
      else
        at_end = other.at_end;
    }

    /// Destructor
    ~iterator_type ()
    {
      if (target.t == type::object)
        (&objit)->~obj_iter ();
      else if (target.t == type::array)
        (&arrit)->~arr_iter ();
    }

    /// Dereference value
    reference operator* ()
    {
      if (target.t == type::object)
      {
        if (objit == target.obj.end ())
          mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
        return *objit->second;
      }
      else if (target.t == type::array)
      {
        if (arrit == target.arr.end ())
          mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
        return **arrit;
      }
      else if (at_end)
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();

      return const_cast<json::node&> (target);
    }

    /// Value pointer
    pointer operator->()
    {
      if (target.t == type::object)
      {
        if (objit == target.obj.end ())
          mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
        return objit->second.get ();
      }
      else if (target.t == type::array)
      {
        if (arrit == target.arr.end ())
          mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
        return arrit->get ();
      }
      else if (at_end)
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
      return const_cast<json::node*> (&target);
    }

    /// Object name
    const std::string& name () const
    {
      if (target.t != type::object)
        mlib::erc (ERR_JSON_ITERTYPE, Errors()).raise ();
      if (objit == target.obj.end ())
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
      return objit->first;
    }

    /// Increment operator (postfix)
    iterator_type<C_> operator++ (int)
    {
      iterator_type<C_> tmp = *this;
      if (target.t == type::object)
        objit.operator++ ();
      else if (target.t == type::array)
        arrit.operator++ ();
      else if (!at_end)
        at_end = true;
      else
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
      return tmp;
    }

    /// Increment operator (prefix)
    iterator_type<C_>& operator++ ()
    {
      if (target.t == type::object)
        objit.operator++ ();
      else if (target.t == type::array)
        arrit.operator++ ();
      else if (!at_end)
        at_end = true;
      else
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();

      return *this;
    }

    /// Decrement operator (postfix)
    iterator_type<C_> operator-- (int)
    {
      iterator_type<C_> tmp = *this;
      if (target.t == type::object)
        objit.operator-- ();
      else if (target.t == type::array)
        arrit.operator-- ();
      else if (at_end)
        at_end = false;
      else
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
      return tmp;
    }

    /// Decrement operator (prefix)
    iterator_type<C_> operator-- ()
    {
      if (target.t == type::object)
        objit.operator-- ();
      else if (target.t == type::array)
        arrit.operator-- ();
      else if (at_end)
        at_end = false;
      else
        mlib::erc (ERR_JSON_ITERPOS, Errors()).raise ();
      return *this;
    }

    /// Equality operator
    bool operator== (const iterator_type<C_>& other) const
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

    /// Inequality operator
    bool operator!= (const iterator_type<C_>& other) const
    {
      return !operator== (other);
    }

  private:
    iterator_type (const node& n, bool end)
      : target (n)
      , at_end (end)
    {}

    iterator_type (const node& n, const arr_iter& it)
      : target (n)
      , arrit (it)
    {}

    iterator_type (const node& n, const obj_iter& it)
      : target (n)
      , objit (it)
    {}

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

  // ------------------ Assignments -------------------------------------------
  // principal assignment operator
  node& operator= (const node& rhs);

  // move assignment operator
  node& operator= (node&& rhs);

  /// Assignment operator
  template <class T, typename B = decltype (&T::to_json)>
  node& operator= (const T& t)
  {
    clear ();
    t.to_json (*this);
    return *this;
  }

  /// Boolean assignment
  node& operator= (bool b)
  {
    clear (type::boolean);
    logic = b;
    return *this;
  }

  /// Numeric assignment
  template <typename T, typename B = std::enable_if_t<std::is_arithmetic_v<T>>>
  node& operator= (T n)
  {
    clear (type::numeric);
    num = n;
    return *this;
  }

  /// String assignment
  node& operator= (const std::string& s)
  {
    clear (type::string);
    str = s;
    return *this;
  }

  /// C string assignment
  node& operator= (const char* s)
  {
    clear (type::string);
    str = s;
    return *this;
  }

  // type conversions
  explicit operator std::string () const;
  explicit operator const char* () const;
  explicit operator double () const;
  explicit operator float () const;
  explicit operator int () const;
  explicit operator bool () const;

  double to_num () const;
  std::string to_str () const;
  bool to_bool () const;

  // indexing
  node& operator[] (const std::string& name);
  const node& operator[] (const std::string& name) const;
  node& at (const std::string& name);
  const node& at (const std::string& name) const;

  node& operator[] (size_t index);
  const node& operator[] (size_t index) const;
  node& at (size_t index);
  const node& at (size_t index) const;

  const_iterator begin () const;
  iterator begin ();
  const_iterator end () const;
  iterator end ();

  //(in)equality operators
  bool operator== (const node& other) const;
  bool operator!= (const node& other) const;

  // streaming (encoding/decoding)
  mlib::erc read (std::istream& s);
  mlib::erc read (const std::string& s);
  mlib::erc write (std::ostream& os, int flags = 0, int spaces = 0, int level = 0) const;
  mlib::erc write (std::string& s, int flags = 0, int spaces = 0) const;

  // other operations
  bool has (const std::string& name) const;
  void erase (const std::string& name);
  void clear (type t = type::null);
  type kind () const;
  int size () const;

private:
  type t;
  union {
    nodes_map obj;
    nodes_array arr;
    std::string str;
    double num;
    bool logic;
  };
};

/// Constructor for a string type node
inline node::node (const std::string& s)
  : t (type::string)
  , str (s)
{}

/// Alternate constructor for a string node
inline node::node (const char* s)
  : t (type::string)
  , str (s)
{}

/// Constructor for a numeric node
inline node::node (double d)
  : t (type::numeric)
  , num (d)
{}

/// Alternate constructor for a numeric node
inline node::node (int d)
  : t (type::numeric)
  , num (d)
{}

/// Constructor for a boolean node
inline node::node (bool b)
  : t (type::boolean)
  , logic (b)
{}

/// Constructor from a vector
template <typename T>
node::node (const std::vector<T>& vec)
  : t (type::array)
{
  new (&obj) nodes_array ();
  for (size_t i = 0; i < vec.size (); ++i)
    arr.emplace_back (std::make_unique<node> (vec[i]));
}

/// Constructor from an object
template <class T>
node::node (const T& t, decltype (&T::to_json)*)
  : t (type::object)
{
  new (&obj) nodes_map ();
  t.to_json (*this);
}

/// Begin iterator (const variant)
inline node::const_iterator node::begin () const
{
  if (t == type::object)
    return const_iterator (*this, obj.begin ());
  else if (t == type::array)
    return const_iterator (*this, arr.begin ());
  else
    return const_iterator (*this, false);
}

/// Begin iterator (non-const variant)
inline node::iterator node::begin ()
{
  if (t == type::object)
    return iterator (*this, obj.begin ());
  else if (t == type::array)
    return iterator (*this, arr.begin ());
  else
    return iterator (*this, false);
}

/// End iterator (const variant)
inline node::const_iterator node::end () const
{
  if (t == type::object)
    return const_iterator (*this, obj.end ());
  else if (t == type::array)
    return const_iterator (*this, arr.end ());
  else
    return const_iterator (*this, true);
}

/// End iterator (non-const variant)
inline node::iterator node::end ()
{
  if (t == type::object)
    return iterator (*this, obj.end ());
  else if (t == type::array)
    return iterator (*this, arr.end ());
  else
    return iterator (*this, true);
}

/// Return node value as a string (if possible)
inline node::operator std::string () const
{
  return to_str ();
}

/*
  Return a pointer to node's string value.
  Any assignment to node invalidates the pointer
*/
inline node::operator const char* () const
{
  if (t != type::string)
    mlib::erc (ERR_JSON_INVTYPE, Errors()).raise ();
  return str.c_str ();
}

/// Return numeric value of a node
inline node::operator double () const
{
  return to_num ();
}

/// Return numeric value of a node
inline node::operator float () const
{
  return (float)to_num ();
}

inline node::operator int () const
{
  return (int)to_num ();
}

/// Return boolean value of the node
inline node::operator bool () const
{
  return to_bool ();
}

/// Remove previous node content
inline void node::clear (type t_)
{
  // clear previous content
  switch (t)
  {
  case type::object:
    (&obj)->~nodes_map ();
    break;
  case type::array:
    (&arr)->~nodes_array ();
    break;
  case type::string:
    (&str)->~basic_string ();
    break;
  }

  t = t_;
  // initialize new type
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

/// Return the type of node
inline type node::kind () const
{
  return t;
}

/// Return number of direct descendants
inline int node::size () const
{
  return (t == type::object)  ? (int)obj.size ()
         : (t == type::array) ? (int)arr.size ()
         : (t != type::null)  ? 1
                              : 0;
}

/// inequality operator
inline bool node::operator!= (const node& other) const
{
  return !operator== (other);
}

/*!
  Return value of an object node element.

  If element doesn't exist it throws an ERR_JSON_MISSING exception.
*/
inline node& node::at (const std::string& name)
{
  return const_cast<node&> (static_cast<const node&> (*this).at (name));
}

/*!
  Return value of an object node element.

  If element doesn't exist it throws an ERR_JSON_MISSING exception.
*/
inline node& node::at (size_t index)
{
  return const_cast<node&> (static_cast<const node&> (*this).at (index));
}

/// \cond not_documented
struct omanip
{
  omanip (void (*fn)(std::ios_base&, int), int val) : pfun(fn), arg(val) {}
  void (* pfun)(std::ios_base&, int);
  int arg;

  friend std::ostream& operator << (std::ostream& strm, const omanip& manip) {
    (*manip.pfun) (strm, manip.arg);
    return strm;
  }
};

// manipulators
void indenter (std::ios_base& os, int spaces);

inline omanip spaces (int nspc)
{
  return omanip (&indenter, nspc);
}

inline std::ostream& indent (std::ostream& os)
{
  indenter (os, 2);
  return os;
};
inline std::ostream& tabs (std::ostream& os)
{
  indenter (os, 0);
  return os;
};

std::ostream& noindent (std::ostream& os);

std::ostream& utf8 (std::ostream& os);

std::ostream& noutf8 (std::ostream& os);

/// \endcond

/// Insertion operator serializes a JSON node to an output stream
std::ostream& operator<< (std::ostream& os, const node& n);

/// Extraction operator retrieves a JSON node from an input stream
std::istream& operator>> (std::istream& is, node& n);

/// Assign array value to a node
template <typename T>
void to_json (node& n, const std::vector<T>& vec)
{
  n.clear (type::array);
  for (int i = 0; i < vec.size (); ++i)
    n[i] = vec[i];
}

} // namespace json
