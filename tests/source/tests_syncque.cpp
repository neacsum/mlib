#include <utpp/utpp.h>
#include <mlib/syncque.h>
#include <mlib/thread.h>
#include <mlib/stopwatch.h>

#include <iostream>
#include <iomanip>

using namespace mlib;
using namespace std;

SUITE (sync_queue) {

TEST (ballroom)
{
  const int pairs = 10;
  event ball_start (event::manual);
  LONG finished = 0;

  sync_queue<int> pairing;

  thread* producer[pairs];
  thread* consumer[pairs];
  criticalsection use_cout;

  for (int i = 0; i < pairs; i++)
  {
    producer[i] = new thread ([&, i]()->int {
      ball_start.wait ();
      pairing.produce (i);
      return 0;
      });
    consumer[i] = new thread ([&, i]()->int {
      ball_start.wait ();
      int v;
      pairing.consume (v);
      {
        lock l (use_cout);
        cout << "Dancing " << i << " - " << v << endl;
      }
      InterlockedIncrement (&finished);
      return 0;
      });
    producer[i]->start ();
    consumer[i]->start ();
  }
  ball_start.signal ();
  Sleep (1000);
  for (int i = 0; i < pairs; i++)
  {
    delete producer[i];
    delete consumer[i];
  }
  CHECK_EQUAL (pairs, finished);
}

}

SUITE (async_queue)
{

// verify consuming with a timeout limit
TEST (water_drops)
{
  const int drops = 10;
  const int drop_interval = 100;
  const int wait_time = 50;
  int missed = 0;
  async_queue<int> faucet;
  thread producer ([&]()->int {
    for (int i = 0; i < drops; i++)
    {
      faucet.produce (i+1);
      Sleep (drop_interval);
    }
    faucet.produce (-1);
    return 0;
  });

  thread consumer ([&]()->int {
    int v=1;
    while (v >= 0)
    {
      if (!faucet.consume (v, wait_time))
        missed++;
    }
    return 0;
  });

  consumer.start ();
  producer.start ();
  consumer.wait ();
  cout << "Water drops - consumer missed " << missed << " waits" << endl;
  CHECK (missed > 0);
}

// verify timeout while producing
TEST (silo_filling)
{
  const int bushels = 10;
  const int empty_interval = 100;
  const int fill_rate = 50;
  int missed = 0;

  async_queue<int> silo(5);

  thread producer ([&]()->int {
    for (int i = 0; i < bushels; i++)
    {
      while (!silo.produce (i + 1, fill_rate))
        missed++;
    }
    silo.produce (-1);
    return 0;
    });

  thread consumer ([&]()->int {
    int v = 1;
    while (v >= 0)
    {
      silo.consume (v);
      Sleep (empty_interval);
    }
    return 0;
    });

  consumer.start ();
  producer.start ();
  consumer.wait ();
  cout << "Silo - producer missed " << missed << " bushels" << endl;
  CHECK (missed > 0);
}


/*
  Various upper limits and the number of primes less than that limit.
  Data from https://primes.utm.edu/nthprime
*/
struct {
  int limit;
  int n_primes;
} checks[] = {        // times for 8 consumers (on my "Captain Slow" machine)
  {  500000,  41538}, //   4 sec
  { 1000000,  78498}, //  18 sec
  { 5000000, 348513}, // 435 sec (243 in release mode)
  {10000000, 664579}, // too long
};

#define CASE 0
#define NTHREADS 8


static bool IsPrime (int n)
{
  bool ret = true;
  for (int i = 2; i <= n / 2 && ret; ret = n % i++ != 0);
  return ret;
}


TEST (primes_queue)
{
  async_queue<int> nums;
  struct result {
    int prime;
    int worker;
  };

  async_queue<result> primes;

  thread* consumers[NTHREADS];
  for (int thnum = 0; thnum < NTHREADS; thnum++)
  {
    auto checker = [&nums, &primes, thnum]()->int {
      int n = 1;
      while (nums.consume (n, 100) && n != 0)
      {
        if (IsPrime (n))
          primes.produce ({ n,thnum });
      }
      return 0;
    };
    consumers[thnum] = new thread (checker);
    consumers[thnum]->start ();
  }
  thread producer ([&nums]()->int {
    for (int i = 2; i < checks[CASE].limit; i++)
      nums.produce (i);

    for (int i = 0; i < NTHREADS; i++)
      nums.produce (0);
    return 0;
    });
  stopwatch t_prod, t_cons;
  t_prod.start ();
  t_cons.start ();

  producer.start ();
  producer.wait ();
  t_prod.stop ();
  cout << "Unbounded queue finished producing" << " in " << fixed
    << setprecision (2) << t_prod.msecEnd ()/1000. << "sec" << endl;

  for (int i = 0; i < NTHREADS; i++)
    consumers[i]->wait ();
  t_cons.stop ();
  cout << "finished consuming" << " in " << fixed
    << setprecision (2) << t_cons.msecEnd () / 1000. << "sec" << endl;

  CHECK_EQUAL (checks[CASE].n_primes, primes.size ());

  int found_by[NTHREADS];
  for (int i = 0; i < NTHREADS; i++)
    found_by[i] = 0;

  while (!primes.empty ())
  {
    result r;
    primes.consume (r);
    found_by[r.worker]++;
  }

  for (int i = 0; i < NTHREADS; i++)
  {
    cout << "Consumer " << i << " found " << found_by[i] << " primes." << endl;
    delete consumers[i];
  }
}

TEST (bounded_primes)
{
  async_queue<int> nums(20);
  struct result {
    int prime;
    int worker;
  };

  async_queue<result> primes;

  thread* consumers[NTHREADS];
  for (int thnum = 0; thnum < NTHREADS; thnum++)
  {
    auto checker = [&nums, &primes, thnum]()->int {
      int n;
      while (1)
      {
        nums.consume (n);
        if (!n)
          break;
        if (IsPrime (n))
          primes.produce ({ n,thnum });
      }
      return 0;
    };

    consumers[thnum] = new thread (checker);
    consumers[thnum]->start ();
  }
  
  thread producer ([&nums]()->int {
    for (int i = 2; i < checks[CASE].limit; i++)
      nums.produce (i);

    for (int i = 0; i < NTHREADS; i++)
      nums.produce (0);
    return 0;
    });

  stopwatch t_prod, t_cons;
  t_prod.start ();
  t_cons.start ();
  producer.start ();
  producer.wait ();
  t_prod.stop ();
  cout << "Bounded queue finished producing" << " in " 
    << fixed << setprecision(2) << t_prod.msecEnd ()/1000. << "sec" << endl;

  for (int i = 0; i < NTHREADS; i++)
    consumers[i]->wait ();

  t_cons.stop ();
  cout << "finished consuming" << " in " << fixed
    << setprecision (2) << t_cons.msecEnd () / 1000. << "sec" << endl;

  CHECK_EQUAL (checks[CASE].n_primes, primes.size ());

  int found_by[NTHREADS];
  for (int i = 0; i < NTHREADS; i++)
    found_by[i] = 0;

  while (!primes.empty ())
  {
    result r;
    primes.consume (r);
    found_by[r.worker]++;
  }

  for (int i = 0; i < NTHREADS; i++)
  {
    cout << "Consumer " << i << " found " << found_by[i] << " primes." << endl;
    delete consumers[i];
  }
}
}
