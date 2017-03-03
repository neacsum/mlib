#pragma once
/*!
  \file ui.h - Definition of ui_context class
  
  (c) Mircea Neacsu 2017. All rights reserved.
*/
#include "httpd.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// JSON variable types
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
};

//JSON data dictionary entry
typedef struct jsonvar_t {
  char *name;           //external name of variable
  void *addr;           //memory address
  js_type type;         //data type (one of JT_... values)
  unsigned short sz;    //element size (used only for strings)
  unsigned short cnt;   //number of elements
} JSONVAR;


/// Mark the beginning of JSON dictionary
#define JSD_START JSONVAR json_dict[] ={

/// Mark the end of JSON dictionary
#define JSD_END {0, 0, JT_INT, 0, 0} }

/*!
  Generates an entry in JSON dictionary for a variable that has the same
  external name (name used in the JSON response) as the variable name.
  \param V  Name of the variable
  \param T  Type of variable (one of js_type values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR type)
*/
#define JSD(V, T, C, S) {#V, &##V, T, S, C}


/*!
  Generates an entry in JSON dictionary for a variable that has a different
  external name (name used in the JSON response) than the variable name.
  \param V  Name of the variable
  \param N  External (JSON) name of the variable
  \param T  Type of variable (one of jst_type values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR type)
*/
#define JSDN(V, N, T, C, S) {N, &##V, T, S, C}


class ui_context {
public:
  ui_context (const char *path);
  ~ui_context ();

  void attach_to (httpd& server);
  void lock ();
  void unlock ();
  const char *path ();

  virtual int jsonify_all (const char *query) = 0;
  virtual void post_parse (const char *query) = 0;

protected:
  bool jsonify (void *var);
  void bprintf (const char *fmt, ...);
  void not_found (const char *varname);
  const JSONVAR *find (const char *name, int *idx);
  http_connection *client;

private:
  void json_begin (http_connection* client);
  void json_end ();
  bool parse_urlencoded (http_connection* client);

  const char *path_;
  char *buffer;
  char *bufptr;
  criticalsection in_use;
  static int callback (const char *uri, http_connection& client, ui_context *ctx);
};


/*==================== INLINE FUNCTIONS ===========================*/

/// Enter the critical section associated with this context
inline
void ui_context::lock (){ in_use.enter (); }

/// Leave the critical section associated with this context
inline
void ui_context::unlock (){ in_use.leave (); }

/// Return the context path 
inline
const char* ui_context::path () { return path_; }

#ifdef MLIBSPACE
}
#endif

