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
#include <memory>

#include <Winsock2.h>
#include <sqlite3/sqlite3.h>
#include "errorcode.h"

#if (defined(_MSVC_LANG) && _MSVC_LANG < 202002L)                             \
 || (!defined(_MSVC_LANG) && (__cplusplus < 202002L))
#error "sqlitepp requires c++20"
#endif

#pragma comment (lib, "sqlite3.lib")

namespace mlib {

class Query;

///Wrapper for database connection handle
class Database
{
public:

  ///Flags for database opening mode
  enum class openflags {
    readonly      = SQLITE_OPEN_READONLY,     ///< Read-only access
    readwrite     = SQLITE_OPEN_READWRITE,    ///< Read/write access on existing database
    create        = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, ///<Read/write/create access
    uri           = SQLITE_OPEN_URI,          ///<filename is interpreted as a URI
    nomutex       = SQLITE_OPEN_NOMUTEX,      ///<database connection opens in multi-threaded mode
    fullmutex     = SQLITE_OPEN_FULLMUTEX,    ///<database connection opens in the serialized mode
    sharedcache   = SQLITE_OPEN_SHAREDCACHE,  ///<database connection is eligible to use shared cache mode
    privatecache  = SQLITE_OPEN_PRIVATECACHE, ///<database connection does not participate in shared cache mode
    memory        = SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  ///<database opened in memory
    nofollow      = SQLITE_OPEN_NOFOLLOW      ///<filename cannot be a symlink
  };

  ///Default constructor
  Database ();

  ///Open a database specified by name
  Database (const std::string& name, openflags flags=openflags::create);

  ///Destructor
  ~Database ();

  /// Assignment operator 
  Database& operator =(const Database& rhs);

  ///Check if database is opened or not
  bool connected () {return (db != 0);};

  /// Return true if database connection is read-only
  bool is_readonly ();

  ///Return handle of database connection
  operator sqlite3* () {return db;};

  ///Return rowid of last successful insert
  __int64 last_rowid ();

  ///Return number of records changed by last query 
  __int64 changes ();

  ///Return total number of changes since database was opened
  __int64 total_changes ();

  ///Open database connection
  erc open (const std::string& name, openflags flags=openflags::create);

  ///Close database connection
  erc close ();

  ///Execute multiple SQL sentences
  erc exec (const std::string& sql);

  ///Return a Query object containing a prepared statement with the given SQL text
  checked<Query> make_query (const std::string& sql);

  /// Return a Query object containing the first prepared statement of the given SQL text
  checked<Query> make_query_multiple (std::string &sql);

  /// Return filename of a database connection
  std::string filename (const std::string& schema = "main") const;

  ///Return schema name for a database connection
  std::string schema (int n) const;
  
  /// Return extended result code
  int extended_error ();

  /// Flush 
  erc flush();

private:
  /// prohibit default function 
  Database (const Database& t) = delete;

  sqlite3 *db;
  std::unique_ptr<errfac> errors;
  friend class Query;
};

///Wrapper for SQL prepared sentences
class Query
{
  friend class Database;
public:

  ///Default constructor
  Query ();

  ///Statement attached to a database but without any SQL
  Query (Database& db);

  ///Build a prepared statement from SQL text
  Query (Database& db, const std::string& sql);

  /// Move constructor
  Query(Query&& other);

  /// Move assignment operator
  Query& operator =(Query&& rhs);

  ~Query ();

  ///Return the underlining statement handle
  operator sqlite3_stmt* ();

  ///Assign SQL text to a query
  Query& operator =(const std::string& sql);

  ///Retrieve SQL text
  std::string sql() const;

  ///Retrieve SQL text
  operator std::string () const;

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

  int           column_int (int nc) const;
  int           column_int (const std::string& colname) const;

  std::string   column_str (int nc) const;
  std::string   column_str (const std::string& colname) const;

  const char*   column_text (int nc) const;
  const char*   column_text (const std::string& colname) const;

  double        column_double (int nc) const;
  double        column_double (const std::string& colname) const;

  __int64       column_int64 (int nc) const;
  __int64       column_int64 (const std::string& name) const;

  const void*   column_blob (int nc) const;
  const void*   column_blob (const std::string& name) const;

  SYSTEMTIME    column_time (int nc) const;
  SYSTEMTIME    column_time (const std::string& name) const;

  int           column_type (int nc) const;
  int           column_type (const std::string& colname) const;

  int           column_size (int nc) const;
  int           column_size (const std::string& colname) const;

#ifndef SQLITE_OMIT_DECLTYPE
  std::string   decl_type (int nc) const;
  std::string   decl_type (const std::string& colname) const;
#endif

#ifdef SQLITE_ENABLE_COLUMN_METADATA
  std::string   table_name (int nc) const;
  std::string   table_name (const std::string& colname) const;

  std::string   database_name (int nc) const;
  std::string   database_name (const std::string& colname) const;
#endif

  std::string   column_name(int nc) const;

  /// Return number of columns in the result set
  int           columns ();

  ///Reset statement to initial state
  Query&        reset ();

  void          finalize ();

private:
  /// prohibit default function 
  Query(const Query& t) = delete;

  /// prohibit default function 
  Query& operator= (const Query& rhs) = delete;

  void          map_columns () const;
  int           find_col (const std::string& colname) const;
  erc           check_errors (int rc);

  sqlite3_stmt* stmt;
  Database*     dbase;

  /*! SQLITE is case insensitive in column names. Make column_xxx functions also
  case insensitive */
  struct iless {
    bool operator () (const std::string& left, const std::string& right) const;
  };

  std::map <std::string, int, Query::iless> mutable index;
  bool mutable col_mapped;
};

/*==================== INLINE FUNCTIONS ===========================*/
inline
__int64 Database::last_rowid () 
{
  return sqlite3_last_insert_rowid (db);
}

inline
__int64 Database::changes () 
{
  return sqlite3_changes64 (db);
}

inline __int64 mlib::Database::total_changes ()
{
  return sqlite3_total_changes64 (db);
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

/*!
  This is just syntactic sugar over Query::sql() function
*/
inline
Query::operator std::string () const
{
  return sql ();
}

inline
Database::openflags operator |(const Database::openflags& f1, const Database::openflags& f2)
{
  return static_cast<Database::openflags>(
    static_cast<int>(f1) | static_cast<int>(f2));
}

};
