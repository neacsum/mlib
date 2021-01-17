/*
  YACB.CPP - Demo program for circular buffer class
  (c) Mircea Neacsu 2021. Licensed under MIT License.
  See README file for full license terms.
  (https://github.com/neacsum/mlib#readme)
*/
#include <mlib/ringbuf.h>
#include <vector>
#include <iostream>
#include <random>
#include <set>
#include <functional>
#include <list>
#include <time.h>

using namespace mlib;
using namespace std;

template <typename T>
void show_buffer (const ring_buffer<T>& b)
{
  for (auto& t : b)
    cout << t << ' ';
  cout << endl;
}

void performance_metrics ();

int main ()
{
  cout << "Constructor and element insertion" << endl;
  ring_buffer <int> b1 (10);
  for (int i = 1; i <= 10; i++)
    b1.push_back (i);
  show_buffer (b1);

  b1.push_back (101); b1.push_back (102);
  cout << "2 elements dropped:";
  show_buffer (b1);
  auto p = b1.begin ();
  cout << "*begin=" << *p << endl;
  p += 9;
  cout << "*(begin+9)=" << *p << endl;

  cout << endl << "Copy constructor" << endl;
  ring_buffer <int> b2 (b1);
  show_buffer (b2);

  cout << endl << "Assignment operator" << endl;
  ring_buffer <int> b3;
  b3 = b2;
  show_buffer (b3);

  cout << endl << "Using initializer list (3 elements)" << endl;
  const ring_buffer<int> b4 ({ 100, 101, 102 });
  show_buffer (b4);

  cout << endl << "Buffer size is " << b4.size () << "; buffer is "
    << (b4.full () ? "full" : "not full") << endl;
  cout << endl;

  auto b41 = b4;

  cout << endl << "New elements push old ones out" << endl;
  for (size_t i = 0; i < b4.size (); i++)
  {
    b41.push_back (0);
    show_buffer (b41);
    cout << endl;
  }

  cout << endl << "Buffers can be resized" << endl;
  ring_buffer<string> b5{ "abc", "def", "ghi" };
  show_buffer (b5);
  cout << "Buffer capacity is " << b5.capacity () << " and buffer size is " << b5.size ()
    << "; buffer is " << (b5.full () ? "full" : "not full") << endl;
  b5.resize (10);
  cout << "After resizing:" << endl;
  cout << "Buffer capacity is " << b5.capacity () << " and buffer size is " << b5.size ()
    << "; buffer is " << (b5.full () ? "full" : "not full") << endl;

  cout << endl << "Buffers can be assigned to vectors" << endl;
  std::vector<string> v;
  v = b5;
  int i = 0;
  for (auto& s : v)
    cout << "v[" << i++ << "]=" << s << ' ';
  cout << endl;

  ring_buffer<string> b6{ "first", "second", "third" };
  b6.pop_front ();
  b6.push_back ("fourth");
  v = b6;
  cout << endl << "Vectors are ordered from oldest to newest:" << endl;
  i = 0;
  for (auto& s : v)
    cout << "v[" << i++ << "]=" << s << ' ';
  cout << endl;

  cout << endl << "Buffers can be passed as arguments to standard algorithms" << endl;
  ring_buffer<int> b7{ 103, 102, 101 };
  cout << "Seraching for " << 102 << " in ";
  show_buffer (b7);
  auto pos = std::find (b7.begin (), b7.end (), 102);
  cout << "Find position is " << (pos - b7.begin ());
  cout << endl << endl;

  performance_metrics ();
  return 0;
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
  for (i = 0; i < count; i++)
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

void performance_metrics ()
{
#ifdef _DEBUG
  size_t sz = 1000000;
#else
  size_t sz = 10000000;
#endif
  std::vector<kvstruct> random_vector;

  clock_t ms = clock ();
  build_random_vector (random_vector, (unsigned int)sz);
  ms = clock () - ms;
  std::cout << "Random vector prepared in " << ms << "ms\n";
  ring_buffer<kvstruct> ring_container (sz);

  ms = clock ();
  for (auto& kv : random_vector)
    ring_container.push_back (kv);
  ms = clock () - ms;
  std::cout << "ring_buffer push_back of " << sz << " elements in " << ms << "ms\n";
  std::cout << "size is " << sz * sizeof (kvstruct) / 1024 << "kb\n";

  std::vector<kvstruct> vector_container;

  ms = clock ();
  for (auto& kv : random_vector)
    vector_container.push_back (kv);
  ms = clock () - ms;
  std::cout << "vector push_back of " << sz << " elements in " << ms << "ms\n";

  std::vector<kvstruct> allocvec;
  allocvec.reserve (sz);

  ms = clock ();
  for (auto& kv : random_vector)
    allocvec.push_back (kv);
  ms = clock () - ms;
  std::cout << "vector with reserve push_back of " << sz << " elements in " << ms << "ms\n";

  std::list<kvstruct> list_container;

  ms = clock ();
  for (auto& kv : random_vector)
    list_container.push_back (kv);
  ms = clock () - ms;
  std::cout << "list push_back of " << sz << " elements in " << ms << "ms\n";

  ring_buffer<kvstruct> ring (sz);
  std::vector<kvstruct> vec;

  for (auto& kv : random_vector)
    ring.push_back (kv);

  ms = clock ();
  vec = ring;
  ms = clock () - ms;
  std::cout << "ring to vector conversion of " << sz << " elements in " << ms << "ms\n";
}
