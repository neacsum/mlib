/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file jbridge.h Definition of mlib::http::jbridge class
#pragma once

#include "http.h"
#include "json.h"

#include <list>
#include <typeindex>

namespace mlib::http {

// Errors
#define HTTP_JSON_NOTFOUND -10 ///< Entry not found
#define HTTP_JSON_DICSTRUC -11 ///< Bad dictionary structure

class jbridge;

/// User defined variable handler function
typedef std::function<int (jbridge& bridge, void* var)> var_handler;

/// JSON objects support
class jbridge
{
public:
  /// Dictionary entry types
  enum jb_type
  {
    JT_UNKNOWN,
    JT_SHORT,   ///< short int
    JT_USHORT,  ///< unsigned short int
    JT_INT,     ///< int
    JT_UINT,    ///< unsigned int
    JT_LONG,    ///< long
    JT_ULONG,   ///< unsigned long
    JT_FLT,     ///< float
    JT_DBL,     ///< double
    JT_PSTR,    ///< char*
    JT_CSTR,    ///< char[]
    JT_STR,     ///< std::string
    JT_BOOL,    ///< bool
    JT_OBJECT,  ///< composite object
    JT_POSTFUN, ///< POST function call
  };

  class entry;

  /// Data dictionary structure
  typedef std::list<entry> dictionary;

  /// Dictionary iterator @{
  typedef dictionary::iterator dict_ptr;
  typedef dictionary::const_iterator dict_cptr;
  /// @}

  /// JSON data dictionary entry
  class entry
  {
  public:
    entry (const std::string& n, void* a, jb_type t, size_t s, size_t c)
      : name (n)
      , addr (a)
      , type (t)
      , cnt (c)
      , sz (s)
    {}

    /*!
      Add an array to data dictionary
      \tparam T data type
      \tparam C number of elements
      \param name external name
    */
    template <typename T, size_t C>
    void add_var (T (&var)[C], const std::string& name)
    {
      using U = typename std::remove_extent<T>::type;
      constexpr size_t sz = sizeof (U) * (std::extent_v<T> ? std::extent_v<T> : 1);
      constexpr jb_type t = std::is_same_v<U, short>            ? JT_SHORT
                            : std::is_same_v<U, unsigned short> ? JT_USHORT
                            : std::is_same_v<U, int>            ? JT_INT
                            : std::is_same_v<U, unsigned int>   ? JT_UINT
                            : std::is_same_v<U, long>           ? JT_LONG
                            : std::is_same_v<U, unsigned long>  ? JT_ULONG
                            : std::is_same_v<U, float>          ? JT_FLT
                            : std::is_same_v<U, double>         ? JT_DBL
                            : std::is_same_v<U, bool>           ? JT_BOOL
                            : std::is_same_v<U, char>           ? JT_CSTR
                            : std::is_same_v<U, std::string>    ? JT_STR
                            : std::is_same_v<U, char*>          ? JT_PSTR
                            : std::is_same_v<U, const char*>    ? JT_PSTR
                                                                : JT_UNKNOWN;
      static_assert (t != JT_UNKNOWN, "Invalid array type");
      if (t == JT_CSTR && sz == 1)
        children.emplace_back (name, var, t, C, 1); // char var[C]
      else
        children.emplace_back (name, var, t, sz, C);
    }

    /*!
      Add a variable to data dictionary
      \tparam T data type
      \param var variable to add
      \param name external name
    */
    template <typename T>
    std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, short>
                     || std::is_same_v<T, unsigned short> || std::is_same_v<T, int>
                     || std::is_same_v<T, unsigned int> || std::is_same_v<T, long>
                     || std::is_same_v<T, unsigned long> || std::is_same_v<T, float>
                     || std::is_same_v<T, double> || std::is_same_v<T, bool>>
    add_var (T& var, const std::string& name)
    {
      constexpr jb_type t = std::is_same_v<T, short>            ? JT_SHORT
                            : std::is_same_v<T, unsigned short> ? JT_USHORT
                            : std::is_same_v<T, int>            ? JT_INT
                            : std::is_same_v<T, unsigned int>   ? JT_UINT
                            : std::is_same_v<T, long>           ? JT_LONG
                            : std::is_same_v<T, unsigned long>  ? JT_ULONG
                            : std::is_same_v<T, float>          ? JT_FLT
                            : std::is_same_v<T, double>         ? JT_DBL
                            : std::is_same_v<T, bool>           ? JT_BOOL
                            : std::is_same_v<T, char>           ? JT_CSTR
                            : std::is_same_v<T, std::string>    ? JT_STR
                            : std::is_same_v<T, char*>          ? JT_PSTR
                            : std::is_same_v<T, const char*>    ? JT_PSTR
                                                                : JT_UNKNOWN;
      static_assert (t != JT_UNKNOWN, "Invalid variable type");
      children.emplace_back (name, &var, t, sizeof (T), 1);
    }

    entry& add_object (const std::string& name)
    {
      children.emplace_back (name, nullptr, JT_OBJECT, 0, 1);
      return *(--children.end ());
    }

  private:
    std::string name; ///< external name of variable
    void* addr;       ///< memory address
    jb_type type;     ///< data type
    size_t sz;
    size_t cnt; ///< number of elements
    dictionary children;
    friend class jbridge;
  };

  jbridge (const char* path);
  ~jbridge ();

  void attach_to (server& server);
  void lock ();
  void unlock ();
  const std::string& path () const;
  void redirect_to (const std::string& uri);
  void set_post_action (uri_handler pfn);
  connection& client ();

  bool parse_urlencoded () const;
  bool parse_jsonencoded () const;

  /// add array
  template <typename T, size_t C>
  void add_var (T (&var)[C], const std::string& name)
  {
    using U = typename std::remove_extent<T>::type;
    constexpr size_t sz = sizeof (U) * (std::extent_v<T> ? std::extent_v<T> : 1);
    constexpr jb_type t = std::is_same_v<U, short>            ? JT_SHORT
                          : std::is_same_v<U, unsigned short> ? JT_USHORT
                          : std::is_same_v<U, int>            ? JT_INT
                          : std::is_same_v<U, unsigned int>   ? JT_UINT
                          : std::is_same_v<U, long>           ? JT_LONG
                          : std::is_same_v<U, unsigned long>  ? JT_ULONG
                          : std::is_same_v<U, float>          ? JT_FLT
                          : std::is_same_v<U, double>         ? JT_DBL
                          : std::is_same_v<U, bool>           ? JT_BOOL
                          : std::is_same_v<U, char>           ? JT_CSTR
                          : std::is_same_v<U, char*>          ? JT_PSTR
                          : std::is_same_v<U, const char*>    ? JT_PSTR
                                                              : JT_UNKNOWN;
    static_assert (t != JT_UNKNOWN, "Invalid array type");
    if (t == JT_CSTR && sz == 1)
      dict_.emplace_back (name, var, t, C, 1); // char var[C]
    else
      dict_.emplace_back (name, var, t, sz, C);
  }

  /// add variable of one of basic types
  template <typename T>
  std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, short>
                    || std::is_same_v<T, unsigned short> || std::is_same_v<T, int>
                    || std::is_same_v<T, unsigned int> || std::is_same_v<T, long>
                    || std::is_same_v<T, unsigned long> || std::is_same_v<T, float>
                    || std::is_same_v<T, double> || std::is_same_v<T, bool>>
  add_var (T& var, const std::string& name)
  {
    constexpr jb_type t = std::is_same_v<T, short>            ? JT_SHORT
                          : std::is_same_v<T, unsigned short> ? JT_USHORT
                          : std::is_same_v<T, int>            ? JT_INT
                          : std::is_same_v<T, unsigned int>   ? JT_UINT
                          : std::is_same_v<T, long>           ? JT_LONG
                          : std::is_same_v<T, unsigned long>  ? JT_ULONG
                          : std::is_same_v<T, float>          ? JT_FLT
                          : std::is_same_v<T, double>         ? JT_DBL
                          : std::is_same_v<T, bool>           ? JT_BOOL
                          : std::is_same_v<T, char>           ? JT_CSTR
                          : std::is_same_v<T, std::string>    ? JT_STR
                          : std::is_same_v<T, char*>          ? JT_PSTR
                          : std::is_same_v<T, const char*>    ? JT_PSTR
                                                              : JT_UNKNOWN;
    static_assert (t != JT_UNKNOWN, "Invalid variable type");
    dict_.emplace_back (name, &var, t, sizeof (T), 1);
  }

  entry& add_object (const std::string& name);
  void add_postfun (const std::string& name, uri_handler pfn);

protected:

  erc jsonify (json::node& n, dict_cptr entry);
  void not_found (const char* varname);
  bool find (const std::string& name, dict_cptr& found, size_t* idx = 0) const;
  bool deep_find (const std::string& name, dict_cptr& found, size_t* idx = 0) const;

private:
  void process_request ();
  erc json_begin (json::node& obj);
  erc json_end (json::node& obj);
  erc serialize_node (json::node& n, dict_cptr v, size_t index = 0);
  erc deserialize_node (const json::node& n, dict_cptr v, size_t index = 0) const;
  std::string path_;
  dictionary dict_;
  connection* client_;
  criticalsection in_use;
  uri_handler post_action;
  std::string redirect_uri;
  std::map<std::string, uri_handler> post_handlers;

  static int callback (connection& client, void* ctx);
  static bool deep_search (const std::string& var, const dictionary& dict, dict_cptr& found);
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Enter the critical section associated with this bridge
inline void jbridge::lock ()
{
  in_use.enter ();
}

/// Leave the critical section associated with this bridge
inline void jbridge::unlock ()
{
  in_use.leave ();
}

/// Return the root path where bridge is attached
inline const std::string& jbridge::path () const
{
  return path_;
}

/// Set default redirection target for POST requests
inline void jbridge::redirect_to (const std::string& uri)
{
  redirect_uri = uri;
}

/// Return currently connected client (if any)
inline connection& jbridge::client ()
{
  if (client_)
    return *client_;
  else
    throw std::logic_error ("Missing client connection");
};

/*!
  Set the function invoked after successful processing of a POST request.

  This function is invoked before redirecting the client to the `redirect_uri`.
  If the function does not return HTTP_OK, the client is not redirected.
*/
inline void jbridge::set_post_action (uri_handler pfn)
{
  post_action = pfn;
}

/*!
  Add an object to data dictionary
  \param name object's name
  \return reference to newly created dictionary entry
*/
inline jbridge::entry& jbridge::add_object (const std::string& name)
{
  dict_.emplace_back (name, nullptr, JT_OBJECT, 0, 1);
  return *(--dict_.end ());
}

/*!
  Add a handler function invoked in response to a POST request with an additional
  parameter.
  \param qparam   additional query parameter
  \param pfn      handler function

  If the handler function does not return HTTP_OK, the `post_action` function
  is not invoked and client is not redirected to `redirect_uri'
*/
inline void jbridge::add_postfun (const std::string& qparam, uri_handler pfn)
{
  post_handlers[qparam] = pfn;
}

} // end namespace mlib::http
