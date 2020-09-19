/*****
 * triple.h
 * John Bowman
 *
 * Stores a three-dimensional point.
 *
 *****/

#ifndef TRIPLE_H
#define TRIPLE_H

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#include "common.h"
#include "angle.h"
#include "pair.h"
#if defined(MIKTEX)
#  include <algorithm>
#endif

namespace camp {

typedef double Triple[3];
  
class triple;
  
bool isIdTransform3(const double* t);
void copyTransform3(double*& d, const double* s, GCPlacement placement=NoGC);
void multiplyTransform3(double*& t, const double* s, const double* r);

void boundstriples(double& x, double& y, double& z, double& X, double& Y,
                   double& Z, size_t n, const triple* v);

class triple : virtual public gc {
  double x;
  double y;
  double z;

public:
  triple() : x(0.0), y(0.0), z(0.0) {}
  triple(double x, double y=0.0, double z=0.0) : x(x), y(y), z(z) {}
  triple(const Triple& v) : x(v[0]), y(v[1]), z(v[2]) {}

  virtual ~triple() {}
  
  void set(double X, double Y=0.0, double Z=0.0) { x=X; y=Y; z=Z; }

  double getx() const { return x; }
  double gety() const { return y; }
  double getz() const { return z; }

  // transform by row-major matrix
  friend triple operator* (const double* t, const triple& v) {
    if(t == NULL)
      return v;

    double f=t[12]*v.x+t[13]*v.y+t[14]*v.z+t[15];
    if(f != 0.0) {
      f=1.0/f;
      
      return triple((t[0]*v.x+t[1]*v.y+t[2]*v.z+t[3])*f,
                    (t[4]*v.x+t[5]*v.y+t[6]*v.z+t[7])*f,
                    (t[8]*v.x+t[9]*v.y+t[10]*v.z+t[11])*f);
    }
    reportError("division by 0 in transform of a triple");
    return 0.0;
  }
  
  friend triple operator* (const triple& v, const double* t) {
    if(t == NULL)
      return v;
    
    double f=t[3]*v.x+t[7]*v.y+t[11]*v.z+t[15];
    if(f != 0.0) {
      f=1.0/f;
      return triple((v.x*t[0]+v.y*t[4]+v.z*t[8]+t[12])*f,
                    (v.x*t[1]+v.y*t[5]+v.z*t[9]+t[13])*f,
                    (v.x*t[2]+v.y*t[6]+v.z*t[10]+t[14])*f);
    }
    reportError("division by 0 in transform of a triple");
    return 0.0;
  }
  
  friend triple Transform3(const triple& v, const double* t) {
    return triple((t[0]*v.x+t[1]*v.y+t[2]*v.z),
                  (t[3]*v.x+t[4]*v.y+t[5]*v.z),
                  (t[6]*v.x+t[7]*v.y+t[8]*v.z));
  }
  
  friend triple Transform3(const double* t, const triple& v) {
    return triple(v.x*t[0]+v.y*t[3]+v.z*t[6],
                  v.x*t[1]+v.y*t[4]+v.z*t[7],
                  v.x*t[2]+v.y*t[5]+v.z*t[8]);
  }
  
  // return x and y components of v*t.
  friend pair Transform2T(const double* t, const triple& v)
  {
    double f=t[3]*v.x+t[7]*v.y+t[11]*v.z+t[15];
    f=1.0/f;
    return pair((t[0]*v.x+t[4]*v.y+t[8]*v.z+t[12])*f,
                (t[1]*v.x+t[5]*v.y+t[9]*v.z+t[13])*f);
  }
  
  friend void transformtriples(const double* t, size_t n, triple* d,
                               const triple* s)
  {
    if(n == 0 || d == NULL || s == NULL)
      return;

    for(size_t i=0; i < n; i++)
      d[i]=t*s[i];
  }
  
  friend void copytriples(size_t n, triple* d, const triple* s)
  {
    if(d == NULL || s == NULL)
      return;
    
    for(size_t i=0; i < n; i++) d[i]=s[i];
  }

  friend void boundstriples(triple& Min, triple& Max, size_t n, const triple* v)
  {
    if(n==0 || v==NULL)
      return;

    double x,y,z;
    double X,Y,Z;
    
    X=x=v[0].getx();
    Y=y=v[0].gety();
    Z=z=v[0].getz();
    for(size_t i=1; i < n; ++i) {
      const double vx=v[i].getx();
      x=fmin(x,vx); X=fmax(X,vx);
      const double vy=v[i].gety();
      y=fmin(y,vy); Y=fmax(Y,vy);
      const double vz=v[i].getz();
      z=fmin(z,vz); Z=fmax(Z,vz);
    }
    
    Min.set(x,y,z);
    Max.set(X,Y,Z);
  }

  friend void ratiotriples(pair &b, double (*m)(double, double), bool &first,
                           size_t n, const triple* v)
  {
    if(n==0 || v==NULL)
      return;

    if(first) {
      first=false;
      const triple& v0=v[0];
      b=pair(v0.x/v0.z,v0.y/v0.z);
    }

    double x=b.getx();
    double y=b.gety();
    for(size_t i=0; i < n; ++i) {
      const triple& vi = v[i];
      x=m(x,vi.x/vi.z);
      y=m(y,vi.y/vi.z);
    }
    b=pair(x,y);
  }
  
  friend triple operator+ (const triple& z, const triple& w)
  {
    return triple(z.x + w.x, z.y + w.y, z.z + w.z);
  }

  friend triple operator- (const triple& z, const triple& w)
  {
    return triple(z.x - w.x, z.y - w.y, z.z - w.z);
  }

  friend triple operator- (const triple& z)
  {
    return triple(-z.x, -z.y, -z.z);
  }

  friend triple operator* (double s, const triple& z)
  {
    return triple(s*z.x, s*z.y, s*z.z);
  }

  friend triple operator* (const triple& z, double s)
  {
    return triple(z.x*s, z.y*s, z.z*s);
  }

  friend triple operator/ (const triple& z, double s)
  {
    if (s == 0.0)
      reportError("division by 0");
    s=1.0/s;
    return triple(z.x*s, z.y*s, z.z*s);
  }

  const triple& operator+= (const triple& w)
  {
    x += w.x;
    y += w.y;
    z += w.z;
    return *this;
  }

  const triple& operator-= (const triple& w)
  {
    x -= w.x;
    y -= w.y;
    z -= w.z;
    return *this;
  }

  friend bool operator== (const triple& z, const triple& w)
  {
    return z.x == w.x && z.y == w.y && z.z == w.z;
  }

  friend bool operator!= (const triple& z, const triple& w)
  {
    return z.x != w.x || z.y != w.y || z.z != w.z;
  }

  double abs2() const
  {
    return x*x+y*y+z*z;
  }
  
  friend double abs2(const triple &v)
  {
    return v.abs2();
  }
  
  double length() const /* r */
  {
    return sqrt(abs2());
  }
  
  friend double length(const triple& v)
  {
    return v.length();
  }

  double polar(bool warn=true) const /* theta */
  {
    double r=length();
    if(r == 0.0) {
      if(warn)
        reportError("taking polar angle of (0,0,0)");
      else
        return 0.0;
    }
    return acos(z/r);
  }
  
  double azimuth(bool warn=true) const /* phi */
  {
    return angle(x,y,warn);
  }
  
  friend triple unit(const triple& v)
  {
    double scale=v.length();
    if(scale == 0.0) return v;
    scale=1.0/scale;
    return triple(v.x*scale,v.y*scale,v.z*scale);
  }
  
  friend double dot(const triple& u, const triple& v)
  {
    return u.x*v.x+u.y*v.y+u.z*v.z;
  }

  friend triple cross(const triple& u, const triple& v) 
  {
    return triple(u.y*v.z-u.z*v.y,
                  u.z*v.x-u.x*v.z,
                  u.x*v.y-u.y*v.x);
  }

  // Returns a unit triple in the direction (theta,phi), in radians.
  friend triple expi(double theta, double phi)
  {
    double sintheta=sin(theta);
    return triple(sintheta*cos(phi),sintheta*sin(phi),cos(theta));
  }
  
  friend istream& operator >> (istream& s, triple& z)
  {
    char c;
    s >> std::ws;
    bool paren=s.peek() == '('; // parenthesis are optional
    if(paren) s >> c;
    s >> z.x >> std::ws;
    if(s.peek() == ',') s >> c >> z.y;
    else {
      if(paren) s >> z.y;
      else z.y=0.0;
    }
    if(s.peek() == ',') s >> c >> z.z;
    else {
      if(paren) s >> z.z;
      else z.z=0.0;
    }
    if(paren) {
      s >> std::ws;
      if(s.peek() == ')') s >> c;
    }
    
    return s;
  }

  friend ostream& operator << (ostream& out, const triple& v)
  {
    out << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
  }
  
  friend jsofstream& operator << (jsofstream& out, const triple& v)
  {
    out << "[" << v.x << "," << v.y << "," << v.z << "]";
    return out;
  }
  
};

triple expi(double theta, double phi);
  
// Return the component of vector v perpendicular to a unit vector u.
inline triple perp(triple v, triple u)
{
  return v-dot(v,u)*u;
}

double xratio(const triple& v);
double yratio(const triple& v);

inline void bounds(double& x, double &X, double v)
{
  if(v < x) x=v;
  else if(v > X) X=v;
}
  
inline void boundstriples(double& x, double& y, double& z,
                          double& X, double& Y, double& Z,
                          size_t n, const triple* v)
{
  X=x=v[0].getx();
  Y=y=v[0].gety();
  Z=z=v[0].getz();
    
  for(size_t i=1; i < n; ++i) {
    triple V=v[i];
    bounds(x,X,V.getx());
    bounds(y,Y,V.gety());
    bounds(z,Z,V.getz());
  }
}

extern const double third;

// return the maximum distance squared of points c0 and c1 from 
// the respective internal control points of z0--z1.
inline double Straightness(const triple& z0, const triple& c0,
                           const triple& c1, const triple& z1)
{
  triple v=third*(z1-z0);
  return std::max(abs2(c0-v-z0),abs2(z1-v-c1));
}

// Return one ninth of the relative flatness squared of a--b and c--d.
inline double Flatness(const triple& a, const triple& b, const triple& c,
                       const triple& d)
{
  static double ninth=1.0/9.0;
  triple u=b-a;
  triple v=d-c;
  return ninth*std::max(abs2(cross(u,unit(v))),abs2(cross(v,unit(u))));
}

// Return one-half of the second derivative of the Bezier curve defined by
// a,b,c,d at t=0.
inline triple bezierPP(const triple& a, const triple& b, const triple& c) {
  return 3.0*(a+c)-6.0*b;
}

// Return one-sixth of the third derivative of the Bezier curve defined by
// a,b,c,d at t=0.
inline triple bezierPPP(const triple& a, const triple& b, const triple& c,
                        const triple& d) {
  return d-a+3.0*(b-c);
}

// Return four-thirds of the first derivative of the Bezier curve defined by
// a,b,c,d at t=1/2.
inline triple bezierPh(triple a, triple b, triple c, triple d)
{
  return c+d-a-b;
}

// Return two-thirds of the second derivative of the Bezier curve defined by
// a,b,c,d at t=1/2.
inline triple bezierPPh(triple a, triple b, triple c, triple d)
{
  return 3.0*a-5.0*b+c+d;
}

} //namespace camp

GC_DECLARE_PTRFREE(camp::triple);

#endif
