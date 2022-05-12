/*
  Sample producer-consumer process.

  (c) Mircea Neacsu 2021-2022

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.


  This program shows how to use the async_queue objects for asynchronous
  communication between threads. 
  
  It finds all prime numbers smaller than a certain value using a multi-threaded
  process. The idea of this demo is taken from:
  https://www.codeproject.com/Articles/1254918/A-Concise-Overview-of-Threads

  A producer thread places all numbers that are to be tested for primality in
  an async_queue structure. It then creates a number of consumer threads that
  each picks a number from the queue and, if it is prime, places the result
  in an output queue.
*/
#include <mlib/syncque.h>
#include <mlib/thread.h>
#include <mlib/stopwatch.h>

#include <windows.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace mlib;


/*
  Various upper limits and the number of primes less than that limit.
  Data from https://primes.utm.edu/nthprime
*/
struct {
  int limit;
  int n_primes;
} checks[] = {        // times for 8 consumers
  {  500000,  41538}, //   4 sec
  { 1000000,  78498}, //  18 sec
  { 5000000, 348513}, // 435 sec (243 in release mode)
  {10000000, 664579}, // too long
};

#define CASE 0
#define NTHREADS 8

/* Primality testing function. */
bool IsPrime (int n)
{
  bool ret = true;
  for (int i = 2; i <= n / 2 && ret; ret = n % i++ != 0)
    ;
  return ret;
}

/* Result placed in output queue. It contains the prime number found and
the ID of the consumer thread that found it.*/
struct result
{
  int prime;
  int worker;
};

// Producer-consumer process with an unlimited queue
void primes_queue (int queue_size)
{
  async_queue<int> nums (queue_size); //numbers to be tested
  async_queue<result> primes; //output results

  thread* consumers[NTHREADS];
  for (int thnum = 0; thnum < NTHREADS; thnum++)
  {
    //body of consumer thread
    auto checker = [&nums, &primes, thnum]()->int {
      while (1)
      {
        int n;
        nums.consume (n); // extract a number from input queue
        if (!n)
          break;  //if zero, job is done. Exit.

        //check if number is prime
        if (IsPrime (n))
          primes.produce ({ n,thnum }); //if yes, add it to output queue
      }
      return 0;
    };

    // create a number of consumer threads...
    consumers[thnum] = new thread (checker);

    // ... and start them
    consumers[thnum]->start ();
  }

  thread producer ([&nums]()->int {
    //producer thread places all numbers in the input queue...
    for (int i = 2; i < checks[CASE].limit; i++)
      nums.produce (i);

    // ... followed by termination flags for each consumer 
    for (int i = 0; i < NTHREADS; i++)
      nums.produce (0);

    return 0;
    });
  stopwatch t_prod, t_cons;
  t_prod.start ();
  t_cons.start ();

  producer.start ();    //Start producer
  producer.wait ();     //Wait to finish producing
  t_prod.stop ();
  
  //Show producer statistics
  cout << ((queue_size == INFINITE)?"Unlimited" : "Bounded")
    << " async_queue finished producing" << " in " << fixed
    << setprecision (2) << t_prod.msecEnd ()/1000. << "sec" << endl;

  //Wait for consumers to finish
  for (int i = 0; i < NTHREADS; i++)
    consumers[i]->wait ();
  t_cons.stop ();
  cout << "finished consuming" << " in " << fixed
    << setprecision (2) << t_cons.msecEnd () / 1000. << "sec" << endl;

  //Did we find all the primes?
  cout << "Expecting " << checks[CASE].n_primes << " primes, found " 
    << primes.size () <<endl;

  //Check who did what
  vector<int> found_by(NTHREADS);
  while (!primes.empty ())
  {
    result r;
    primes.consume (r);
    found_by[r.worker]++;
  }

  //Show consumers statistics
  for (int i = 0; i < NTHREADS; i++)
    cout << "Consumer " << i << " found " << found_by[i] << " primes." << endl;
}


int main ()
{
  //run test with an unbounded queue...
  primes_queue (INFINITE);

  //... then with a bounded queue
  primes_queue (20);

  return 0;
}
