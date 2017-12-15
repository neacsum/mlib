#include <utpp/utpp.h>
#include <mlib/serenum.h>

using namespace std;
using namespace mlib;

TEST (SerEnum_Test)
{
  vector<int>ports, ports1, ports2;
  vector<string> names1;
  UnitTest::Timer timer;
  int dt;

  timer.Start ();
  SerEnum_UsingCreateFile (ports);
  dt = timer.GetTimeInMs ();
  printf ("\nCOM Ports according to CreateFile (%d msec):\n", dt);
  for (int i = 0; i < ports.size (); i++)
    printf ("COM%d\n", ports[i]);

  timer.Start ();
  SerEnum_UsingSetupAPI (ports1, names1);
  dt = timer.GetTimeInMs ();
  CHECK_EQUAL (ports.size (), ports1.size());
  printf ("\nCOM Ports according to SetupAPI (%d msec):\n", dt);
  for (int i = 0; i < ports1.size (); i++)
    printf ("COM%d - %s\n", ports1[i], names1[i].c_str ());

  timer.Start ();
  SerEnum_UsingRegistry (ports2);
  dt = timer.GetTimeInMs ();
  CHECK_EQUAL (ports.size (), ports2.size ());
  printf ("\nCOM Ports according to Registry (%d msec):\n", dt);
  for (int i = 0; i < ports2.size (); i++)
    printf ("COM%d\n", ports2[i]);
}