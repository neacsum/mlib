/*!
  \file jbridge.h Definition of JSONBridge class
  
  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#include "httpd.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

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
typedef struct jsonvar_t {
  char *name;           ///< external name of variable
  void *addr;           ///< memory address
  js_type type;         ///< data type (one of JT_... values)
  unsigned short sz;    ///< element size (used only for strings)
  unsigned short cnt;   ///< number of elements
} JSONVAR;


/// Mark the beginning of JSON dictionary
#define JSD_STARTDIC JSONVAR json_dict[] ={

/// Mark the end of JSON dictionary
#define JSD_ENDDIC {0, 0, JT_INT, 0, 0} }

/*!
  Generates an entry in JSON dictionary for a variable that has the same
  external name (name used in the JSON response) as the variable name.
  \param V  Name of variable
  \param T  Type of variable (one of JT_... values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR and JT_PSTR types)
*/
#define JSD(V, T, C, S) {#V, &##V, T, S, C}


/*!
  Generates an entry in JSON dictionary for a variable that has a different
  external name (name used in the JSON response) than the variable name.
  \param V  variable name
  \param N  External (JSON) name of the variable
  \param T  Type of variable (one of JT_... values)
  \param C  Number of elements (for arrays)
  \param S  Element size (only for JT_STR and JT_PSTR types)
*/
#define JSDN(V, N, T, C, S) {N, &##V, T, S, C}

///Generate entry for a composite object
#define JSD_OBJECT(N) {N, 0, JT_OBJECT, 0, 1}

///Generate entry for end of composite object
#define JSD_ENDOBJ {"", 0, JT_ENDOBJ, 0, 1}

///Generate entry for POST function call
#define JSDN_POSTFUNC(F, N) {N, F, JT_POSTFUN, 0, 1}

/// JSON objects support
class JSONBridge {
public:
  JSONBridge (const char *path);
  ~JSONBridge ();

  void attach_to (httpd& server);
  void lock ();
  void unlock ();
  const char *path ();
  bool set_var (const char *name, void *addr, unsigned short count = 1, unsigned short sz = 0);
  virtual void post_parse (http_connection& client);

protected:
  bool jsonify (const JSONVAR*& entry);
  void bprintf (const char *fmt, ...);
  void not_found (const char *varname);
  bool strquote (const char *str);
  JSONVAR *find (const char *name, int *idx);
  http_connection *client;

private:
  bool json_begin (http_connection& client);
  void json_end (http_connection& client);
  bool parse_urlencoded (http_connection& client);

  const char *path_;
  char *buffer;
  char *bufptr;
  criticalsection in_use;
  static int callback (const char *uri, http_connection& client, JSONBridge *ctx);
};


/*==================== INLINE FUNCTIONS ===========================*/

/// Enter the critical section associated with this context
inline
void JSONBridge::lock (){ in_use.enter (); }

/// Leave the critical section associated with this context
inline
void JSONBridge::unlock (){ in_use.leave (); }

/// Return the context path 
inline
const char* JSONBridge::path () { return path_; }

#ifdef MLIBSPACE
}
#endif

