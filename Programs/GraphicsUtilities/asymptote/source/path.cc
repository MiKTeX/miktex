/*****
 * path.cc
 * Andy Hammerlindl 2002/06/06
 *
 * Stores and returns information on a predefined path.
 *
 * When changing the path algorithms, also update the corresponding 
 * three-dimensional algorithms in path3.cc.
 *****/

#include "path.h"
#include "util.h"
#include "angle.h"
#include "camperror.h"
#include "mathop.h"
#include "predicates.h"
#include "rounding.h"

namespace camp {

const double Fuzz2=1000.0*DBL_EPSILON;
const double Fuzz=sqrt(Fuzz2);
const double Fuzz4=Fuzz2*Fuzz2;
const double BigFuzz=10.0*Fuzz2;
const double fuzzFactor=100.0;

const double third=1.0/3.0;

path nullpath;
  
const char *nopoints="nullpath has no points";

void checkEmpty(Int n) {
  if(n == 0)
    reportError(nopoints);
}

// Accurate computation of sqrt(1+x)-1.
inline double sqrt1pxm1(double x)
{
  return x/(sqrt(1.0+x)+1.0);
}
inline pair sqrt1pxm1(pair x)
{
  return x/(Sqrt(1.0+x)+1.0);
}
  
// Solve for the real roots of the quadratic equation ax^2+bx+c=0.
quadraticroots::quadraticroots(double a, double b, double c)
{
  // Remove roots at numerical infinity.
  if(fabs(a) <= Fuzz2*fabs(b)+Fuzz4*fabs(c)) {
    if(fabs(b) > Fuzz2*fabs(c)) {
      distinct=quadraticroots::ONE;
      roots=1;
      t1=-c/b;
    } else if(c == 0.0) {
      distinct=quadraticroots::MANY;
      roots=1;
      t1=0.0;
    } else {
      distinct=quadraticroots::NONE;
      roots=0;
    }
  } else {
    double factor=0.5*b/a;
    double denom=b*factor;
    if(fabs(denom) <= Fuzz2*fabs(c)) {
      double x=-c/a;
      if(x >= 0.0) {
        distinct=quadraticroots::TWO;
        roots=2;
        t2=sqrt(x);
        t1=-t2;
      } else {
        distinct=quadraticroots::NONE;
        roots=0;
      }
    } else {
      double x=-2.0*c/denom;
      if(x > -1.0) {
        distinct=quadraticroots::TWO;
        roots=2;
        double r2=factor*sqrt1pxm1(x);
        double r1=-r2-2.0*factor;
        if(r1 <= r2) {
          t1=r1;
          t2=r2;
        } else {
          t1=r2;
          t2=r1;
        }
      } else if(x == -1.0) {
        distinct=quadraticroots::ONE;
        roots=2;
        t1=t2=-factor;
      } else {
        distinct=quadraticroots::NONE;
        roots=0;
      }
    }
  }
}

// Solve for the complex roots of the quadratic equation ax^2+bx+c=0.
Quadraticroots::Quadraticroots(pair a, pair b, pair c)
{
  if(a == 0.0) {
    if(b != 0.0) {
      roots=1;
      z1=-c/b;
    } else if(c == 0.0) {
      roots=1;
      z1=0.0;
    } else
      roots=0;
  } else {
    roots=2;
    pair factor=0.5*b/a;
    pair denom=b*factor;
    if(denom == 0.0) {
      z1=Sqrt(-c/a);
      z2=-z1;
    } else {
      z1=factor*sqrt1pxm1(-2.0*c/denom);
      z2=-z1-2.0*factor;
    }
  }
}

inline bool goodroot(double a, double b, double c, double t)
{
  return goodroot(t) && quadratic(a,b,c,t) >= 0.0;
}

// Accurate computation of cbrt(sqrt(1+x)+1)-cbrt(sqrt(1+x)-1).
inline double cbrtsqrt1pxm(double x)
{
  double s=sqrt1pxm1(x);
  return 2.0/(cbrt(x+2.0*(sqrt(1.0+x)+1.0))+cbrt(x)+cbrt(s*s));
}
  
// Taylor series of cos((atan(1.0/w)+pi)/3.0).
static inline double costhetapi3(double w)
{
  static const double c1=1.0/3.0;
  static const double c3=-19.0/162.0;
  static const double c5=425.0/5832.0;
  static const double c7=-16829.0/314928.0;
  double w2=w*w;
  double w3=w2*w;
  double w5=w3*w2;
  return c1*w+c3*w3+c5*w5+c7*w5*w2;
}
      
// Solve for the real roots of the cubic equation ax^3+bx^2+cx+d=0.
cubicroots::cubicroots(double a, double b, double c, double d) 
{
  static const double ninth=1.0/9.0;
  static const double fiftyfourth=1.0/54.0;
  
  // Remove roots at numerical infinity.
  if(fabs(a) <= Fuzz2*(fabs(b)+fabs(c)*Fuzz2+fabs(d)*Fuzz4)) {
    quadraticroots q(b,c,d);
    roots=q.roots;
    if(q.roots >= 1) t1=q.t1;
    if(q.roots == 2) t2=q.t2;
    return;
  }
  
  // Detect roots at numerical zero.
  if(fabs(d) <= Fuzz2*(fabs(c)+fabs(b)*Fuzz2+fabs(a)*Fuzz4)) {
    quadraticroots q(a,b,c);
    roots=q.roots+1;
    t1=0;
    if(q.roots >= 1) t2=q.t1;
    if(q.roots == 2) t3=q.t2;
    return;
  }
  
  b /= a;
  c /= a;
  d /= a;
  
  double b2=b*b;
  double Q=3.0*c-b2;
  if(fabs(Q) < Fuzz2*(3.0*fabs(c)+fabs(b2)))
    Q=0.0;
  
  double R=(3.0*Q+b2)*b-27.0*d;
  if(fabs(R) < Fuzz2*((3.0*fabs(Q)+fabs(b2))*fabs(b)+27.0*fabs(d)))
    R=0.0;
  
  Q *= ninth;
  R *= fiftyfourth;
  
  double Q3=Q*Q*Q;
  double R2=R*R;
  double D=Q3+R2;
  double mthirdb=-b*third;
  
  if(D > 0.0) {
    roots=1;
    t1=mthirdb;
    if(R2 != 0.0) t1 += cbrt(R)*cbrtsqrt1pxm(Q3/R2);
  } else {
    roots=3;
    double v=0.0,theta;
    if(R2 > 0.0) {
      v=sqrt(-D/R2);
      theta=atan(v);
    } else theta=0.5*PI;
    double factor=2.0*sqrt(-Q)*(R >= 0 ? 1 : -1);
      
    t1=mthirdb+factor*cos(third*theta);
    t2=mthirdb-factor*cos(third*(theta-PI));
    t3=mthirdb;
    if(R2 > 0.0)
      t3 -= factor*((v < 100.0) ? cos(third*(theta+PI)) : costhetapi3(1.0/v)); 
  }
}
  
pair path::point(double t) const
{
  checkEmpty(n);
    
  Int i = Floor(t);
  Int iplus;
  t = fmod(t,1);
  if (t < 0) t += 1;

  if (cycles) {
    i = imod(i,n);
    iplus = imod(i+1,n);
  }
  else if (i < 0)
    return nodes[0].point;
  else if (i >= n-1)
    return nodes[n-1].point;
  else
    iplus = i+1;

  double one_t = 1.0-t;

  pair a = nodes[i].point,
    b = nodes[i].post,
    c = nodes[iplus].pre,
    d = nodes[iplus].point,
    ab   = one_t*a   + t*b,
    bc   = one_t*b   + t*c,
    cd   = one_t*c   + t*d,
    abc  = one_t*ab  + t*bc,
    bcd  = one_t*bc  + t*cd,
    abcd = one_t*abc + t*bcd;

  return abcd;
}

pair path::precontrol(double t) const
{
  checkEmpty(n);
                     
  Int i = Floor(t);
  Int iplus;
  t = fmod(t,1);
  if (t < 0) t += 1;

  if (cycles) {
    i = imod(i,n);
    iplus = imod(i+1,n);
  }
  else if (i < 0)
    return nodes[0].pre;
  else if (i >= n-1)
    return nodes[n-1].pre;
  else
    iplus = i+1;

  double one_t = 1.0-t;

  pair a = nodes[i].point,
    b = nodes[i].post,
    c = nodes[iplus].pre,
    ab   = one_t*a   + t*b,
    bc   = one_t*b   + t*c,
    abc  = one_t*ab  + t*bc;

  return (abc == a) ? nodes[i].pre : abc;
}
        
 
pair path::postcontrol(double t) const
{
  checkEmpty(n);
  
  Int i = Floor(t);
  Int iplus;
  t = fmod(t,1);
  if (t < 0) t += 1;

  if (cycles) {
    i = imod(i,n);
    iplus = imod(i+1,n);
  }
  else if (i < 0)
    return nodes[0].post;
  else if (i >= n-1)
    return nodes[n-1].post;
  else
    iplus = i+1;

  double one_t = 1.0-t;
  
  pair b = nodes[i].post,
    c = nodes[iplus].pre,
    d = nodes[iplus].point,
    bc   = one_t*b   + t*c,
    cd   = one_t*c   + t*d,
    bcd  = one_t*bc  + t*cd;

  return (bcd == d) ? nodes[iplus].post : bcd;
}

path path::reverse() const
{
  mem::vector<solvedKnot> nodes(n);
  Int len=length();
  for (Int i = 0, j = len; i < n; i++, j--) {
    nodes[i].pre = postcontrol(j);
    nodes[i].point = point(j);
    nodes[i].post = precontrol(j);
    nodes[i].straight = straight(j-1);
  }
  return path(nodes, n, cycles);
}

path path::subpath(Int a, Int b) const
{
  if(empty()) return path();

  if (a > b) {
    const path &rp = reverse();
    Int len=length();
    path result = rp.subpath(len-a, len-b);
    return result;
  }

  if (!cycles) {
    if (a < 0) {
      a = 0;
      if(b < 0)
        b = 0;
    }
    if (b > n-1) {
      b = n-1;
      if(a > b)
        a = b;
    }
  }

  Int sn = b-a+1;
  mem::vector<solvedKnot> nodes(sn);

  for (Int i = 0, j = a; j <= b; i++, j++) {
    nodes[i].pre = precontrol(j);
    nodes[i].point = point(j);
    nodes[i].post = postcontrol(j);
    nodes[i].straight = straight(j);
  }
  nodes[0].pre = nodes[0].point;
  nodes[sn-1].post = nodes[sn-1].point;

  return path(nodes, sn);
}

inline pair split(double t, const pair& x, const pair& y) { return x+(y-x)*t; }

inline void splitCubic(solvedKnot sn[], double t, const solvedKnot& left_,
                       const solvedKnot& right_)
{
  solvedKnot &left=(sn[0]=left_), &mid=sn[1], &right=(sn[2]=right_);
  if(left.straight) {
    mid.point=split(t,left.point,right.point);
    pair deltaL=third*(mid.point-left.point);
    left.post=left.point+deltaL;
    mid.pre=mid.point-deltaL;
    pair deltaR=third*(right.point-mid.point);
    mid.post=mid.point+deltaR;
    right.pre=right.point-deltaR;
    mid.straight=true;
  } else {
    pair x=split(t,left.post,right.pre); // m1
    left.post=split(t,left.point,left.post); // m0
    right.pre=split(t,right.pre,right.point); // m2
    mid.pre=split(t,left.post,x); // m3
    mid.post=split(t,x,right.pre); // m4 
    mid.point=split(t,mid.pre,mid.post); // m5
  }
}

path path::subpath(double a, double b) const
{
  if(empty()) return path();
  
  if (a > b) {
    const path &rp = reverse();
    Int len=length();
    return rp.subpath(len-a, len-b);
  }

  solvedKnot aL, aR, bL, bR;
  if (!cycles) {
    if (a < 0) {
      a = 0;
      if (b < 0)
        b = 0;
    }   
    if (b > n-1) {
      b = n-1;
      if (a > b)
        a = b;
    }
    aL = nodes[(Int)floor(a)];
    aR = nodes[(Int)ceil(a)];
    bL = nodes[(Int)floor(b)];
    bR = nodes[(Int)ceil(b)];
  } else {
    if(run::validInt(a) && run::validInt(b)) {
      aL = nodes[imod((Int) floor(a),n)];
      aR = nodes[imod((Int) ceil(a),n)];
      bL = nodes[imod((Int) floor(b),n)];
      bR = nodes[imod((Int) ceil(b),n)];
    } else reportError("invalid path index");
  }

  if (a == b) return path(point(a));

  solvedKnot sn[3];
  path p = subpath(Ceil(a), Floor(b));
  if (a > floor(a)) {
    if (b < ceil(a)) {
      splitCubic(sn,a-floor(a),aL,aR);
      splitCubic(sn,(b-a)/(ceil(b)-a),sn[1],sn[2]);
      return path(sn[0],sn[1]);
    }
    splitCubic(sn,a-floor(a),aL,aR);
    p=concat(path(sn[1],sn[2]),p);
  }
  if (ceil(b) > b) {
    splitCubic(sn,b-floor(b),bL,bR);
    p=concat(p,path(sn[0],sn[1]));
  }
  return p;
}

// Special case of subpath for paths of length 1 used by intersect.
void path::halve(path &first, path &second) const
{
  solvedKnot sn[3];
  splitCubic(sn,0.5,nodes[0],nodes[1]);
  first=path(sn[0],sn[1]);
  second=path(sn[1],sn[2]);
}
  
// Calculate the coefficients of a Bezier derivative divided by 3.
static inline void derivative(pair& a, pair& b, pair& c,
                              const pair& z0, const pair& c0,
                              const pair& c1, const pair& z1)
{
  a=z1-z0+3.0*(c0-c1);
  b=2.0*(z0+c1)-4.0*c0;
  c=c0-z0;
}

bbox path::bounds() const
{
  if(!box.empty) return box;
  
  if (empty()) {
    // No bounds
    return bbox();
  }
  
  Int len=length();
  box.add(point(len));
  times=bbox(len,len,len,len);

  for (Int i = 0; i < len; i++) {
    addpoint(box,i);
    if(straight(i)) continue;
    
    pair a,b,c;
    derivative(a,b,c,point(i),postcontrol(i),precontrol(i+1),point(i+1));
    
    // Check x coordinate
    quadraticroots x(a.getx(),b.getx(),c.getx());
    if(x.distinct != quadraticroots::NONE && goodroot(x.t1))
      addpoint(box,i+x.t1);
    if(x.distinct == quadraticroots::TWO && goodroot(x.t2))
      addpoint(box,i+x.t2);
    
    // Check y coordinate
    quadraticroots y(a.gety(),b.gety(),c.gety());
    if(y.distinct != quadraticroots::NONE && goodroot(y.t1))
      addpoint(box,i+y.t1);
    if(y.distinct == quadraticroots::TWO && goodroot(y.t2))
      addpoint(box,i+y.t2);
  }
  return box;
}

bbox path::bounds(double min, double max) const
{
  bbox box;
  
  Int len=length();
  for (Int i = 0; i < len; i++) {
    addpoint(box,i,min,max);
    if(straight(i)) continue;
    
    pair a,b,c;
    derivative(a,b,c,point(i),postcontrol(i),precontrol(i+1),point(i+1));
    
    // Check x coordinate
    quadraticroots x(a.getx(),b.getx(),c.getx());
    if(x.distinct != quadraticroots::NONE && goodroot(x.t1))
      addpoint(box,i+x.t1,min,max);

    if(x.distinct == quadraticroots::TWO && goodroot(x.t2))
      addpoint(box,i+x.t2,min,max);
    
    // Check y coordinate
    quadraticroots y(a.gety(),b.gety(),c.gety());
    if(y.distinct != quadraticroots::NONE && goodroot(y.t1))
      addpoint(box,i+y.t1,min,max);
    if(y.distinct == quadraticroots::TWO && goodroot(y.t2))
      addpoint(box,i+y.t2,min,max);
  }
  addpoint(box,len,min,max);
  return box;
}
  
inline void add(bbox& box, const pair& z, const pair& min, const pair& max)
{
  box += z+min;
  box += z+max;
}

bbox path::internalbounds(const bbox& padding) const
{
  bbox box;
  
  // Check interior nodes.
  Int len=length();
  for (Int i = 1; i < len; i++) {
    pair pre=point(i)-precontrol(i);
    pair post=postcontrol(i)-point(i);
    
    // Check node x coordinate
    if((pre.getx() >= 0.0) ^ (post.getx() >= 0))
      add(box,point(i),padding.left,padding.right);
                              
    // Check node y coordinate
    if((pre.gety() >= 0.0) ^ (post.gety() >= 0))
      add(box,point(i),pair(0,padding.bottom),pair(0,padding.top));
  }
                              
  // Check interior segments.
  for (Int i = 0; i < len; i++) {
    if(straight(i)) continue;
    
    pair a,b,c;
    derivative(a,b,c,point(i),postcontrol(i),precontrol(i+1),point(i+1));
    
    // Check x coordinate
    quadraticroots x(a.getx(),b.getx(),c.getx());
    if(x.distinct != quadraticroots::NONE && goodroot(x.t1))
      add(box,point(i+x.t1),padding.left,padding.right);
    if(x.distinct == quadraticroots::TWO && goodroot(x.t2))
      add(box,point(i+x.t2),padding.left,padding.right);
    
    // Check y coordinate
    quadraticroots y(a.gety(),b.gety(),c.gety());
    if(y.distinct != quadraticroots::NONE && goodroot(y.t1))
      add(box,point(i+y.t1),pair(0,padding.bottom),pair(0,padding.top));
    if(y.distinct == quadraticroots::TWO && goodroot(y.t2))
      add(box,point(i+y.t2),pair(0,padding.bottom),pair(0,padding.top));
  }
  return box;
}

// {{{ Arclength Calculations

static pair a,b,c;

static double ds(double t)
{
  double dx=quadratic(a.getx(),b.getx(),c.getx(),t);
  double dy=quadratic(a.gety(),b.gety(),c.gety(),t);
  return sqrt(dx*dx+dy*dy);
}

// Calculates arclength of a cubic Bezier curve using adaptive Simpson
// integration.
double arcLength(const pair& z0, const pair& c0, const pair& c1,
                 const pair& z1)
{
  double integral;
  derivative(a,b,c,z0,c0,c1,z1);
  
  if(!simpson(integral,ds,0.0,1.0,DBL_EPSILON,1.0))
    reportError("nesting capacity exceeded in computing arclength");
  return integral;
}

double path::cubiclength(Int i, double goal) const
{
  const pair& z0=point(i);
  const pair& z1=point(i+1);
  double L;
  if(straight(i)) {
    L=(z1-z0).length();
    return (goal < 0 || goal >= L) ? L : -goal/L;
  }
  
  double integral=arcLength(z0,postcontrol(i),precontrol(i+1),z1);

  L=3.0*integral;
  if(goal < 0 || goal >= L) return L;
  
  double t=goal/L;
  goal *= third;
  static double dxmin=sqrt(DBL_EPSILON);
  if(!unsimpson(goal,ds,0.0,t,100.0*DBL_EPSILON,integral,1.0,dxmin))
    reportError("nesting capacity exceeded in computing arctime");
  return -t;
}

double path::arclength() const 
{
  if (cached_length != -1) return cached_length;

  double L=0.0;
  for (Int i = 0; i < n-1; i++) {
    L += cubiclength(i);
  }
  if(cycles) L += cubiclength(n-1);
  cached_length = L;
  return cached_length;
}

double path::arctime(double goal) const
{
  if (cycles) {
    if (goal == 0 || cached_length == 0) return 0;
    if (goal < 0)  {
      const path &rp = this->reverse();
      double result = -rp.arctime(-goal);
      return result;
    }
    if (cached_length > 0 && goal >= cached_length) {
      Int loops = (Int)(goal / cached_length);
      goal -= loops*cached_length;
      return loops*n+arctime(goal);
    }      
  } else {
    if (goal <= 0)
      return 0;
    if (cached_length > 0 && goal >= cached_length)
      return n-1;
  }
    
  double l,L=0;
  for (Int i = 0; i < n-1; i++) {
    l = cubiclength(i,goal);
    if (l < 0)
      return (-l+i);
    else {
      L += l;
      goal -= l;
      if (goal <= 0)
        return i+1;
    }
  }
  if (cycles) {
    l = cubiclength(n-1,goal);
    if (l < 0)
      return -l+n-1;
    if (cached_length > 0 && cached_length != L+l) {
      reportError("arclength != length.\n"
                  "path::arclength(double) must have broken semantics.\n"
                  "Please report this error.");
    }
    cached_length = L += l;
    goal -= l;
    return arctime(goal)+n;
  }
  else {
    cached_length = L;
    return length();
  }
}

// }}}

// {{{ Direction Time Calulation
// Algorithm Stolen from Knuth's MetaFont
inline double cubicDir(const solvedKnot& left, const solvedKnot& right,
                       const pair& rot)
{
  pair a,b,c;
  derivative(a,b,c,left.point,left.post,right.pre,right.point);
  a *= rot; b *= rot; c *= rot;
  
  quadraticroots ret(a.gety(),b.gety(),c.gety());
  switch(ret.distinct) {
    case quadraticroots::MANY:
    case quadraticroots::ONE:
    {
      if(goodroot(a.getx(),b.getx(),c.getx(),ret.t1)) return ret.t1;
    } break;

    case quadraticroots::TWO:
    {
      if(goodroot(a.getx(),b.getx(),c.getx(),ret.t1)) return ret.t1;
      if(goodroot(a.getx(),b.getx(),c.getx(),ret.t2)) return ret.t2;
    } break;

    case quadraticroots::NONE:
      break;
  }

  return -1;
}

// TODO: Check that we handle corner cases.
// Velocity(t) == (0,0)
double path::directiontime(const pair& dir) const {
  if (dir == pair(0,0)) return 0;
  pair rot = pair(1,0)/unit(dir);
    
  double t; double pre,post;
  for (Int i = 0; i < n-1+cycles; ) {
    t = cubicDir(this->nodes[i],(cycles && i==n-1) ? nodes[0]:nodes[i+1],rot);
    if (t >= 0) return i+t;
    i++;
    if (cycles || i != n-1) {
      pair Pre = (point(i)-precontrol(i))*rot;
      pair Post = (postcontrol(i)-point(i))*rot;
      static pair zero(0.0,0.0);
      if(Pre != zero && Post != zero) {
        pre = angle(Pre);
        post = angle(Post);
        if ((pre <= 0 && post >= 0 && pre >= post - PI) ||
            (pre >= 0 && post <= 0 && pre <= post + PI))
          return i;
      }
    }
  }
  
  return -1;
}
// }}}

// {{{ Path Intersection Calculations

const unsigned maxdepth=DBL_MANT_DIG;
const unsigned mindepth=maxdepth-16;

void roots(std::vector<double> &roots, double a, double b, double c, double d)
{
  cubicroots r(a,b,c,d);
  if(r.roots >= 1) roots.push_back(r.t1);
  if(r.roots >= 2) roots.push_back(r.t2);
  if(r.roots == 3) roots.push_back(r.t3);
}
  
void roots(std::vector<double> &r, double x0, double c0, double c1, double x1,
           double x)
{
  double a=x1-x0+3.0*(c0-c1);
  double b=3.0*(x0+c1)-6.0*c0;
  double c=3.0*(c0-x0);
  double d=x0-x;
  roots(r,a,b,c,d);
}

// Return all intersection times of path g with the pair z.
void intersections(std::vector<double>& T, const path& g, const pair& z,
                   double fuzz)
{
  double fuzz2=fuzz*fuzz;
  Int n=g.length();
  bool cycles=g.cyclic();
  for(Int i=0; i < n; ++i) {
    // Check both directions to circumvent degeneracy.
    std::vector<double> r;
    roots(r,g.point(i).getx(),g.postcontrol(i).getx(),
          g.precontrol(i+1).getx(),g.point(i+1).getx(),z.getx());
    roots(r,g.point(i).gety(),g.postcontrol(i).gety(),
          g.precontrol(i+1).gety(),g.point(i+1).gety(),z.gety());
    
    size_t m=r.size();
    for(size_t j=0 ; j < m; ++j) {
      double t=r[j];
      if(t >= -Fuzz2 && t <= 1.0+Fuzz2) {
        double s=i+t;
        if((g.point(s)-z).abs2() <= fuzz2) {
          if(cycles && s >= n-Fuzz2) s=0;
          T.push_back(s);
        }
      }
    }
  }
}

inline bool online(const pair&p, const pair& q, const pair& z, double fuzz)
{
  if(p == q) return (z-p).abs2() <= fuzz*fuzz;
  return (z.getx()-p.getx())*(q.gety()-p.gety()) ==
    (q.getx()-p.getx())*(z.gety()-p.gety());
}

// Return all intersection times of path g with the (infinite)
// line through p and q; if there are an infinite number of intersection points,
// the returned list is guaranteed to include the endpoint times of
// the intersection if endpoints=true.
void lineintersections(std::vector<double>& T, const path& g,
                       const pair& p, const pair& q, double fuzz,
                       bool endpoints=false)
{
  Int n=g.length();
  if(n == 0) {
    if(online(p,q,g.point((Int) 0),fuzz)) T.push_back(0.0);
    return;
  }
  bool cycles=g.cyclic();
  double dx=q.getx()-p.getx();
  double dy=q.gety()-p.gety();
  double det=p.gety()*q.getx()-p.getx()*q.gety();
  for(Int i=0; i < n; ++i) {
    pair z0=g.point(i);
    pair c0=g.postcontrol(i);
    pair c1=g.precontrol(i+1);
    pair z1=g.point(i+1);
    pair t3=z1-z0+3.0*(c0-c1);
    pair t2=3.0*(z0+c1)-6.0*c0;
    pair t1=3.0*(c0-z0);
    double a=dy*t3.getx()-dx*t3.gety();
    double b=dy*t2.getx()-dx*t2.gety();
    double c=dy*t1.getx()-dx*t1.gety();
    double d=dy*z0.getx()-dx*z0.gety()+det;
    std::vector<double> r;
    if(max(max(max(a*a,b*b),c*c),d*d) >
       Fuzz4*max(max(max(z0.abs2(),z1.abs2()),c0.abs2()),c1.abs2()))
      roots(r,a,b,c,d);
    else r.push_back(0.0);
    if(endpoints) {
      path h=g.subpath(i,i+1);
      intersections(r,h,p,fuzz);
      intersections(r,h,q,fuzz);
      if(online(p,q,z0,fuzz)) r.push_back(0.0);
      if(online(p,q,z1,fuzz)) r.push_back(1.0);
    }
    size_t m=r.size();
    for(size_t j=0 ; j < m; ++j) {
      double t=r[j];
      if(t >= -Fuzz2 && t <= 1.0+Fuzz2) {
        double s=i+t;
        if(cycles && s >= n-Fuzz2) s=0;
        T.push_back(s);
      }
    }
  }
}

// An optimized implementation of intersections(g,p--q);
// if there are an infinite number of intersection points, the returned list is
// only guaranteed to include the endpoint times of the intersection.
void intersections(std::vector<double>& S, std::vector<double>& T,
                   const path& g, const pair& p, const pair& q, double fuzz)
{
  double length2=(q-p).abs2();
  if(length2 == 0.0) {
    std::vector<double> S1;
    intersections(S1,g,p,fuzz);
    size_t n=S1.size();
    for(size_t i=0; i < n; ++i) {
      S.push_back(S1[i]);
      T.push_back(0.0);
    }
  } else {
    pair factor=(q-p)/length2;
    std::vector<double> S1;
    lineintersections(S1,g,p,q,fuzz,true);
    size_t n=S1.size();
    for(size_t i=0; i < n; ++i) {
      double s=S1[i];
      double t=dot(g.point(s)-p,factor);
      if(t >= -Fuzz2 && t <= 1.0+Fuzz2) {
        S.push_back(s);
        T.push_back(t);
      }
    }
  }
}

void add(std::vector<double>& S, double s, const path& p, double fuzz2)
{
  pair z=p.point(s);
  size_t n=S.size();
  for(size_t i=0; i < n; ++i)
    if((p.point(S[i])-z).abs2() <= fuzz2) return;
  S.push_back(s);
}
  
void add(std::vector<double>& S, std::vector<double>& T, double s, double t,
         const path& p, double fuzz2)
{
  pair z=p.point(s);
  size_t n=S.size();
  for(size_t i=0; i < n; ++i)
    if((p.point(S[i])-z).abs2() <= fuzz2) return;
  S.push_back(s);
  T.push_back(t);
}
  
void add(double& s, double& t, std::vector<double>& S, std::vector<double>& T,
         std::vector<double>& S1, std::vector<double>& T1,
         double pscale, double qscale, double poffset, double qoffset,
         const path& p, double fuzz2, bool single)
{
  if(single) {
    s=s*pscale+poffset;
    t=t*qscale+qoffset;
  } else {
    size_t n=S1.size();
    for(size_t i=0; i < n; ++i)
      add(S,T,pscale*S1[i]+poffset,qscale*T1[i]+qoffset,p,fuzz2);
  }
}

void add(double& s, double& t, std::vector<double>& S, std::vector<double>& T,
         std::vector<double>& S1, std::vector<double>& T1,
         const path& p, double fuzz2, bool single)
{
  size_t n=S1.size();
  if(single) {
    if(n > 0) {
      s=S1[0];
      t=T1[0];
    }
  } else {
    for(size_t i=0; i < n; ++i)
      add(S,T,S1[i],T1[i],p,fuzz2);
  }
}

void intersections(std::vector<double>& S, path& g,
                   const pair& p, const pair& q, double fuzz)
{       
  double fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);
  std::vector<double> S1;
  lineintersections(S1,g,p,q,fuzz);
  size_t n=S1.size();
  for(size_t i=0; i < n; ++i)
    add(S,S1[i],g,fuzz2);
}

bool intersections(double &s, double &t, std::vector<double>& S,
                   std::vector<double>& T, path& p, path& q,
                   double fuzz, bool single, bool exact, unsigned depth)
{
  if(errorstream::interrupt) throw interrupted();
  
  double fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);
  
  Int lp=p.length();
  if(((lp == 1 && p.straight(0)) || lp == 0) && exact) {
    std::vector<double> T1,S1;
    intersections(T1,S1,q,p.point((Int) 0),p.point(lp),fuzz);
    add(s,t,S,T,S1,T1,p,fuzz2,single);
    return S1.size() > 0;
  }

  Int lq=q.length();
  if(((lq == 1 && q.straight(0)) || lq == 0) && exact) {
    std::vector<double> S1,T1;
    intersections(S1,T1,p,q.point((Int) 0),q.point(lq),fuzz);
    add(s,t,S,T,S1,T1,p,fuzz2,single);
    return S1.size() > 0;
  }
  
  pair maxp=p.max();
  pair minp=p.min();
  pair maxq=q.max();
  pair minq=q.min();
  
  if(maxp.getx()+fuzz >= minq.getx() &&
     maxp.gety()+fuzz >= minq.gety() && 
     maxq.getx()+fuzz >= minp.getx() &&
     maxq.gety()+fuzz >= minp.gety()) {
    // Overlapping bounding boxes

    --depth;
//    fuzz *= 2;
//    fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);

    if((maxp-minp).length()+(maxq-minq).length() <= fuzz || depth == 0) {
      if(single) {
        s=0.5;
        t=0.5;
      } else {
        S.push_back(0.5);
        T.push_back(0.5);
      }
      return true;
    }
    
    path p1,p2;
    double pscale,poffset;
    std::vector<double> S1,T1;
    
    if(lp <= 1) {
      if(lp == 1) p.halve(p1,p2);
      if(lp == 0 || p1 == p || p2 == p) {
        intersections(T1,S1,q,p.point((Int) 0),p.point((Int) 0),fuzz);
        add(s,t,S,T,S1,T1,p,fuzz2,single);
        return S1.size() > 0;
      }
      pscale=poffset=0.5;
    } else {
      Int tp=lp/2;
      p1=p.subpath(0,tp);
      p2=p.subpath(tp,lp);
      poffset=tp;
      pscale=1.0;
    }
      
    path q1,q2;
    double qscale,qoffset;
    
    if(lq <= 1) {
      if(lq == 1) q.halve(q1,q2);
      if(lq == 0 || q1 == q || q2 == q) {
        intersections(S1,T1,p,q.point((Int) 0),q.point((Int) 0),fuzz);
        add(s,t,S,T,S1,T1,p,fuzz2,single);
        return S1.size() > 0;
      }
      qscale=qoffset=0.5;
    } else {
      Int tq=lq/2;
      q1=q.subpath(0,tq);
      q2=q.subpath(tq,lq);
      qoffset=tq;
      qscale=1.0;
    }
      
    bool Short=lp == 1 && lq == 1;
    
    static size_t maxcount=9;
    size_t count=0;
    
    if(intersections(s,t,S1,T1,p1,q1,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,0.0,0.0,p,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
     
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p1,q2,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,0.0,qoffset,p,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p2,q1,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,poffset,0.0,p,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p2,q2,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,poffset,qoffset,p,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    return S.size() > 0;
  }
  return false;
}

// }}}

ostream& operator<< (ostream& out, const path& p)
{
  Int n = p.length();
  if(n < 0)
    out << "<nullpath>";
  else {
    for(Int i = 0; i < n; i++) {
      out << p.point(i);
      if(p.straight(i)) out << "--";
      else
        out << ".. controls " << p.postcontrol(i) << " and "
            << p.precontrol(i+1) << newl << " ..";
    }
    if(p.cycles) 
      out << "cycle";
    else
      out << p.point(n);
  }
  return out;
}

path concat(const path& p1, const path& p2)
{
  Int n1 = p1.length(), n2 = p2.length();

  if (n1 == -1) return p2;
  if (n2 == -1) return p1;

  mem::vector<solvedKnot> nodes(n1+n2+1);

  Int i = 0;
  nodes[0].pre = p1.point((Int) 0);
  for (Int j = 0; j < n1; j++) {
    nodes[i].point = p1.point(j);
    nodes[i].straight = p1.straight(j);
    nodes[i].post = p1.postcontrol(j);
    nodes[i+1].pre = p1.precontrol(j+1);
    i++;
  }
  for (Int j = 0; j < n2; j++) {
    nodes[i].point = p2.point(j);
    nodes[i].straight = p2.straight(j);
    nodes[i].post = p2.postcontrol(j);
    nodes[i+1].pre = p2.precontrol(j+1);
    i++;
  }
  nodes[i].point = nodes[i].post = p2.point(n2);

  return path(nodes, i+1);
}

// Interface to orient2d predicate optimized for pairs.
double orient2d(const pair& a, const pair& b, const pair& c)
{
  double detleft, detright, det;
  double detsum, errbound;
  double orient;

  FPU_ROUND_DOUBLE;

  detleft = (a.getx() - c.getx()) * (b.gety() - c.gety());
  detright = (a.gety() - c.gety()) * (b.getx() - c.getx());
  det = detleft - detright;

  if (detleft > 0.0) {
    if (detright <= 0.0) {
      FPU_RESTORE;
      return det;
    } else {
      detsum = detleft + detright;
    }
  } else if (detleft < 0.0) {
    if (detright >= 0.0) {
      FPU_RESTORE;
      return det;
    } else {
      detsum = -detleft - detright;
    }
  } else {
    FPU_RESTORE;
    return det;
  }

  errbound = ccwerrboundA * detsum;
  if ((det >= errbound) || (-det >= errbound)) {
    FPU_RESTORE;
    return det;
  }

  double pa[]={a.getx(),a.gety()};
  double pb[]={b.getx(),b.gety()};
  double pc[]={c.getx(),c.gety()};
  
  orient = orient2dadapt(pa, pb, pc, detsum);
  FPU_RESTORE;
  return orient;
}

// Returns true iff the point z lies in or on the bounding box
// of a,b,c, and d.
bool insidebbox(const pair& a, const pair& b, const pair& c, const pair& d,
                const pair& z)
{
  bbox B(a);
  B.addnonempty(b);
  B.addnonempty(c);
  B.addnonempty(d);
  return B.left <= z.getx() && z.getx() <= B.right && B.bottom <= z.gety() 
    && z.gety() <= B.top;
}

inline bool inrange(double x0, double x1, double x)
{
  return (x0 <= x && x <= x1) || (x1 <= x && x <= x0);
}

// Return true if point z is on z0--z1; otherwise compute contribution to 
// winding number.
bool checkstraight(const pair& z0, const pair& z1, const pair& z, Int& count)
{
  if(z0.gety() <= z.gety() && z.gety() <= z1.gety()) {
    double side=orient2d(z0,z1,z);
    if(side == 0.0 && inrange(z0.getx(),z1.getx(),z.getx()))
      return true;
    if(z.gety() < z1.gety() && side > 0) ++count;
  } else if(z1.gety() <= z.gety() && z.gety() <= z0.gety()) {
    double side=orient2d(z0,z1,z);
    if(side == 0.0 && inrange(z0.getx(),z1.getx(),z.getx()))
      return true;
    if(z.gety() < z0.gety() && side < 0) --count;
  }
  return false;
}

// returns true if point is on curve; otherwise compute contribution to 
// winding number.
bool checkcurve(const pair& z0, const pair& c0, const pair& c1,
               const pair& z1, const pair& z, Int& count, unsigned depth) 
{
  if(depth == 0) return true;
  --depth;

  if(insidebbox(z0,c0,c1,z1,z)) {
    const pair m0=0.5*(z0+c0);
    const pair m1=0.5*(c0+c1);
    const pair m2=0.5*(c1+z1);
    const pair m3=0.5*(m0+m1);
    const pair m4=0.5*(m1+m2);
    const pair m5=0.5*(m3+m4);
    if(checkcurve(z0,m0,m3,m5,z,count,depth) || 
       checkcurve(m5,m4,m2,z1,z,count,depth)) return true;
  } else
    if(checkstraight(z0,z1,z,count)) return true;
  return false;
}

// Return the winding number of the region bounded by the (cyclic) path
// relative to the point z, or the largest odd integer if the point lies on
// the path.
Int path::windingnumber(const pair& z) const
{
  static const Int undefined=Int_MAX % 2 ? Int_MAX : Int_MAX-1;
  
  if(!cycles)
    reportError("path is not cyclic");
  
  bbox b=bounds();
  
  if(z.getx() < b.left || z.getx() > b.right ||
     z.gety() < b.bottom || z.gety() > b.top) return 0;
  
  Int count=0;
  for(Int i=0; i < n; ++i)
    if(straight(i)) {
      if(checkstraight(point(i),point(i+1),z,count))
        return undefined;
    } else
      if(checkcurve(point(i),postcontrol(i),precontrol(i+1),point(i+1),z,count,
                    maxdepth)) return undefined;
  return count;
}

path path::transformed(const transform& t) const
{
  mem::vector<solvedKnot> nodes(n);

  for (Int i = 0; i < n; ++i) {
    nodes[i].pre = t * this->nodes[i].pre;
    nodes[i].point = t * this->nodes[i].point;
    nodes[i].post = t * this->nodes[i].post;
    nodes[i].straight = this->nodes[i].straight;
  }

  path p(nodes, n, cyclic());
  return p;
}

path transformed(const transform& t, const path& p)
{
  Int n = p.size();
  mem::vector<solvedKnot> nodes(n);

  for (Int i = 0; i < n; ++i) {
    nodes[i].pre = t * p.precontrol(i);
    nodes[i].point = t * p.point(i);
    nodes[i].post = t * p.postcontrol(i);
    nodes[i].straight = p.straight(i);
  }

  return path(nodes, n, p.cyclic());
}

path nurb(pair z0, pair z1, pair z2, pair z3,
          double w0, double w1, double w2, double w3, Int m)
{
  mem::vector<solvedKnot> nodes(m+1);

  if(m < 1) reportError("invalid sampling interval");

  double step=1.0/m;
  for(Int i=0; i <= m; ++i) { 
    double t=i*step;
    double t2=t*t;
    double onemt=1.0-t;
    double onemt2=onemt*onemt;
    double W0=w0*onemt2*onemt;
    double W1=w1*3.0*t*onemt2;
    double W2=w2*3.0*t2*onemt;
    double W3=w3*t2*t;
    nodes[i].point=(W0*z0+W1*z1+W2*z2+W3*z3)/(W0+W1+W2+W3);
  }
  
  static const double twothirds=2.0/3.0;
  pair z=nodes[0].point;
  nodes[0].pre=z;
  nodes[0].post=twothirds*z+third*nodes[1].point;
  for(int i=1; i < m; ++i) {
    pair z0=nodes[i].point;
    pair zm=nodes[i-1].point;
    pair zp=nodes[i+1].point;
    pair pre=twothirds*z0+third*zm;
    pair pos=twothirds*z0+third*zp;
    pair dir=unit(pos-pre);
    nodes[i].pre=z0-length(z0-pre)*dir;
    nodes[i].post=z0+length(pos-z0)*dir;
  }
  z=nodes[m].point;
  nodes[m].pre=twothirds*z+third*nodes[m-1].point;
  nodes[m].post=z;
  return path(nodes,m+1);
}

} //namespace camp
