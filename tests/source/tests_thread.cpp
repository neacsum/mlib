#include <utpp/utpp.h>
#include <mlib/thread.h>
#include <iostream>

using namespace mlib;
using namespace std;

SUITE (threads)
{
  event f_run;
  int f (int i)
  {
    f_run.signal ();
    cout << "f(" << i << ") running." << endl;
    Sleep (200);
    return i;
  }

  TEST (bind)
  {
    auto f1 = std::bind (f, 1);
    auto f2 = std::bind (f, 2);

    thread th1 (f1);
    thread th2 (f2);
    th1.start ();
    th2.start ();
    std::vector<syncbase> p{ th1, th2 };
    multiwait (p);
    CHECK_EQUAL (1, th1.result ());

  }

  TEST (thread_states)
  {
    f_run.reset ();
    thread th (std::bind (f, 3));
    CHECK (!th.is_running ());
    CHECK (th.get_state () == thread::state::ready);
    th.start ();
    CHECK (th.get_state () == thread::state::starting);
    f_run.wait ();
    CHECK (th.get_state () == thread::state::running);
    CHECK (th.is_running ());
    th.wait ();
    CHECK (!th.is_running ());
    CHECK (th.get_state () == thread::state::finished);
  }

  TEST (ctor_current_thread)
  {
    current_thread me;
    cout << "Current thread id=0x" << hex << me.id () << endl;
    CHECK (me.is_running ());
  }

  class my_thread : public thread
  {
  public:
    my_thread (int arg1, int arg2);
  private:
    void run () { f (m1); };
    int m1, m2;
  };
}
