#include <mlib/mlib.h>
#include <utpp/utpp.h>
#pragma hdrstop

#include <vector>

using namespace mlib;
using namespace std::literals::chrono_literals;

SUITE (syncro)
{

TEST (wait_any_infinite)
{
  auto_event e1, e2;
  thread th ([&]()-> unsigned int {
    Sleep (50); e1.signal (); return 0; });

  th.start ();
  unsigned int ret = wait_any ({ &e1, &e2 });
  th.wait ();
  CHECK_EQUAL (WAIT_OBJECT_0, ret);
}

TEST (wait_any_timeout)
{
  auto_event e1, e2;
  thread th ([&]()-> unsigned int {
    Sleep (100); e1.signal (); return 0; });

  th.start ();
  auto to = 50000us;
  unsigned int ret = wait_any ({ &e1, &e2 }, 
    std::chrono::duration_cast<std::chrono::milliseconds>(to));
  th.wait ();
  CHECK_EQUAL (WAIT_TIMEOUT, ret);
}

//
TEST (mutex_no_leaks)
{
  DWORD initial_handle_count, final_handle_count;
  GetProcessHandleCount (GetCurrentProcess (), &initial_handle_count);
  
  std::vector<mutex> mv(10);
  GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);

  mv.clear();
  GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);

  CHECK_EQUAL (initial_handle_count, final_handle_count);
}


TEST (duplicate_has_same_handle)
{

  DWORD initial_handle_count, final_handle_count;
  GetProcessHandleCount (GetCurrentProcess (), &initial_handle_count);
  {
    std::vector<mlib::mutex> mv;
    mv.emplace_back ();

    for (int i = 1; i < 10; i++)
      mv.insert (mv.end (), mv[0]);

    GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
    CHECK_EQUAL (initial_handle_count + 1, final_handle_count);
  }

  GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
  CHECK_EQUAL (initial_handle_count, final_handle_count);
}

TEST (move_ctor)
{
  DWORD initial_handle_count, final_handle_count;
  GetProcessHandleCount (GetCurrentProcess (), &initial_handle_count);
  {
    mutex m1;
    mutex m2 = std::move (m1);

    GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
    CHECK_EQUAL (initial_handle_count + 1, final_handle_count);
  }
  GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
  CHECK_EQUAL (initial_handle_count, final_handle_count);
}

TEST (move_assignment)
{
  DWORD initial_handle_count, final_handle_count;
  GetProcessHandleCount (GetCurrentProcess (), &initial_handle_count);
  {
    mutex m1, m2;
    m2 = std::move (m1);

    GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
    CHECK_EQUAL (initial_handle_count + 1, final_handle_count);
  }
  GetProcessHandleCount (GetCurrentProcess (), &final_handle_count);
  CHECK_EQUAL (initial_handle_count, final_handle_count);
}

TEST (wait_duration)
{
  auto_event evt;
  auto ret = evt.wait (50ms);
  CHECK_EQUAL (WAIT_TIMEOUT, ret);
}

TEST (event_is_signaled)
{
  auto_event evt_auto;

  evt_auto.signal ();

  CHECK (evt_auto.is_signaled ());

  //calling is_signaled doesn't change state
  CHECK (evt_auto.is_signaled ());

  //a "real" wait resets the event
  evt_auto.wait (1ms);
  CHECK (!evt_auto.is_signaled ());

  manual_event evt_manual;
  evt_manual.signal ();
  CHECK (evt_manual.is_signaled ());
  
  // calling is_signaled doesn't change state
  CHECK (evt_manual.is_signaled ());

  // a "real" wait does not reset the event
  evt_manual.wait (1ms);
  CHECK (evt_manual.is_signaled ());

  evt_manual.reset ();
  CHECK (!evt_manual.is_signaled ());
}

}
