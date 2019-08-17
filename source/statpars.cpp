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
values (n), sum (0), adev (0), sdev (0), var (0), skew (0), kurt (0), calc (false), nmax (n)
{
}

/*!
  Constructor for calculator class.

  \param nvec vector of samples.
*/
statpars::statpars (std::vector<double> vec) :
  values (vec.size()), nmax (vec.size()), sum (0), adev (0), sdev (0), var (0), skew (0), kurt (0), calc (false)
{
  add (vec);
}

/*!
  Internal function used to preform computations for higher order moments.

  Formulas adapted from "Numerical Recipes in C", W. H. Press, S. A. Teukolsky,
  W. T. Vetterling, B. P. Flannery
*/
void statpars::calculate()
{
  
  double ep = 0., s, p, ave;

  sdev = adev = var = skew = kurt = 0.;

  int n = (int)values.size ();
  if (n <= 1) 
    return; //n must be at least 2

  ave = sum / n;

  //Second pass to get the first absolute, second, third, and fourth order moments
  for (auto v : values)
  {
    adev += fabs (s = v - (ave));
    ep += s;
    var += (p = s * s);
    skew += (p *= s);
    kurt += (p *= s);
  }

  adev /= n;
  var = (var - ep * ep / n) / (n - 1);        //Corrected two-pass formula.

  //Put the pieces together according to Excel definitions.
  sdev = sqrt (var);
  if (var && n > 2)     //No skew/kurtosis when variance = 0
  {
    /* This is adjusted Fisher-Pearson standardized moment coefficient, different from Numerical Recipes value
    (that one is called Fisher-Pearson coefficient of skewness) but it matches Excel formula*/
    skew = n * skew / (var*sdev*(n - 1)*(n - 2));
    if (n > 3)
      /* Here again we use the Excel formula instead of Numerical Recipes one.*/
      kurt = (n * (n + 1)*kurt / (var*var*(n - 1)) - 3.0*(n - 1)*(n - 1)) / ((n-2)*(n-3));
  }
  calc = true;
}

#ifdef MLIBSPACE
};
#endif
