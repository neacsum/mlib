/*!
  \file jbridge.h Definition of JSONBridge class
  
  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#include "httpd.h"
#include "json.h"

namespace mlib {

//Errors
#define ERR_JSON_NOTFOUND   -10   ///< Entry not found
#define ERR_JSON_DICSTRUC   -11   ///< Bad dictionary structure
/// JSON dictionary entry types
enum js_type {
  JT_SHORT,     ///< short int
  JT_USHORT,    ///< unsigned short int
  JT_INT,       ///< int
  JT_UINT,      ///< unsigned int
  JT_LONG,      ///< long
  JT_ULONG,     ///< unsigned long
  JT_FLT,       ///< float
  JT_DBL,       ///< double
  JT_PSTR,      ///< char*
  JT_STR,       ///< char
  JT_BOOL,      ///< bool
  JT_OBJECT,    ///< start of a composite object
  JT_ENDOBJ,    ///< end of composite object
  JT_POSTFUN,   ///< POST function call
};

/// JSON data dictionary entry
struct jb_var {
  jb_var (const std::string& n, void* a, js_type t, unsigned short c=1, unsigned short s=0)
    : name{ n }, addr{ a }, type{ t }, cnt{ c }, sz{ s } {};

  std::string name;     ///< external name of variable
  void *addr;           ///< memory address
  js_type type;         ///< data type (one of JT_... values)
  unsigned short sz;    ///< element size (used only for strings)
  unsigned short cnt;   ///< number of elements
};


typedef std::vector<jb_var> jb_dictionary;

/// Mark the beginning of JSON dictionary
#define JSD_STARTDIC(dict) jb_dictionary dict ={

/// Mark the end of JSON dictionary
#define JSD_ENDDIC {"", 0, JT_ENDOBJ, 0, 0} }

/*!
  Generates an entry in JSON dictionary for a variable that has the same
  external name (name used in the JSON response) as the variable name.
  \param V  Name of variable
  \param T  Type of variable (one of JT_... values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR types)
*/
#define JSD(V, T, ...) {#V, &##V, T, __VA_ARGS__}


/*!
  Generates an entry in JSON dictionary for a variable that has a different
  external name (name used in the JSON response) than the variable name.
  \param V  variable name
  \param N  External (JSON) name of the variable
  \param T  Type of variable (one of JT_... values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR and JT_PSTR types)
*/
#define JSDN(V, N, T, ...) {N, &##V, T, __VA_ARGS__}

///Generate entry for a composite object
#define JSD_OBJECT(N) {N, nullptr, JT_OBJECT}

///Generate entry for end of composite object
#define JSD_ENDOBJ {"", nullptr, JT_ENDOBJ}


class JSONBridge;
typedef void (*post_action)(JSONBridge&);

/// JSON objects support
class JSONBridge {
public:
  JSONBridge (const char *path, jb_dictionary& dict);
  ~JSONBridge ();

  void attach_to (httpd& server);
  void lock ();
  void unlock ();
  const std::string& path () const;
  jb_dictionary& dictionary ();
  void set_action (post_action pfn);
  http_connection* client ();

  bool parse_urlencoded () const;
  bool parse_jsonencoded () const;

protected:
  erc jsonify (json::node& n, const jb_var*& entry);
  void not_found (const char *varname);
  const jb_var* find (const std::string& name, int* idx = 0) const;

private:
  erc json_begin (json::node& obj);
  erc json_end (json::node& obj);
  erc serialize_node (json::node& n, const jb_var*& entry, int index=0);
  erc deserialize_node (const json::node& n, const jb_var*& entry, int index = 0) const;

  std::string path_;
  std::vector <jb_var> dict_;
  http_connection *client_;
  criticalsection in_use;
  post_action action;
  static int callback (const char *uri, http_connection& client, JSONBridge *ctx);
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

/// Return data dictionary
inline
jb_dictionary& JSONBridge::dictionary () 
{
  return dict_;
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


} // end namespace mlib

