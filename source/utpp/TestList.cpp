#include <utpp/TestList.h>
#include <utpp/Test.h>

#include <cassert>

namespace UnitTest {

TestList::TestList ()
  : head (0)
  , tail (0)
{
}

void TestList::Add (Test* test)
{
  if (tail == 0)
  {
    assert (head == 0);
    head = test;
    tail = test;
  }
  else
  {
    tail->next = test;
    tail = test;
  }
}

Test* TestList::GetHead () const
{
  return head;
}

ListAdder::ListAdder (TestList& list, Test* test)
{
  list.Add (test);
}

}
