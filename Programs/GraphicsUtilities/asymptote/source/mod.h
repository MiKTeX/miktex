/*****
 * mod.h
 * Andy Hammerlindl 2005/03/16
 *
 * Definition of implementation-independent mod function.
 *****/

#ifndef MOD_H
#define MOD_H

#include <cmath>

#include "common.h"

using std::fmod;

inline Int Mod(Int x, Int y) {return x % y;}
inline double Mod(double x, double y) {return fmod(x,y);}

template<typename T>
inline T portableMod(T x,T y)
{
// Implementation-independent definition of mod; ensure result has
// same sign as divisor
  T val=Mod(x,y);
  return ((y > 0 && val >= 0) || (y < 0 && val <= 0)) ? val : val+y;
}

inline Int imod(Int x, size_t y) {
  if((y & (y-1)) == 0)
    return x & (y-1); // Use mask if y is a power of two
  x %= (Int) y;
  return (x >= 0) ? x : x+(Int) y;
}

inline Int imod(Int x, Int y)
{
  if(y > 0)
    return imod(x,(size_t) y);
  else
    return portableMod<Int>(x,y);
}

#endif
