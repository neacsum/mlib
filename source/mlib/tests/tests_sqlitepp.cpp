#include <mlib/sqlitepp.h>
#include <utpp/utpp.h>

using namespace MLIBSPACE;

TEST (NotConnectedDbObject)
{
  Database db;
  sqlite3* hdb = db;
  CHECK_EQUAL (0, (int)hdb);
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

TEST (DbExecStatements)
{
  Database db("");
  db.exec ("CREATE TABLE tab (col);"
           "INSERT INTO tab VALUES (123)");
  db.close ();
}

struct TestDatabase {
  TestDatabase () : db(""), q(db) {
    db.exec ("CREATE TABLE tab (col)");
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
    char msg[256];
    x.message (msg, sizeof(msg));
    printf ("Message %s\n", msg);
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
  CHECK_THROW_EQUAL (q.column_int ("no_such_column"), erc, SQLITE_RANGE);
}

TEST_FIXTURE (TestDatabase, NonExistingParameter)
{
  q = "SELECT (:par)";
  CHECK_THROW_EQUAL (q.bind (":no_such_par", 123), erc, SQLITE_RANGE);
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