#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop

#include <iostream>
#include <iomanip>

using namespace mlib;
using namespace std;

SUITE (sync_queue) {

TEST (ballroom)
{
  const int pairs = 10;
  manual_event ball_start;
  LONG finished = 0;

  sync_queue<int> pairing;

  mlib::thread* producer[pairs];
  mlib::thread* consumer[pairs];
  criticalsection use_cout;

  for (int i = 0; i < pairs; i++)
  {
    producer[i] = new mlib::thread ([&, i]()->int {
      ball_start.wait ();
      pairing.produce (i);
      return 0;
      });
    consumer[i] = new mlib::thread ([&, i]()->int {
      ball_start.wait ();
      int v;
      pairing.consume (v);
      {
        mlib::lock l (use_cout);
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

TEST (shared_pointer)
{
  typedef async_queue<int> int_queue;
  typedef std::shared_ptr<int_queue> queue_ptr;

  auto ptr1 = std::make_shared<int_queue> (20);
  auto ptr2 = ptr1;

  ptr1->produce (10);
  int x;
  ptr2->consume (x, 0);
  CHECK_EQUAL (10, x);
}

// verify consuming with a timeout limit
TEST (water_drops)
{
  /* Producer faucet drips a drop of water every `drop-interval`. Consumer
  waits `wait-time` to catch a drop.*/
  const int drops = 10;
  const int drop_interval = 100;
  const int wait_time = 50;
  int missed = 0;
  async_queue<int> faucet;
  mlib::thread producer ([&]()->int {
    for (int i = 0; i < drops; i++)
    {
      faucet.produce (i+1);
      Sleep (drop_interval);
    }
    faucet.produce (-1);
    return 0;
  });

  mlib::thread consumer ([&]()->int {
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
  /* A limited capacity silo receives a bushel of grain every `fill-rate`.
  Consumer picks up a bushel every `empty-interval`.*/
  const int bushels = 10;
  const int empty_interval = 500;
  const int fill_rate = 50;
  int missed = 0;

  async_queue<int> silo(5);

  mlib::thread producer ([&]()->int {
    for (int i = 0; i < bushels; i++)
    {
      Sleep (fill_rate);
      if (!silo.produce (i + 1, 0))
        missed++;
    }

    //make sure the end signal is caught
    while (silo.full ())
      Sleep (10);
    silo.produce (-1);
    return 0;
    });

  mlib::thread consumer ([&]()->int {
    int v;
    std::vector<int> consumed;

    do
    {
      Sleep (empty_interval);
      silo.consume (v);
      consumed.push_back (v);
    } while (v >= 0);
    cout << "Consumed: ";
    for (auto& v : consumed)
      cout << v << ' ';
    cout << std::endl;

    return 0;
    });

  consumer.start ();
  producer.start ();
  consumer.wait ();
  std::cout << "Silo - producer missed " << missed << " bushels" << endl;
  CHECK (missed > 0);
}

TEST (ringbuf_basic)
{
  async_buffer<int> buf (3);

  CHECK (buf.empty ());
  buf.produce (1);
  buf.produce (2);
  CHECK (!buf.empty ());
  CHECK (!buf.full ());
  buf.produce (3);
  CHECK (buf.full ());

  buf.clear ();
  CHECK (buf.empty ());

  int val;
  CHECK (!buf.consume (val, 0));
}

TEST (ringbuf_silo)
{
  /* Same as above but silo is a ring buffer where oldest bushels are thrown out.*/
  const int bushels = 10;
  const int empty_interval = 500;
  const int fill_rate = 50;
  std::vector<int> consumed;

  async_buffer<int> silo (5);
  
  mlib::thread producer ([&] () -> int {
    for (int i = 0; i < bushels; i++)
    {
      Sleep (fill_rate);
      silo.produce (i+1);
    }
    silo.produce (-1);
    return 0;
  });

  mlib::thread consumer ([&] () -> int {
    int v;
    do
    {
      Sleep (empty_interval);
      silo.consume (v);
      consumed.push_back (v);
    } while (v >= 0);

    return 0;
  });

  consumer.start ();
  producer.start ();
  consumer.wait ();
  cout << "Consumed: ";
  for (auto& v : consumed)
    cout << v << ' ';
  cout << std::endl;
  CHECK (consumed.size () < bushels);
}

} //end suite
