/*****
 * mathop.h
 * Tom Prince 2005/3/18
 *
 * Defines some runtime functions used by the stack machine.
 *
 *****/

#ifndef MATHOP_H
#define MATHOP_H

#include <sstream>

#include "stack.h"
#include "mod.h"
#include "triple.h"

namespace run {

template <typename T>
struct less {
  bool operator() (T x, T y, size_t=0) {return x < y;}
};

template <typename T>
struct lessequals {
  bool operator() (T x, T y, size_t=0) {return x <= y;}
};

template <typename T>
struct equals {
  bool operator() (T x, T y, size_t=0) {return x == y;}
};

template <typename T>
struct greaterequals {
  bool operator() (T x, T y, size_t=0) {return x >= y;}
};

template <typename T>
struct greater {
  bool operator() (T x, T y, size_t=0) {return x > y;}
};

template <typename T>
struct notequals {
  bool operator() (T x, T y, size_t=0) {return x != y;}
};

template <typename T>
struct And {
  bool operator() (T x, T y, size_t=0) {return x && y;}
};

template <typename T>
struct Or {
  bool operator() (T x, T y, size_t=0) {return x || y;}
};

template <typename T>
struct Xor {
  bool operator() (T x, T y, size_t=0) {return x ^ y;}
};

template <typename T>
struct plus {
  T operator() (T x, T y, size_t=0) {return x+y;}
};

template <typename T>
struct minus {
  T operator() (T x, T y, size_t=0) {return x-y;}
};
  
template <typename T>
struct times {
  T operator() (T x, T y, size_t=0) {return x*y;}
};

template <>
struct times<camp::triple> {
  camp::triple operator() (double x, camp::triple y, size_t=0) {return x*y;}
};

template <typename T>
struct timesR {
  T operator () (T y, double x, size_t=0) {return x*y;}
};

extern void dividebyzero(size_t i=0);  
extern void integeroverflow(size_t i=0);  
  
template <typename T>
struct divide {
  T operator() (T x, T y,  size_t i=0) {
    if(y == 0) dividebyzero(i);
    return x/y;
  }
};

template <>
struct divide<camp::triple> {
  camp::triple operator() (camp::triple x, double y, size_t=0) {return x/y;}
};

inline bool validInt(double x) {
  return x > Int_MIN-0.5 && x < Int_MAX+0.5;
}
  
inline void checkInt(double x, size_t i)
{
  if(validInt(x)) return;
  integeroverflow(i);
}
  
inline Int Intcast(double x)
{
  if(validInt(x)) return (Int) x;
  integeroverflow(0);
  return 0;
}
  
template<>
struct plus<Int> {
  Int operator() (Int x, Int y, size_t i=0) {
    if((y > 0 && x > Int_MAX-y) || (y < 0 && x < Int_MIN-y))
      integeroverflow(i);
    return x+y;
  }
};

template<>
struct minus<Int> {
  Int operator() (Int x, Int y, size_t i=0) {
    if((y < 0 && x > Int_MAX+y) || (y > 0 && x < Int_MIN+y))
      integeroverflow(i);
    return x-y;
  }
};

template<>
struct times<Int> {
  Int operator() (Int x, Int y, size_t i=0) {
    if(y == 0) return 0;
    if(y < 0) {y=-y; x=-x;}
    if((y > int_MAX || x > int_MAX/(int) y || x < int_MIN/(int) y) && 
       (x > Int_MAX/y || x < Int_MIN/y))
      integeroverflow(i);
    return x*y;
  }
};

template<>
struct divide<Int> {
  double operator() (Int x, Int y, size_t i=0) {
    if(y == 0) dividebyzero(i);
    return ((double) x)/(double) y;
  }
};

template <class T>
void Negate(vm::stack *s)
{
  T a=vm::pop<T>(s);
  s->push(-a);
}

inline Int Negate(Int x, size_t i=0) {
  if(x < -Int_MAX) integeroverflow(i);
  return -x;
}
  
template<>
inline void Negate<Int>(vm::stack *s)
{
  s->push(Negate(vm::pop<Int>(s)));
}

inline double pow(double x, double y)
{
  return ::pow(x,y);
}

template<class T>
T pow(T x, Int y)
{
  if(y == 0) return 1.0;
  if(x == 0.0 && y > 0) return 0.0;
  if(y < 0) {y=-y; x=1/x;}
        
  T r=1.0;
  for(;;) {
    if(y & 1) r *= x;
    if((y >>= 1) == 0)  return r;
    x *= x;
  }
}
  
template <typename T>
struct power {
  T operator() (T x, T y, size_t=0) {return pow(x,y);}
};

template <>
struct power<Int> {
  Int operator() (Int x, Int p,  size_t i=0) {
    if(p == 0) return 1;
    Int sign=1;
    if(x < 0) {
      if(p % 2) sign=-1;
      x=-x;
    }
    if(p > 0) {
      if(x == 0) return 0;
      Int r = 1;
      for(;;) {
        if(p & 1) {
          if(r > Int_MAX/x) integeroverflow(i);
          r *= x;
        }
        if((p >>= 1) == 0)
          return sign*r;
        if(x > Int_MAX/x) integeroverflow(i);
        x *= x;
      }
    } else {
      if(x == 1) return sign;
      ostringstream buf;
      if(i > 0) buf << "array element " << i << ": ";
      buf << "Only 1 and -1 can be raised to negative exponents as integers.";
      vm::error(buf);
      return 0;
    }
  }
};
 
template <typename T>
struct mod {
  T operator() (T x, T y,  size_t i=0) {
    if(y == 0) dividebyzero(i);
    return portableMod(x,y);
  }
};

template <typename>
struct quotient {
  Int operator() (Int x, Int y,  size_t i=0) {
    if(y == 0) dividebyzero(i);
    if(y == -1) return Negate(x);
// Implementation-independent definition of integer division: round down
    Int q=x/y;
    if(q >= 0 || y*q == x) return q;
    return q-1;
  }
};

template <typename T>
struct min {
  T operator() (T x, T y, size_t=0) {return x < y ? x : y;}
};

template <typename T>
struct max {
  T operator() (T x, T y, size_t=0) {return x > y ? x : y;}
};

template<class T>
inline T Min(T a, T b)
{
  return (a < b) ? a : b;
}

template<class T>
inline T Max(T a, T b)
{
  return (a > b) ? a : b;
}

template <typename T>
struct minbound {
  camp::pair operator() (camp::pair z, camp::pair w) {
    return camp::pair(Min(z.getx(),w.getx()),Min(z.gety(),w.gety()));
  }
  camp::triple operator() (camp::triple u, camp::triple v) {
    return camp::triple(Min(u.getx(),v.getx()),Min(u.gety(),v.gety()),
                        Min(u.getz(),v.getz()));
  }
};

template <typename T>
struct maxbound {
  camp::pair operator() (camp::pair z, camp::pair w) {
    return camp::pair(Max(z.getx(),w.getx()),Max(z.gety(),w.gety()));
  }
  camp::triple operator() (camp::triple u, camp::triple v) {
    return camp::triple(Max(u.getx(),v.getx()),Max(u.gety(),v.gety()),
                        Max(u.getz(),v.getz()));
  }
};

template <double (*func)(double)>
void realReal(vm::stack *s) 
{
  double x=vm::pop<double>(s);
  s->push(func(x));
}

template <class T, template <class S> class op>
void binaryOp(vm::stack *s)
{
  T b=vm::pop<T>(s);
  T a=vm::pop<T>(s);
  s->push(op<T>()(a,b));
}

template <class T>
void interp(vm::stack *s)
{
  double t=vm::pop<double>(s);
  T b=vm::pop<T>(s);
  T a=vm::pop<T>(s);
  s->push((1-t)*a+t*b);
}

} // namespace run

#endif //MATHOP_H

