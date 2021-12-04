#include <utpp/utpp.h>
#include <mlib/syncque.h>
#include <mlib/thread.h>
#include <mlib/stopwatch.h>

#include <iostream>
#include <iomanip>

using namespace mlib;
using namespace std;


SUITE (syncque)
{

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
      while (n = nums.consume ())
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
  cout << "sync_queue finished producing" << " in " << fixed
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
    result r = primes.consume ();
    found_by[r.worker]++;
  }

  for (int i = 0; i < NTHREADS; i++)
    cout << "Consumer " << i << " found " << found_by[i] << " primes." << endl;
}

TEST (bounded_primes)
{
  bounded_queue<int> nums(20);
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
      while (n = nums.consume ())
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
  cout << "bounded_queue finished producing" << " in " 
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
    result r = primes.consume ();
    found_by[r.worker]++;
  }

  for (int i = 0; i < NTHREADS; i++)
    cout << "Consumer " << i << " found " << found_by[i] << " primes." << endl;
}
}
