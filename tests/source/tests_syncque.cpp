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

} //end suite
