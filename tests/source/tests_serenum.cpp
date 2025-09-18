#include <mlib/mlib.h>
#include <utpp/utpp.h>
#pragma hdrstop

using namespace std;
using namespace mlib;

TEST (SerEnum_Test)
{
  vector<int>ports, ports1, ports2;
  vector<string> names1;
  UnitTest::Timer timer;

  timer.Start ();
  SerEnum_UsingCreateFile (ports);
  auto dt = timer.GetTimeInMs ();
  cout << "\nCOM Ports according to CreateFile (" << dt << "):\n";
  for (auto& i : ports)
    cout << "COM" << i << endl;

  timer.Start ();
  SerEnum_UsingSetupAPI (ports1, names1);
  dt = timer.GetTimeInMs ();
  CHECK_EQUAL (ports.size (), ports1.size());
  cout << "\nCOM Ports according to SetupAPI (" << dt << "):\n" ;
  for (size_t i = 0; i < ports1.size (); i++)
    cout << "COM" << ports1[i] << " - "<< names1[i] << endl;

  timer.Start ();
  SerEnum_UsingRegistry (ports2);
  dt = timer.GetTimeInMs ();
  CHECK_EQUAL (ports.size (), ports2.size ());
  cout << "\nCOM Ports according to Registry (" << dt <<"):\n";
  for (auto& i : ports2)
    cout << "COM" << i << endl;
}