/*****
 * path.h
 * John Bowman
 *
 * Stores a 3D piecewise cubic spline with known control points.
 *
 *****/

#ifndef PATH3_H
#define PATH3_H

#include <cfloat>

#include "mod.h"
#include "triple.h"
#include "bbox3.h"
#include "path.h"
#include "arrayop.h"

// For CYGWIN
#undef near
#undef far

namespace camp {
  
void checkEmpty3(Int n);

// Used in the storage of solved path3 knots.
struct solvedKnot3 : public gc {
  triple pre;
  triple point;
  triple post;
  bool straight;
  solvedKnot3() : straight(false) {}
  
  friend bool operator== (const solvedKnot3& p, const solvedKnot3& q)
  {
    return p.pre == q.pre && p.point == q.point && p.post == q.post;
  }
};

class path3 : public gc {
  bool cycles;  // If the path3 is closed in a loop

  Int n; // The number of knots

  mem::vector<solvedKnot3> nodes;
  mutable double cached_length; // Cache length since path3 is immutable.
  
  mutable bbox3 box;
  mutable bbox3 times; // Times where minimum and maximum extents are attained.

public:
  path3()
    : cycles(false), n(0), nodes(), cached_length(-1) {}

  // Create a path3 of a single point
  path3(triple z, bool = false)
    : cycles(false), n(1), nodes(1), cached_length(-1)
  {
    nodes[0].pre = nodes[0].point = nodes[0].post = z;
    nodes[0].straight = false;
  }  

  // Creates path3 from a list of knots.  This will be used by camp
  // methods such as the guide solver, but should probably not be used by a
  // user of the system unless he knows what he is doing.
  path3(mem::vector<solvedKnot3>& nodes, Int n, bool cycles = false)
    : cycles(cycles), n(n), nodes(nodes), cached_length(-1)
  {
  }

  friend bool operator== (const path3& p, const path3& q)
  {
    return p.cycles == q.cycles && p.nodes == q.nodes;
  }

public:
  path3(solvedKnot3 n1, solvedKnot3 n2)
    : cycles(false), n(2), nodes(2), cached_length(-1)
  {
    nodes[0] = n1;
    nodes[1] = n2;
    nodes[0].pre = nodes[0].point;
    nodes[1].post = nodes[1].point;
  }
  
  // Copy constructor
  path3(const path3& p)
    : cycles(p.cycles), n(p.n), nodes(p.nodes), cached_length(p.cached_length),
      box(p.box)
  {}

  path3 unstraighten() const
  {
    path3 P=path3(*this);
    for(int i=0; i < n; ++i)
      P.nodes[i].straight=false;
    return P;
  }
  
  virtual ~path3()
  {
  }

  // Getting control points
  Int size() const
  {
    return n;
  }

  bool empty() const
  {
    return n == 0;
  }

  Int length() const
  {
    return cycles ? n : n-1;
  }

  bool cyclic() const
  {
    return cycles;
  }
  
  mem::vector<solvedKnot3>& Nodes() {
    return nodes;
  }
  
  bool straight(Int t) const
  {
    if (cycles) return nodes[imod(t,n)].straight;
    return (t >= 0 && t < n) ? nodes[t].straight : false;
  }
  
  bool piecewisestraight() const
  {
    Int L=length();
    for(Int i=0; i < L; ++i)
      if(!straight(i)) return false;
    return true;
  }
  
  triple point(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].point;
  }

  triple point(double t) const;
  
  triple precontrol(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].pre;
  }

  triple precontrol(double t) const;
  
  triple postcontrol(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].post;
  }

  triple postcontrol(double t) const;
  
  inline double norm(const triple& z0, const triple& c0, const triple& c1,
                     const triple& z1) const {
    return Fuzz2*camp::max((c0-z0).abs2(),
                           camp::max((c1-z0).abs2(),(z1-z0).abs2()));
  }

  triple predir(Int t, bool normalize=true) const {
    if(!cycles && t <= 0) return triple(0,0,0);
    triple z1=point(t);
    triple c1=precontrol(t);
    triple dir=3.0*(z1-c1);
    if(!normalize) return dir;
    triple z0=point(t-1);
    triple c0=postcontrol(t-1);
    double epsilon=norm(z0,c0,c1,z1);
    if(dir.abs2() > epsilon) return unit(dir);
    dir=2.0*c1-c0-z1;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(z1-z0+3.0*(c0-c1));
  }

  triple postdir(Int t, bool normalize=true) const {
    if(!cycles && t >= n-1) return triple(0,0,0);
    triple c0=postcontrol(t);
    triple z0=point(t);
    triple dir=3.0*(c0-z0);
    triple z1=point(t+1);
    triple c1=precontrol(t+1);
    double epsilon=norm(z0,c0,c1,z1);
    if(!normalize) return dir;
    if(dir.abs2() > epsilon) return unit(dir);
    dir=z0-2.0*c0+c1;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(z1-z0+3.0*(c0-c1));
  }

  triple dir(Int t, Int sign, bool normalize=true) const {
    if(sign == 0) {
      triple v=predir(t,normalize)+postdir(t,normalize);
      return normalize ? unit(v) : 0.5*v;
    }
    if(sign > 0) return postdir(t,normalize);
    return predir(t,normalize);
  }

  triple dir(double t, bool normalize=true) const {
    if(!cycles) {
      if(t <= 0) return postdir((Int) 0,normalize);
      if(t >= n-1) return predir(n-1,normalize);
    }
    Int i=Floor(t);
    t -= i;
    if(t == 0) return dir(i,0,normalize);
    triple z0=point(i);
    triple c0=postcontrol(i);
    triple c1=precontrol(i+1);
    triple z1=point(i+1);
    triple a=3.0*(z1-z0)+9.0*(c0-c1);
    triple b=6.0*(z0+c1)-12.0*c0;
    triple c=3.0*(c0-z0);
    triple dir=a*t*t+b*t+c;
    if(!normalize) return dir;
    double epsilon=norm(z0,c0,c1,z1);
    if(dir.abs2() > epsilon) return unit(dir);
    dir=2.0*a*t+b;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(a);
  }

  triple postaccel(Int t) const {
    if(!cycles && t >= n-1) return triple(0,0,0);
    triple z0=point(t);
    triple c0=postcontrol(t);
    triple c1=precontrol(t+1);
    return 6.0*(z0+c1)-12.0*c0;
  }

  triple preaccel(Int t) const {
    if(!cycles && t <= 0) return triple(0,0,0);
    triple z0=point(t-1);
    triple c0=postcontrol(t-1);
    triple c1=precontrol(t);
    triple z1=point(t);
    return 6.0*(z1+c0)-12.0*c1;
  }
  
  triple accel(Int t, Int sign) const {
    if(sign == 0) return 0.5*(preaccel(t)+postaccel(t));
    if(sign > 0) return postaccel(t);
    return preaccel(t);
  }

  triple accel(double t) const {
    if(!cycles) {
      if(t <= 0) return postaccel((Int) 0);
      if(t >= n-1) return preaccel(n-1);
    }
    Int i=Floor(t);
    t -= i;
    if(t == 0) return 0.5*(postaccel(i)+preaccel(i));
    triple z0=point(i);
    triple c0=postcontrol(i);
    triple c1=precontrol(i+1);
    triple z1=point(i+1);
    return 6.0*t*(z1-z0+3.0*(c0-c1))+6.0*(z0+c1)-12.0*c0;
  }

  // Returns the path3 traced out in reverse.
  path3 reverse() const;

  // Generates a path3 that is a section of the old path3, using the time
  // interval given.
  path3 subpath(Int start, Int end) const;
  path3 subpath(double start, double end) const;

  // Special case of subpath used by intersect.
  void halve(path3 &first, path3 &second) const;
  
  // Used by picture to determine bounding box.
  bbox3 bounds() const;
  
  triple mintimes() const {
    checkEmpty3(n);
    bounds();
    return camp::triple(times.left,times.bottom,times.near);
  }
  
  triple maxtimes() const {
    checkEmpty3(n);
    bounds();
    return camp::triple(times.right,times.top,times.far);
  }
  
  template<class T>
  void addpoint(bbox3& box, T i) const {
    box.addnonempty(point(i),times,(double) i);
  }

  double cubiclength(Int i, double goal=-1) const;
  double arclength () const;
  double arctime (double l) const;
 
  triple max() const {
    checkEmpty3(n);
    return bounds().Max();
  }

  triple min() const {
    checkEmpty3(n);
    return bounds().Min();
  }
  
  pair ratio(double (*m)(double, double)) const;
  
// Increment count if the path3 has a vertical component at t.
  bool Count(Int& count, double t) const;
  
// Count if t is in (begin,end] and z lies to the left of point(i+t).
  void countleft(Int& count, double x, Int i, double t,
                 double begin, double end, double& mint, double& maxt) const;

// Return the winding number of the region bounded by the (cyclic) path3
// relative to the point z.
  Int windingnumber(const triple& z) const;
};

double arcLength(const triple& z0, const triple& c0, const triple& c1,
                 const triple& z1);
  
path3 transformed(const vm::array& t, const path3& p);
path3 transformed(const double* t, const path3& p);
  
extern path3 nullpath3;
extern const unsigned maxdepth;
 
bool intersect(double& S, double& T, path3& p, path3& q, double fuzz,
               unsigned depth=maxdepth);
bool intersections(double& s, double& t, std::vector<double>& S,
                   std::vector<double>& T, path3& p, path3& q,
                   double fuzz, bool single, bool exact, 
                   unsigned depth=maxdepth);
void intersections(std::vector<double>& S, path3& g,
                   const triple& p, const triple& q, double fuzz);

bool intersections(std::vector<double>& T, std::vector<double>& U,
                   std::vector<double>& V, path3& p, triple *P,
                   double fuzz, bool single, unsigned depth=maxdepth);
bool intersections(double& U, double& V, const triple& v, triple *P,
                   double fuzz, unsigned depth=maxdepth);

// Concatenates two path3s into a new one.
path3 concat(const path3& p1, const path3& p2);

// return the perpendicular displacement of a point z from the line through 
// points p and q.
inline triple displacement(const triple& z, const triple& p, const triple& q)
{
  triple Z=z-p;
  triple Q=unit(q-p);
  return Z-dot(Z,Q)*Q;
}

typedef double bound_double(double *P, double (*m)(double, double), double b,
                     double fuzz, int depth);

typedef double bound_triple(triple *P, double (*m)(double, double),
                       double (*f)(const triple&), double b, double fuzz,
                       int depth);
  
bound_double bound,boundtri;

double bound(triple z0, triple c0, triple c1, triple z1,
             double (*m)(double, double),
             double (*f)(const triple&),
             double b, double fuzz, int depth=maxdepth);
double bound(double *p, double (*m)(double, double),
             double b, double fuzz, int depth);
double bound(triple *P, double (*m)(double, double),
             double (*f)(const triple&), double b, double fuzz,
             int depth);

double boundtri(double *P, double (*m)(double, double), double b,
                double fuzz, int depth);
double boundtri(triple *P, double (*m)(double, double),
                double (*f)(const triple&), double b, double fuzz,
                int depth);
}

#ifndef BROKEN_COMPILER
// Delete the following line to work around problems with old broken compilers.
GC_DECLARE_PTRFREE(camp::solvedKnot3);
#endif

#endif
