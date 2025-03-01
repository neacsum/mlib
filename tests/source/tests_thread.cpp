#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <iostream>

using namespace mlib;
using namespace std;

SUITE (threads)
{
  auto_event f_run;
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

    mlib::thread th1 (f1);
    th1.name ("f(1)");
    mlib::thread th2 (f2);
    th2.name ("f(2)");
    // if a breakpoint set on the next line, the "Threads" window of VS should 
    // show thread names as "f(1)" and "f(2)"
    th1.start ();
    th2.start ();
    wait_all ({&th1, &th2});
    CHECK_EQUAL (1, th1.result ());
  }

  TEST (thread_exception)
  {
    auto bad_func = []()->unsigned int {
      char t = std::string ().at (1); // this generates an std::out_of_range
      return 0;
    };

    mlib::thread th (bad_func);
    th.start ();
    CHECK_THROW (th.wait (), std::out_of_range);
  }

  TEST (thread_exception2)
  {
    auto bad_func = [] () -> unsigned int {
      Sleep (50);
      char t = std::string ().at (1); // this generates an std::out_of_range
      return 0;
    };

    mlib::thread th1 (bad_func), th2 (bad_func);
    th1.start ();
    th2.start ();

    CHECK_THROW (wait_all ({&th1, &th2}), std::out_of_range);
  }


  TEST (thread_states)
  {
    f_run.reset ();
    mlib::thread th (std::bind (f, 3));
    CHECK (!th.is_running ());
    CHECK (th.get_state () == mlib::thread::state::ready);
    th.start ();
    CHECK (th.get_state () == mlib::thread::state::starting || th.get_state () == mlib::thread::state::running);
    f_run.wait ();
    CHECK (th.get_state () == mlib::thread::state::running);
    CHECK (th.is_running ());
    th.wait ();
    CHECK (!th.is_running ());
    CHECK (th.get_state () == mlib::thread::state::finished);
  }

  TEST (ctor_current_thread)
  {
    current_thread me;
    cout << "Current thread id=0x" << hex << me.id () << endl;
    CHECK_EQUAL (GetCurrentThread(), me.handle ());
  }
}
