/*!
  \file jbridge.h Definition of JSONBridge class
  
  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#include "httpd.h"
#include "json.h"

#include <list>
#include <typeindex>

namespace mlib {

//Errors
#define ERR_JSON_NOTFOUND   -10   ///< Entry not found
#define ERR_JSON_DICSTRUC   -11   ///< Bad dictionary structure

class JSONBridge;
typedef void (*post_action)(JSONBridge&);
typedef int (*post_fun)(const std::string& uri, JSONBridge& ui);

/// JSON objects support
class JSONBridge {
public:
  /// Dictionary entry types
  enum jb_type {
    JT_UNKNOWN,
    JT_SHORT,     ///< short int
    JT_USHORT,    ///< unsigned short int
    JT_INT,       ///< int
    JT_UINT,      ///< unsigned int
    JT_LONG,      ///< long
    JT_ULONG,     ///< unsigned long
    JT_FLT,       ///< float
    JT_DBL,       ///< double
    JT_PSTR,      ///< char*
    JT_CSTR,      ///< char[]
    JT_STR,       ///< std::string 
    JT_BOOL,      ///< bool
    JT_OBJECT,    ///< composite object
    JT_POSTFUN,   ///< POST function call
  };

  class entry;
  typedef std::list<entry> dictionary;
  typedef dictionary::iterator dict_ptr;
  typedef dictionary::const_iterator dict_cptr;

  /// JSON data dictionary entry
  class entry {
  public:
    entry (const std::string& n, void* a, jb_type t, size_t s, size_t c=1)
      : name (n), addr (a), type (t), cnt (c), sz (s) {}

    template <typename T, size_t C>
    void add_var (T (&var)[C], const std::string& name)
    {
      using U = typename std::remove_extent<T>::type;
      constexpr size_t sz = sizeof(U)*(std::extent_v<T>? std::extent_v<T> : 1);
      constexpr jb_type t =
        std::is_same_v<U, short> ? JT_SHORT :
        std::is_same_v<U, unsigned short> ? JT_USHORT :
        std::is_same_v<U, int> ? JT_INT :
        std::is_same_v<U, unsigned int> ? JT_UINT :
        std::is_same_v<U, long> ? JT_LONG :
        std::is_same_v<U, unsigned long> ? JT_ULONG :
        std::is_same_v<U, float> ? JT_FLT :
        std::is_same_v<U, double> ? JT_DBL :
        std::is_same_v<U, bool> ? JT_BOOL :
        std::is_same_v<U, char> ? JT_CSTR :
        std::is_same_v<U, std::string> ? JT_STR :
        std::is_same_v<U, char*> ? JT_PSTR :
        std::is_same_v<U, const char*> ? JT_PSTR : JT_UNKNOWN;
      static_assert (t != JT_UNKNOWN, "Invalid array type");
      if (t == JT_CSTR && sz == 1)
        children.emplace_back (name, var, t, C); // char var[C]
      else
        children.emplace_back (name, var, t, sz, C);
    }

    template <typename T>
    std::enable_if_t<std::is_same_v<T, std::string>
                  || std::is_same_v<T, short>
                  || std::is_same_v<T, unsigned short>
                  || std::is_same_v<T, int>
                  || std::is_same_v<T, unsigned int>
                  || std::is_same_v<T, long>
                  || std::is_same_v<T, unsigned long>
                  || std::is_same_v<T, float>
                  || std::is_same_v<T, double>
                  || std::is_same_v<T, bool>> add_var (T& var, const std::string& name)
    {
      constexpr jb_type t =
        std::is_same_v<T, short> ? JT_SHORT :
        std::is_same_v<T, unsigned short> ? JT_USHORT :
        std::is_same_v<T, int> ? JT_INT :
        std::is_same_v<T, unsigned int> ? JT_UINT :
        std::is_same_v<T, long> ? JT_LONG :
        std::is_same_v<T, unsigned long> ? JT_ULONG :
        std::is_same_v<T, float> ? JT_FLT :
        std::is_same_v<T, double> ? JT_DBL :
        std::is_same_v<T, bool> ? JT_BOOL :
        std::is_same_v<T, char> ? JT_CSTR :
        std::is_same_v<T, std::string> ? JT_STR :
        std::is_same_v<T, char*> ? JT_PSTR :
        std::is_same_v<T, const char*> ? JT_PSTR : JT_UNKNOWN;
      static_assert (t != JT_UNKNOWN, "Invalid array type");
      children.emplace_back (name, &var, t, sizeof (T));
    }

    entry& add_object (const std::string& name)
    {
      children.emplace_back (name, nullptr, JT_OBJECT, 0, 1);
      return *(--children.end ());
    }

  private:
    std::string name;     ///< external name of variable
    void* addr;           ///< memory address
    jb_type type;         ///< data type
    size_t sz;
    size_t cnt;           ///< number of elements
    dictionary children;
    friend class JSONBridge;
  };

  JSONBridge (const char *path);
  ~JSONBridge ();

  void attach_to (httpd& server);
  void lock ();
  void unlock ();
  const std::string& path () const;
  void set_action (post_action pfn);
  http_connection* client ();

  bool parse_urlencoded () const;
  bool parse_jsonencoded () const;

  /// add array
  template <typename T, size_t C>
  void add_var (T (&var)[C], const std::string& name)
  {
    using U = typename std::remove_extent<T>::type;
    constexpr size_t sz = sizeof (U) * (std::extent_v<T> ? std::extent_v<T> : 1);
    constexpr jb_type t =
      std::is_same_v<U, short> ? JT_SHORT :
      std::is_same_v<U, unsigned short> ? JT_USHORT :
      std::is_same_v<U, int> ? JT_INT :
      std::is_same_v<U, unsigned int> ? JT_UINT :
      std::is_same_v<U, long> ? JT_LONG :
      std::is_same_v<U, unsigned long> ? JT_ULONG :
      std::is_same_v<U, float> ? JT_FLT :
      std::is_same_v<U, double> ? JT_DBL :
      std::is_same_v<U, bool> ? JT_BOOL :
      std::is_same_v<U, char> ? JT_CSTR :
      std::is_same_v<U, char*> ? JT_PSTR :
      std::is_same_v<U, const char*> ? JT_PSTR : JT_UNKNOWN;
    static_assert (t != JT_UNKNOWN, "Invalid array type");
    if (t == JT_CSTR && sz == 1)
      dict_.emplace_back (name, var, t, C, 1); // char var[C]
    else
      dict_.emplace_back (name, var, t, sz, C);
  }

  /// add variable of one of basic types
  template <typename T>
  std::enable_if_t<std::is_same_v<T, std::string>
                || std::is_same_v<T, short>
                || std::is_same_v<T, unsigned short>
                || std::is_same_v<T, int>
                || std::is_same_v<T, unsigned int>
                || std::is_same_v<T, long>
                || std::is_same_v<T, unsigned long>
                || std::is_same_v<T, float>
                || std::is_same_v<T, double>
                || std::is_same_v<T, bool>> add_var (T& var, const std::string& name)
  {
    constexpr jb_type t =
      std::is_same_v<T, short> ? JT_SHORT :
      std::is_same_v<T, unsigned short> ? JT_USHORT :
      std::is_same_v<T, int> ? JT_INT :
      std::is_same_v<T, unsigned int> ? JT_UINT :
      std::is_same_v<T, long> ? JT_LONG :
      std::is_same_v<T, unsigned long> ? JT_ULONG :
      std::is_same_v<T, float> ? JT_FLT :
      std::is_same_v<T, double> ? JT_DBL :
      std::is_same_v<T, bool> ? JT_BOOL :
      std::is_same_v<T, char> ? JT_CSTR :
      std::is_same_v<T, std::string> ? JT_STR :
      std::is_same_v<T, char*> ? JT_PSTR :
      std::is_same_v<T, const char*> ? JT_PSTR : JT_UNKNOWN;
    static_assert (t != JT_UNKNOWN, "Invalid array type");
    dict_.emplace_back (name, &var, t, sizeof (T));
  }

  entry& add_object (const std::string& name);
  void add_postfun (const std::string& name, post_fun pfn);

protected:
  erc jsonify (json::node& n, dict_cptr entry);
  void not_found (const char *varname);
  bool find (const std::string& name, dict_cptr& found, size_t* idx = 0) const;
  bool deep_find (const std::string& name, dict_cptr& found, size_t* idx = 0) const;

private:
  erc json_begin (json::node& obj);
  erc json_end (json::node& obj);
  erc serialize_node (json::node& n, dict_cptr v, size_t index=0);
  erc deserialize_node (const json::node& n, dict_cptr v, size_t index = 0) const;
  std::string path_;
  dictionary dict_;
  http_connection *client_;
  criticalsection in_use;
  post_action action;
  std::map<std::string, post_fun> post_handlers;

  static int callback (const char *uri, http_connection& client, JSONBridge *ctx);
  static bool deep_search (const std::string& var, const dictionary& dict, dict_cptr& found);
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Enter the critical section associated with this context
inline
void JSONBridge::lock ()
{
  in_use.enter ();
}

/// Leave the critical section associated with this context
inline
void JSONBridge::unlock ()
{
  in_use.leave ();
}

/// Return the context path 
inline
const std::string& JSONBridge::path () const 
{
  return path_;
}

/// Return currently connected client (if any)
inline
http_connection* JSONBridge::client () 
{
  return client_;
};

inline
void JSONBridge::set_action (post_action pfn)
{
  action = pfn;
}

inline
JSONBridge::entry& JSONBridge::add_object (const std::string& name)
{
  dict_.emplace_back (name, nullptr, JT_OBJECT, 0, 1);
  return *(--dict_.end ());
}

inline 
void JSONBridge::add_postfun (const std::string& name, post_fun pfn)
{
  post_handlers[name] = pfn;
}


} // end namespace mlib

