#include <mlib/defs.h>
#include <mlib/shmem.h>
#include <utpp/utpp.h>
#include <string>
#include <mlib/thread.h>

using namespace mlib;
using namespace std;

struct S{
  char str[10];
  double fval;
  int ival;
};

S rd, wr = { "String", 12.3, 1 };
event shrd, shwr;

//add some delay while reading and writing
class SlowMem : public shmem_base
{
public:
  SlowMem (const char * name, size_t size) : shmem_base (name, size) {};

  bool read (void *data);
  bool write (const void *data);
};

bool SlowMem::read (void *data)
{
  if (rdlock ())
  {
    shrd.signal ();
    Sleep (50);
    get (data);
    rdunlock ();
    return true;
  }
  return false;
}

bool SlowMem::write (const void *data)
{
  if (wrlock ())
  {
    shwr.signal ();
    Sleep (50);
    put (data);
    wrunlock ();
    return true;
  }
  return false;
}


TEST (Create_shmem)
{
  shmem<S> smem ("Shared");
  CHECK (smem.is_opened ());
  CHECK (smem.created ());
  CHECK_EQUAL (sizeof (S), smem.size ());
  string n (smem.name ());
  CHECK_EQUAL ("Shared", n);
}

TEST (ReadWrite_shmem)
{
  memset (&rd, 0, sizeof (S));

  shmem<S> smem ("Shared");
  smem << wr;
  smem >> rd;
  CHECK (!memcmp (&wr, &rd, sizeof (S)));
}


TEST (TwoTheard_shmem)
{
  shmem<S> smem ("Shared");       //create shared memory

  thread t1 ([](void *)->int
  {
    memset (&rd, 0, sizeof (S));

    shmem<S> smem ("Shared");     //open shared memory
    shwr.wait ();                 //wait for other thread to populate it
    smem >> rd;
    return (!memcmp (&wr, &rd, sizeof (S)));  //return 1 if rd == wr
  }
  );

  thread t2 ([](void *)->int
  {
    shmem<S> smem ("Shared");       //open shared memory
    smem << wr;                     //write data
    shwr.signal ();                 //signal memory full
    return 0;
  }
  );

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (50);                     //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t1.exitcode ());        //verify rd == wr
}


TEST (SlowWriter_shmem)
{
  shmem<S, SlowMem> smem ("Shared");       //create shared memory

  //reader thread
  thread t1 ([](void *)->int
  {
    memset (&rd, 0, sizeof (S));

    shmem<S, SlowMem> smem ("Shared");     //open shared memory
    smem.rtmo (5);                         //5 msec read timeout

    shwr.wait ();                 //wait for other thread to populate it
    if (smem.read (&rd))
      return 0;                   //that should not happen; shared memory should be busy

    return 1;                     //all good
  }
  );

  //writer thread
  thread t2 ([](void *)->int
  {
    shmem<S, SlowMem> smem ("Shared");  //open shared memory
    smem << wr;                         //write data. Signaling to reader thread is done in SlowMem::write
    return 0;
  }
  );

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (100);                    //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t1.exitcode ());        //verify thread 1 could not acquire read lock
}

TEST (SlowReader_shmem)
{
  shmem<S, SlowMem> smem ("Shared");      //create shared memory

  //reader thread
  thread t1 ([](void *)->int
  {
    memset (&rd, 0, sizeof (S));

    shmem<S, SlowMem> smem ("Shared");    //open shared memory

    shwr.wait ();                 //wait for other thread to populate it
    smem >> rd;                   //read data. This trigger a 50 ms delay and signals shrd event flag
    smem >> rd;                   //one more read, mostly for fun
    return 0;
  }
  );

  //writer thread
  thread t2 ([](void *)->int
  {
    shmem<S, SlowMem> smem ("Shared");      //open shared memory
    smem.wtmo (5);                          //5ms write timeout
    smem << wr;                     //write data.
    shrd.wait ();                   //wait for reader to signal that it has started to read
    wr.ival++;
    if (smem.write (&wr))           //try to write again
      return 0;                     //should not happen. Reader is holding rdlock
    return 1;     //all good
  }
  );

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (200);                    //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t2.exitcode ());         //verify thread 2 could not acquire write lock
  CHECK_EQUAL (1, rd.ival);       //check read area was updated only once
}

