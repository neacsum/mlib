#include <mlib/sqlitepp.h>
#include <utpp/utpp.h>
#include <iostream>
#include <utf8/utf8.h>

using namespace mlib;

SUITE (sqlitepp)
{

TEST (NotConnectedDbObject)
{
  Database db;
  sqlite3* hdb = db;
  CHECK_EQUAL ((sqlite3*)nullptr, hdb);
  db.open ("");
  hdb = db;
  CHECK(hdb != 0);
}

TEST (ConnectedDbObject)
{
  Database db("");
  sqlite3* hdb = db;
  CHECK (hdb != 0);
}

TEST (DbReadonly)
{
  Database db;
  CHECK_EX (db.is_readonly (), "Not connected database should be read-only");
  db.open ("", Database::openflags::readonly);
  CHECK ((sqlite3*)db != 0);
  CHECK_EX (db.is_readonly (), "Read-only database should be read-only");
  db.close ();
  db.open ("");
  CHECK_EX (!db.is_readonly (), "Read-write database should not be read-only");
}

//check Database::filename returns correct name
TEST(DbFilename)
{
  Database db("testdb.sqlite");
  auto s = db.filename();
  auto ss = s.substr(s.rfind('\\') + 1);
  CHECK_EQUAL(ss, "testdb.sqlite");
  db.close();
  utf8::remove(s);
}


TEST(Db_make_query_ok)
{
  Database db("");
  db.exec("CREATE TABLE tab (col);"
    "INSERT INTO tab VALUES (123)");
  auto [q, ret] = db.make_query("SELECT * FROM tab");
  CHECK_EQUAL(ERROR_SUCCESS, ret);
  q.step();
  CHECK_EQUAL(123, q.column_int(0));
  q.finalize();
  db.close();
}

TEST(Db_make_query_err)
{
  Database db("");
  db.exec("CREATE TABLE tab (col);"
    "INSERT INTO tab VALUES (123)");
  auto [q, ret] = db.make_query("SELECT * MROM tab"); //syntax error
  CHECK_EQUAL(SQLITE_ERROR, ret);
  db.close();
}

TEST(Db_make_query_throw)
{
  Database db("");
  bool thrown = false;
  db.exec("CREATE TABLE tab (col);"
    "INSERT INTO tab VALUES (123)");
  try {
    auto [q, ret] = db.make_query("SELECT * MROM tab"); //syntax error
  }
  catch (erc& x) {
    CHECK_EQUAL(SQLITE_ERROR, x);
    thrown = true;
  }
  CHECK(thrown);
}

//parse multiple queries from sql string
TEST(db_make_query_multiple)
{
  Database db("");
  std::string sql{ "CREATE TABLE tab (col);INSERT INTO tab VALUES (123)" };
  int count = 0;
  do {
    auto [q, ret] = db.make_query(sql);
    if (ret)
      break;
    q.step();
    ++count;
  } while (!sql.empty());
  CHECK_EQUAL(2, count);

  auto [q, ret] = db.make_query("SELECT * FROM tab");
  CHECK_EQUAL(ERROR_SUCCESS, ret);
  q.step();
  CHECK_EQUAL(123, q.column_int(0));
}

TEST (DbExecStatements)
{
  Database db("testdb.sqlite");
  db.exec ("CREATE TABLE tab (col);"
           "INSERT INTO tab VALUES (123)");
  db.close ();
  db.open("testdb.sqlite");
  Query q(db, "SELECT * FROM tab");
  q.step();
  CHECK_EQUAL(123, q.column_int(0));
  q.finalize();
  db.close();
  utf8::remove ("testdb.sqlite");
}

//Assign an existing database
TEST (DbAssign_Existing)
{
  remove ("disk.db");
  Database db_from ("disk.db");
  db_from.exec (R"(
    CREATE TABLE tab (col);
    INSERT INTO tab VALUES (1);
    INSERT INTO tab VALUES (2);
  )");

  Database db_to ("memory.db", Database::openflags::memory);
  db_to = db_from;
  db_from.close ();
  remove ("disk.db");

  Query q (db_to, "SELECT * FROM tab");
  q.step ();
  CHECK_EQUAL (1, q.column_int (0));
  q.step ();
  CHECK_EQUAL (2, q.column_int (0));
}

TEST (DbAssign_Empty)
{
  Database db_to, db_from;
  db_to = db_from;

  CHECK (!db_to.connected ());
}

//cannot assign database if one is opened and the other is not
TEST (DbAssign_Fail)
{
  Database db_to (""), db_from;

  CHECK_THROW (erc, db_to = db_from );
}

//cannot copy database if a query is active
TEST (DbAssign_Busy)
{
  Database db_to ("to.db", Database::openflags::memory);
  db_to.exec (R"(
    CREATE TABLE tab (col);
    INSERT INTO tab VALUES (1);
    INSERT INTO tab VALUES (2);
  )");
  Database db_from ("from.db", Database::openflags::memory);
  Query q (db_to, "SELECT * FROM tab");
  q.step ();

  CHECK_THROW (erc, db_to = db_from);

  q.finalize (); // now db is free, we can copy
  db_to = db_from;

}

struct TestDatabase {
  TestDatabase () : db(""), q(db) {
    db.exec ("CREATE TABLE tab (col);CREATE TABLE tab2 (a PRIMARY KEY, b UNIQUE);");
  };
  ~TestDatabase () {q.finalize (); db.close ();};

  Database db;
  Query q;
};

TEST_FIXTURE (TestDatabase, SqlSyntaxError)
{
  bool caught = false;
  try {
    db.exec ("SELECT ;");
  }
  catch (erc & x)
  {
    std::string s ("Message " + x.message ());
    std::cout << s << std::endl;
    caught = true;
  }
  CHECK (caught);
}

TEST_FIXTURE (TestDatabase, QueryHasGoodDbHandle)
{
  q = "SELECT (1)";
  sqlite3 *h = sqlite3_db_handle(q);
  sqlite3 *hdb = db;
  CHECK_EQUAL (hdb, h);
}

TEST_FIXTURE (TestDatabase, QueryStep)
{
  int rc;
  q = "SELECT (1)";
  rc = q.step ();
  CHECK_EQUAL (SQLITE_ROW, rc);
  CHECK_EQUAL (1, q.column_int(0));
}

TEST_FIXTURE (TestDatabase, BindIntTest)
{
  q = "SELECT (?)";
  q.bind (1, 123);
  q.step ();
  CHECK_EQUAL (123, q.column_int(0));
}

TEST_FIXTURE (TestDatabase, BindFloatTest)
{
  q = "SELECT (?)";
  q.bind (1, 123.456);
  q.step ();
  CHECK_EQUAL (123.456, q.column_double(0));
}

TEST_FIXTURE (TestDatabase, BindStringTest)
{
  q = "SELECT (?)";
  q.bind (1, "Quick brown fox");
  q.step ();
  CHECK_EQUAL ("Quick brown fox", q.column_str(0));
}

TEST_FIXTURE (TestDatabase, BindByName)
{
  q = "SELECT (:par)";
  q.bind (":par", 123);
  q.step ();
  CHECK_EQUAL (123, q.column_int(0));
}

TEST_FIXTURE (TestDatabase, ColumnByName)
{
  q = "INSERT INTO tab VALUES (123)";
  q.step ();
  q = "SELECT * FROM tab";
  q.step ();
  CHECK_EQUAL (123, q.column_int("col"));
}

TEST_FIXTURE (TestDatabase, NonExisitngColumnName)
{
  q = "SELECT * FROM tab";
  q.step ();
  CHECK_THROW_EQUAL (erc, SQLITE_RANGE, q.column_int ("no_such_column"));
}

TEST_FIXTURE (TestDatabase, NonExistingParameter)
{
  q = "SELECT (:par)";
  CHECK_THROW_EQUAL (erc, SQLITE_RANGE, q.bind (":no_such_par", 123));
}

// test for Query::sql function and string conversion operator
TEST_FIXTURE (TestDatabase, Get_SQL_Text)
{
  std::string s{ "SELECT (:par)" };
  Query q1 (db, s);
  std::string s1 = q1.sql ();
  CHECK_EQUAL (s, s1);

  std::string s2 = q1;
  CHECK_EQUAL (s, s2);

}

TEST_FIXTURE (TestDatabase, InsertBlob)
{
  struct x {
    int ix;
    char str[256];
  };
  
  x in, out;
  in.ix = 123;
  strcpy (in.str, "Quick brown fox jumps over lazy dog.");

  q = "INSERT INTO tab VALUES (?)";
  q.bind (1, &in, sizeof(in));
  q.step ();
  q = "SELECT * FROM tab";
  q.step ();
  int sz = q.column_size (0);
  CHECK_EQUAL (sizeof(x), sz);

  memcpy (&out, q.column_blob (0), sz);
  CHECK_EQUAL (in.ix, out.ix);
  CHECK_EQUAL (in.str, out.str);
}

TEST_FIXTURE (TestDatabase, InsertDuplicate)
{
  q = "INSERT INTO tab2 values (1, 2);";
  CHECK_EQUAL (SQLITE_DONE, q.step ());
  q = "INSERT INTO tab2 values (2, 2);";
  CHECK_EQUAL (SQLITE_CONSTRAINT, q.step ());
  CHECK_EQUAL (SQLITE_CONSTRAINT_UNIQUE, db.extended_error ());
}

TEST_FIXTURE (TestDatabase, ChangesCount)
{
  db.exec (R"(
    INSERT INTO tab VALUES (1);
    INSERT INTO tab VALUES (2);
  )");
  auto chg = db.changes ();
  CHECK_EQUAL (1, chg);
  auto total = db.total_changes ();
  CHECK_EQUAL (2, total);
}

#ifndef SQLITE_OMIT_DECLTYPE
TEST (decl_type)
{
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE t1(c1 VARIANT)");  // schema
  Query q (db, "SELECT c1 FROM t1");        // SELECT query
  auto s = q.decl_type ("c1");              // s == "VARIANT"

  CHECK_EQUAL ("VARIANT", s);
}
#endif

#ifdef SQLITE_ENABLE_COLUMN_METADATA
TEST (table_name1)
{
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  Query q (db, "SELECT c1 FROM tbl");       // SELECT query
  auto s = q.table_name ("c1");             // s == "tbl"

  CHECK_EQUAL ("tbl", s);
}

TEST (table_name2)
{
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  Query q (db, "SELECT c1 FROM tbl");       // SELECT query
  auto s = q.table_name (0);                // s == "tbl"

  CHECK_EQUAL ("tbl", s);
}

TEST (database_name)
{
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  db.exec ("ATTACH \":memory:\" AS db2");
  db.exec ("CREATE TABLE db2.tbl2(c2 TEXT)");
  Query q (db, "SELECT c1, c2 FROM tbl JOIN tbl2");       // SELECT query
  auto s = q.database_name (0);          // s == "main"
  CHECK_EQUAL ("main", s);
  s = q.database_name (1);               // s == "db2"
  CHECK_EQUAL ("db2", s);
}

TEST (database_name2)
{
  Database db ("");                         // create temporary database
  db.exec ("CREATE TABLE tbl(c1 TEXT)");    // schema
  db.exec ("ATTACH \":memory:\" AS db2");
  db.exec ("CREATE TABLE db2.tbl2(c2 TEXT)");
  Query q (db, "SELECT c1, c2 FROM tbl JOIN tbl2");  // SELECT query
  auto s = q.database_name ("c1");          // s == "main"
  CHECK_EQUAL ("main", s);
  s = q.database_name ("c2");               // s == "db2"
  CHECK_EQUAL ("db2", s);
}
#endif

TEST (method_schema)
{
  remove ("disk1.db");
  Database db1 ("disk1.db");
  db1.exec (R"(
    CREATE TABLE tab (col);
    INSERT INTO tab VALUES (1);
    INSERT INTO tab VALUES (2);
  )");
  remove ("disk2.db");
  Database db2 ("disk2.db");
  db2.exec (R"(
    CREATE TABLE tab2 (col);
    INSERT INTO tab2 VALUES (11);
    INSERT INTO tab2 VALUES (12);
  )");
  db2.close ();

  CHECK_EQUAL(0, db1.exec ("ATTACH 'disk2.db' AS schema2"));
  CHECK_EQUAL ("main", db1.schema (0));
  CHECK_EQUAL ("schema2", db1.schema (2));
  CHECK_EQUAL (utf8::fullpath("disk2.db"), db1.filename (db1.schema (2)));
 
  db1.close ();
  remove ("disk1.db");
  remove ("disk2.db");
}
}