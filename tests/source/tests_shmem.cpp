#include <mlib/defs.h>
#include <mlib/shmem.h>
#include <utpp/utpp.h>
#include <string>
#include <mlib/thread.h>
#include <mlib/trace.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif
using namespace std;

struct S{
  char str[10];
  double fval;
  int ival;
};

event shrd, shwr;

struct shmem_fixture {
  shmem_fixture () 
  { 
    strcpy (wr.str, "String"); 
    wr.fval = 12.3;
    wr.ival = 1;
    shrd.reset ();
    shwr.reset ();
  };
  S rd, wr;
};


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
    TRACE ("SlowMem::read - got rdlock");
    shrd.signal ();
    Sleep (50);
    get (data);
    TRACE ("SlowMem::read - will rdunlock");
    rdunlock ();
    return true;
  }
  TRACE ("SlowMem::read - failed");
  return false;
}

bool SlowMem::write (const void *data)
{
  if (wrlock ())
  {
    TRACE ("SlowMem::write - got wrlock");
    shwr.signal ();
    Sleep (50);
    put (data);
    TRACE ("SlowMem::write - will wrunlock");
    wrunlock ();
    return true;
  }
  TRACE ("SlowMem::write - failed");
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

TEST_FIXTURE (shmem_fixture, ReadWrite_shmem)
{
  memset (&rd, 0, sizeof (S));

  shmem<S> smem ("Shared");
  smem << wr;
  smem >> rd;
  CHECK (!memcmp (&wr, &rd, sizeof (S)));
}


TEST_FIXTURE (shmem_fixture, TwoTheard_shmem)
{
  thread t1 ([=](void *)->int
  {
    shmem<S> smem ("Shared");       //create shared memory
    CHECK (smem.is_opened ());
    CHECK (smem.created ());

    memset (&rd, 0, sizeof (S));
    shwr.wait ();                 //wait for other thread to populate it
    smem >> rd;
    return (!memcmp (&wr, &rd, sizeof (S)));  //return 1 if rd == wr
  }
  );

  thread t2 ([=](void *)->int
  {
    Sleep (50);
    shmem<S> smem ("Shared");       //open shared memory
    CHECK (smem.is_opened ());
    CHECK (!smem.created ());
    smem << wr;                     //write data
    shwr.signal ();                 //signal memory full
    return 0;
  }
  );

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (100);                     //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t1.exitcode ());        //verify rd == wr
}


TEST_FIXTURE (shmem_fixture, SlowWriter_shmem)
{
  shmem<S, SlowMem> smem ("Shared");       //create shared memory

  //reader thread
  thread t1 ([=](void *)->int
  {
    memset (&rd, 0, sizeof (S));

    shmem<S, SlowMem> smem ("Shared");     //open shared memory
    smem.rtmo (5);                         //5 msec read timeout

    shwr.wait ();                 //wait for other thread to populate it
    if (smem.read (&rd))
      return 0;                   //that should not happen; shared memory should be busy

    return 1;                     //all good
  });

  //writer thread
  thread t2 ([=](void *)->int
  {
    shmem<S, SlowMem> smem ("Shared");  //open shared memory
    smem << wr;                         //write data. Signaling to reader thread is done in SlowMem::write
    return 0;
  });

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (100);                    //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t1.exitcode ());        //verify thread 1 could not acquire read lock
}

TEST_FIXTURE (shmem_fixture, SlowReader_shmem)
{
  shmem<S, SlowMem> smem ("Shared");      //create shared memory

  //reader thread
  thread t1 ([=](void *)->int
  {
    memset (&rd, 0, sizeof (S));

    shmem<S, SlowMem> smem ("Shared");    //open shared memory

    shwr.wait ();                 //wait for other thread to populate it
    smem >> rd;                   //read data. This triggers a 50 ms delay and signals shrd event flag
    smem >> rd;                   //one more read, mostly for fun
    return 0;
  });

  //writer thread
  thread t2 ([=](void *)->int
  {
    shmem<S, SlowMem> smem ("Shared");      //open shared memory
    smem.wtmo (5);                          //5ms write timeout
    smem << wr;                     //write data.
    shrd.wait ();                   //wait for reader to signal that it has started to read
    wr.ival++;
    if (smem.write (&wr))           //try to write again
      return 0;                     //should not happen. Reader is holding rdlock
    return 1;     //all good
  });

  //start both threads
  t1.start ();
  t2.start ();

  Sleep (500);                    //let them finish
  CHECK (!t1.is_running ());      //verify they have finished
  CHECK (!t2.is_running ());
  CHECK (t2.exitcode ());         //verify thread 2 could not acquire write lock
  CHECK_EQUAL (1, rd.ival);       //check read area was updated only once
}

