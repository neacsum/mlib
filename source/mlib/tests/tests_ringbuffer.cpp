#include <utpp/utpp.h>
#include <mlib/ringbuf.h>
#include <vector>

SUITE (RingBuffer)
{
  const int BUFSZ = 10;

  struct make_intbuf {
    ring_buffer <int> intbuf;
    make_intbuf () : intbuf (BUFSZ) {}
    void fill (int sz)
    {
      for (int i = 0; i < sz; i++)
        intbuf.push_back (i);
    }
  };

  TEST_FIXTURE (make_intbuf, Constructor)
  {
    intbuf.push_back (1);

    ring_buffer<int>::iterator it = intbuf.front ();
    int front_val = *it;
    CHECK_EQUAL (1, front_val);
  }

  TEST_FIXTURE (make_intbuf, Full)
  {
    for (int i = 0; i < BUFSZ; i++)
    {
      CHECK (!intbuf.full ());
      intbuf.push_back (i);
      CHECK_EQUAL (i + 1, intbuf.size ());
    }
    CHECK (intbuf.full ());

    for (int i = 0; i < 5; i++)
    {
      intbuf.push_back (i);
      CHECK (intbuf.full ());
    }
  }

  TEST_FIXTURE (make_intbuf, Empty)
  {
    CHECK (intbuf.empty ());

    //fill and overfill buffer
    for (int i = 0; i < BUFSZ+5; i++)
    {
      intbuf.push_back (i);
      CHECK (!intbuf.empty ());
    }

    for (int i = 0; i < BUFSZ; i++)
    {
      CHECK (!intbuf.empty ());
      intbuf.pop_front ();
    }
    CHECK (intbuf.empty ());
  }

  TEST_FIXTURE (make_intbuf, Size)
  {
    //until buffer is full size increases with each push
    for (int i = 0; i < BUFSZ; i++)
    {
      CHECK_EQUAL (i, intbuf.size ());
      intbuf.push_back (i);
    }

    //size doesn't increase when buffer is full
    for (int i = 0; i < 5; i++)
    {
      CHECK_EQUAL (BUFSZ, intbuf.size ());
      intbuf.push_back (i);
    }

    //size goes down when popping elements
    for (int i = BUFSZ; i > 0; i--)
    {
      CHECK_EQUAL (i, intbuf.size ());
      intbuf.pop_front ();
    }
    CHECK (intbuf.empty ());
  }

  TEST_FIXTURE (make_intbuf, front_iterator)
  {
    fill (BUFSZ);

    //postfix operator
    auto it = intbuf.front ();
    int val;
    for (int i = 0; i < BUFSZ; i++)
    {
      val = *it++;
      CHECK_EQUAL (i, val);
    }

    //cannot increment past back
    for (int i = 0; i < 5; i++)
    {
      val = *it++;
      CHECK_EQUAL (BUFSZ-1, val);
    }

    //repeat for prefix operator
    it = intbuf.front ();
    for (int i = 0; i < BUFSZ; i++)
    {
      val = *it;
      CHECK_EQUAL (i, val);
      ++it;
    }

    for (int i = 0; i < 5; i++)
    {
      ++it;
      val = *it;
      CHECK_EQUAL (BUFSZ - 1, val);
    }
  }

  TEST_FIXTURE (make_intbuf, back_iterator)
  {
    fill (BUFSZ);

    auto it = intbuf.back ();
    int val;
    for (int i = BUFSZ - 1; i >= 0; i--)
    {
      val = *it--;
      CHECK_EQUAL (i, val);
    }

    //cannot decrement past front
    for (int i = 0; i < 5; i++)
    {
      val = *it--;
      CHECK_EQUAL (0, val);
    }

    //repeat for prefix operator
    it = intbuf.back ();
    for (int i = BUFSZ - 1; i >= 0; i--)
    {
      val = *it;
      CHECK_EQUAL (i, val);
      --it;
    }

    for (int i = 0; i < 5; i++)
    {
      val = *--it;
      CHECK_EQUAL (0, val);
    }
  }

  //Same test as front_iterator but with buffer half full
  TEST_FIXTURE (make_intbuf, front_iterator1)
  {
    fill (BUFSZ / 2);

    //postfix operator
    auto it = intbuf.front ();
    int val;
    for (int i = 0; i < BUFSZ/2; i++)
    {
      val = *it++;
      CHECK_EQUAL (i, val);
    }

    //cannot increment past back
    for (int i = 0; i < 5; i++)
    {
      val = *it++;
      CHECK_EQUAL (BUFSZ/2 - 1, val);
    }

    //repeat for prefix operator
    it = intbuf.front ();
    for (int i = 0; i < BUFSZ/2; i++)
    {
      val = *it;
      CHECK_EQUAL (i, val);
      ++it;
    }

    for (int i = 0; i < 5; i++)
    {
      ++it;
      val = *it;
      CHECK_EQUAL (BUFSZ/2 - 1, val);
    }
  }

  TEST_FIXTURE (make_intbuf, Comparison)
  {
    fill (BUFSZ);
    std::vector<int> vi, vo;

    for (int i = 0; i < BUFSZ; i++)
      vi.push_back (i);

    // a nice template to iterate the whole buffer
    auto it = intbuf.front ();
    do 
    {
      vo.push_back (*it);
    } while (it != it++);

    CHECK (vi == vo);
  }

  TEST_FIXTURE (make_intbuf, Copy_constructor)
  {
    fill (BUFSZ + 5);

    ring_buffer <int>copybuf (intbuf);

    auto it = intbuf.front ();
    auto it1 = copybuf.front ();
    do
    {
      CHECK_EQUAL (*it, *it1++);
    } while (it != it++);
  }

  TEST_FIXTURE (make_intbuf, Assignment_operator)
  {
    fill (BUFSZ + 5);
    ring_buffer <int>otherbuf (5);

    otherbuf = intbuf;

    auto it = intbuf.front ();
    auto it1 = otherbuf.front ();
    do
    {
      CHECK_EQUAL (*it, *it1++);
    } while (it != it++);
  }
}

