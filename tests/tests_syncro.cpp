#include <utpp/utpp.h>
#include <mlib/mutex.h>
#include <vector>

using namespace mlib;

SUITE (syncro)
{

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
