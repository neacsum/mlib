#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <vector>
#include <iostream>
#include <random>
#include <set>
#include <functional>
#include <list>

using namespace mlib;
using namespace std;

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
  bool operator == (const counted_int& rhs) const {
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
      // Calls object's default constructor once for each object in container.
      ring_buffer <counted_int> testbuf (10);
      CHECK_EQUAL (10, counted_int::counter);
    }
    // VERIFY: when ring buffer goes out of scope all elements are destroyed.
    CHECK_EQUAL (0, counted_int::counter);
  }

  TEST (ctor_dtor2)
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
    ring_buffer <int> testbuf (10);
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
    CHECK (empty1.empty () && empty2.empty ());
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
  TEST (const_buffer)
  {
    ring_buffer<int> b1 (10);
    for (int i = 1; i <= 10; i++)
      b1.push_back (i);

    const ring_buffer<int> b2 (b1);
    auto p1 = b1.cbegin ();
    auto p2 = b2.begin ();

    while (p1 != b1.cend ())
    {
      CHECK (*p1++ == *p2++);
    }
  }

  TEST (begin)
  {
    ring_buffer<int> testbuf (10);
    testbuf.push_back (100);
    testbuf.push_back (101);

    auto iter = testbuf.begin ();

    // VERIFY: begin points to first object pushed
    CHECK_EQUAL (100, *iter);

    iter = --testbuf.begin ();

    // VERIFY: begin iterator doesn't change when decremented
    CHECK (iter == testbuf.begin ());

    iter--;

    // VERIFY: postfix decrement doesn't change begin iterator
    CHECK (iter == testbuf.begin ());
  }

  TEST (end)
  {
    ring_buffer<int> testbuf (10);
    testbuf.push_back (100);
    testbuf.push_back (101);

    auto last = --testbuf.end ();

    // VERIFY: pointer to last object becomes end after increment
    CHECK (++last == testbuf.end ());

    // VERIFY: dereferencing end pointer throws exception
    CHECK_THROW (*last == 0, std::range_error);


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
    const ring_buffer<int> testbuf ({ 100, 101, 102 });

    auto it1 = testbuf.begin ();

    // VERIFY: increment and addition give the same result
    auto it2 = it1++ + 1;
    CHECK (it1 == it2);

    // VERIFY: addition works
    int v = *(testbuf.begin () + 2);
    CHECK_EQUAL (102, v);

    // VERIFY: addition stops at end
    it2 = it1 + 5;
    CHECK (it2 == std::end (testbuf));
  }

  TEST (addition2)
  {
    ring_buffer<int> t (10);
    for (int i = 0; i < 5; i++)
      t.push_back (100 + i);

    for (int i = 0; i < 10; i++)
    {
      auto ptr = t.begin () + 4;
      cout << "i=" << i << " *ptr=" << *ptr << endl;
      t.push_back (200+i);
      t.pop_front ();
    }
    cout << endl;
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

  TEST (subtraction2)
  {
    ring_buffer<int> t (10);
    for (int i = 0; i < 5; i++)
      t.push_back (100 + i);

    for (int i = 0; i < 10; i++)
    {
      auto ptr = t.end () - 2;
      cout << "i=" << i << " *ptr=" << *ptr << endl;
      t.push_back (200 + i);
      t.pop_front ();
    }
    cout << endl;

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

  TEST (difference_op)
  {
    ring_buffer<int> testbuf ({ 100, 101, 102 });
    size_t d;

    //VERIFY: end - begin == size when buffer full
    d = testbuf.end () - testbuf.begin ();
    CHECK_EQUAL (testbuf.size (), d);

    //VERIFY: end - begin  == size after popping one object
    testbuf.pop_front ();
    d = testbuf.end () - testbuf.begin ();
    CHECK_EQUAL (testbuf.size () , d);

    //VERIFY: end - begin == 0 when buffer empty
    testbuf.clear ();
    d = testbuf.end () - testbuf.begin ();
    CHECK_EQUAL (0, d);

    testbuf.push_back (0);
    for (size_t i = 0; i < testbuf.capacity (); i++)
    {
      //VERIFY: end - begin == 1 for different positions in buffer
      d = testbuf.end () - testbuf.begin ();
      CHECK_EQUAL (1, d);
      testbuf.push_back ((int)i+1);
      testbuf.pop_front ();
    }
  }

// Usage and performance samples

  //Using ring buffer with a standard algorithm
  TEST (find_algorithm)
  {
    const ring_buffer<int> testbuf ({ 100, 101, 102 });

    auto it = std::find (testbuf.begin (), testbuf.end (), 101);

    //VERIFY: found second element
    CHECK (testbuf.begin () + 1 == it);
  }

  //Using ring buffer with a range for sentence
  TEST (for_loop)
  {
    const ring_buffer<int> testbuf ({ 100, 101, 102 });

    std::cout << "Circular buffer: ";
    for (auto i : testbuf)
      std::cout << i << " ";
    std::cout << "\n";
  }

  /*
    Testing performance of ring buffer
    Using code inspired from "Performance of a Circular Buffer vs. Vector, Deque, and List"
    (https://www.codeproject.com/Articles/1185449/Performance-of-a-Circular-Buffer-vs-Vector-Deque-a)
  */

  //key-value structure used for performance testing 
  struct kvstruct {
    char key[9];
    unsigned value; //  could be anything at all
    kvstruct (unsigned k = 0) : value (k)
    {
      char buf[9];
      strcpy_s (key, stringify (k, buf));
    }

    kvstruct (kvstruct const& other) : value (other.value) {
      strcpy_s (key, other.key);
    }

    static char const* stringify (unsigned i, char* buf) {
      buf[8] = 0;
      char* bufp = &buf[8];
      do {
        *--bufp = "0123456789ABCDEF"[i & 0xf];
        i >>= 4;
      } while (i != 0);
      return bufp;
    }

    bool operator<(kvstruct const& that)  const { return strcmp (this->key, that.key) < 0; }
    bool operator==(kvstruct const& that) const { return strcmp (this->key, that.key) == 0; }
  };

  //Fill a vector with randomly ordered kvstruct objects
  void build_random_vector (std::vector<kvstruct>& v, unsigned count)
  {
    unsigned i;
    for (i = 0; i<count; i++)
      v.push_back (kvstruct (i));

    //Fisher-Yates shuffle using Durstenfeld algorithm
    std::default_random_engine eng;
    for (i = count - 1; i > 0; i--)
    {
      std::uniform_int_distribution<unsigned> d (0, i);
      unsigned r = d (eng);
      std::swap (v[r], v[i]);
    }
  }

  TEST (assignment_timming)
  {
    UnitTest::Timer t;
    int ms;
#ifdef _DEBUG
    size_t sz = 1000000;
#else
    size_t sz = 10000000;
#endif
    std::vector<kvstruct> random_vector;

    t.Start ();
    build_random_vector (random_vector, (unsigned int)sz);
    std::cout << "Random vector prepared in " << t.GetTimeInMs () << "ms\n";

    {
      ring_buffer<kvstruct> test_container (sz);

      t.Start ();
      for (auto& kv : random_vector)
        test_container.push_back (kv);
      int ms = t.GetTimeInMs ();
      std::cout << "ring_buffer push_back of " << sz << " elements in " << ms << "ms\n";
      std::cout << "size is " << sz * sizeof (kvstruct) / 1024 << "kb\n";
    }

    {
      std::vector<kvstruct> test_container;

      t.Start ();
      for (auto const& kv : random_vector)
        test_container.push_back (kv);
      ms = t.GetTimeInMs ();
      std::cout << "vector push_back of " << sz << " elements in " << ms << "ms\n";
    }

    {
      std::vector<kvstruct> test_container;
      test_container.reserve (sz);

      t.Start ();
      for (auto const& kv : random_vector)
        test_container.push_back (kv);
      ms = t.GetTimeInMs ();
      std::cout << "vector with reserve push_back of " << sz << " elements in " << ms << "ms\n";
    }

    {
      std::list<kvstruct> test_container;

      t.Start ();
      for (auto& kv : random_vector)
        test_container.push_back (kv);
      ms = t.GetTimeInMs ();
      std::cout << "list push_back of " << sz << " elements in " << ms << "ms\n";
    }

    {
      ring_buffer<kvstruct> test_container (sz);
      std::vector<kvstruct> test_vector;

      for (auto& kv : random_vector)
        test_container.push_back (kv);

      t.Start ();
      test_vector = test_container;
      ms = t.GetTimeInMs ();
      std::cout << "ring to vector conversion of " << sz << " elements in " << ms << "ms\n";
      CHECK (test_vector == random_vector);
    }
  }
}
