#include <utpp/utpp.h>
#include <mlib/ringbuf.h>
#include <vector>
#include <iostream>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

///Used to count ctor/dtor calls
struct counted_int {
  counted_int (int i=0) : val (i), inst (counter++) {
#ifdef COUNTER_DETAILS
    std::cout << "ctor int(" << inst << ")=" << val << "\n";
#endif
  }
  counted_int (const counted_int& other) : val (other.val), inst (counter++) {
#ifdef COUNTER_DETAILS
    std::cout << "copy ctor int(" << inst << ")=" << val << " copied from " << other.inst << "\n";
#endif
  }
  counted_int& operator =(const counted_int& rhs) {
    val = rhs.val;
    return *this;
  }
  bool operator == (const counted_int& rhs) {
    return val == rhs.val;
  }
  ~counted_int () {
    --counter; 
#ifdef COUNTER_DETAILS
    std::cout << "dtor int(" << inst << ")=" << val << " remain=" << counter << "\n";
#endif
  }

  operator int () { return val; }

  int val, inst;
  static int counter;
};

int counted_int::counter = 0;

SUITE (RingBuffer)
{
  TEST (ctor_dtor1)
  {
    counted_int::counter = 0;
    {
      // VERIFY: ring_buffer constructor with a size argument calls 
      // object's default constructor once for each object in container.
      ring_buffer <counted_int> testbuf (10);
      CHECK_EQUAL (10, counted_int::counter);
    }
    // VERIFY: when ring buffer goes out of scope all elements are destroyed.
    CHECK_EQUAL (0, counted_int::counter);
  }

  TEST(ctor_dtor2)
  {
    counted_int::counter = 0;
    ring_buffer <counted_int> emptybuf;
    // VERIFY: default constructor doesn't call object constructor.
    CHECK_EQUAL (0, counted_int::counter);
  }

  TEST (copy_ctor1)
  {
    counted_int::counter = 0;
    ring_buffer <counted_int> testbuf (10);
    ring_buffer <counted_int> otherbuf (testbuf);
    
    // VERIFY: object constructor called for each element in container
    CHECK_EQUAL (20, counted_int::counter);
  }

  TEST (copy_ctor2)
  {
    counted_int::counter = 0;
    ring_buffer <counted_int> testbuf;
    ring_buffer <counted_int> otherbuf (testbuf);

    // VERIFY: object constructor not called if source container is empty
    CHECK_EQUAL (0, counted_int::counter);
  }

  TEST (copy_ctor3)
  {
    ring_buffer <int> testbuf(10);
    for (int i = 1; i <= 10; i++)
      testbuf.push_back (i);

    ring_buffer <int> otherbuf (testbuf);

    // VERIFY: copied objects are equal
    CHECK (testbuf == otherbuf);
  }

  TEST (copy_ctor4)
  {
    ring_buffer <int> testbuf;
    ring_buffer <int> otherbuf (testbuf);

    // VERIFY: empty copied containers are equal
    CHECK (testbuf == otherbuf);
  }

  TEST (assignment_op1)
  {
    counted_int::counter = 0;
    ring_buffer <counted_int> testbuf (10);
    for (int i = 1; i <= 10; i++)
      testbuf.push_back (i);

    ring_buffer <counted_int> otherbuf;
    testbuf = otherbuf;

    // VERIFY: assigned objects are equal
    CHECK (testbuf == otherbuf);
  }

  TEST (assignment_op2)
  {
    ring_buffer <counted_int> testbuf (10);
    for (int i = 1; i <= 10; i++)
      testbuf.push_back (i);

    ring_buffer <counted_int> emptybuf;
    testbuf = emptybuf;

    // VERIFY: assigning an empty container invokes object destructor for
    // all objects currently in container
    CHECK_EQUAL (0, counted_int::counter);

    // VERIFY: assigned container is empty
    CHECK (testbuf.empty ());
  }

  TEST (assignment_op3)
  {
    ring_buffer <counted_int> empty1;
    ring_buffer <counted_int> empty2;
    empty1 = empty2;

    // VERIFY: assigning empty to empty is OK
    CHECK (empty1.empty() && empty2.empty());
  }

  TEST (assignment_op4)
  {
    counted_int::counter = 0;
    {
      ring_buffer<counted_int> b1 ({ 100, 101, 102 });
      ring_buffer<counted_int> b2;

      b2 = b1;
      //VERIFY: new objects are constructed
      CHECK_EQUAL (6, counted_int::counter);

      //VERIFY: objects are equal
      CHECK (b1 == b2);
    }

    //VERIFY: all objects are destructed when containers are destructed
    CHECK (counted_int::counter == 0);
  }

  TEST (equal_op)
  {
    ring_buffer<int> buf1 (10);
    ring_buffer<int> buf2 (10);
    ring_buffer<int> buf3 (5);

    // VERIFY: empty container equals empty container of same size
    CHECK (buf1 == buf2);

    // VERIFY: containers of different sizes are not equal even if both are empty
    CHECK (buf1 != buf3);

    for (int i = 1; i <= 10; i++)
    {
      buf1.push_back (i);
      buf2.push_back (i);

      // VERIFY: containers are equal while filling up
      CHECK (buf1 == buf2);
    }

    for (int i = 1; i <= 10; i++)
      buf2.push_back (i);

    // VERIFY: containers are equal after one of them overflows
    CHECK (buf1 == buf2);
  }

  TEST (push_back)
  {
    ring_buffer<int> testbuf (10);
    testbuf.push_back (100);

    // VERIFY: size is increased after push_back
    CHECK_EQUAL (1, testbuf.size ());

    // VERIFY: back element is the one pushed
    CHECK_EQUAL (100, testbuf.back ());

    auto bptr = testbuf.begin ();

    // VERIFY: begin points to the object pushed
    CHECK_EQUAL (100, *bptr);

    auto eptr = testbuf.end ();

    // VERIFY: end points one after the object pushed
    CHECK_EQUAL (100, *--eptr);
  }

  TEST (begin)
  {
    ring_buffer<int> testbuf (10);
    testbuf.push_back (100);
    testbuf.push_back (101);

    auto iter = testbuf.begin ();

    // VERIFY: begin points to first object pushed
    CHECK_EQUAL (100, *iter);

    iter == --testbuf.begin ();

    // VERIFY: begin iterator doesn't change when decremented
    CHECK (iter == testbuf.begin());

    iter--;

    // VERIFY: postfix decrement doesn't change begin iterator
    CHECK (iter == testbuf.begin());
  }

  TEST (end)
  {
    ring_buffer<int> testbuf (10);
    testbuf.push_back (100);
    testbuf.push_back (101);

    auto last = --testbuf.end ();

    // VERIFY: pointer to last object becomes end after increment
    CHECK (++last == testbuf.end ());

    last--;

    // VERIFY: postfix decrement brings pointer to last object pushed
    CHECK_EQUAL (101, *last);

    last++;

    // VERIFY: postfix increment brings pointer to end
    CHECK (last == testbuf.end ());

    // VERIFY: prefix increment doesn't change end pointer
    CHECK (++last == testbuf.end ());

    last++;

    // VERIFY: postfix increment doesn't change end pointer
    CHECK (last == testbuf.end ());
  }

  TEST (addition)
  {
    const ring_buffer<int> testbuf ({100, 101, 102});

    auto it1 = testbuf.begin();
    
    // VERIFY: increment and addition give the same result
    auto it2 = it1++ + 1;
    CHECK (it1 == it2);

    // VERIFY: addition works
    int v = *(testbuf.begin() + 2);
    CHECK_EQUAL (102, v);

    // VERIFY: addition stops at end
    it2 = it1 + (testbuf.size ()-1);
    CHECK (it2 == std::end (testbuf));
  }

  TEST (subtraction)
  {
    const ring_buffer<int> testbuf ({ 100, 101, 102 });

    auto it1 = ++testbuf.begin ();

    // VERIFY: decrement and subtraction give same result
    auto it2 = it1-- - 1;
    CHECK (it1 == it2);

    // VERIFY: subtraction works
    int v = *(testbuf.end () - 2);
    CHECK_EQUAL (101, v);
  }

  TEST (resize)
  {
    counted_int::counter = 0;
    ring_buffer<counted_int> testbuf ({ 100, 101, 102 });

    testbuf.resize (5);
    //VERIFY: number of objects created equals new container size
    CHECK_EQUAL (5, counted_int::counter);

    //VERIFY: resize can deallocate container
    testbuf.resize (0);
    CHECK_EQUAL (0, testbuf.capacity ());
    CHECK_EQUAL (0, counted_int::counter);

    //VERIFY: resize can reallocate container
    testbuf.resize (5);
    CHECK_EQUAL (5, testbuf.capacity ());
    CHECK_EQUAL (5, counted_int::counter);
  }

  TEST (full)
  {
    ring_buffer<int> testbuf ({ 100, 101, 102 });

    //VERIFY: container created from initializer list is full (size == number of elements in list)
    CHECK (testbuf.full ());

    //VERIFY: container remains full after pushing more objects
    testbuf.push_back (103);
    CHECK (testbuf.full ());

    //VERIFY: not full after popping one object
    testbuf.pop_front ();
    CHECK (!testbuf.full ());
  }

  TEST (empty)
  {
    ring_buffer<int> testbuf;

    //VERIFY: unallocated container is empty
    CHECK (testbuf.empty ());

    //VERIFY: newly allocated container is still empty
    testbuf.resize (3);
    CHECK (testbuf.empty ());

    //VERIFY: container not empty after inserting an object
    testbuf.push_back (100);
    CHECK (!testbuf.empty ());

    //VERIFY: container becomes empty after popping (last) object
    testbuf.pop_front ();
    CHECK (testbuf.empty ());
  }

  TEST (size)
  {
    ring_buffer<int> testbuf ({ 100, 101, 102 });

    //VERIFY: container created from initializer list has size and capacity == number of elements in list
    CHECK_EQUAL (3, testbuf.size ());
    CHECK_EQUAL (3, testbuf.capacity ());

    //VERIFY: inserting more objects doesn't increase size
    testbuf.push_back (103);
    CHECK_EQUAL (3, testbuf.size ());

    //VERIFY: size decreases when objects are popped
    testbuf.pop_front ();
    CHECK_EQUAL (2, testbuf.size ());
    testbuf.pop_front ();
    CHECK_EQUAL (1, testbuf.size ());
    testbuf.pop_front ();
    CHECK_EQUAL (0, testbuf.size ());
  }

  TEST (vector_op)
  {
    const ring_buffer<int> testbuf ({ 100, 101, 102 });
    const std::vector<int> expected ({ 100, 101, 102 });
    std::vector<int> actual;
    actual = testbuf;
    CHECK (expected == actual);
  }

  TEST (find_algorithm)
  {
    const ring_buffer<int> testbuf ({ 100, 101, 102 });

    auto it = std::find (testbuf.begin (), testbuf.end (), 101);

    //VERIFY: found second element
    CHECK (testbuf.begin () + 1 == it);

    *it = 104;
  }
}

