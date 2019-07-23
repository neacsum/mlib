/*!
  \file statpars.cpp Implementation of statistical parameters calculator class

  (c) Mircea Neacsu 2019
*/
#include <math.h>
#include <queue>
#include <mlib/statpars.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif
/*!
  \class statpars

  Use one of the add() functions to add new values to the calculator. Then you
  can call one of the functions that return a statistical parameter (average,
  variance, etc.). When one of these functions is called, all the other parameters
  are also computed. The computation is valid until a new value is added to the
  calculator.

  If the calculator object was constructed with a limited number of samples,
  only the last "n" samples are used to compute the statistical parameters.
*/

/*!
  Constructor for calculator class.

  \param n  number of samples. If not 0, only the last n samples are used
            for computations.
*/
statpars::statpars (int n) :
ave (0), adev (0), sdev (0), var (0), skew (0), kurt (0), calc (false), nmax (n)
{
}

/*!
  Internal function used to preform all computations.

  Formulas adapted from "Numerical Recipes in C", W. H. Press, S. A. Teukolsky,
  W. T. Vetterling, B. P. Flannery
*/
void statpars::calculate()
{
  
  double ep = 0., s, p;

  ave = sdev = adev = var = skew = kurt = 0.;

  int n = (int)values.size ();
  if (n <= 1) 
    return; //n must be at least 2

  //First pass to get the mean.
  for (auto vptr = values.begin(); vptr != values.end(); vptr++)
    ave += *vptr;
  ave /= n; 
  
  //Second pass to get the first (absolute), second, third, and fourth order moments
  for (auto vptr = values.begin (); vptr != values.end (); vptr++)
  {
    adev += fabs (s = *vptr - (ave));
    ep += s;
    var += (p = s * s);
    skew += (p *= s);
    kurt += (p *= s);
  }

  adev /= n;
  var = (var - ep * ep / n) / (n - 1);        //Corrected two-pass formula.

  //Put the pieces together according to the conventional definitions.
  sdev = sqrt (var);
  if (var)     //No skew/kurtosis when variance = 0
  {
    skew /= (n*(var)*(sdev));
    kurt /= (n*(var)*(var)) - 3.0;
  }
  calc = true;
}

#ifdef MLIBSPACE
};
#endif
