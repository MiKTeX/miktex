/*****
 * pair.h
 * Andy Hammerlindl 2002/05/16
 *
 * Stores a two-dimensional point similar to the pair type in MetaPost.
 * In some cases, pairs behave as complex numbers.
 *
 * A pair is a guide as a pair alone can be used to describe a path.
 * The solve and subsolve methods are fairly straight forward as solve
 * returns a path with just the pair and subsolve just adds the pair to
 * the structure.
 *****/

#ifndef PAIR_H
#define PAIR_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>

#include "common.h"
#include "angle.h"

namespace camp {

class jsofstream : public std::ofstream {
public:
  jsofstream() {}
  jsofstream(const string& name) : std::ofstream(name.c_str()) {}
  void open(const string& name) {std::ofstream::open(name.c_str());}
  
  template<class T>
  jsofstream& operator << (const T& x) {
    (std::ofstream&)(*this) << x;
  return *this;
  }
};

class pair : public gc {
  double x;
  double y;

public:
  pair() : x(0.0), y(0.0) {}
  pair(double x, double y=0.0) : x(x), y(y) {}

  double getx() const { return x; }
  double gety() const { return y; }

  bool isreal() {return y == 0;}
  
  friend pair operator+ (const pair& z, const pair& w)
  {
    return pair(z.x+w.x,z.y+w.y);
  }

  friend pair operator- (const pair& z, const pair& w)
  {
    return pair(z.x-w.x,z.y-w.y);
  }

  friend pair operator- (const pair& z)
  {
    return pair(-z.x,-z.y);
  }

  // Complex multiplication
  friend pair operator* (const pair& z, const pair& w)
  {
    return pair(z.x*w.x-z.y*w.y,z.x*w.y+w.x*z.y);
  }

  const pair& operator+= (const pair& w)
  {
    x += w.x;
    y += w.y;
    return *this;
  }

  const pair& operator-= (const pair& w)
  {
    x -= w.x;
    y -= w.y;
    return *this;
  }

  const pair& operator*= (const pair& w)
  {
    (*this) = (*this) * w;
    return (*this);
  }

  const pair& operator/= (const pair& w)
  {
    (*this) = (*this) / w;
    return (*this);
  }

  const pair& scale (double xscale, double yscale)
  {
    x *= xscale;
    y *= yscale;
    return *this;
  }

  friend pair operator/ (const pair &z, double t)
  {
    if (t == 0.0)
      reportError("division by 0");
    t=1.0/t;
    return pair(z.x*t, z.y*t);
  }

  friend pair operator/ (const pair& z, const pair& w)
  {
    if (!w.nonZero())
      reportError("division by pair (0,0)");

    double t = 1.0 / (w.x*w.x + w.y*w.y);
    return pair(t*(z.x*w.x + z.y*w.y),
                t*(-z.x*w.y + w.x*z.y));
  }

  friend bool operator== (const pair& z, const pair& w)
  {
    return z.x == w.x && z.y == w.y;
  }

  friend bool operator!= (const pair& z, const pair& w)
  {
    return z.x != w.x || z.y != w.y;
  }

  double abs2() const
  {
    return x*x + y*y;
  }
  
  double length() const
  {
    return sqrt(abs2());
  }
  
  friend double length(const pair& z)
  {
    return z.length();
  }

  double angle(bool warn=true) const
  {
    return camp::angle(x,y,warn);
  }
  
  friend double angle(const pair& z, bool warn=true)
  {
    return z.angle(warn);
  }
  
  friend pair unit(const pair& z)
  {
    double scale=z.length();
    if(scale == 0.0) return z;
    scale=1.0/scale;
    return pair(z.x*scale,z.y*scale);
  }
  
  friend pair conj(const pair& z)
  {
    return pair(z.x,-z.y);
  }
  
  friend double dot(const pair& z, const pair& w)
  {
    return z.x*w.x+z.y*w.y;
  }
  
  friend double cross(const pair& z, const pair& w)
  {
    return z.x*w.y-z.y*w.x;
  }
  
// Return the principal branch of the square root (non-negative real part).
  friend pair Sqrt(const pair& z) {
    double mag=z.length();
    if(mag == 0.0) return pair(0.0,0.0);
    else if(z.x > 0) {
      double re=sqrt(0.5*(mag+z.x));
      return pair(re,0.5*z.y/re);
    } else {
      double im=sqrt(0.5*(mag-z.x));
      if(z.y < 0) im=-im;
      return pair(0.5*z.y/im,im);
    }
  }

  bool nonZero() const
  {
    return x != 0.0 || y != 0.0;
  }

  friend istream& operator >> (istream& s, pair& z)
  {
    char c;
    s >> std::ws;
    bool paren=s.peek() == '('; // parenthesis are optional
    if(paren) s >> c;
    s >> z.x >> std::ws;
    if(!s.eof() && s.peek() == ',') s >> c >> z.y;
    else {
      if(paren && !s.eof()) s >> z.y;
      else z.y=0.0;
    }
    if(paren) {
      s >> std::ws;
      if(s.peek() == ')') s >> c;
    }
    
    return s;
  }

  friend ostream& operator << (ostream& out, const pair& z)
  {
    out << "(" << z.x << "," << z.y << ")";
    return out;
  }
  
  friend jsofstream& operator << (jsofstream& out, const pair& z)
  {
    out << "[" << z.x << "," << z.y << "]";
    return out;
  }
  
  friend class box;
};

// Calculates exp(i * theta), useful for unit vectors.
inline pair expi(double theta)
{
  if(theta == 0.0) return pair(1.0,0.0); // Frequently occurring case
  return pair(cos(theta),sin(theta));
}

// Complex exponentiation
inline pair pow(const pair& z, const pair& w)
{
  double u=w.getx();
  double v=w.gety();
  if(z == 0.0) return w == 0.0 ? 1.0 : 0.0;
  double logr=0.5*log(z.abs2());
  double th=z.angle();
  return exp(logr*u-th*v)*expi(logr*v+th*u);
}

} //namespace camp

GC_DECLARE_PTRFREE(camp::pair);

#endif
