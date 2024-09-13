/*!
  \file sqlitepp.h C++ wrapper for SQLITE3

  (c) Mircea Neacsu 2017. All rights reserved.
*/
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <cstring>

#include "safe_winsock.h"
#include <sqlite3/sqlite3.h>
#include "errorcode.h"

#ifndef _WIN32
//#include <strings.h>
#endif

#if (defined(_MSVC_LANG) && _MSVC_LANG < 202002L)                                                  \
  || (!defined(_MSVC_LANG) && (__cplusplus < 202002L))
#error "sqlitepp requires c++20"
#endif

#pragma comment(lib, "sqlite3.lib")

namespace mlib {

class Query;

/// Wrapper for database connection handle
class Database
{
public:
  /// Flags for database opening mode
  enum class openflags
  {
    readonly = SQLITE_OPEN_READONLY,                     ///< Read-only access
    readwrite = SQLITE_OPEN_READWRITE,                   ///< Read/write access on existing database
    create          = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, ///< Read/write/create access
    uri =           SQLITE_OPEN_URI,                               ///< filename is interpreted as a URI
    nomutex =       SQLITE_OPEN_NOMUTEX,     ///< database connection opens in multi-threaded mode
    fullmutex     = SQLITE_OPEN_FULLMUTEX, ///< database connection opens in the serialized mode
    sharedcache   = SQLITE_OPEN_SHAREDCACHE, ///< database connection is eligible to use shared cache mode
    privatecache  = SQLITE_OPEN_PRIVATECACHE, ///< database connection does not participate in shared cache mode
    memory        = SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  ///< database opened in memory
    nofollow      = SQLITE_OPEN_NOFOLLOW ///< filename cannot be a symlink
  };

  /// Default constructor
  Database ();

  /// Open a database specified by name
  Database (const std::string& name, openflags flags = openflags::create);

  /// Copy database content
  Database& copy (Database& src);

  /// Check if database is opened or not
  bool connected ()
  {
    return (bool)db;
  };

  /// Return true if database connection is read-only
  bool is_readonly ();

  /// Return handle of database connection
  operator sqlite3* ()
  {
    return handle ();
  }

  /// Return handle of database connection
  sqlite3* handle () const
  {
    return db ? db.get () : nullptr;
  }

  /// Return rowid of last successful insert
  sqlite3_int64 last_rowid ();

  /// Return number of records changed by last query
  sqlite3_int64 changes ();

  /// Return total number of changes since database was opened
  sqlite3_int64 total_changes ();

  /// Open database connection
  erc open (const std::string& name, openflags flags = openflags::create);

  /// Close database connection
  erc close ();

  /// Execute multiple SQL sentences
  erc exec (const std::string& sql);

  /// Return a Query object containing a prepared statement with the given SQL text
  checked<Query> make_query (const std::string& sql);

  /// Return a Query object containing the first prepared statement of the given SQL text
  checked<Query> make_query_multiple (std::string& sql);

  /// Return filename of a database connection
  std::string filename (const std::string& schema = "main") const;

  /// Return schema name for a database connection
  std::string schema (int n) const;

  /// Return extended result code
  int extended_error ();

  /// Flush
  erc flush ();

  /// Set error facility for all SQLITEPP errors
  inline static void Errors (mlib::errfac& fac)
  {
    sqlite_errors = &fac;
  }

  /// Return error facility used by SQLITEPP
  inline static const errfac& Errors ()
  {
    return *sqlite_errors;
  }

private:
  std::shared_ptr<sqlite3> db;
  friend class Query;

  static errfac* sqlite_errors;
};

/// Wrapper for SQL prepared sentences
class Query
{
  friend class Database;

public:
  /// Default constructor
  Query ();

  /// Build a prepared statement
  Query (Database& db, const std::string& sql = std::string ());

  /// Copy constructor
  Query (const Query& other);

  /// Move constructor
  Query (Query&& other);

  /// Move assignment operator
  Query& operator= (Query&& rhs);

  /// Principal assignment operator
  Query& operator= (const Query& rhs);

  ~Query ();

  /// Return the underlining statement handle
  operator sqlite3_stmt* ();

  /// Assign SQL text to a query
  Query& operator= (const std::string& str);

  /// Assign SQL text to a query
  Query& sql (const std::string& str);

  /// Retrieve SQL text
  std::string sql () const;

  /// Retrieve SQL text
  operator std::string () const;

  /// Evaluate the statement
  erc step ();

  Query& bind (int par, const std::string& val);
  Query& bind (const std::string& parname, const std::string& val);
  Query& bind (int par, int val);
  Query& bind (const std::string& parname, int val);
  Query& bind (int par, double val);
  Query& bind (const std::string& parname, double val);
  Query& bind (int par, sqlite3_int64 val);
  Query& bind (const std::string& parname, sqlite3_int64 val);
  Query& bind (int par, void* val, int len);
  Query& bind (const std::string& parname, void* val, int len);
#ifdef _WIN32
  Query& bind (int par, const SYSTEMTIME& st);
  Query& bind (const std::string& parname, const SYSTEMTIME& st);
#endif

  Query& clear_bindings ();

  int column_int (int nc) const;
  int column_int (const std::string& colname) const;

  std::string column_str (int nc) const;
  std::string column_str (const std::string& colname) const;

  const char* column_text (int nc) const;
  const char* column_text (const std::string& colname) const;

  double column_double (int nc) const;
  double column_double (const std::string& colname) const;

  sqlite3_int64 column_int64 (int nc) const;
  sqlite3_int64 column_int64 (const std::string& name) const;

  const void* column_blob (int nc) const;
  const void* column_blob (const std::string& name) const;

#ifdef _WIN32
  SYSTEMTIME column_time (int nc) const;
  SYSTEMTIME column_time (const std::string& name) const;
#endif

  int column_type (int nc) const;
  int column_type (const std::string& colname) const;

  int column_size (int nc) const;
  int column_size (const std::string& colname) const;

#ifndef SQLITE_OMIT_DECLTYPE
  std::string decl_type (int nc) const;
  std::string decl_type (const std::string& colname) const;
#endif

#ifdef SQLITE_ENABLE_COLUMN_METADATA
  std::string table_name (int nc) const;
  std::string table_name (const std::string& colname) const;

  std::string database_name (int nc) const;
  std::string database_name (const std::string& colname) const;
#endif

  std::string column_name (int nc) const;

  /// Return number of columns in the result set
  int columns ();

  /// Reset statement to initial state
  erc reset ();

  ///  Finalizes the statement and removes the database connection.
  void clear ();

private:
  void map_columns () const;
  int find_col (const std::string& colname) const;
  erc check_errors (int rc);

  sqlite3_stmt* stmt;
  Database dbase;

  /*! SQLITE is case insensitive in column names. Make column_xxx functions also
  case insensitive */
  struct iless
  {
    bool operator() (const std::string& left, const std::string& right) const;
  };

  std::map<std::string, int, Query::iless> mutable index;
  bool mutable col_mapped;
};

/*==================== INLINE FUNCTIONS ===========================*/
inline sqlite3_int64 Database::last_rowid ()
{
  assert (db);
  return sqlite3_last_insert_rowid (db.get ());
}

inline sqlite3_int64 Database::changes ()
{
  assert (db);
  return sqlite3_changes64 (db.get ());
}

inline sqlite3_int64 mlib::Database::total_changes ()
{
  assert (db);
  return sqlite3_total_changes64 (db.get ());
}

inline Query::Query ()
  : stmt (0)
  , col_mapped (false)
{}

inline Query::operator sqlite3_stmt* ()
{
  return stmt;
}

inline int Query::columns ()
{
  return sqlite3_column_count (stmt);
}

/*!
  The object must have the database reference set before calling this function.
  If a previous statement was attached to the object, it is finalized before the
  new statement is prepared.
*/
inline Query& Query::operator= (const std::string& str)
{
  return sql (str);
}

inline bool Query::iless::operator() (const std::string& left, const std::string& right) const
{
#ifdef _WIN32
  return _strcmpi (left.c_str (), right.c_str ()) < 0;
#else
  return strcasecmp (left.c_str (), right.c_str ()) < 0;
#endif
}

/*!
  This is just syntactic sugar over Query::sql() function
*/
inline Query::operator std::string () const
{
  return sql ();
}

inline Database::openflags operator| (const Database::openflags& f1, const Database::openflags& f2)
{
  return static_cast<Database::openflags> (static_cast<int> (f1) | static_cast<int> (f2));
}

}; // namespace mlib
