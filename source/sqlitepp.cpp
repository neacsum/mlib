/*!
  \file SQLITEPP.CPP Implementation of Database and Query classes

  \defgroup sqlite SQLite Interface
  \brief Object-oriented wrappers for SQLITE3 functions.

*/
#include <stdio.h>
#include <mlib/sqlitepp.h>
#include <mlib/trace.h>
#include <assert.h>

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  Error facility used by Database and Query objects. Keeps track of the last
  database connection that has thrown the error and formats the error message
  using sqlite3_errmessg function.
  \ingroup sqlite
*/
class sqlitefac : public errfac
{
public:
  sqlitefac () : errfac ("SQLite Error") {};
  sqlite3* db;                              ///<handle of db that generated last error
protected:
  std::string message (const erc& e) const;
};

/*!
  Error codes returned by Database and Query objects. The constructor takes the
  additional database parameter needed by sqlitefac to format the error message 
  \ingroup sqlite
*/
class sqerc : public erc
{
public:
  sqerc (int value , sqlite3 *db, short int priority=ERROR_PRI_ERROR);
};

///Error facility used by Database and Query objects
sqlitefac errors;

/*!
  Pointer to error facility. All errors are dispatched using this pointer so users
  can redirect the errors by assigning another error facility it.
*/
errfac *sqlite_errors = &errors;

/*!
  \class Database

  The Database class is a wrapper around the <a href="http://sqlite.org/c3ref/sqlite3.html">sqlite3</a>
  handle returned by sqlite_open_v2 function.
  \ingroup sqlite
*/

//-----------------------------------------------------------------------------
/*!
  The default constructor is used to create an object that is not yet connected
  to a database. It has to be connected using the open() function.
*/
Database::Database() :
  db(0),
  handle_owned (true)
{
}

/*!
  \param  name      database name
  \param  flags     open flags
*/
Database::Database(const std::string& name, int flags) :
  db(0),
  handle_owned (true)
{
  int rc;
  if ((rc=sqlite3_open_v2 (name.c_str(), &db, flags, 0)) != SQLITE_OK)
    throw sqerc (rc, db);
}

/*!
  If the wrapper owns the database handle, it closes it.
*/
Database::~Database ()
{
  if (handle_owned && db)
    sqlite3_close (db);
}

/*!
  Return an empty string if database is not opened or there is no database
  connection with that name or if the database is a temporary or in-memory
  database.
*/
string Database::filename (const string& conn)
{
  const char *pfn = 0;
  if (db)
    pfn = sqlite3_db_filename (db, conn.c_str());
  return pfn ? pfn : string ();
}

/*!
  Returns FALSE only for connected read-write databases.
  If database is not connected or it is read-only, returns TRUE.
*/
bool
Database::is_readonly ()
{
  if (db)
    return (sqlite3_db_readonly (db, "main") == 1);
  else
    return true;
}


/*!
  If the object is associated with another connection, the previous connection
  is closed and a new one is opened.
  \param name     Database name
  \param flags    Combination of openflags flags
*/
erc Database::open (const string &name, int flags)
{
  int rc;

  if (handle_owned && db)
    sqlite3_close (db);
  handle_owned = true;
  if ((rc=sqlite3_open_v2 (name.c_str(), &db, flags, 0)) != SQLITE_OK)
    return sqerc (rc, db);
  else 
    return ERR_SUCCESS;
}

void Database::close ()
{
  if (handle_owned && db)
    sqlite3_close (db);
  db = 0;
  handle_owned = true;
}

erc Database::exec (const string& sql)
{
  int rc;
  if ((rc=sqlite3_exec (db, sql.c_str(), 0, 0, 0)) != SQLITE_OK)
    return sqerc (rc, db);
  else
    return ERR_SUCCESS;
}

/*!
  If the query is associated with a prepared statement, the previous one is 
  finalized before creating a new query.
*/
erc Database::make_query (Query &q, const std::string &sql)
{
  if (q.stmt)
  {
    sqlite3_finalize (q.stmt);
    q.stmt = 0;
    q.index.clear ();
  }

  int rc;
  if ((rc = sqlite3_prepare_v2 (db, sql.c_str(), -1, &q.stmt, 0)) != SQLITE_OK)
    return sqerc (rc, db);

  q.dbase = db;
  q.col_mapped = false;
  return ERR_SUCCESS;
}

int Database::extended_error ()
{
  assert (db);
  return sqlite3_extended_errcode (db);
}

//-----------------------------------------------------------------------------
/*!
  \class Query
  The Query class is a wrapper around a prepared <a href="http://sqlite.org/c3ref/stmt.html">sqlite3</a>
  statement. There are no copy constructors as the status of a prepared statement
  cannot be fully reproduced.

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
  \param  sql     SQL statement
*/
Query::Query(Database &db, const std::string& sql) :
  stmt (0),
  dbase(db),
  col_mapped (false)
{
  int rc;
  if ((rc = sqlite3_prepare_v2 (dbase, sql.c_str(), -1, &stmt, 0)) != SQLITE_OK)
    throw sqerc (rc, dbase);
}

/*!
  The object is linked to a database but doesn't have yet a prepared statement
  associated with it. This is convenient for statements that will latter receive
  their SQL through the assignment operator.
*/
Query::Query(Database &db) :
  stmt (0),
  dbase(db),
  col_mapped (false)
{
}

Query::~Query()
{
  if (stmt)
    sqlite3_finalize (stmt);
}

/*!
  The object must have the database reference set before calling this function.
  If a previous statement was attached to the object, it is finalized before the
  new statement is prepared.
*/
Query& Query::operator =(const std::string& sql)
{
  int rc;
  if (!dbase)
    throw sqerc (SQLITE_MISUSE, 0);
  if (stmt)
    sqlite3_finalize (stmt);
  if ((rc = sqlite3_prepare_v2 (dbase, sql.c_str(), -1, &stmt, 0)) != SQLITE_OK)
  {
    TRACE ("= error %s", sqlite3_errmsg (dbase));
    throw sqerc (rc, dbase);
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
    return std::string();
  else
    return sqlite3_sql (stmt);
}


/*!
  When a new row of results is available the function returns SQLITE_ROW. When
  there are no more result the function returns SQLITE_DONE. Both codes are 
  wrapped in ercs with priority level ERROR_PRI_INFO that normally doesn't throw
  an exception. Any other error is return at the normal priority level ERROR_PRI_ERROR.
 */ 
erc Query::step()
{
  int rc = sqlite3_step (stmt);

  if (rc == SQLITE_ROW || rc == SQLITE_DONE)
    return sqerc (rc, dbase, ERROR_PRI_INFO);

  return sqerc(rc, dbase);
}

Query& Query::reset()
{
  check_errors (sqlite3_reset (stmt));
  return *this;
}

/*!
  Statements are automatically finalized when Query objects are destructed or
  assigned a new SQL text. Occasionally user might need to manually finalize a
  query (for instance if he needs to close a database connection).
*/
void Query::finalize ()
{
  sqlite3_finalize (stmt);
  stmt = 0;
}

//  ---------------------------------------------------------------------------
/*!  \name Bind functions
    SQL statement can have parameters specified by number (? or ?nnn) or
    by name (:aaa or \@aaa or \$aaa) where 'nnn' is a number and 'aaa' is an alphanumeric
    identifier. Bind functions assign values to these parameters.

    If the parameter with that name or number is not found the functions throw 
    an erc with code SQLITE_RANGE.
*/
///@{  

/// Bind a parameter specified by number to a character string
Query& Query::bind (int par, const std::string& str)
{
  check_errors (sqlite3_bind_text (stmt, par, str.c_str(), -1, SQLITE_TRANSIENT));
  return *this;
}

/// Bind a parameter specified by name to a character string
Query& Query::bind (const char *parname, const std::string& str)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, str);
}

/// Bind a parameter specified by number to an integer value
Query& Query::bind (int par, int val)
{
  check_errors (sqlite3_bind_int (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to a floating point value
Query& Query::bind (const char *parname, double val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, val);
}

/// Bind a parameter specified by number to a floating point value
Query& Query::bind (int par, double val)
{
  check_errors (sqlite3_bind_double (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to a large integer
Query& Query::bind (const char *parname, __int64 val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, val);
}

/// Bind a parameter specified by number to a large integer
Query& Query::bind (int par, __int64 val)
{
  check_errors (sqlite3_bind_int64 (stmt, par, val));
  return *this;
}

/// Bind a parameter specified by name to an integer value
Query& Query::bind (const char *parname, int val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, val);
}

/*! 
  Bind a parameter specified by number to an arbitrary memory area (BLOB)
  \param  par parameter index (starting from 1)
  \param  val pointer to BLOB
  \param  len size of BLOB

  The function makes a local copy of the value so user can free it immediately.
*/
Query& Query::bind (int par, void *val, int len)
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
Query& Query::bind (const char *parname, void *val, int len)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, val, len);
}

/*!
  Bind a parameter specified by name to a SYSTEMTIME value.
  The value is written in a format compatible with SQLITE strftime function
*/
Query& Query::bind (const char *parname, const SYSTEMTIME& val)
{
  int idx = sqlite3_bind_parameter_index (stmt, parname);
  return bind (idx, val);
}

/*!
  Bind a parameter specified by number to a SYSTEMTIME value.
  The value is written in a format compatible with SQLITE strftime function
*/
Query& Query::bind (int par, const SYSTEMTIME& val)
{
  char text[256];
  sprintf (text, "%4d-%02d-%02d", val.wYear, val.wMonth, val.wDay);
  if (val.wHour != 0 || val.wMinute != 0 || val.wSecond != 0 || val.wMilliseconds != 0)
  {
    char *ptr = text+strlen(text);
    sprintf (ptr, "T%02d:%02d:%02d", val.wHour, val.wMinute, val.wSecond);

    if (val.wMilliseconds != 0)
    {
      ptr = text + strlen(text);
      sprintf (ptr, ".%03d", val.wMilliseconds);
    }
  }
  check_errors (sqlite3_bind_text (stmt, par, text, -1, SQLITE_TRANSIENT));
  return *this;
}

/// Reset all parameters to NULL values.
Query& Query::clear_bindings()
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
int Query::column_type (int nc)
{
  return sqlite3_column_type (stmt, nc);
}

/// \copydoc Query::column_type(int)
int Query::column_type (const char *colname)
{
  return column_type (find_col(colname));
}

/*!
  Return number of bytes in a column that contains a BLOB or a string. If
  the result is NULL the function returns 0. For a numerical column, the function
  returns the size of the string representation of the value.
*/
int Query::column_size (int nc)
{
  return sqlite3_column_bytes (stmt, nc);
}

/// \copydoc Query::column_size(int)
int Query::column_size(const char *colname)
{
  return column_size (find_col(colname));
}

///@}


//-----------------------------------------------------------------------------
/*!  \name Result functions
  All result functions return the value of a column specified by name or number
  (starting from 0). Data type of the result is determined by the result function
  called (column_int returns an integer, column_double returns a double, etc.).

  If a column with that name cannot be found, the functions throw an erc with code
  SQLITE_RANGE.

  For a detailed discussion of the conversions applied see
  <a href="http://sqlite.org/c3ref/column_blob.html">SQLITE documentation</a>
*/
///@{  

int Query::column_int (int nc)
{
  return sqlite3_column_int (stmt, nc);
}

int Query::column_int (const char *colname)
{
  return column_int (find_col(colname));
}

string Query::column_str(int nc)
{
  char *str = (char*)sqlite3_column_text(stmt, nc);
  if (str)
    return string (str);
  else
    return string ();
}

string Query::column_str (const char *colname)
{
  return column_str (find_col(colname));
}

/*!
  Return a pointer to a NULL-terminated text with the column content. 
  The memory for the string is freed automatically.
*/
const char *Query::column_text(int nc)
{
  return (const char*)sqlite3_column_text (stmt, nc);
}

const char *Query::column_text (const char *colname)
{
  return column_text (find_col(colname));
}

double Query::column_double (int nc)
{
  return sqlite3_column_double (stmt, nc);
}

double Query::column_double(const char *colname)
{
  return column_double (find_col(colname));
}

__int64 Query::column_int64 (int nc)
{
  return sqlite3_column_int64 (stmt, nc);
}

__int64 Query::column_int64(const char *colname)
{
  return column_int64 (find_col(colname));
}

/*!
  Return a pointer to a BLOB with the column content. The memory for the BLOB
  is freed automatically. Call column_size() function \e after calling this function
  to determine the size of the object returned.
*/
const void* Query::column_blob (int nc)
{
  return sqlite3_column_blob (stmt, nc);
}

/// \copydoc Query::column_blob(int)
const void* Query::column_blob(const char *colname)
{
  return column_blob (find_col(colname));
}
///@}

/*!
  Return a SYSTEMTIME structure with the column content.
*/
SYSTEMTIME Query::column_time (int nc)
{
  SYSTEMTIME st;
  float sec;
  memset (&st, 0, sizeof(st));
  const char *pstr = (const char *)sqlite3_column_text (stmt, nc);
  if (pstr)
  {
    sscanf (pstr, "%hd-%hd-%hd %hd:%hd:%f", 
      &st.wYear, &st.wMonth, &st.wDay, &st.wHour, &st.wMinute, &sec);
    st.wSecond = (int)sec;
    st.wMilliseconds = (int)((sec - st.wSecond)*1000.);
  }
  return st;
}

/// \copydoc Query::column_time(int)
SYSTEMTIME Query::column_time(const char *colname)
{
  return column_time (find_col(colname));
}
///@}


//---
void Query::map_columns()
{
  if (col_mapped)
    return;
  int nc = sqlite3_column_count (stmt);
  for (int i=0; i<nc; i++)
  {
    const char *n = sqlite3_column_name(stmt, i);
    if (n)
      index[n] = i;
  }
  col_mapped = true;
}

int Query::find_col(const char *colname)
{
  if (!col_mapped)
    map_columns ();

  if (index.find (colname) == index.end ())
    throw sqerc(SQLITE_RANGE, 0);

  return index[colname];
}

erc Query::check_errors(int rc)
{
  if (rc != SQLITE_OK)
    return sqerc (rc, sqlite3_db_handle (stmt));
  return ERR_SUCCESS;
}

//-----------------------------------------------------------------------------

/*!
  If there is a handle to the database that produced the error, calls
  <a href="http://sqlite.org/c3ref/errcode.html">sqlite3_errmsg</a>
  to obtain the error message. Otherwise generates a bland message with the error
  code numerical value.

  \param  e       error code
*/
std::string sqlitefac::message (const erc &e) const
{
  if (db)
  {
    const char *file = sqlite3_db_filename (db, "main");
    string s = name () + ' ' + to_string (e.code ()) + ' ' + sqlite3_errmsg (db);
    if (file && strlen(file))
      s += " Database: " + string(file);
    return s;
  }
  else
    return name () + ' ' + to_string (e.code ()) + " (cannot find message)";
}


//-----------------------------------------------------------------------------
/*!
  \param value    error code
  \param db       handle to database connection that triggered the error. Can be NULL.
  \param pri      error severity
*/
sqerc::sqerc (int value, sqlite3* db, short int pri) : 
  erc (value, pri, sqlite_errors)
{
  sqlitefac *f = dynamic_cast<sqlitefac *>(sqlite_errors);
  if (f)
    f->db = db;
}

#ifdef MLIBSPACE
};
#endif
