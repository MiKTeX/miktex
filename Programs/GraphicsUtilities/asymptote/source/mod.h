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
// Implementation-independent definition of mod; ensure that result has
// same sign as divisor
  T val=Mod(x,y);
  if((y > 0 && val < 0) || (y < 0 && val > 0)) val += y;
  return val;
}
  
inline Int imod(Int x, Int y)
{
  return portableMod<Int>(x,y);
}

inline Int imod(Int i, size_t n) {
  i %= (Int) n;
  if(i < 0) i += (Int) n;
  return i;
}

#endif
