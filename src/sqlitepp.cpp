/*!
  \file sqlitepp.cpp Implementation of Database and Query classes

  \defgroup sqlite SQLite C++ Wrappers
  \brief Object-oriented wrappers for SQLITE3 functions.

  This is a thin wrapper for SQLITE API. The API is wrapped in two objects,
  Database and Query.

  <h3>Compile-time Options</h3>
  Some of the functions in these wrappers depend on optional support in the
  SQLITE code. Consequently, the amalgamation has to be compiled with matching
  options. This is accomplished by including the `mlib/defs.h` file as a custom
  header.

  The amalgamation is compiled with the flag `/D "SQLITE_CUSTOM_INCLUDE=mlib/defs.h"`
  on the command line. Any desired compile-time options can hence be set in the
  `mlib/defs.h` file.
*/
#include <mlib/mlib.h>
#pragma hdrstop
#include <stdio.h>
#include <assert.h>

using namespace std;

#define STR(X)         #X
#define STRINGERIZE(X) STR (X)

namespace mlib {

static void set_erc_message (erc& err, sqlite3* db);

errfac default_sqlitepp_errors ("SQLITEPP");
errfac* Database::sqlite_errors = &default_sqlitepp_errors;

/*!
\class Database

The Database class is a wrapper around the <a
href="http://sqlite.org/c3ref/sqlite3.html">sqlite3</a> handle returned by sqlite_open_v2 function.
\ingroup sqlite
*/

//-----------------------------------------------------------------------------
/*!
  The default constructor is used to create an object that is not yet connected
  to a database. It has to be connected using the open() function.
*/
Database::Database ()
{}

/*!
  \param  name      database name
  \param  flags     open flags
*/
Database::Database (const std::string& name, openflags flags)
{
  sqlite3* pdb;
  int rc = sqlite3_open_v2 (name.c_str (), &pdb, static_cast<int> (flags), 0);
  db.reset (pdb, sqlite3_close);
  if ((rc) != SQLITE_OK)
  {
    erc err (rc, Database::Errors());
    set_erc_message (err, handle ());
    db.reset ();
    err.raise ();
  }
}

/*!
  Perform a copy operation using the [SQLITE Backup API](https://sqlite.org/backup.html).

  Both databases should be opened.
*/
Database& Database::copy (Database& src)
{
  int rc;
  if (db && src.db)
  {
    auto bkp = sqlite3_backup_init (handle (), "main", src, "main");
    if (!bkp)
    {
      rc = sqlite3_errcode (handle ());
      erc err (rc, Database::Errors());
      set_erc_message (err, handle ());
      err.raise ();
    }
    else
    {
      if ((rc = sqlite3_backup_step (bkp, -1)) != SQLITE_DONE)
      {
        erc err (rc, Database::Errors ());
        set_erc_message (err, handle ());
        err.raise ();
      }
      sqlite3_backup_finish (bkp);
    }
  }
  else
  {
    erc err (SQLITE_MISUSE, *sqlite_errors);
    err.message ("Error " STRINGERIZE (SQLITE_MISUSE) " database is not opened");
    err.raise (); // one db is not opened
  }
  return *this;
}

/*!
  Return an empty string if database is not opened or there is no database
  connection with that name or if the database is a temporary or in-memory
  database. Otherwise returns the filename of the database that was attached
  using an SQL ATTACH statement.
*/
string Database::filename (const string& schema) const
{
  const char* pfn = 0;
  if (db)
    pfn = sqlite3_db_filename (handle (), schema.c_str ());
  return pfn ? pfn : string ();
}

/*!
  Return schema name for a database connection.
  \return "main" if n==0 (the schema name for the main database connection)
  \return "temp" if n==1 (schema name for temporary tables)
  \return schema names for other ATTACHED databases if n>1

  \note this is a wrapper for the [sqlite3_db_name](https://sqlite.org/c3ref/db_name.html)function
*/
std::string Database::schema (int n) const
{
  const char* psch = 0;
  if (db)
    psch = sqlite3_db_name (handle (), n);
  return psch ? psch : string ();
}

/*!
  Returns FALSE only for connected read-write databases.
  If database is not connected or it is read-only, returns TRUE.
*/
bool Database::is_readonly ()
{
  if (db)
    return (sqlite3_db_readonly (handle (), "main") == 1);
  else
    return true;
}

/*!
  If the object is associated with another connection, the previous connection
  is closed and a new one is opened.
  \param name     Database name
  \param flags    Combination of openflags flags
*/
erc Database::open (const string& name, openflags flags)
{
  sqlite3* pdb;
  auto rc = sqlite3_open_v2 (name.c_str (), &pdb, static_cast<int> (flags), 0);
  db.reset (pdb, sqlite3_close);
  if (rc != SQLITE_OK)
  {
    erc err (rc, *sqlite_errors);
    set_erc_message (err, handle ());
    return err;
  }
  else
    return erc::success;
}

erc Database::close ()
{
  if (db)
    db.reset ();
  return erc::success;
}

erc Database::exec (const string& sql)
{
  int rc;
  if ((rc = sqlite3_exec (handle (), sql.c_str (), 0, 0, 0)) != SQLITE_OK)
  {
    auto ret = erc (rc, *sqlite_errors);
    set_erc_message (ret, handle ());
    return ret;
  }
  else
    return erc::success;
}

/*!
  This function compiles only the first statement in the \p sql string.

  The function  returns a `checked<Query>` object that can be tested.
*/
checked<Query> Database::make_query (const std::string& sql)
{
  Query q (*this);
  erc ret;
  int rc;
  if ((rc = sqlite3_prepare_v2 (handle (), sql.c_str (), -1, &q.stmt, 0)) != SQLITE_OK)
  {
    erc err (rc, Database::Errors());
    set_erc_message (err, handle ());
    ret = err;
  }
  return {std::move (q), ret};
}

/*!
  This function compiles only the first statement in the \p sql string.
  It updates \p sql string with what's left after the end of the query.
*/
checked<Query> Database::make_query_multiple (std::string& sql)
{
  erc ret;
  Query q (*this);
  int rc;
  const char* tail;
  if ((rc = sqlite3_prepare_v2 (handle (), sql.c_str (), -1, &q.stmt, &tail)) != SQLITE_OK)
  {
    erc err (rc, Database::Errors ());
    set_erc_message (err, handle ());
    ret = err;
  }
  else
    sql = tail;
  return {std::move (q), ret};
}

int Database::extended_error ()
{
  assert (db);
  return sqlite3_extended_errcode (handle ());
}

erc Database::flush ()
{
  assert (db);
  int rc = sqlite3_db_cacheflush (handle ());
  if (rc != SQLITE_OK)
  {
    erc err (rc, Database::Errors ());
    set_erc_message (err, handle ());
    return err;
  }
  return erc::success;
}

//-----------------------------------------------------------------------------
/*!
  \class Query
  The Query class is a wrapper around a prepared <a
href="http://sqlite.org/c3ref/stmt.html">sqlite3</a> statement.

  The class provides a number of overloaded \c bind functions that can be used to
  bind parameters specified either by name or by number  (first parameter is 1)
  to values of different types.

  All bind functions return a reference to the object allowing them to be syntactically
  chained as in the following example:
\code
  Query  (db, "INSERT INTO table (col1, col2) VALUES (:param1, :param2);")
    .bind (":param1", value1)
    .bind (":param2", value2)
    .step ();
\endcode

  For data extraction, the class provides \c column_xxx functions that return the value
  of a column specified either by name or by index (first column index is 0).

  \ingroup sqlite
*/

/*!
  \param  db      database connection
  \param  sql     SQL statement. It can be empty, in which case the query is
                  attached to a database but the SQL has to be set later using
                  the string assignment operator.
*/
Query::Query (Database& db, const std::string& sql)
  : stmt (0)
  , dbase (db)
  , col_mapped (false)
{
  if (!sql.empty ())
  {
    int rc;
    if ((rc = sqlite3_prepare_v2 (dbase, sql.c_str (), -1, &stmt, 0)) != SQLITE_OK)
    {
      erc err (rc, Database::Errors ());
      set_erc_message (err, dbase);
      err.raise ();
    }
  }
}

Query::Query (const Query& other)
  : stmt (other.stmt)
  , dbase (other.dbase)
  , col_mapped (other.col_mapped)
  , index (other.index)
{}

Query::Query (Query&& other)
  : stmt (other.stmt)
  , col_mapped (other.col_mapped)
  , index (other.index)
  , dbase (other.dbase)
{
  other.stmt = 0;
  other.col_mapped = false;
  other.index.clear ();
}

Query& Query::operator= (Query&& rhs)
{
  if (stmt)
    sqlite3_finalize (stmt);

  stmt = rhs.stmt;
  rhs.stmt = nullptr;

  dbase = rhs.dbase;

  col_mapped = rhs.col_mapped;
  index = rhs.index;
  rhs.index.clear ();
  rhs.dbase = Database ();

  return *this;
}

Query& Query::operator= (const Query& rhs)
{
  if (&rhs != this)
  {
    if (stmt)
      sqlite3_finalize (stmt);

    stmt = rhs.stmt;
    dbase = rhs.dbase;

    col_mapped = rhs.col_mapped;
    index = rhs.index;
  }
  return *this;
}

Query::~Query ()
{
  if (stmt)
    sqlite3_finalize (stmt);
}

/*!
  The object must have the database reference set before calling this function.
  If a previous statement was attached to the object, it is finalized before the
  new statement is prepared.
*/
Query& Query::sql (const std::string& str)
{
  int rc;
  if (!dbase)
    erc (SQLITE_MISUSE, Database::Errors ()).raise ();
  if (stmt)
    sqlite3_finalize (stmt);
  if ((rc = sqlite3_prepare_v2 (dbase, str.c_str (), -1, &stmt, 0)) != SQLITE_OK)
  {
    erc err (rc, Database::Errors ());
    set_erc_message (err, dbase);
    err.raise ();
  }
  col_mapped = false;
  return *this;
}

/*!
  If the object is not connected with a prepared statement returns an empty string.
*/
std::string Query::sql () const
{
  if (!stmt)
    return std::string ();
  else
    return sqlite3_sql (stmt);
}

/*!
  When a new row of results is available the function returns SQLITE_ROW. When
  there are no more result the function returns SQLITE_DONE. Both codes are
  wrapped in \ref erc objects with priority level `ERROR_PRI_INFO` that normally don't throw
  an exception. Any other error is return at the normal priority level `erc::error`.
 */
erc Query::step ()
{
  int rc = sqlite3_step (stmt);

  if (rc == SQLITE_ROW || rc == SQLITE_DONE)
    return erc (rc, Database::Errors () , erc::info);

  erc err (rc, Database::Errors ());
  set_erc_message (err, dbase);
  return err;
}

erc Query::reset ()
{
  return check_errors (sqlite3_reset (stmt));
}

/*!
  Statements are automatically finalized when Query objects are destructed or
  assigned a new SQL text. Occasionally user might need to manually clear a
  query (for instance if he needs to close a database connection).
*/
void Query::clear ()
{
  sqlite3_finalize (stmt);
  dbase = Database ();
  stmt = 0;
}

//  ---------------------------------------------------------------------------
/*!
  \name Bind functions
  SQL statement can have parameters specified by number (`?` or `?nnn`) or
  by name (`:aaa` or `@aaa` or `$aaa`) where `nnn` is a number (starting from 1)
  and `aaa` is an alphanumeric identifier. Bind functions assign values to
  these parameters.

  If the parameter with that name or number is not found the functions throw
  an \ref erc with code `SQLITE_RANGE`.
*/
///@{

/// Bind a parameter specified by number to a character string
Query& Query::bind (int par, const std::string& str)
{
  check_errors (sqlite3_bind_text (stmt, par, str.c_str (), -1, SQLITE_TRANSIENT));
  return *this;
}

/// Bind a parameter specified by name to a character string
Query& Query::bind (const std::string& parname, const std::string& str)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, str);
}

/// Bind a parameter specified by number to an integer value
Query& Query::bind (int par, int val)
{
  check_errors (sqlite3_bind_int (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to a floating point value
Query& Query::bind (const std::string& parname, double val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, val);
}

/// Bind a parameter specified by number to a floating point value
Query& Query::bind (int par, double val)
{
  check_errors (sqlite3_bind_double (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to a large integer
Query& Query::bind (const std::string& parname, sqlite3_int64 val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, val);
}

/// Bind a parameter specified by number to a large integer
Query& Query::bind (int par, sqlite3_int64 val)
{
  check_errors (sqlite3_bind_int64 (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to an integer value
Query& Query::bind (const std::string& parname, int val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, val);
}

/*!
  Bind a parameter specified by number to an arbitrary memory area (BLOB)
  \param  par parameter index (starting from 1)
  \param  val pointer to BLOB
  \param  len size of BLOB

  The function makes a local copy of the value so user can free it immediately.
*/
Query& Query::bind (int par, void* val, int len)
{
  int rc;
  if (len == 0 || !val)
    rc = sqlite3_bind_null (stmt, par);
  else
    rc = sqlite3_bind_blob (stmt, par, val, len, SQLITE_TRANSIENT);
  check_errors (rc);
  return *this;
}

/*!
  Bind a parameter specified by name to an arbitrary memory area (BLOB)
  \param  parname parameter name
  \param  val pointer to BLOB
  \param  len size of BLOB

  The function makes a local copy of the value so user can free it immediately.
*/
Query& Query::bind (const std::string& parname, void* val, int len)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, val, len);
}

#ifdef _WIN32
/*!
  Bind a parameter specified by name to a `SYSTEMTIME` value.
  The value is written in a format compatible with SQLITE `strftime` function
*/
Query& Query::bind (const std::string& parname, const SYSTEMTIME& val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname.c_str ());
  return bind (idx, val);
}

/*!
  Bind a parameter specified by number to a `SYSTEMTIME` value.
  The value is written in a format compatible with SQLITE `strftime` function
*/
Query& Query::bind (int par, const SYSTEMTIME& val)
{
  char text[256];
  sprintf (text, "%4d-%02d-%02d", val.wYear, val.wMonth, val.wDay);
  if (val.wHour != 0 || val.wMinute != 0 || val.wSecond != 0 || val.wMilliseconds != 0)
  {
    char* ptr = text + strlen (text);
    sprintf (ptr, "T%02d:%02d:%02d", val.wHour, val.wMinute, val.wSecond);

    if (val.wMilliseconds != 0)
    {
      ptr = text + strlen (text);
      sprintf (ptr, ".%03d", val.wMilliseconds);
    }
  }
  check_errors (sqlite3_bind_text (stmt, par, text, -1, SQLITE_TRANSIENT));
  return *this;
}
#endif

/// Reset all parameters to NULL values.
Query& Query::clear_bindings ()
{
  check_errors (sqlite3_clear_bindings (stmt));
  return *this;
}
///@}

///  \name Other column functions
///@{
/*!
  Return data type for a column specified by name or number.
  \retval SQLITE_INTEGER  64-bit signed integer
  \retval SQLITE_FLOAT    64-bit IEEE floating point
  \retval SQLITE_TEXT     string
  \retval SQLITE_BLOB     BLOB
  \retval SQLITE_NULL     NULL
*/
int Query::column_type (int nc) const
{
  return sqlite3_column_type (stmt, nc);
}

/// \copydoc Query::column_type
int Query::column_type (const std::string& colname) const
{
  return column_type (find_col (colname));
}

/*!
  Return number of bytes in a column that contains a BLOB or a string. If
  the result is NULL the function returns 0. For a numerical column, the function
  returns the size of the string representation of the value.
*/
int Query::column_size (int nc) const
{
  return sqlite3_column_bytes (stmt, nc);
}

/// \copydoc Query::column_size (int) const
int Query::column_size (const std::string& colname) const
{
  return column_size (find_col (colname));
}

/*!
  \param nc - column number
  If the query is a SELECT statement and `nc` is a table column (not an
  expression or subquery) the function returns  the declared type of the table
  column. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db("");                          // create in-memory database
  db.exec ("CREATE TABLE t1(c1 VARIANT)");  // schema
  Query q (db, "SELECT c1 FROM t1");        // SELECT query
  auto s = q.decl_type(0);                  // s == "VARIANT"
\endcode
*/
std::string Query::decl_type (int nc) const
{
  return sqlite3_column_decltype (stmt, nc);
}

/*!
  \param colname - column name
  If the query is a SELECT statement and `colname` is a table column (not an
  expression or subquery) the function returns  the declared type of the table
  column. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db("");                          // create temporary database
  db.exec ("CREATE TABLE t1(c1 VARIANT)");  // schema
  Query q (db, "SELECT c1 FROM t1");        // SELECT query
  auto s = q.decl_type("c1");               // s == "VARIANT"
\endcode
*/
std::string Query::decl_type (const std::string& colname) const
{
  return sqlite3_column_decltype (stmt, find_col (colname));
}

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*!
  Return originating table name.

  \param nc - column number
  If the query is a SELECT statement and \p nc is a table column (not an
  expression or subquery) the function returns  the name of the table where
  the column originated from. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  Query q (db, "SELECT c1 FROM tbl");       // SELECT query
  auto s = q.table_name (0);                // s == "tbl"
\endcode

  SQLITE must be compiled with the SQLITE_ENABLE_COLUMN_METADATA for this
  function to be available.
*/
std::string Query::table_name (int nc) const
{
  return sqlite3_column_table_name (stmt, nc);
}

/*!
  Return originating table name.

  \param colname - column name
  If the query is a SELECT statement and \p colname is a table column (not an
  expression or subquery) the function returns  the name of the table where
  the column originated from. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  Query q (db, "SELECT c1 FROM tbl");       // SELECT query
  auto s = q.table_name ("c1");             // s == "tbl"
\endcode

  SQLITE must be compiled with the SQLITE_ENABLE_COLUMN_METADATA for this
  function to be available.
*/
std::string Query::table_name (const std::string& colname) const
{
  return sqlite3_column_table_name (stmt, find_col (colname));
}

/*!
  Return originating schema name

  \param nc - column number

  If the query is a SELECT statement and \p nc is a table column (not an
  expression or subquery) the function returns  the name of the schema where
  the column originated from. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  db.exec ("ATTACH \":memory:\" AS db2");
  db.exec ("CREATE TABLE db2.tbl2(c2 TEXT)");
  Query q (db, "SELECT c1, c2 FROM tbl JOIN tbl2");  // SELECT query
  auto s = q.database_name (0);             // s == "main"
  s = q.database_name (1);                  // s == "db2"
\endcode

  SQLITE must be compiled with the SQLITE_ENABLE_COLUMN_METADATA for this
  function to be available.
*/
std::string Query::database_name (int nc) const
{
  return sqlite3_column_database_name (stmt, nc);
}

/*!
  Return originating schema name.
  \param colname - column name

  If the query is a SELECT statement and \p colname is a table column (not an
  expression or subquery) the function returns  the name of the schema where
  the column originated from. Otherwise the function returns an empty string.

  The returned string is always UTF-8 encoded.

  Example:
\code
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  db.exec ("ATTACH \":memory:\" AS db2");
  db.exec ("CREATE TABLE db2.tbl2(c2 TEXT)");
  Query q (db, "SELECT c1, c2 FROM tbl JOIN tbl2");  // SELECT query
  auto s = q.database_name ("c1");          // s == "main"
  s = q.database_name ("c2");               // s == "db2"
\endcode

  SQLITE must be compiled with the SQLITE_ENABLE_COLUMN_METADATA for this
  function to be available.
*/
std::string Query::database_name (const std::string& colname) const
{
  return sqlite3_column_database_name (stmt, find_col (colname));
}

/*!
  Return column name.

  \param nc - column number

  If the query is a SELECT statement and \p nc is a valid column number,
  the function returns the name of the column. Otherwise the function returns
  an empty string. The name of a result column is the value of the `AS` clause
  for that column, if there is an `AS` clause. If there is no `AS` clause then
  the name of the column is unspecified and may change from one release of SQLite
  to the next.

  The returned string is always UTF-8 encoded.
*/
std::string Query::column_name (int nc) const
{
  if (0 < nc && nc < sqlite3_column_count (stmt))
    return sqlite3_column_name (stmt, nc);
  return std::string ();
}
#endif

///@}

//-----------------------------------------------------------------------------
/*!  \name Data retrieval functions
  `columm_...` functions retrieve data from a column specified by name or number
  (starting from 0). Data type of the result is determined by the result function
  called (`column_int` returns an integer, `column_double` returns a double, and so on).

  Note that column name *does not* include table name. If you need to disambiguate
  column names, use an `AS` clause in the `SELECT` statement.

  If a column with that name cannot be found, the functions throw an \ref erc
  with code `SQLITE_RANGE`.

  For a detailed discussion of the conversions applied see
  <a href="http://sqlite.org/c3ref/column_blob.html">SQLITE documentation</a>
*/
///@{

/*!
  Return column value converted to an integer

  The following rules apply:
  - `NULL` value is converted to 0
  - `REAL` value is rounded to nearest integer
  - `TEXT` or BLOB value is converted to number. Longest possible prefix of the
     value that can be interpreted as an integer number is extracted and the
     remainder ignored. Any leading spaces are ignored.
*/
int Query::column_int (int nc) const
{
  return sqlite3_column_int (stmt, nc);
}

/// \copydoc column_int
int Query::column_int (const std::string& colname) const
{
  return column_int (find_col (colname));
}

/*!
  Return column value as an UTF-8 encoded string.

  If the column is `NULL`, the result is an empty string.
*/
string Query::column_str (int nc) const
{
  char* str = (char*)sqlite3_column_text (stmt, nc);
  if (str)
    return string (str);
  else
    return string ();
}

/// \copydoc column_str
string Query::column_str (const std::string& colname) const
{
  return column_str (find_col (colname));
}

/*!
  Return a pointer to a NULL-terminated text with the column content.
  The memory for the string is freed automatically.

  If the column value is `NULL` the result is a `NULL` pointer
*/
const char* Query::column_text (int nc) const
{
  return (const char*)sqlite3_column_text (stmt, nc);
}

/// \copydoc column_text
const char* Query::column_text (const std::string& colname) const
{
  return column_text (find_col (colname));
}

/*!
  Return floating point value of the column.

  The following rules apply:
  - `NULL` value is converted to 0.0
  - `INTEGER` value is promoted to double
  - `TEXT` or BLOB value is converted to number. Longest possible prefix of the
     value that can be interpreted as a real number is extracted and the
     remainder ignored. Any leading spaces are ignored.
*/
double Query::column_double (int nc) const
{
  return sqlite3_column_double (stmt, nc);
}

/// \copydoc column_double
double Query::column_double (const std::string& colname) const
{
  return column_double (find_col (colname));
}

/*!
  Return column value converted to a 64-bit integer

  The following rules apply:
  - `NULL` value is converted to 0
  - `REAL` value is rounded to nearest integer
  - `TEXT` or `BLOB` value is converted to number. Longest possible prefix of the
     value that can be interpreted as an integer number is extracted and the
     remainder ignored. Any leading spaces are ignored.
*/
sqlite3_int64 Query::column_int64 (int nc) const
{
  return sqlite3_column_int64 (stmt, nc);
}

/// \copydoc column_int64
sqlite3_int64 Query::column_int64 (const std::string& colname) const
{
  return column_int64 (find_col (colname));
}

/*!
  Return a pointer to a BLOB with the column content. The memory for the BLOB
  is freed automatically. Call column_size() function **after** calling this function
  to determine the size of the object returned.
*/
const void* Query::column_blob (int nc) const
{
  return sqlite3_column_blob (stmt, nc);
}

/// \copydoc column_blob (int) const
const void* Query::column_blob (const std::string& colname) const
{
  return column_blob (find_col (colname));
}

#ifdef _WIN32
/*!
  Return a SYSTEMTIME structure with the column content.
*/
SYSTEMTIME
Query::column_time (int nc) const
{
  SYSTEMTIME st;
  float sec;
  memset (&st, 0, sizeof (st));
  const char* pstr = (const char*)sqlite3_column_text (stmt, nc);
  if (pstr)
  {
    sscanf (pstr, "%hd-%hd-%hd %hd:%hd:%f", &st.wYear, &st.wMonth, &st.wDay, &st.wHour, &st.wMinute,
            &sec);
    st.wSecond = (int)sec;
    st.wMilliseconds = (int)((sec - st.wSecond) * 1000.);
  }
  return st;
}

/// \copydoc column_time
SYSTEMTIME
Query::column_time (const std::string& colname) const
{
  return column_time (find_col (colname));
}
#endif

///@}

//---
void Query::map_columns () const
{
  if (col_mapped)
    return;
  int nc = sqlite3_column_count (stmt);
  for (int i = 0; i < nc; i++)
  {
    const char* n = sqlite3_column_name (stmt, i);
    if (n)
      index[n] = i;
  }
  col_mapped = true;
}

int Query::find_col (const std::string& colname) const
{
  if (!col_mapped)
    map_columns ();

  if (index.find (colname) == index.end ())
  {
    erc err (SQLITE_RANGE, Database::Errors ());
    string s = "Error " STRINGERIZE (SQLITE_RANGE) " column '" + colname + "' not found ";
    err.message (s);
    err.raise ();
  }

  return index[colname];
}

erc Query::check_errors (int rc)
{
  if (rc != SQLITE_OK)
  {
    erc err (rc, Database::Errors ());
    set_erc_message (err, dbase);
    return err;
  }
  return erc::success;
}

//-----------------------------------------------------------------------------

/*!
  If there is a handle to the database that produced the error, calls
  <a href="http://sqlite.org/c3ref/errcode.html">sqlite3_errmsg</a>
  to obtain the error message. Otherwise generates a bland message with the error
  code numerical value.

  \param  err       Error code
  \param  db        Database connection handle
*/
static void set_erc_message (erc& err, sqlite3* db)
{
  if (db)
  {
    const char* file = sqlite3_db_filename (db, "main");
    string s = "Error " + to_string (err.code ()) + ' ' + sqlite3_errmsg (db);
    if (file && strlen (file))
      s += " Database: " + string (file);
    err.message (s);
  }
}

} // namespace mlib
