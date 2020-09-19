/*****
 * path.h
 * Andy Hammerlindl 2002/05/16
 *
 * Stores a piecewise cubic spline with known control points.
 *
 * When changing the path algorithms, also update the corresponding 
 * three-dimensional algorithms in path3.cc and three.asy.
 *****/

#ifndef PATH_H
#define PATH_H

#include <cfloat>

#include "mod.h"
#include "pair.h"
#include "transform.h"
#include "bbox.h"

inline double Intcap(double t) {
  if(t <= Int_MIN) return Int_MIN;
  if(t >= Int_MAX) return Int_MAX;
  return t;
}
  
// The are like floor and ceil, except they return an integer;
// if the argument cannot be converted to a valid integer, they return
// Int_MAX (for positive arguments) or Int_MIN (for negative arguments).

inline Int Floor(double t) {return (Int) floor(Intcap(t));}
inline Int Ceil(double t) {return (Int) ceil(Intcap(t));}

bool simpson(double& integral, double (*)(double), double a, double b,
             double acc, double dxmax);

bool unsimpson(double integral, double (*)(double), double a, double& b,
               double acc, double& area, double dxmax, double dxmin=0);

namespace camp {

void checkEmpty(Int n);
  
inline Int adjustedIndex(Int i, Int n, bool cycles)
{
  checkEmpty(n);
  if(cycles)
    return imod(i,n);
  else if(i < 0)
    return 0;
  else if(i >= n)
    return n-1;
  else
    return i;
}

// Used in the storage of solved path knots.
struct solvedKnot : public gc {
  pair pre;
  pair point;
  pair post;
  bool straight;
  solvedKnot() : straight(false) {}
  
  friend bool operator== (const solvedKnot& p, const solvedKnot& q)
  {
    return p.pre == q.pre && p.point == q.point && p.post == q.post;
  }
};

extern const double Fuzz;
extern const double Fuzz2;
extern const double Fuzz4;
extern const double sqrtFuzz;
extern const double BigFuzz;
extern const double fuzzFactor;
  
class path : public gc {
  bool cycles;  // If the path is closed in a loop

  Int n; // The number of knots

  mem::vector<solvedKnot> nodes;
  mutable double cached_length; // Cache length since path is immutable.
  
  mutable bbox box;
  mutable bbox times; // Times where minimum and maximum extents are attained.

public:
  path()
    : cycles(false), n(0), nodes(), cached_length(-1) {}

  // Create a path of a single point
  path(pair z, bool = false)
    : cycles(false), n(1), nodes(1), cached_length(-1)
  {
    nodes[0].pre = nodes[0].point = nodes[0].post = z;
    nodes[0].straight = false;
  }  

  // Creates path from a list of knots.  This will be used by camp
  // methods such as the guide solver, but should probably not be used by a
  // user of the system unless he knows what he is doing.
  path(mem::vector<solvedKnot>& nodes, Int n, bool cycles = false)
    : cycles(cycles), n(n), nodes(nodes), cached_length(-1)
  {
  }

  friend bool operator== (const path& p, const path& q)
  {
    return p.cycles == q.cycles && p.nodes == q.nodes;
  }

public:
  path(solvedKnot n1, solvedKnot n2)
    : cycles(false), n(2), nodes(2), cached_length(-1)
  {
    nodes[0] = n1;
    nodes[1] = n2;
    nodes[0].pre = nodes[0].point;
    nodes[1].post = nodes[1].point;
  }
  
  // Copy constructor
  path(const path& p)
    : cycles(p.cycles), n(p.n), nodes(p.nodes), cached_length(p.cached_length),
      box(p.box)
  {}

  path unstraighten() const
  {
    path P=path(*this);
    for(int i=0; i < n; ++i)
      P.nodes[i].straight=false;
    return P;
  }
  
  virtual ~path()
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
  
  mem::vector<solvedKnot>& Nodes() {
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
  
  pair point(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].point;
  }

  pair point(double t) const;
  
  pair precontrol(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].pre;
  }

  pair precontrol(double t) const;
  
  pair postcontrol(Int t) const
  {
    return nodes[adjustedIndex(t,n,cycles)].post;
  }

  pair postcontrol(double t) const;
  
  inline double norm(const pair& z0, const pair& c0, const pair& c1,
                     const pair& z1) const {
    return Fuzz2*camp::max((c0-z0).abs2(),
                           camp::max((c1-z0).abs2(),(z1-z0).abs2()));
  }

  pair predir(Int t, bool normalize=true) const {
    if(!cycles && t <= 0) return pair(0,0);
    pair z1=point(t);
    pair c1=precontrol(t);
    pair dir=3.0*(z1-c1);
    if(!normalize) return dir;
    pair z0=point(t-1);
    pair c0=postcontrol(t-1);
    double epsilon=norm(z0,c0,c1,z1);
    if(dir.abs2() > epsilon) return unit(dir);
    dir=2.0*c1-c0-z1;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(z1-z0+3.0*(c0-c1));
  }

  pair postdir(Int t, bool normalize=true) const {
    if(!cycles && t >= n-1) return pair(0,0);
    pair c0=postcontrol(t);
    pair z0=point(t);
    pair dir=3.0*(c0-z0);
    if(!normalize) return dir;
    pair z1=point(t+1);
    pair c1=precontrol(t+1);
    double epsilon=norm(z0,c0,c1,z1);
    if(dir.abs2() > epsilon) return unit(dir);
    dir=z0-2.0*c0+c1;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(z1-z0+3.0*(c0-c1));
  }

  pair dir(Int t, Int sign, bool normalize=true) const {
    if(sign == 0) {
      pair v=predir(t,normalize)+postdir(t,normalize);
      return normalize ? unit(v) : 0.5*v;
    }
    if(sign > 0) return postdir(t,normalize);
    return predir(t,normalize);
  }

  pair dir(double t, bool normalize=true) const {
    if(!cycles) {
      if(t <= 0) return postdir((Int) 0,normalize);
      if(t >= n-1) return predir(n-1,normalize);
    }
    Int i=Floor(t);
    t -= i;
    if(t == 0) return dir(i,0,normalize);
    pair z0=point(i);
    pair c0=postcontrol(i);
    pair c1=precontrol(i+1);
    pair z1=point(i+1);
    pair a=3.0*(z1-z0)+9.0*(c0-c1);
    pair b=6.0*(z0+c1)-12.0*c0;
    pair c=3.0*(c0-z0);
    pair dir=a*t*t+b*t+c;
    if(!normalize) return dir;
    double epsilon=norm(z0,c0,c1,z1);
    if(dir.abs2() > epsilon) return unit(dir);
    dir=2.0*a*t+b;
    if(dir.abs2() > epsilon) return unit(dir);
    return unit(a);
  }

  pair postaccel(Int t) const {
    if(!cycles && t >= n-1) return pair(0,0);
    pair z0=point(t);
    pair c0=postcontrol(t);
    pair c1=precontrol(t+1);
    return 6.0*(z0+c1)-12.0*c0;
  }

  pair preaccel(Int t) const {
    if(!cycles && t <= 0) return pair(0,0);
    pair c0=postcontrol(t-1);
    pair c1=precontrol(t);
    pair z1=point(t);
    return 6.0*(z1+c0)-12.0*c1;
  }
  
  pair accel(Int t, Int sign) const {
    if(sign == 0) return 0.5*(preaccel(t)+postaccel(t));
    if(sign > 0) return postaccel(t);
    return preaccel(t);
  }

  pair accel(double t) const {
    if(!cycles) {
      if(t <= 0) return postaccel((Int) 0);
      if(t >= n-1) return preaccel(n-1);
    }
    Int i=Floor(t);
    t -= i;
    if(t == 0) return 0.5*(postaccel(i)+preaccel(i));
    pair z0=point(i);
    pair c0=postcontrol(i);
    pair c1=precontrol(i+1);
    pair z1=point(i+1);
    return 6.0*t*(z1-z0+3.0*(c0-c1))+6.0*(z0+c1)-12.0*c0;
  }

  // Returns the path traced out in reverse.
  path reverse() const;

  // Generates a path that is a section of the old path, using the time
  // interval given.
  path subpath(Int start, Int end) const;
  path subpath(double start, double end) const;

  // Special case of subpath used by intersect.
  void halve(path &first, path &second) const;
  
  // Used by picture to determine bounding box.
  bbox bounds() const;
  
  pair mintimes() const {
    checkEmpty(n);
    bounds();
    return camp::pair(times.left,times.bottom);
  }
  
  pair maxtimes() const {
    checkEmpty(n);
    bounds();
    return camp::pair(times.right,times.top);
  }
  
  template<class T>
  void addpoint(bbox& box, T i) const {
    box.addnonempty(point(i),times,(double) i);
  }

  template<class T>
  void addpoint(bbox& box, T i, double min, double max) const {
    static const pair I(0,1);
    pair v=I*dir(i);
    pair z=point(i);
    box.add(z+min*v);
    box.addnonempty(z+max*v);
  }

  // Return bounding box accounting for padding perpendicular to path.
  bbox bounds(double min, double max) const;
  
  // Return bounding box accounting for internal pen padding (but not pencap).
  bbox internalbounds(const bbox &padding) const;
  
  double cubiclength(Int i, double goal=-1) const;
  double arclength () const;
  double arctime (double l) const;
  double directiontime(const pair& z) const;
 
  pair max() const {
    checkEmpty(n);
    return bounds().Max();
  }

  pair min() const {
    checkEmpty(n);
    return bounds().Min();
  }
  
  // Debugging output
  friend std::ostream& operator<< (std::ostream& out, const path& p);

// Increment count if the path has a vertical component at t.
  bool Count(Int& count, double t) const;
  
// Count if t is in (begin,end] and z lies to the left of point(i+t).
  void countleft(Int& count, double x, Int i, double t,
                 double begin, double end, double& mint, double& maxt) const;

// Return the winding number of the region bounded by the (cyclic) path
// relative to the point z.
  Int windingnumber(const pair& z) const;

  // Transformation
  path transformed(const transform& t) const;
  
};

double arcLength(const pair& z0, const pair& c0, const pair& c1,
                 const pair& z1);

extern path nullpath;
extern const unsigned maxdepth;
extern const unsigned mindepth;
extern const char *nopoints;
 
bool intersect(double& S, double& T, path& p, path& q, double fuzz,
               unsigned depth=maxdepth);
bool intersections(double& s, double& t, std::vector<double>& S,
                   std::vector<double>& T, path& p, path& q,
                   double fuzz, bool single, bool exact,
                   unsigned depth=maxdepth);
void intersections(std::vector<double>& S, path& g,
                   const pair& p, const pair& q, double fuzz);

  
// Concatenates two paths into a new one.
path concat(const path& p1, const path& p2);

// Applies a transformation to the path
path transformed(const transform& t, const path& p);
  
inline double quadratic(double a, double b, double c, double x)
{
  return a*x*x+b*x+c;
}
  
class quadraticroots {
public:
  enum {NONE=0, ONE=1, TWO=2, MANY} distinct; // Number of distinct real roots.
  unsigned roots; // Total number of real roots.
  double t1,t2;   // Real roots
  
  quadraticroots(double a, double b, double c);
};
  
class Quadraticroots {
public:
  unsigned roots; // Total number of roots.
  pair z1,z2;     // Complex roots
  Quadraticroots(pair a, pair b, pair c);
};

class cubicroots {
public:  
  unsigned roots; // Total number of real roots.
  double t1,t2,t3;
  cubicroots(double a, double b, double c, double d);
};

path nurb(pair z0, pair z1, pair z2, pair z3,
          double w0, double w1, double w2, double w3, Int m);
  
double orient2d(const pair& a, const pair& b, const pair& c);

void roots(std::vector<double> &roots, double a, double b, double c, double d);
void roots(std::vector<double> &r, double x0, double c0, double c1, double x1,
           double x);
  
inline bool goodroot(double t)
{
  return 0.0 <= t && t <= 1.0;
}

extern const double third;

}

#ifndef BROKEN_COMPILER
// Delete the following line to work around problems with old broken compilers.
GC_DECLARE_PTRFREE(camp::solvedKnot);
#endif

#endif
