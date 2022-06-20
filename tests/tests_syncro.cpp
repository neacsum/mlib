#include <utpp/utpp.h>
#include <mlib/mutex.h>
#include <mlib/event.h>
#include <mlib/thread.h>

#include <vector>

using namespace mlib;
using namespace std::literals::chrono_literals;

SUITE (syncro)
{

TEST (wait_any_infinite)
{
  event e1, e2;
  thread th ([&]()-> unsigned int {
    Sleep (50); e1.signal (); return 0; });

  th.start ();
  unsigned int ret = wait_any ({ &e1, &e2 });
  th.wait ();
  CHECK_EQUAL (WAIT_OBJECT_0, ret);
}

TEST (wait_any_timeout)
{
  event e1, e2;
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

}
