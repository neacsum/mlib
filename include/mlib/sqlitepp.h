/*!
  \file sqlitepp.h C++ wrapper for SQLITE3

  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>
#include <map>

#include <Winsock2.h>
#include "sqlite3.h"
#include "errorcode.h"

namespace mlib {

class Query;

///Wrapper for database connection handle
class Database
{
public:

  ///Flags for database opening mode
  enum openflags {
    readonly      = SQLITE_OPEN_READONLY,   ///< Read-only access
    readwrite     = SQLITE_OPEN_READWRITE,  ///< Read/write access on existing database
    create        = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, ///<Read/write/create access
    uri           = SQLITE_OPEN_URI,        ///<filename is interpreted as a URI
    nomutex       = SQLITE_OPEN_NOMUTEX,    ///<database connection opens in multi-threaded mode
    fullmutex     = SQLITE_OPEN_FULLMUTEX,  ///<database connection opens in the serialized mode
    sharedcache   = SQLITE_OPEN_SHAREDCACHE,///<database connection is eligible to use shared cache mode
    privatecache  = SQLITE_OPEN_PRIVATECACHE///<database connection does not participate in shared cache mode
  };

  ///Default constructor
  Database ();

  ///Open a database specified by name
  Database (const std::string& name, int flags=create);

  ///Destructor
  ~Database ();

  ///Check if database is opened or not
  bool connected () {return (db != 0);};

  /// Return true if database connection is read-only
  bool is_readonly ();

  ///Return handle of database connection
  operator sqlite3* () {return db;};

  ///Return rowid of last successful insert
  __int64 last_rowid ();

  ///Return number of records changed by last query 
  int changes ();

  ///Open database connection
  erc open (const std::string& name, int flags=create);

  ///Close database connection
  void close ();

  ///Execute multiple SQL sentences
  erc exec (const std::string& sql);

  ///Change or set the prepared statement of a Query object
  erc make_query (Query& q, const std::string& sql);

  ///Return filename of a database connection
  std::string filename (const std::string& conn = "main");

  /// Return extended result code
  int extended_error ();

private:
  /// prohibit default function 
  Database (const Database& t) = delete;
  /// prohibit default function 
  Database& operator =(const Database& rhs) = delete;

  sqlite3 *db;
  bool handle_owned;
};

///Wrapper for SQL prepared sentences
class Query
{
  friend class Database;
public:

  ///Default constructor
  Query ();

  ///Object without SQL statement
  Query (Database& db);

  ///Build a prepared statement from SQL text
  Query (Database& db, const std::string& sql);

  ~Query ();

  ///Return the underlining statement handle
  operator sqlite3_stmt* ();

  ///Assign SQL text to a query
  Query& operator =(const std::string& sql);

  ///retrieve SQL text
  std::string sql() const;

  ///Evaluate the statement
  erc           step ();

  Query&        bind (int par, const std::string& val);
  Query&        bind (const std::string& parname, const std::string& val);
  Query&        bind (int par, int val);
  Query&        bind (const std::string& parname, int val);
  Query&        bind (int par, double val);
  Query&        bind (const std::string& parname, double val);
  Query&        bind (int par, __int64 val);
  Query&        bind (const std::string& parname, __int64 val);
  Query&        bind (int par, void *val, int len);
  Query&        bind (const std::string& parname, void* val, int len);
  Query&        bind (int par, const SYSTEMTIME& st);
  Query&        bind (const std::string& parname, const SYSTEMTIME& st);

  Query&        clear_bindings ();

  int           column_int (int nc);
  std::string   column_str (int nc);
  const char*   column_text(int nc);
  double        column_double (int nc);
  __int64       column_int64 (int nc);
  const void*   column_blob (int nc);
  SYSTEMTIME    column_time (int nc);

  int           column_int (const std::string& colname);
  std::string   column_str (const std::string& colname);
  const char*   column_text(const std::string& colname);
  double        column_double (const std::string& colname);
  __int64       column_int64 (const std::string& name);
  const void*   column_blob (const std::string& name);
  SYSTEMTIME    column_time (const std::string& name);

  int           column_type (int nc);
  int           column_type (const std::string& colname);

  int           column_size (int nc);
  int           column_size (const std::string& colname);

  /// Return number of columns in the result set
  int           columns ();

  ///Reset statement to initial state
  Query&        reset ();

  void          finalize ();

private:
  /// prohibit default function 
  Query (const Query& t);
  /// prohibit default function 
  Query& operator= (const Query& rhs);

  void          map_columns ();
  int           find_col (const std::string& colname);
  erc           check_errors (int rc);

  sqlite3_stmt* stmt;
  sqlite3*      dbase;    

  /*! SQLITE is case insensitive in column names. Make column_xxx functions also
  case insensitive */
  struct iless {
    bool operator () (const std::string& left, const std::string& right) const;
  };

  std::map <std::string, int, Query::iless> index;
  bool col_mapped;
};

extern errfac *sqlite_errors;

/*==================== INLINE FUNCTIONS ===========================*/
inline
__int64 Database::last_rowid () 
{
  return sqlite3_last_insert_rowid(db);
}

inline
int Database::changes () 
{
  return sqlite3_changes(db);
}

inline
Query::Query() : stmt (0), dbase(0), col_mapped (false) 
{
}

inline
Query::operator sqlite3_stmt* () 
{
  return stmt;
}

inline
int Query::columns () 
{
  return sqlite3_column_count (stmt);
}

inline
bool Query::iless::operator () (const std::string& left, const std::string& right) const 
{
  return _stricmp(left.c_str(), right.c_str())<0;
}


};
