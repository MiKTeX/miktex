/*****
 * gsl.cc
 * 2010/05/19
 *
 * Initialize gsl builtins.
 *****/

#include "config.h"

#ifdef HAVE_LIBGSL

#include "vm.h"
#include "types.h"
#include "entry.h"
#include "builtin.h"

#include "record.h"
#include "stack.h"
#include "errormsg.h"
#include "array.h"
#include "triple.h"
#include "callable.h"

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_version.h>

#include "opsymbols.h"

#ifndef NOSYM
#include "gsl.symbols.h"
#endif

namespace trans {

using types::formal;
using types::primVoid;
using types::primInt;
using types::primReal;
using types::primPair;
using types::primTriple;
using types::primString;
using types::IntArray;
using types::realArray;
using types::stringArray;

using vm::stack;
using vm::array;
using vm::pop;
using vm::error;
using run::copyArrayC;
using run::copyCArray;

using camp::pair;
using camp::triple;

const char* GSLrngnull = "GSL random number generator not initialized";
const char* GSLinvalid = "invalid argument";

bool GSLerror=false;
types::dummyRecord *GSLModule;

types::record *getGSLModule()
{
  return GSLModule;
}

inline void checkGSLerror()
{
  if(GSLerror) {
    GSLerror=false;
    throw handled_error();
  }
}

template <double (*func)(double)>
void realRealGSL(stack *s)
{
  double x=pop<double>(s);
  s->push(func(x));
  checkGSLerror();
}

template <double (*func)(double, gsl_mode_t)>
void realRealDOUBLE(stack *s)
{
  double x=pop<double>(s);
  s->push(func(x,GSL_PREC_DOUBLE));
  checkGSLerror();
}

template <double (*func)(double, double, gsl_mode_t)>
void realRealRealDOUBLE(stack *s)
{
  double y=pop<double>(s);
  double x=pop<double>(s);
  s->push(func(x,y,GSL_PREC_DOUBLE));
  checkGSLerror();
}

template <double (*func)(unsigned)>
void realIntGSL(stack *s)
{
  s->push(func(unsignedcast(pop<Int>(s))));
  checkGSLerror();
}

template <double (*func)(int, double)>
void realIntRealGSL(stack *s)
{
  double x=pop<double>(s);
  s->push(func(intcast(pop<Int>(s)),x));
  checkGSLerror();
}

template <double (*func)(double, double)>
void realRealRealGSL(stack *s)
{
  double x=pop<double>(s);
  double n=pop<double>(s);
  s->push(func(n,x));
  checkGSLerror();
}

template <int (*func)(double, double, double)>
void intRealRealRealGSL(stack *s)
{
  double x=pop<double>(s);
  double n=pop<double>(s);
  double a=pop<double>(s);
  s->push(func(a,n,x));
  checkGSLerror();
}

template <double (*func)(double, double, double)>
void realRealRealRealGSL(stack *s)
{
  double x=pop<double>(s);
  double n=pop<double>(s);
  double a=pop<double>(s);
  s->push(func(a,n,x));
  checkGSLerror();
}

template <double (*func)(double, unsigned)>
void realRealIntGSL(stack *s)
{
  Int n=pop<Int>(s);
  double x=pop<double>(s);
  s->push(func(x,unsignedcast(n)));
  checkGSLerror();
}

// Add a GSL special function from the GNU GSL library
template<double (*fcn)(double)>
void addGSLRealFunc(symbol name, symbol arg1=SYM(x))
{
  addFunc(GSLModule->e.ve, realRealGSL<fcn>, primReal(), name,
          formal(primReal(),arg1));
}

// Add a GSL_PREC_DOUBLE GSL special function.
template<double (*fcn)(double, gsl_mode_t)>
void addGSLDOUBLEFunc(symbol name, symbol arg1=SYM(x))
{
  addFunc(GSLModule->e.ve, realRealDOUBLE<fcn>, primReal(), name,
          formal(primReal(),arg1));
}

template<double (*fcn)(double, double, gsl_mode_t)>
void addGSLDOUBLE2Func(symbol name, symbol arg1=SYM(phi),
                       symbol arg2=SYM(k))
{
  addFunc(GSLModule->e.ve, realRealRealDOUBLE<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primReal(),arg2));
}

template <double (*func)(double, double, double, gsl_mode_t)>
void realRealRealRealDOUBLE(stack *s)
{
  double z=pop<double>(s);
  double y=pop<double>(s);
  double x=pop<double>(s);
  s->push(func(x,y,z,GSL_PREC_DOUBLE));
  checkGSLerror();
}

template<double (*fcn)(double, double, double, gsl_mode_t)>
void addGSLDOUBLE3Func(symbol name, symbol arg1, symbol arg2,
                       symbol arg3)
{
  addFunc(GSLModule->e.ve, realRealRealRealDOUBLE<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primReal(),arg2),
          formal(primReal(), arg3));
}

template <double (*func)(double, double, double, double, gsl_mode_t)>
void realRealRealRealRealDOUBLE(stack *s)
{
  double z=pop<double>(s);
  double y=pop<double>(s);
  double x=pop<double>(s);
  double w=pop<double>(s);
  s->push(func(w,x,y,z,GSL_PREC_DOUBLE));
  checkGSLerror();
}

template<double (*fcn)(double, double, double, double, gsl_mode_t)>
void addGSLDOUBLE4Func(symbol name, symbol arg1, symbol arg2,
                       symbol arg3, symbol arg4)
{
  addFunc(GSLModule->e.ve, realRealRealRealRealDOUBLE<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primReal(),arg2),
          formal(primReal(), arg3), formal(primReal(), arg4));
}

template<double (*fcn)(unsigned)>
void addGSLIntFunc(symbol name)
{
  addFunc(GSLModule->e.ve, realIntGSL<fcn>, primReal(), name,
          formal(primInt(),SYM(s)));
}

template <double (*func)(int)>
void realSignedGSL(stack *s)
{
  Int a = pop<Int>(s);
  s->push(func(intcast(a)));
  checkGSLerror();
}

template<double (*fcn)(int)>
void addGSLSignedFunc(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, realSignedGSL<fcn>, primReal(), name,
          formal(primInt(),arg1));
}

template<double (*fcn)(int, double)>
void addGSLIntRealFunc(symbol name, symbol arg1=SYM(n),
                       symbol arg2=SYM(x))
{
  addFunc(GSLModule->e.ve, realIntRealGSL<fcn>, primReal(), name,
          formal(primInt(),arg1), formal(primReal(),arg2));
}

template<double (*fcn)(double, double)>
void addGSLRealRealFunc(symbol name, symbol arg1=SYM(nu),
                        symbol arg2=SYM(x))
{
  addFunc(GSLModule->e.ve, realRealRealGSL<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primReal(),arg2));
}

template<double (*fcn)(double, double, double)>
void addGSLRealRealRealFunc(symbol name, symbol arg1,
                            symbol arg2, symbol arg3)
{
  addFunc(GSLModule->e.ve, realRealRealRealGSL<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primReal(),arg2),
          formal(primReal(), arg3));
}

template<int (*fcn)(double, double, double)>
void addGSLRealRealRealFuncInt(symbol name, symbol arg1,
                               symbol arg2, symbol arg3)
{
  addFunc(GSLModule->e.ve, intRealRealRealGSL<fcn>, primInt(), name,
          formal(primReal(),arg1), formal(primReal(),arg2),
          formal(primReal(), arg3));
}

template<double (*fcn)(double, unsigned)>
void addGSLRealIntFunc(symbol name, symbol arg1=SYM(nu),
                       symbol arg2=SYM(s))
{
  addFunc(GSLModule->e.ve, realRealIntGSL<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primInt(),arg2));
}

template<double (*func)(double, int)>
void realRealSignedGSL(stack *s)
{
  Int b = pop<Int>(s);
  double a = pop<double>(s);
  s->push(func(a, intcast(b)));
  checkGSLerror();
}

template<double (*fcn)(double, int)>
void addGSLRealSignedFunc(symbol name, symbol arg1, symbol arg2)
{
  addFunc(GSLModule->e.ve, realRealSignedGSL<fcn>, primReal(), name,
          formal(primReal(),arg1), formal(primInt(),arg2));
}

template<double (*func)(unsigned int, unsigned int)>
void realUnsignedUnsignedGSL(stack *s)
{
  Int b = pop<Int>(s);
  Int a = pop<Int>(s);
  s->push(func(unsignedcast(a), unsignedcast(b)));
  checkGSLerror();
}

template<double (*fcn)(unsigned int, unsigned int)>
void addGSLUnsignedUnsignedFunc(symbol name, symbol arg1,
                                symbol arg2)
{
  addFunc(GSLModule->e.ve, realUnsignedUnsignedGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primInt(), arg2));
}

template<double (*func)(int, double, double)>
void realIntRealRealGSL(stack *s)
{
  double c = pop<double>(s);
  double b = pop<double>(s);
  Int a = pop<Int>(s);
  s->push(func(intcast(a), b, c));
  checkGSLerror();
}

template<double (*fcn)(int, double, double)>
void addGSLIntRealRealFunc(symbol name, symbol arg1,
                           symbol arg2, symbol arg3)
{
  addFunc(GSLModule->e.ve, realIntRealRealGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primReal(), arg2),
          formal(primReal(), arg3));
}

template<double (*func)(int, int, double)>
void realIntIntRealGSL(stack *s)
{
  double c = pop<double>(s);
  Int b = pop<Int>(s);
  Int a = pop<Int>(s);
  s->push(func(intcast(a), intcast(b), c));
  checkGSLerror();
}

template<double (*fcn)(int, int, double)>
void addGSLIntIntRealFunc(symbol name, symbol arg1, symbol arg2,
                          symbol arg3)
{
  addFunc(GSLModule->e.ve, realIntIntRealGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primReal(), arg3));
}

template<double (*func)(int, int, double, double)>
void realIntIntRealRealGSL(stack *s)
{
  double d = pop<double>(s);
  double c = pop<double>(s);
  Int b = pop<Int>(s);
  Int a = pop<Int>(s);
  s->push(func(intcast(a), intcast(b), c, d));
  checkGSLerror();
}

template<double (*fcn)(int, int, double, double)>
void addGSLIntIntRealRealFunc(symbol name, symbol arg1,
                              symbol arg2, symbol arg3,
                              symbol arg4)
{
  addFunc(GSLModule->e.ve, realIntIntRealRealGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primReal(), arg3), formal(primReal(), arg4));
}

template<double (*func)(double, double, double, double)>
void realRealRealRealRealGSL(stack *s)
{
  double d = pop<double>(s);
  double c = pop<double>(s);
  double b = pop<double>(s);
  double a = pop<double>(s);
  s->push(func(a, b, c, d));
  checkGSLerror();
}

template<double (*fcn)(double, double, double, double)>
void addGSLRealRealRealRealFunc(symbol name, symbol arg1,
                                symbol arg2, symbol arg3,
                                symbol arg4)
{
  addFunc(GSLModule->e.ve, realRealRealRealRealGSL<fcn>, primReal(), name,
          formal(primReal(), arg1), formal(primReal(), arg2),
          formal(primReal(), arg3), formal(primReal(), arg4));
}

template<double (*func)(int, int, int, int, int, int)>
void realIntIntIntIntIntIntGSL(stack *s)
{
  Int f = pop<Int>(s);
  Int e = pop<Int>(s);
  Int d = pop<Int>(s);
  Int c = pop<Int>(s);
  Int b = pop<Int>(s);
  Int a = pop<Int>(s);
  s->push(func(intcast(a), intcast(b), intcast(c), intcast(d), intcast(e),
               intcast(f)));
  checkGSLerror();
}

template<double (*fcn)(int, int, int, int, int, int)>
void addGSLIntIntIntIntIntIntFunc(symbol name, symbol arg1,
                                  symbol arg2, symbol arg3,
                                  symbol arg4, symbol arg5,
                                  symbol arg6)
{
  addFunc(GSLModule->e.ve, realIntIntIntIntIntIntGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primInt(), arg3), formal(primInt(), arg4),
          formal(primInt(), arg5), formal(primInt(), arg6));
}

template<double (*func)(int, int, int, int, int, int, int, int, int)>
void realIntIntIntIntIntIntIntIntIntGSL(stack *s)
{
  Int i = pop<Int>(s);
  Int h = pop<Int>(s);
  Int g = pop<Int>(s);
  Int f = pop<Int>(s);
  Int e = pop<Int>(s);
  Int d = pop<Int>(s);
  Int c = pop<Int>(s);
  Int b = pop<Int>(s);
  Int a = pop<Int>(s);
  s->push(func(intcast(a), intcast(b), intcast(c), intcast(d), intcast(e),
               intcast(f), intcast(g), intcast(h), intcast(i)));
  checkGSLerror();
}

template<double (*fcn)(int, int, int, int, int, int, int, int, int)>
void addGSLIntIntIntIntIntIntIntIntIntFunc(symbol name, symbol arg1,
                                           symbol arg2, symbol arg3,
                                           symbol arg4, symbol arg5,
                                           symbol arg6, symbol arg7,
                                           symbol arg8, symbol arg9)
{
  addFunc(GSLModule->e.ve, realIntIntIntIntIntIntIntIntIntGSL<fcn>, primReal(),
          name, formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primInt(), arg3), formal(primInt(), arg4),
          formal(primInt(), arg5), formal(primInt(), arg6),
          formal(primInt(), arg7), formal(primInt(), arg8),
          formal(primInt(), arg9));
}

template<double (*func)(unsigned int, double)>
void realUIntRealGSL(stack *s)
{
  double a = pop<double>(s);
  unsigned int k = unsignedcast(pop<Int>(s));
  s->push(func(k,a));
  checkGSLerror();
}

template<double (*fcn)(unsigned int, double)>
void addGSLUIntRealFunc(symbol name, symbol arg1, symbol arg2)
{
  addFunc(GSLModule->e.ve, realUIntRealGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primReal(), arg2));
}

template<double (*func)(unsigned int, double, unsigned int)>
void realUIntRealUIntGSL(stack *s)
{
  unsigned int n = unsignedcast(pop<Int>(s));
  double a = pop<double>(s);
  unsigned int k = unsignedcast(pop<Int>(s));
  s->push(func(k,a,n));
  checkGSLerror();
}

template<double (*fcn)(unsigned int, double, unsigned int)>
void addGSLUIntRealUIntFunc(symbol name, symbol arg1, symbol arg2, symbol arg3)
{
  addFunc(GSLModule->e.ve, realUIntRealUIntGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primReal(), arg2),
          formal(primInt(), arg3));
}

template<double (*func)(unsigned int, double, double)>
void realUIntRealRealGSL(stack *s)
{
  double b = pop<double>(s);
  double a = pop<double>(s);
  unsigned int k = unsignedcast(pop<Int>(s));
  s->push(func(k,a,b));
  checkGSLerror();
}

template<double (*fcn)(unsigned int, double, double)>
void addGSLUIntRealRealFunc(symbol name, symbol arg1, symbol arg2, symbol arg3)
{
  addFunc(GSLModule->e.ve, realUIntRealRealGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primReal(), arg2),
          formal(primReal(), arg3));
}

template<double (*func)(unsigned int, unsigned int, unsigned int, unsigned int)>
void realUIntUIntUIntUIntGSL(stack *s)
{
  unsigned int t = unsignedcast(pop<Int>(s));
  unsigned int n2 = unsignedcast(pop<Int>(s));
  unsigned int n1 = unsignedcast(pop<Int>(s));
  unsigned int k = unsignedcast(pop<Int>(s));
  s->push(func(k,n1,n2,t));
  checkGSLerror();
}

template<double (*fcn)(unsigned int, unsigned int, unsigned int, unsigned int)>
void addGSLUIntUIntUIntUIntFunc(symbol name, symbol arg1, symbol arg2,
                                symbol arg3, symbol arg4)
{
  addFunc(GSLModule->e.ve, realUIntUIntUIntUIntGSL<fcn>, primReal(), name,
          formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primInt(), arg3), formal(primInt(), arg4));
}

// GSL random number generators
gsl_rng *GSLrng=0;
const gsl_rng_type **GSLrngFirstType=gsl_rng_types_setup();

inline void checkGSLrng()
{
  if(GSLrng == 0) error(GSLrngnull);
}

void GSLrngFree()
{
  if(GSLrng != 0) gsl_rng_free(GSLrng);
  GSLrng=0;
}

void GSLrngInit(stack *s)
{
  string n = pop<string>(s,string());
  const gsl_rng_type **t;
  if(n.empty()) t = &gsl_rng_default;
  else {
    for(t=GSLrngFirstType; *t!=0; ++t)
      if(n == string((*t)->name)) break;
    if(*t == 0) error(GSLinvalid);
  }
  GSLrngFree();
  GSLrng = gsl_rng_alloc(*t);
  if(GSLrng == 0) {
    GSLerror=false;
    error("insufficient memory for allocation of GSL random number generator");
  }
}

void GSLrngList(stack *s)
{
  array* a = new array(0);
  const gsl_rng_type **t;
  for(t=GSLrngFirstType; *t!=0; ++t) {
    a->push(string((*t)->name));
  }
  s->push<array*>(a);
  checkGSLerror();
}

void GSLrngSet(stack *s)
{
  Int i=pop<Int>(s,-1);
  checkGSLrng();
  if(i < 0) gsl_rng_set(GSLrng,gsl_rng_default_seed);
  else gsl_rng_set(GSLrng,unsignedcast(i));
  checkGSLerror();
}

template<unsigned long int (*func)(const gsl_rng*)>
void intVoidGSLrng(stack *s)
{
  s->push<Int>(func(GSLrng));
  checkGSLrng();
  checkGSLerror();
}

template<unsigned long int (*fcn)(const gsl_rng*)>
void addGSLrngVoidFuncInt(symbol name)
{
  addFunc(GSLModule->e.ve, intVoidGSLrng<fcn>, primInt(), name);
}

template<unsigned long int (*func)(const gsl_rng*, unsigned long int)>
void intULongGSLrng(stack *s)
{
  unsigned long int i = unsignedcast(pop<Int>(s));
  checkGSLrng();
  s->push<Int>(func(GSLrng,i));
  checkGSLerror();
}

template<unsigned long int (*fcn)(const gsl_rng*, unsigned long int)>
void addGSLrngULongFuncInt(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, intULongGSLrng<fcn>, primInt(), name,
          formal(primInt(), arg1));
}

template<unsigned int (*func)(const gsl_rng*, double)>
void intRealGSLrng(stack *s)
{
  double x = pop<double>(s);
  checkGSLrng();
  s->push<Int>(func(GSLrng,x));
  checkGSLerror();
}

template<unsigned int (*fcn)(const gsl_rng*, double)>
void addGSLrngRealFuncInt(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, intRealGSLrng<fcn>, primInt(), name,
          formal(primReal(), arg1));
}

template<unsigned int (*func)(const gsl_rng*, double, double)>
void intRealRealGSLrng(stack *s)
{
  double y = pop<double>(s);
  double x = pop<double>(s);
  checkGSLrng();
  s->push<Int>(func(GSLrng,x,y));
  checkGSLerror();
}

template<unsigned int (*fcn)(const gsl_rng*, double, double)>
void addGSLrngRealRealFuncInt(symbol name, symbol arg1, symbol arg2)
{
  addFunc(GSLModule->e.ve, intRealRealGSLrng<fcn>, primInt(), name,
          formal(primReal(), arg1), formal(primReal(), arg2));
}

template<double (*func)(const gsl_rng*)>
void realVoidGSLrng(stack *s)
{
  checkGSLrng();
  s->push(func(GSLrng));
  checkGSLerror();
}

template<double (*fcn)(const gsl_rng*)>
void addGSLrngVoidFunc(symbol name)
{
  addFunc(GSLModule->e.ve, realVoidGSLrng<fcn>, primReal(), name);
}

template<double (*func)(const gsl_rng*, double)>
void realRealGSLrng(stack *s)
{
  double x = pop<double>(s);
  checkGSLrng();
  s->push(func(GSLrng,x));
  checkGSLerror();
}

template<double (*fcn)(const gsl_rng*, double)>
void addGSLrngRealFunc(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, realRealGSLrng<fcn>, primReal(), name,
          formal(primReal(), arg1));
}

template<double (*func)(const gsl_rng*, double, double)>
void realRealRealGSLrng(stack *s)
{
  double b = pop<double>(s);
  double a = pop<double>(s);
  checkGSLrng();
  s->push(func(GSLrng,a,b));
  checkGSLerror();
}

template<double (*fcn)(const gsl_rng*, double, double)>
void addGSLrngRealRealFunc(symbol name, symbol arg1, symbol arg2)
{
  addFunc(GSLModule->e.ve, realRealRealGSLrng<fcn>, primReal(), name,
          formal(primReal(), arg1), formal(primReal(), arg2));
}

template<unsigned int (*func)(const gsl_rng*, double, unsigned int)>
void intRealUIntGSLrng(stack *s)
{
  unsigned int n = unsignedcast(pop<Int>(s));
  double a = pop<double>(s);
  checkGSLrng();
  s->push<Int>(func(GSLrng,a,n));
  checkGSLerror();
}

template<unsigned int (*fcn)(const gsl_rng*, double, unsigned int)>
void addGSLrngRealUIntFuncInt(symbol name, symbol arg1, symbol arg2)
{
  addFunc(GSLModule->e.ve, intRealUIntGSLrng<fcn>, primInt(), name,
          formal(primReal(), arg1), formal(primInt(), arg2));
}

template<unsigned int (*func)(const gsl_rng*, unsigned int, unsigned int,
                              unsigned int)>
void intUIntUIntUIntGSLrng(stack *s)
{
  unsigned int t = unsignedcast(pop<Int>(s));
  unsigned int n2 = unsignedcast(pop<Int>(s));
  unsigned int n1 = unsignedcast(pop<Int>(s));
  checkGSLrng();
  s->push<Int>(func(GSLrng,n1,n2,t));
  checkGSLerror();
}

template<unsigned int (*fcn)(const gsl_rng*, unsigned int, unsigned int,
                             unsigned int)>
void addGSLrngUIntUIntUIntFuncInt(symbol name, symbol arg1, symbol arg2,
                                  symbol arg3)
{
  addFunc(GSLModule->e.ve, intUIntUIntUIntGSLrng<fcn>, primInt(), name,
          formal(primInt(), arg1), formal(primInt(), arg2),
          formal(primInt(), arg3));
}

template<const char* (*func)(const gsl_rng*)>
void stringVoidGSLrng(stack *s)
{
  checkGSLrng();
  s->push<string>(func(GSLrng));
  checkGSLerror();
}

template<const char* (*fcn)(const gsl_rng*)>
void addGSLrngVoidFuncString(symbol name)
{
  addFunc(GSLModule->e.ve, stringVoidGSLrng<fcn>, primString(), name);
}

void GSLrng_gaussian(stack *s)
{
  string method = pop<string>(s,string("polar"));
  double sigma = pop<double>(s,1.0);
  double mu = pop<double>(s,0.0);
  checkGSLrng();
  double x=mu;
  if(method == "polar") x += gsl_ran_gaussian(GSLrng,sigma);
  else if(method == "ziggurat") x += gsl_ran_gaussian_ziggurat(GSLrng,sigma);
  else if(method == "ratio") x += gsl_ran_gaussian_ratio_method(GSLrng,sigma);
  else error(GSLinvalid);
  s->push(x);
  checkGSLerror();
}

template<double (*func)(double, double)>
void realRealRealRealGSLgaussian(stack *s)
{
  double sigma = pop<double>(s,1.0);
  double mu = pop<double>(s,0.0);
  double x = pop<double>(s);
  s->push(func(x-mu,sigma));
  checkGSLerror();
}

template<double (*fcn)(double, double)>
void addGSLgaussianrealRealRealRealFunc(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, realRealRealRealGSLgaussian<fcn>, primReal(), name,
          formal(primReal(), arg1),
          formal(primReal(), SYM(mu), true, false),
          formal(primReal(), SYM(sigma), true, false));
}

template<double (*func)(double, double)>
void realRealRealRealGSLinvgaussian(stack *s)
{
  double sigma = pop<double>(s,1.0);
  double mu = pop<double>(s,0.0);
  double x = pop<double>(s);
  s->push(func(x,sigma)+mu);
  checkGSLerror();
}

template<double (*fcn)(double, double)>
void addGSLinvgaussianrealRealRealRealFunc(symbol name, symbol arg1)
{
  addFunc(GSLModule->e.ve, realRealRealRealGSLinvgaussian<fcn>, primReal(),
          name,
          formal(primReal(), arg1),
          formal(primReal(), SYM(mu), true, false),
          formal(primReal(), SYM(sigma), true, false));
}

void GSLrng_bivariate_gaussian(stack *s)
{
  double rho = pop<double>(s,0.0);
  pair sigma = pop<pair>(s,pair(1.0,1.0));
  pair mu = pop<pair>(s,pair(0.0,0.0));
  checkGSLrng();
  double x,y;
  gsl_ran_bivariate_gaussian(GSLrng,sigma.getx(),sigma.gety(),rho,&x,&y);
  s->push(pair(x,y)+mu);
  checkGSLerror();
}

void GSLpdf_bivariate_gaussian(stack *s)
{
  double rho = pop<double>(s,0.0);
  pair sigma = pop<pair>(s,pair(1.0,1.0));
  pair mu = pop<pair>(s,pair(0.0,0.0));
  pair z = pop<pair>(s);
  s->push(gsl_ran_bivariate_gaussian_pdf(z.getx()+mu.getx(),z.gety()+mu.gety(),
                                         sigma.getx(),sigma.gety(),rho));
  checkGSLerror();
}

void GSLrng_levy(stack *s)
{
  double beta = pop<double>(s,0.0);
  double alpha = pop<double>(s);
  double c = pop<double>(s);
  if((alpha<=0) || (alpha>2)) error(GSLinvalid);
  if((beta<-1) || (beta>1)) error(GSLinvalid);
  checkGSLrng();
  double x;
  if(beta==0) x=gsl_ran_levy(GSLrng,c,alpha);
  else x=gsl_ran_levy_skew(GSLrng,c,alpha,beta);
  s->push(x);
  checkGSLerror();
}

void GSLrng_gamma(stack *s)
{
  string method = pop<string>(s,string("mt"));
  double b = pop<double>(s);
  double a = pop<double>(s);
  checkGSLrng();
  double x=0.0;
  if(method == "mt") x = gsl_ran_gamma(GSLrng,a,b);
  else if(method == "knuth") x = gsl_ran_gamma_knuth(GSLrng,a,b);
  else error(GSLinvalid);
  s->push(x);
  checkGSLerror();
}

void GSLrng_dir2d(stack *s)
{
  string method = pop<string>(s,string("neumann"));
  checkGSLrng();
  double x=0, y=0;
  if(method == "neumann") gsl_ran_dir_2d(GSLrng,&x,&y);
  else if(method == "trig") gsl_ran_dir_2d_trig_method(GSLrng,&x,&y);
  else error(GSLinvalid);
  s->push(pair(x,y));
  checkGSLerror();
}

void GSLrng_dir3d(stack *s)
{
  checkGSLrng();
  double x,y,z;
  gsl_ran_dir_3d(GSLrng,&x,&y,&z);
  s->push(triple(x,y,z));
  checkGSLerror();
}

void GSLrng_dir(stack *s)
{
  size_t n = (size_t) unsignedcast(pop<Int>(s));
  if(n==0) error(GSLinvalid);
  checkGSLrng();
  double* p = new double[n];
  gsl_ran_dir_nd(GSLrng,n,p);
  s->push<array*>(copyCArray(n,p));
  delete[] p;
  checkGSLerror();
}

void GSLrng_dirichlet(stack *s)
{
  array* alpha = pop<array*>(s);
  size_t K = checkArray(alpha);
  checkGSLrng();
  double* calpha;
  copyArrayC(calpha,alpha);
  double* ctheta = new double[K];
  gsl_ran_dirichlet(GSLrng,K,calpha,ctheta);
  s->push<array*>(copyCArray(K,ctheta));
  delete[] ctheta;
  delete[] calpha;
  checkGSLerror();
}

void GSLpdf_dirichlet(stack *s)
{
  array* theta = pop<array*>(s);
  array* alpha = pop<array*>(s);
  size_t K = checkArray(alpha);
  if(checkArray(theta) != K)
    error(GSLinvalid);
  double* calpha;
  copyArrayC(calpha,alpha);
  double* ctheta;
  copyArrayC(ctheta,theta);
  s->push(gsl_ran_dirichlet_pdf(K,calpha,ctheta));
  delete[] ctheta;
  delete[] calpha;
  checkGSLerror();
}

void GSLrng_multinomial(stack *s)
{
  array* p = pop<array*>(s);
  unsigned int N = unsignedcast(pop<Int>(s));
  size_t K = checkArray(p);
  checkGSLrng();
  double* cp;
  copyArrayC(cp,p);
  unsigned int* cn = new unsigned int[K];
  gsl_ran_multinomial(GSLrng,K,N,cp,cn);
  s->push<array*>(copyCArray(K,cn));
  delete[] cn;
  delete[] cp;
  checkGSLerror();
}

void GSLpdf_multinomial(stack *s)
{
  array* n = pop<array*>(s);
  array* p = pop<array*>(s);
  size_t K = checkArray(p);
  if(K != checkArray(n)) error(GSLinvalid);
  double* cp;
  copyArrayC(cp,p);
  unsigned int* cn;
  copyArrayC<unsigned int,Int>(cn,n,unsignedcast);
  s->push(gsl_ran_multinomial_pdf(K,cp,cn));
  delete[] cn;
  delete[] cp;
  checkGSLerror();
}

void GSLsf_elljac_e(stack *s)
{
  double m = pop<double>(s);
  double u = pop<double>(s);
  double sn,cn,dn;
  gsl_sf_elljac_e(u,m,&sn,&cn,&dn);
  array *result=new array(3);
  (*result)[0]=sn;
  (*result)[1]=cn;
  (*result)[2]=dn;
  s->push(result);
}

// Handle GSL errors gracefully.
void GSLerrorhandler(const char *reason, const char *, int, int)
{
  if(!GSLerror) {
    vm::errornothrow(reason);
    GSLerror=true;
  }
}

void gen_rungsl_venv(venv &ve)
{
  GSLModule=new types::dummyRecord(SYM(gsl));
  gsl_set_error_handler(GSLerrorhandler);

  // Common functions
  addGSLRealRealFunc<gsl_hypot>(SYM(hypot),SYM(x),SYM(y));
//  addGSLRealRealRealFunc<gsl_hypot3>(SYM(hypot),SYM(x),SYM(y),SYM(z));
  addGSLRealRealRealFuncInt<gsl_fcmp>(SYM(fcmp),SYM(x),SYM(y),SYM(epsilon));

  // Airy functions
  addGSLDOUBLEFunc<gsl_sf_airy_Ai>(SYM(Ai));
  addGSLDOUBLEFunc<gsl_sf_airy_Bi>(SYM(Bi));
  addGSLDOUBLEFunc<gsl_sf_airy_Ai_scaled>(SYM(Ai_scaled));
  addGSLDOUBLEFunc<gsl_sf_airy_Bi_scaled>(SYM(Bi_scaled));
  addGSLDOUBLEFunc<gsl_sf_airy_Ai_deriv>(SYM(Ai_deriv));
  addGSLDOUBLEFunc<gsl_sf_airy_Bi_deriv>(SYM(Bi_deriv));
  addGSLDOUBLEFunc<gsl_sf_airy_Ai_deriv_scaled>(SYM(Ai_deriv_scaled));
  addGSLDOUBLEFunc<gsl_sf_airy_Bi_deriv_scaled>(SYM(Bi_deriv_scaled));
  addGSLIntFunc<gsl_sf_airy_zero_Ai>(SYM(zero_Ai));
  addGSLIntFunc<gsl_sf_airy_zero_Bi>(SYM(zero_Bi));
  addGSLIntFunc<gsl_sf_airy_zero_Ai_deriv>(SYM(zero_Ai_deriv));
  addGSLIntFunc<gsl_sf_airy_zero_Bi_deriv>(SYM(zero_Bi_deriv));

  // Bessel functions
  addGSLRealFunc<gsl_sf_bessel_J0>(SYM(J0));
  addGSLRealFunc<gsl_sf_bessel_J1>(SYM(J1));
  addGSLIntRealFunc<gsl_sf_bessel_Jn>(SYM(Jn));
  addGSLRealFunc<gsl_sf_bessel_Y0>(SYM(Y0));
  addGSLRealFunc<gsl_sf_bessel_Y1>(SYM(Y1));
  addGSLIntRealFunc<gsl_sf_bessel_Yn>(SYM(Yn));
  addGSLRealFunc<gsl_sf_bessel_I0>(SYM(I0));
  addGSLRealFunc<gsl_sf_bessel_I1>(SYM(I1));
  addGSLIntRealFunc<gsl_sf_bessel_In>(SYM(I));
  addGSLRealFunc<gsl_sf_bessel_I0_scaled>(SYM(I0_scaled));
  addGSLRealFunc<gsl_sf_bessel_I1_scaled>(SYM(I1_scaled));
  addGSLIntRealFunc<gsl_sf_bessel_In_scaled>(SYM(I_scaled));
  addGSLRealFunc<gsl_sf_bessel_K0>(SYM(K0));
  addGSLRealFunc<gsl_sf_bessel_K1>(SYM(K1));
  addGSLIntRealFunc<gsl_sf_bessel_Kn>(SYM(K));
  addGSLRealFunc<gsl_sf_bessel_K0_scaled>(SYM(K0_scaled));
  addGSLRealFunc<gsl_sf_bessel_K1_scaled>(SYM(K1_scaled));
  addGSLIntRealFunc<gsl_sf_bessel_Kn_scaled>(SYM(K_scaled));
  addGSLRealFunc<gsl_sf_bessel_j0>(SYM(j0));
  addGSLRealFunc<gsl_sf_bessel_j1>(SYM(j1));
  addGSLRealFunc<gsl_sf_bessel_j2>(SYM(j2));
  addGSLIntRealFunc<gsl_sf_bessel_jl>(SYM(j),SYM(l));
  addGSLRealFunc<gsl_sf_bessel_y0>(SYM(y0));
  addGSLRealFunc<gsl_sf_bessel_y1>(SYM(y1));
  addGSLRealFunc<gsl_sf_bessel_y2>(SYM(y2));
  addGSLIntRealFunc<gsl_sf_bessel_yl>(SYM(y),SYM(l));
  addGSLRealFunc<gsl_sf_bessel_i0_scaled>(SYM(i0_scaled));
  addGSLRealFunc<gsl_sf_bessel_i1_scaled>(SYM(i1_scaled));
  addGSLRealFunc<gsl_sf_bessel_i2_scaled>(SYM(i2_scaled));
  addGSLIntRealFunc<gsl_sf_bessel_il_scaled>(SYM(i_scaled),SYM(l));
  addGSLRealFunc<gsl_sf_bessel_k0_scaled>(SYM(k0_scaled));
  addGSLRealFunc<gsl_sf_bessel_k1_scaled>(SYM(k1_scaled));
  addGSLRealFunc<gsl_sf_bessel_k2_scaled>(SYM(k2_scaled));
  addGSLIntRealFunc<gsl_sf_bessel_kl_scaled>(SYM(k_scaled),SYM(l));
  addGSLRealRealFunc<gsl_sf_bessel_Jnu>(SYM(J));
  addGSLRealRealFunc<gsl_sf_bessel_Ynu>(SYM(Y));
  addGSLRealRealFunc<gsl_sf_bessel_Inu>(SYM(I));
  addGSLRealRealFunc<gsl_sf_bessel_Inu_scaled>(SYM(I_scaled));
  addGSLRealRealFunc<gsl_sf_bessel_Knu>(SYM(K));
  addGSLRealRealFunc<gsl_sf_bessel_lnKnu>(SYM(lnK));
  addGSLRealRealFunc<gsl_sf_bessel_Knu_scaled>(SYM(K_scaled));
  addGSLIntFunc<gsl_sf_bessel_zero_J0>(SYM(zero_J0));
  addGSLIntFunc<gsl_sf_bessel_zero_J1>(SYM(zero_J1));
  addGSLRealIntFunc<gsl_sf_bessel_zero_Jnu>(SYM(zero_J));

  // Clausen functions
  addGSLRealFunc<gsl_sf_clausen>(SYM(clausen));

  // Coulomb functions
  addGSLRealRealFunc<gsl_sf_hydrogenicR_1>(SYM(hydrogenicR_1),SYM(Z),SYM(r));
  addGSLIntIntRealRealFunc<gsl_sf_hydrogenicR>(SYM(hydrogenicR),SYM(n),SYM(l),
                                               SYM(Z),SYM(r));
  // Missing: F_L(eta,x), G_L(eta,x), C_L(eta)

  // Coupling coefficients
  addGSLIntIntIntIntIntIntFunc<gsl_sf_coupling_3j>(SYM(coupling_3j),SYM(two_ja),
                                                   SYM(two_jb),SYM(two_jc),
                                                   SYM(two_ma),
                                                   SYM(two_mb),SYM(two_mc));
  addGSLIntIntIntIntIntIntFunc<gsl_sf_coupling_6j>(SYM(coupling_6j),SYM(two_ja),
                                                   SYM(two_jb),SYM(two_jc),
                                                   SYM(two_jd),
                                                   SYM(two_je),SYM(two_jf));
  addGSLIntIntIntIntIntIntIntIntIntFunc<gsl_sf_coupling_9j>(SYM(coupling_9j),
                                                            SYM(two_ja),
                                                            SYM(two_jb),
                                                            SYM(two_jc),
                                                            SYM(two_jd),
                                                            SYM(two_je),
                                                            SYM(two_jf),
                                                            SYM(two_jg),
                                                            SYM(two_jh),
                                                            SYM(two_ji));
  // Dawson function
  addGSLRealFunc<gsl_sf_dawson>(SYM(dawson));

  // Debye functions
  addGSLRealFunc<gsl_sf_debye_1>(SYM(debye_1));
  addGSLRealFunc<gsl_sf_debye_2>(SYM(debye_2));
  addGSLRealFunc<gsl_sf_debye_3>(SYM(debye_3));
  addGSLRealFunc<gsl_sf_debye_4>(SYM(debye_4));
  addGSLRealFunc<gsl_sf_debye_5>(SYM(debye_5));
  addGSLRealFunc<gsl_sf_debye_6>(SYM(debye_6));

  // Dilogarithm
  addGSLRealFunc<gsl_sf_dilog>(SYM(dilog));
  // Missing: complex dilogarithm

  // Elementary operations
  // we don't support errors at the moment

  // Elliptic integrals
  addGSLDOUBLEFunc<gsl_sf_ellint_Kcomp>(SYM(K),SYM(k));
  addGSLDOUBLEFunc<gsl_sf_ellint_Ecomp>(SYM(E),SYM(k));
  addGSLDOUBLE2Func<gsl_sf_ellint_Pcomp>(SYM(P),SYM(k),SYM(n));
  addGSLDOUBLE2Func<gsl_sf_ellint_F>(SYM(F));
  addGSLDOUBLE2Func<gsl_sf_ellint_E>(SYM(E));
  addGSLDOUBLE3Func<gsl_sf_ellint_P>(SYM(P),SYM(phi),SYM(k),SYM(n));
#if GSL_MAJOR_VERSION >= 2
  addGSLDOUBLE2Func<gsl_sf_ellint_D>(SYM(D),SYM(phi),SYM(k));
#else
  addGSLDOUBLE3Func<gsl_sf_ellint_D>(SYM(D),SYM(phi),SYM(k),SYM(n));
#endif
  addGSLDOUBLE2Func<gsl_sf_ellint_RC>(SYM(RC),SYM(x),SYM(y));
  addGSLDOUBLE3Func<gsl_sf_ellint_RD>(SYM(RD),SYM(x),SYM(y),SYM(z));
  addGSLDOUBLE3Func<gsl_sf_ellint_RF>(SYM(RF),SYM(x),SYM(y),SYM(z));
  addGSLDOUBLE4Func<gsl_sf_ellint_RJ>(SYM(RJ),SYM(x),SYM(y),SYM(z),SYM(p));

  // Error functions
  addGSLRealFunc<gsl_sf_erf>(SYM(erf));
  addGSLRealFunc<gsl_sf_erfc>(SYM(erfc));
  addGSLRealFunc<gsl_sf_log_erfc>(SYM(log_erfc));
  addGSLRealFunc<gsl_sf_erf_Z>(SYM(erf_Z));
  addGSLRealFunc<gsl_sf_erf_Q>(SYM(erf_Q));
  addGSLRealFunc<gsl_sf_hazard>(SYM(hazard));

  // Exponential functions
  addGSLRealRealFunc<gsl_sf_exp_mult>(SYM(exp_mult),SYM(x),SYM(y));
//  addGSLRealFunc<gsl_sf_expm1>(SYM(expm1));
  addGSLRealFunc<gsl_sf_exprel>(SYM(exprel));
  addGSLRealFunc<gsl_sf_exprel_2>(SYM(exprel_2));
  addGSLIntRealFunc<gsl_sf_exprel_n>(SYM(exprel),SYM(n),SYM(x));

  // Exponential integrals
  addGSLRealFunc<gsl_sf_expint_E1>(SYM(E1));
  addGSLRealFunc<gsl_sf_expint_E2>(SYM(E2));
//  addGSLIntRealFunc<gsl_sf_expint_En>(SYM(En),SYM(n),SYM(x));
  addGSLRealFunc<gsl_sf_expint_Ei>(SYM(Ei));
  addGSLRealFunc<gsl_sf_Shi>(SYM(Shi));
  addGSLRealFunc<gsl_sf_Chi>(SYM(Chi));
  addGSLRealFunc<gsl_sf_expint_3>(SYM(Ei3));
  addGSLRealFunc<gsl_sf_Si>(SYM(Si));
  addGSLRealFunc<gsl_sf_Ci>(SYM(Ci));
  addGSLRealFunc<gsl_sf_atanint>(SYM(atanint));

  // Fermi--Dirac function
  addGSLRealFunc<gsl_sf_fermi_dirac_m1>(SYM(FermiDiracM1));
  addGSLRealFunc<gsl_sf_fermi_dirac_0>(SYM(FermiDirac0));
  addGSLRealFunc<gsl_sf_fermi_dirac_1>(SYM(FermiDirac1));
  addGSLRealFunc<gsl_sf_fermi_dirac_2>(SYM(FermiDirac2));
  addGSLIntRealFunc<gsl_sf_fermi_dirac_int>(SYM(FermiDirac),SYM(j),SYM(x));
  addGSLRealFunc<gsl_sf_fermi_dirac_mhalf>(SYM(FermiDiracMHalf));
  addGSLRealFunc<gsl_sf_fermi_dirac_half>(SYM(FermiDiracHalf));
  addGSLRealFunc<gsl_sf_fermi_dirac_3half>(SYM(FermiDirac3Half));
  addGSLRealRealFunc<gsl_sf_fermi_dirac_inc_0>(SYM(FermiDiracInc0),SYM(x),
                                               SYM(b));

  // Gamma and beta functions
  addGSLRealFunc<gsl_sf_gamma>(SYM(gamma));
  addGSLRealFunc<gsl_sf_lngamma>(SYM(lngamma));
  addGSLRealFunc<gsl_sf_gammastar>(SYM(gammastar));
  addGSLRealFunc<gsl_sf_gammainv>(SYM(gammainv));
  addGSLIntFunc<gsl_sf_fact>(SYM(fact));
  addGSLIntFunc<gsl_sf_doublefact>(SYM(doublefact));
  addGSLIntFunc<gsl_sf_lnfact>(SYM(lnfact));
  addGSLIntFunc<gsl_sf_lndoublefact>(SYM(lndoublefact));
  addGSLUnsignedUnsignedFunc<gsl_sf_choose>(SYM(choose),SYM(n),SYM(m));
  addGSLUnsignedUnsignedFunc<gsl_sf_lnchoose>(SYM(lnchoose),SYM(n),SYM(m));
  addGSLIntRealFunc<gsl_sf_taylorcoeff>(SYM(taylorcoeff),SYM(n),SYM(x));
  addGSLRealRealFunc<gsl_sf_poch>(SYM(poch),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_lnpoch>(SYM(lnpoch),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_pochrel>(SYM(pochrel),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_gamma_inc>(SYM(gamma),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_gamma_inc_Q>(SYM(gamma_Q),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_gamma_inc_P>(SYM(gamma_P),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_beta>(SYM(beta),SYM(a),SYM(b));
  addGSLRealRealFunc<gsl_sf_lnbeta>(SYM(lnbeta),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_sf_beta_inc>(SYM(beta),SYM(a),SYM(b),SYM(x));

  // Gegenbauer functions
  addGSLRealRealFunc<gsl_sf_gegenpoly_1>(SYM(gegenpoly_1),SYM(lambda),SYM(x));
  addGSLRealRealFunc<gsl_sf_gegenpoly_2>(SYM(gegenpoly_2),SYM(lambda),SYM(x));
  addGSLRealRealFunc<gsl_sf_gegenpoly_3>(SYM(gegenpoly_3),SYM(lambda),SYM(x));
  addGSLIntRealRealFunc<gsl_sf_gegenpoly_n>(SYM(gegenpoly),SYM(n),SYM(lambda),
                                            SYM(x));

  // Hypergeometric functions
  addGSLRealRealFunc<gsl_sf_hyperg_0F1>(SYM(hy0F1),SYM(c),SYM(x));
  addGSLIntIntRealFunc<gsl_sf_hyperg_1F1_int>(SYM(hy1F1),SYM(m),SYM(n),SYM(x));
  addGSLRealRealRealFunc<gsl_sf_hyperg_1F1>(SYM(hy1F1),SYM(a),SYM(b),SYM(x));
  addGSLIntIntRealFunc<gsl_sf_hyperg_U_int>(SYM(U),SYM(m),SYM(n),SYM(x));
  addGSLRealRealRealFunc<gsl_sf_hyperg_U>(SYM(U),SYM(a),SYM(b),SYM(x));
  addGSLRealRealRealRealFunc<gsl_sf_hyperg_2F1>(SYM(hy2F1),SYM(a),SYM(b),SYM(c),
                                                SYM(x));
  addGSLRealRealRealRealFunc<gsl_sf_hyperg_2F1_conj>(SYM(hy2F1_conj),SYM(aR),
                                                     SYM(aI),SYM(c),SYM(x));
  addGSLRealRealRealRealFunc<gsl_sf_hyperg_2F1_renorm>(SYM(hy2F1_renorm),SYM(a),
                                                       SYM(b),SYM(c),SYM(x));
  addGSLRealRealRealRealFunc<gsl_sf_hyperg_2F1_conj_renorm>
    (SYM(hy2F1_conj_renorm),SYM(aR),SYM(aI),SYM(c),SYM(x));
  addGSLRealRealRealFunc<gsl_sf_hyperg_2F0>(SYM(hy2F0),SYM(a),SYM(b),SYM(x));

  // Laguerre functions
  addGSLRealRealFunc<gsl_sf_laguerre_1>(SYM(L1),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_laguerre_2>(SYM(L2),SYM(a),SYM(x));
  addGSLRealRealFunc<gsl_sf_laguerre_3>(SYM(L3),SYM(a),SYM(x));
  addGSLIntRealRealFunc<gsl_sf_laguerre_n>(SYM(L),SYM(n),SYM(a),SYM(x));

  // Lambert W functions
  addGSLRealFunc<gsl_sf_lambert_W0>(SYM(W0));
  addGSLRealFunc<gsl_sf_lambert_Wm1>(SYM(Wm1));

  // Legendre functions and spherical harmonics
  addGSLRealFunc<gsl_sf_legendre_P1>(SYM(P1));
  addGSLRealFunc<gsl_sf_legendre_P2>(SYM(P2));
  addGSLRealFunc<gsl_sf_legendre_P3>(SYM(P3));
  addGSLIntRealFunc<gsl_sf_legendre_Pl>(SYM(Pl),SYM(l));
  addGSLRealFunc<gsl_sf_legendre_Q0>(SYM(Q0));
  addGSLRealFunc<gsl_sf_legendre_Q1>(SYM(Q1));
  addGSLIntRealFunc<gsl_sf_legendre_Ql>(SYM(Ql),SYM(l));
  addGSLIntIntRealFunc<gsl_sf_legendre_Plm>(SYM(Plm),SYM(l),SYM(m),SYM(x));
  addGSLIntIntRealFunc<gsl_sf_legendre_sphPlm>(SYM(sphPlm),SYM(l),SYM(m),
                                               SYM(x));
  addGSLRealRealFunc<gsl_sf_conicalP_half>(SYM(conicalP_half),SYM(lambda),
                                           SYM(x));
  addGSLRealRealFunc<gsl_sf_conicalP_mhalf>(SYM(conicalP_mhalf),SYM(lambda),
                                            SYM(x));
  addGSLRealRealFunc<gsl_sf_conicalP_0>(SYM(conicalP_0),SYM(lambda),SYM(x));
  addGSLRealRealFunc<gsl_sf_conicalP_1>(SYM(conicalP_1),SYM(lambda),SYM(x));
  addGSLIntRealRealFunc<gsl_sf_conicalP_sph_reg>(SYM(conicalP_sph_reg),SYM(l),
                                                 SYM(lambda),SYM(x));
  addGSLIntRealRealFunc<gsl_sf_conicalP_cyl_reg>(SYM(conicalP_cyl_reg),SYM(m),
                                                 SYM(lambda),SYM(x));
  addGSLRealRealFunc<gsl_sf_legendre_H3d_0>(SYM(H3d0),SYM(lambda),SYM(eta));
  addGSLRealRealFunc<gsl_sf_legendre_H3d_1>(SYM(H3d1),SYM(lambda),SYM(eta));
  addGSLIntRealRealFunc<gsl_sf_legendre_H3d>(SYM(H3d),SYM(l),SYM(lambda),
                                             SYM(eta));

  // Logarithm and related functions
  addGSLRealFunc<gsl_sf_log_abs>(SYM(logabs));
//  addGSLRealFunc<gsl_sf_log_1plusx>(SYM(log1p));
  addGSLRealFunc<gsl_sf_log_1plusx_mx>(SYM(log1pm));

  // Matthieu functions
  // to be implemented

  // Power function
  addGSLRealSignedFunc<gsl_sf_pow_int>(SYM(pow),SYM(x),SYM(n));

  // Psi (digamma) function
  addGSLSignedFunc<gsl_sf_psi_int>(SYM(psi),SYM(n));
  addGSLRealFunc<gsl_sf_psi>(SYM(psi));
  addGSLRealFunc<gsl_sf_psi_1piy>(SYM(psi_1piy),SYM(y));
  addGSLSignedFunc<gsl_sf_psi_1_int>(SYM(psi1),SYM(n));
  addGSLRealFunc<gsl_sf_psi_1>(SYM(psi1),SYM(x));
  addGSLIntRealFunc<gsl_sf_psi_n>(SYM(psi),SYM(n),SYM(x));

  // Synchrotron functions
  addGSLRealFunc<gsl_sf_synchrotron_1>(SYM(synchrotron_1));
  addGSLRealFunc<gsl_sf_synchrotron_2>(SYM(synchrotron_2));

  // Transport functions
  addGSLRealFunc<gsl_sf_transport_2>(SYM(transport_2));
  addGSLRealFunc<gsl_sf_transport_3>(SYM(transport_3));
  addGSLRealFunc<gsl_sf_transport_4>(SYM(transport_4));
  addGSLRealFunc<gsl_sf_transport_5>(SYM(transport_5));

  // Trigonometric functions
  addGSLRealFunc<gsl_sf_sinc>(SYM(sinc));
  addGSLRealFunc<gsl_sf_lnsinh>(SYM(lnsinh));
  addGSLRealFunc<gsl_sf_lncosh>(SYM(lncosh));

  // Zeta functions
  addGSLSignedFunc<gsl_sf_zeta_int>(SYM(zeta),SYM(n));
  addGSLRealFunc<gsl_sf_zeta>(SYM(zeta),SYM(s));
  addGSLSignedFunc<gsl_sf_zetam1_int>(SYM(zetam1),SYM(n));
  addGSLRealFunc<gsl_sf_zetam1>(SYM(zetam1),SYM(s));
  addGSLRealRealFunc<gsl_sf_hzeta>(SYM(hzeta),SYM(s),SYM(q));
  addGSLSignedFunc<gsl_sf_eta_int>(SYM(eta),SYM(n));
  addGSLRealFunc<gsl_sf_eta>(SYM(eta),SYM(s));

  // Random number generation
  gsl_rng_env_setup();
  addFunc(GSLModule->e.ve,GSLrngInit,primVoid(),SYM(rng_init),
          formal(primString(),SYM(name),true,false));
  addFunc(GSLModule->e.ve,GSLrngList,stringArray(),SYM(rng_list));
  addFunc(GSLModule->e.ve,GSLrngSet,primVoid(),SYM(rng_set),
          formal(primInt(),SYM(seed),true,false));
  addGSLrngVoidFuncString<gsl_rng_name>(SYM(rng_name));
  addGSLrngVoidFuncInt<gsl_rng_min>(SYM(rng_min));
  addGSLrngVoidFuncInt<gsl_rng_max>(SYM(rng_max));
  addGSLrngVoidFuncInt<gsl_rng_get>(SYM(rng_get));
  addGSLrngULongFuncInt<gsl_rng_uniform_int>(SYM(rng_uniform_int),SYM(n));
  addGSLrngVoidFunc<gsl_rng_uniform>(SYM(rng_uniform));
  addGSLrngVoidFunc<gsl_rng_uniform_pos>(SYM(rng_uniform_pos));

  // Gaussian distribution
  addFunc(GSLModule->e.ve,GSLrng_gaussian,primReal(),SYM(rng_gaussian),
          formal(primReal(),SYM(mu),true,false),
          formal(primReal(),SYM(sigma),true,false),
          formal(primString(),SYM(method),true,false));
  addGSLgaussianrealRealRealRealFunc<gsl_ran_gaussian_pdf>(SYM(pdf_gaussian),
                                                           SYM(x));
  addGSLgaussianrealRealRealRealFunc<gsl_cdf_gaussian_P>(SYM(cdf_gaussian_P),
                                                         SYM(x));
  addGSLgaussianrealRealRealRealFunc<gsl_cdf_gaussian_Q>(SYM(cdf_gaussian_Q),
                                                         SYM(x));
  addGSLinvgaussianrealRealRealRealFunc<gsl_cdf_gaussian_Pinv>
    (SYM(cdf_gaussian_Pinv),SYM(x));
  addGSLinvgaussianrealRealRealRealFunc<gsl_cdf_gaussian_Qinv>
    (SYM(cdf_gaussian_Qinv),SYM(x));

  // Gaussian tail distribution
  addGSLrngRealRealFunc<gsl_ran_gaussian_tail>(SYM(rng_gaussian_tail),SYM(a),
                                               SYM(sigma));
  addGSLRealRealRealFunc<gsl_ran_gaussian_tail_pdf>(SYM(pdf_gaussian_tail),
                                                    SYM(x),SYM(a),SYM(sigma));

  // Bivariate Gaussian distribution
  addFunc(GSLModule->e.ve,GSLrng_bivariate_gaussian,primPair(),
          SYM(rng_bivariate_gaussian),
          formal(primPair(),SYM(mu),true,true),
          formal(primPair(),SYM(sigma),true,true),
          formal(primReal(),SYM(rho),true,false));
  addFunc(GSLModule->e.ve,GSLpdf_bivariate_gaussian,primReal(),
          SYM(pdf_bivariate_gaussian),
          formal(primPair(),SYM(z),false,true),
          formal(primPair(),SYM(mu),true,true),
          formal(primPair(),SYM(sigma),true,true),
          formal(primReal(),SYM(rho),true,false));

#define addGSLrealdist1param(NAME,ARG)          \
  addGSLrngRealFunc<gsl_ran_##NAME>             \
    (SYM(rng_##NAME),SYM(ARG));                 \
  addGSLRealRealFunc<gsl_ran_##NAME##_pdf>      \
    (SYM(pdf_##NAME),SYM(x),SYM(ARG));          \
  addGSLRealRealFunc<gsl_cdf_##NAME##_P>        \
    (SYM(cdf_##NAME##_P),SYM(x),SYM(ARG));      \
  addGSLRealRealFunc<gsl_cdf_##NAME##_Q>        \
    (SYM(cdf_##NAME##_Q),SYM(x),SYM(ARG));      \
  addGSLRealRealFunc<gsl_cdf_##NAME##_Pinv>     \
    (SYM(cdf_##NAME##_Pinv),SYM(P),SYM(ARG));   \
  addGSLRealRealFunc<gsl_cdf_##NAME##_Qinv>     \
    (SYM(cdf_##NAME##_Qinv),SYM(Q),SYM(ARG))

  // Exponential, Laplace, Cauchy, Rayleigh, Chi-squared, t,
  // and Logistic distribution
  addGSLrealdist1param(exponential,mu);
  addGSLrealdist1param(laplace,a);
  addGSLrealdist1param(cauchy,a);
  addGSLrealdist1param(rayleigh,mu);
  addGSLrealdist1param(chisq,mu);
  addGSLrealdist1param(tdist,mu);
  addGSLrealdist1param(logistic,mu);
#undef addGSLrealdist1param

#define addGSLrealdist2param(NAME,ARG1,ARG2)                    \
  addGSLrngRealRealFunc<gsl_ran_##NAME>                         \
    (SYM(rng_##NAME),SYM(ARG1),SYM(ARG2));                      \
  addGSLRealRealRealFunc<gsl_ran_##NAME##_pdf>                  \
    (SYM(pdf_##NAME),SYM(x),SYM(ARG1),SYM(ARG2));               \
  addGSLRealRealRealFunc<gsl_cdf_##NAME##_P>                    \
    (SYM(cdf_##NAME##_P),SYM(x),SYM(ARG1),SYM(ARG2));           \
  addGSLRealRealRealFunc<gsl_cdf_##NAME##_Q>                    \
    (SYM(cdf_##NAME##_Q),SYM(x),SYM(ARG1),SYM(ARG2));           \
  addGSLRealRealRealFunc<gsl_cdf_##NAME##_Pinv>                 \
    (SYM(cdf_##NAME##_Pinv),SYM(P),SYM(ARG1),SYM(ARG2));        \
  addGSLRealRealRealFunc<gsl_cdf_##NAME##_Qinv>                 \
    (SYM(cdf_##NAME##_Qinv),SYM(Q),SYM(ARG1),SYM(ARG2))

  // Uniform, log-normal, F, Beta, Pareto, Weibull, Type-1 Gumbel,
  // and Type-2 Gumbel distribution
  addGSLrealdist2param(flat,a,b);
  addGSLrealdist2param(lognormal,zeta,sigma);
  addGSLrealdist2param(fdist,nu1,nu2);
  addGSLrealdist2param(beta,a,b);
  addGSLrealdist2param(pareto,a,b);
  addGSLrealdist2param(weibull,a,b);
  addGSLrealdist2param(gumbel1,a,b);
  addGSLrealdist2param(gumbel2,a,b);
#undef addGSLrealdist2param

  // Exponential power distribution
  addGSLrngRealRealFunc<gsl_ran_exppow>
    (SYM(rng_exppow),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_ran_exppow_pdf>
    (SYM(pdf_exppow),SYM(x),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_exppow_P>
    (SYM(cdf_exppow_P),SYM(x),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_exppow_Q>
    (SYM(cdf_exppow_Q),SYM(x),SYM(a),SYM(b));

  // Exponential power distribution
  addGSLrngRealRealFunc<gsl_ran_rayleigh_tail>
    (SYM(rng_rayleigh_tail),SYM(a),SYM(sigma));
  addGSLRealRealRealFunc<gsl_ran_rayleigh_tail_pdf>
    (SYM(pdf_rayleigh_tail),SYM(x),SYM(a),SYM(sigma));

  // Landau distribution
  addGSLrngVoidFunc<gsl_ran_landau>(SYM(rng_landau));
  addGSLRealFunc<gsl_ran_landau_pdf>(SYM(pdf_landau),SYM(x));

  // Levy skwew alpha-stable distribution
  addFunc(GSLModule->e.ve,GSLrng_levy,primReal(),SYM(rng_levy),
          formal(primReal(),SYM(c)),
          formal(primReal(),SYM(alpha)),
          formal(primReal(),SYM(beta),true,false));

  // Gamma distribution
  addFunc(GSLModule->e.ve,GSLrng_gamma,primReal(),SYM(rng_gamma),
          formal(primReal(),SYM(a)),
          formal(primReal(),SYM(b)),
          formal(primString(),SYM(method),true,false));
  addGSLRealRealRealFunc<gsl_ran_gamma_pdf>
    (SYM(pdf_gamma),SYM(x),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_gamma_P>
    (SYM(cdf_gamma_P),SYM(x),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_gamma_Q>
    (SYM(cdf_gamma_Q),SYM(x),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_gamma_Pinv>
    (SYM(cdf_gamma_Pinv),SYM(P),SYM(a),SYM(b));
  addGSLRealRealRealFunc<gsl_cdf_gamma_Qinv>
    (SYM(cdf_gamma_Qinv),SYM(Q),SYM(a),SYM(b));

  // Sperical distributions
  addFunc(GSLModule->e.ve,GSLrng_dir2d,primPair(),SYM(rng_dir2d),
          formal(primString(),SYM(method),true,false));
  addFunc(GSLModule->e.ve,GSLrng_dir3d,primTriple(),SYM(rng_dir3d));
  addFunc(GSLModule->e.ve,GSLrng_dir,realArray(),SYM(rng_dir),
          formal(primInt(),SYM(n)));

  // Elliptic functions (Jacobi)
  addFunc(GSLModule->e.ve,GSLsf_elljac_e,realArray(),SYM(sncndn),
          formal(primReal(),SYM(u)),formal(primReal(),SYM(m)));

  // Dirirchlet distribution
  addFunc(GSLModule->e.ve,GSLrng_dirichlet,realArray(),SYM(rng_dirichlet),
          formal(realArray(),SYM(alpha)));
  addFunc(GSLModule->e.ve,GSLpdf_dirichlet,primReal(),SYM(pdf_dirichlet),
          formal(realArray(),SYM(alpha)),
          formal(realArray(),SYM(theta)));

  // General discrete distributions
  // to be implemented

#define addGSLdiscdist1param(NAME,ARG,TYPE)     \
  addGSLrng##TYPE##FuncInt<gsl_ran_##NAME>      \
    (SYM(rng_##NAME),SYM(ARG));                 \
  addGSLUInt##TYPE##Func<gsl_ran_##NAME##_pdf>  \
    (SYM(pdf_##NAME),SYM(k),SYM(ARG));          \
  addGSLUInt##TYPE##Func<gsl_cdf_##NAME##_P>    \
    (SYM(cdf_##NAME##_P),SYM(k),SYM(ARG));      \
  addGSLUInt##TYPE##Func<gsl_cdf_##NAME##_Q>    \
    (SYM(cdf_##NAME##_Q),SYM(k),SYM(ARG))

  // Poisson, geometric distributions
  addGSLdiscdist1param(poisson,mu,Real);
  addGSLdiscdist1param(geometric,p,Real);
#undef addGSLdiscdist1param

#define addGSLdiscdist2param(NAME,ARG1,TYPE1,ARG2,TYPE2)        \
  addGSLrng##TYPE1##TYPE2##FuncInt<gsl_ran_##NAME>              \
    (SYM(rng_##NAME),SYM(ARG1),SYM(ARG2));                      \
  addGSLUInt##TYPE1##TYPE2##Func<gsl_ran_##NAME##_pdf>          \
    (SYM(pdf_##NAME),SYM(k),SYM(ARG1),SYM(ARG2));               \
  addGSLUInt##TYPE1##TYPE2##Func<gsl_cdf_##NAME##_P>            \
    (SYM(cdf_##NAME##_P),SYM(k),SYM(ARG1),SYM(ARG2));           \
  addGSLUInt##TYPE1##TYPE2##Func<gsl_cdf_##NAME##_Q>            \
    (SYM(cdf_##NAME##_Q),SYM(k),SYM(ARG1),SYM(ARG2))

  // Binomial, negative binomial distributions
  addGSLdiscdist2param(binomial,p,Real,n,UInt);
  addGSLdiscdist2param(negative_binomial,p,Real,n,Real);
#undef addGSLdiscdist2param

  // Logarithmic distribution
  addGSLrngRealFuncInt<gsl_ran_logarithmic>(SYM(rng_logarithmic),SYM(p));
  addGSLUIntRealFunc<gsl_ran_logarithmic_pdf>(SYM(pdf_logarithmic),SYM(k),
                                              SYM(p));

  // Bernoulli distribution
  addGSLrngRealFuncInt<gsl_ran_bernoulli>(SYM(rng_bernoulli),SYM(p));
  addGSLUIntRealFunc<gsl_ran_bernoulli_pdf>(SYM(pdf_bernoulli),SYM(k),SYM(p));

  // Multinomial distribution
  addFunc(GSLModule->e.ve,GSLrng_multinomial,IntArray(),SYM(rng_multinomial),
          formal(primInt(),SYM(n)),
          formal(realArray(),SYM(p)));
  addFunc(GSLModule->e.ve,GSLpdf_multinomial,primReal(),SYM(pdf_multinomial),
          formal(realArray(),SYM(p)),
          formal(IntArray(),SYM(n)));

  // Hypergeometric distribution
  addGSLrngUIntUIntUIntFuncInt<gsl_ran_hypergeometric>
    (SYM(rng_hypergeometric),SYM(n1),SYM(n2),SYM(t));
  addGSLUIntUIntUIntUIntFunc<gsl_ran_hypergeometric_pdf>
    (SYM(pdf_hypergeometric),SYM(k),SYM(n1),SYM(n2),SYM(t));
  addGSLUIntUIntUIntUIntFunc<gsl_cdf_hypergeometric_P>
    (SYM(cdf_hypergeometric_P),SYM(k),SYM(n1),SYM(n2),SYM(t));
  addGSLUIntUIntUIntUIntFunc<gsl_cdf_hypergeometric_Q>
    (SYM(cdf_hypergeometric_Q),SYM(k),SYM(n1),SYM(n2),SYM(t));
}

} // namespace trans

#endif
