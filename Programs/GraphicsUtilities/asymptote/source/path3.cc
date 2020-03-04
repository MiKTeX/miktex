/*****
 * path3.cc
 * John Bowman
 *
 * Compute information for a three-dimensional path.
 *****/

#include <cfloat>

#include "path3.h"
#include "util.h"
#include "camperror.h"
#include "mathop.h"

namespace camp {

using run::operator *;
using vm::array;

path3 nullpath3;
  
void checkEmpty3(Int n) {
  if(n == 0)
    reportError("nullpath3 has no points");
}

triple path3::point(double t) const
{
  checkEmpty3(n);
    
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

  triple a = nodes[i].point,
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

triple path3::precontrol(double t) const
{
  checkEmpty3(n);
                     
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

  triple a = nodes[i].point,
    b = nodes[i].post,
    c = nodes[iplus].pre,
    ab   = one_t*a   + t*b,
    bc   = one_t*b   + t*c,
    abc  = one_t*ab  + t*bc;

  return (abc == a) ? nodes[i].pre : abc;
}
        
 
triple path3::postcontrol(double t) const
{
  checkEmpty3(n);
  
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
  
  triple b = nodes[i].post,
    c = nodes[iplus].pre,
    d = nodes[iplus].point,
    bc   = one_t*b   + t*c,
    cd   = one_t*c   + t*d,
    bcd  = one_t*bc  + t*cd;

  return (bcd == d) ? nodes[iplus].post : bcd;
}

path3 path3::reverse() const
{
  mem::vector<solvedKnot3> nodes(n);
  Int len=length();
  for (Int i = 0, j = len; i < n; i++, j--) {
    nodes[i].pre = postcontrol(j);
    nodes[i].point = point(j);
    nodes[i].post = precontrol(j);
    nodes[i].straight = straight(j-1);
  }
  return path3(nodes, n, cycles);
}

path3 path3::subpath(Int a, Int b) const
{
  if(empty()) return path3();

  if (a > b) {
    const path3 &rp = reverse();
    Int len=length();
    path3 result = rp.subpath(len-a, len-b);
    return result;
  }

  if (!cycles) {
    if (a < 0)
      a = 0;
    if (b > n-1)
      b = n-1;
  }

  Int sn = b-a+1;
  mem::vector<solvedKnot3> nodes(sn);

  for (Int i = 0, j = a; j <= b; i++, j++) {
    nodes[i].pre = precontrol(j);
    nodes[i].point = point(j);
    nodes[i].post = postcontrol(j);
    nodes[i].straight = straight(j);
  }
  nodes[0].pre = nodes[0].point;
  nodes[sn-1].post = nodes[sn-1].point;

  return path3(nodes, sn);
}

inline triple split(double t, const triple& x, const triple& y) {
  return x+(y-x)*t;
}

inline void splitCubic(solvedKnot3 sn[], double t, const solvedKnot3& left_,
                       const solvedKnot3& right_)
{
  solvedKnot3 &left=(sn[0]=left_), &mid=sn[1], &right=(sn[2]=right_);
  if(left.straight) {
    mid.point=split(t,left.point,right.point);
    triple deltaL=third*(mid.point-left.point);
    left.post=left.point+deltaL;
    mid.pre=mid.point-deltaL;
    triple deltaR=third*(right.point-mid.point);
    mid.post=mid.point+deltaR;
    right.pre=right.point-deltaR;
    mid.straight=true;
  } else {
    triple x=split(t,left.post,right.pre); // m1
    left.post=split(t,left.point,left.post); // m0
    right.pre=split(t,right.pre,right.point); // m2
    mid.pre=split(t,left.post,x); // m3
    mid.post=split(t,x,right.pre); // m4 
    mid.point=split(t,mid.pre,mid.post); // m5
  }
}

path3 path3::subpath(double a, double b) const
{
  if(empty()) return path3();
  
  if (a > b) {
    const path3 &rp = reverse();
    Int len=length();
    return rp.subpath(len-a, len-b);
  }

  solvedKnot3 aL, aR, bL, bR;
  if (!cycles) {
    if (a < 0) {
      a = 0;
      if (b < 0)
        b = 0;
    }   
    if (b > n-1) {
      b = n-1;
      if (a > n-1)
        a = n-1;
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
    } else reportError("invalid path3 index");
  }

  if (a == b) return path3(point(a));

  solvedKnot3 sn[3];
  path3 p = subpath(Ceil(a), Floor(b));
  if (a > floor(a)) {
    if (b < ceil(a)) {
      splitCubic(sn,a-floor(a),aL,aR);
      splitCubic(sn,(b-a)/(ceil(b)-a),sn[1],sn[2]);
      return path3(sn[0],sn[1]);
    }
    splitCubic(sn,a-floor(a),aL,aR);
    p=concat(path3(sn[1],sn[2]),p);
  }
  if (ceil(b) > b) {
    splitCubic(sn,b-floor(b),bL,bR);
    p=concat(p,path3(sn[0],sn[1]));
  }
  return p;
}

// Special case of subpath for paths of length 1 used by intersect.
void path3::halve(path3 &first, path3 &second) const
{
  solvedKnot3 sn[3];
  splitCubic(sn,0.5,nodes[0],nodes[1]);
  first=path3(sn[0],sn[1]);
  second=path3(sn[1],sn[2]);
}
  
// Calculate the coefficients of a Bezier derivative divided by 3.
static inline void derivative(triple& a, triple& b, triple& c,
                              const triple& z0, const triple& c0,
                              const triple& c1, const triple& z1)
{
  a=z1-z0+3.0*(c0-c1);
  b=2.0*(z0+c1)-4.0*c0;
  c=c0-z0;
}

bbox3 path3::bounds() const
{
  if(!box.empty) return box;
  
  if (empty()) {
    // No bounds
    return bbox3();
  }
  
  Int len=length();
  box.add(point(len));
  times=bbox3(len,len,len,len,len,len);

  for (Int i = 0; i < len; i++) {
    addpoint(box,i);
    if(straight(i)) continue;
    
    triple a,b,c;
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
    
    // Check z coordinate
    quadraticroots z(a.getz(),b.getz(),c.getz());
    if(z.distinct != quadraticroots::NONE && goodroot(z.t1))
      addpoint(box,i+z.t1);
    if(z.distinct == quadraticroots::TWO && goodroot(z.t2))
      addpoint(box,i+z.t2);
  }
  return box;
}

// Return f evaluated at controlling vertex of bounding box of convex hull for
// similiar-triangle transform x'=x/z, y'=y/z, where z < 0.
double ratiobound(triple z0, triple c0, triple c1, triple z1,
                  double (*m)(double, double), 
                  double (*f)(const triple&))
{
  double MX=m(m(m(-z0.getx(),-c0.getx()),-c1.getx()),-z1.getx());
  double MY=m(m(m(-z0.gety(),-c0.gety()),-c1.gety()),-z1.gety());
  double Z=m(m(m(z0.getz(),c0.getz()),c1.getz()),z1.getz());
  double MZ=m(m(m(-z0.getz(),-c0.getz()),-c1.getz()),-z1.getz());
  return m(f(triple(-MX,-MY,Z)),f(triple(-MX,-MY,-MZ)));
}

double bound(triple z0, triple c0, triple c1, triple z1,
             double (*m)(double, double),
             double (*f)(const triple&), double b, double fuzz, int depth)
{
  b=m(b,m(f(z0),f(z1)));
  if(m(-1.0,1.0)*(b-ratiobound(z0,c0,c1,z1,m,f)) >= -fuzz || depth == 0)
    return b;
  
  --depth;
  fuzz *= 2;

  triple m0=0.5*(z0+c0);
  triple m1=0.5*(c0+c1);
  triple m2=0.5*(c1+z1);
  triple m3=0.5*(m0+m1);
  triple m4=0.5*(m1+m2);
  triple m5=0.5*(m3+m4);

  // Check both Bezier subpaths.
  b=bound(z0,m0,m3,m5,m,f,b,fuzz,depth);
  return bound(m5,m4,m2,z1,m,f,b,fuzz,depth);
}

pair path3::ratio(double (*m)(double, double)) const
{
  double fuzz=Fuzz*(max()-min()).length();
  checkEmpty3(n);
  
  triple v=point((Int) 0);
  pair B=pair(xratio(v),yratio(v));
  
  Int n=length();
  for(Int i=0; i <= n; ++i) {
    if(straight(i)) {
      triple v=point(i);
      B=pair(m(B.getx(),xratio(v)),m(B.gety(),yratio(v)));
    } else {
      triple z0=point(i);
      triple c0=postcontrol(i);
      triple c1=precontrol(i+1);
      triple z1=point(i+1);
      B=pair(bound(z0,c0,c1,z1,m,xratio,B.getx(),fuzz),
             bound(z0,c0,c1,z1,m,yratio,B.gety(),fuzz));
    }
  }
  return B;
}

// {{{ Arclength Calculations

static triple a,b,c;

static double ds(double t)
{
  double dx=quadratic(a.getx(),b.getx(),c.getx(),t);
  double dy=quadratic(a.gety(),b.gety(),c.gety(),t);
  double dz=quadratic(a.getz(),b.getz(),c.getz(),t);
  return sqrt(dx*dx+dy*dy+dz*dz);
}

// Calculates arclength of a cubic Bezier curve using adaptive Simpson
// integration.
double arcLength(const triple& z0, const triple& c0, const triple& c1,
                 const triple& z1)
{
  double integral;
  derivative(a,b,c,z0,c0,c1,z1);
  
  if(!simpson(integral,ds,0.0,1.0,DBL_EPSILON,1.0))
    reportError("nesting capacity exceeded in computing arclength");
  return integral;
}

double path3::cubiclength(Int i, double goal) const
{
  const triple& z0=point(i);
  const triple& z1=point(i+1);
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

double path3::arclength() const
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

double path3::arctime(double goal) const
{
  if (cycles) {
    if (goal == 0 || cached_length == 0) return 0;
    if (goal < 0)  {
      const path3 &rp = this->reverse();
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
                  "path3::arclength(double) must have broken semantics.\n"
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

// {{{ Path3 Intersection Calculations

// Return all intersection times of path3 g with the triple v.
void intersections(std::vector<double>& T, const path3& g, const triple& v,
                   double fuzz)
{
  double fuzz2=fuzz*fuzz;
  Int n=g.length();
  bool cycles=g.cyclic();
  for(Int i=0; i < n; ++i) {
    // Check all directions to circumvent degeneracy.
    std::vector<double> r;
    roots(r,g.point(i).getx(),g.postcontrol(i).getx(),
          g.precontrol(i+1).getx(),g.point(i+1).getx(),v.getx());
    roots(r,g.point(i).gety(),g.postcontrol(i).gety(),
          g.precontrol(i+1).gety(),g.point(i+1).gety(),v.gety());
    roots(r,g.point(i).getz(),g.postcontrol(i).getz(),
          g.precontrol(i+1).getz(),g.point(i+1).getz(),v.getz());
    
    size_t m=r.size();
    for(size_t j=0 ; j < m; ++j) {
      double t=r[j];
      if(t >= -Fuzz2 && t <= 1.0+Fuzz2) {
        double s=i+t;
        if((g.point(s)-v).abs2() <= fuzz2) {
          if(cycles && s >= n-Fuzz2) s=0;
          T.push_back(s);
        }
      }
    }
  }
}

// An optimized implementation of intersections(g,p--q);
// if there are an infinite number of intersection points, the returned list is
// only guaranteed to include the endpoint times of the intersection.
void intersections(std::vector<double>& S, std::vector<double>& T,
                   const path3& g, const triple& p, double fuzz)
{
  std::vector<double> S1;
  intersections(S1,g,p,fuzz);
  size_t n=S1.size();
  for(size_t i=0; i < n; ++i) {
    S.push_back(S1[i]);
    T.push_back(0.0);
  }
}

void add(std::vector<double>& S, std::vector<double>& T, double s, double t,
         const path3& p, const path3& q, double fuzz2)
{
  triple P=p.point(s);
  for(size_t i=0; i < S.size(); ++i)
    if((p.point(S[i])-P).abs2() <= fuzz2) return;
  S.push_back(s);
  T.push_back(t);
}
  
void add(double& s, double& t, std::vector<double>& S, std::vector<double>& T,
         std::vector<double>& S1, std::vector<double>& T1,
         double pscale, double qscale, double poffset, double qoffset,
         const path3& p, const path3& q, double fuzz2, bool single)
{
  if(single) {
    s=s*pscale+poffset;
    t=t*qscale+qoffset;
  } else {
    size_t n=S1.size();
    for(size_t i=0; i < n; ++i)
      add(S,T,pscale*S1[i]+poffset,qscale*T1[i]+qoffset,p,q,fuzz2);
  }
}

void add(double& s, double& t, std::vector<double>& S, std::vector<double>& T,
         std::vector<double>& S1, std::vector<double>& T1,
         const path3& p, const path3& q, double fuzz2, bool single)
{
  size_t n=S1.size();
  if(single) {
    if(n > 0) {
      s=S1[0];
      t=T1[0];
    }
  } else {
    for(size_t i=0; i < n; ++i)
      add(S,T,S1[i],T1[i],p,q,fuzz2);
  }
}

bool intersections(double &s, double &t, std::vector<double>& S,
                   std::vector<double>& T, path3& p, path3& q,
                   double fuzz, bool single, bool exact, unsigned depth)
{
  if(errorstream::interrupt) throw interrupted();
  
  double fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);
  
  Int lp=p.length();
  if(lp == 0 && exact) {
    std::vector<double> T1,S1;
    intersections(T1,S1,q,p.point(lp),fuzz);
    add(s,t,S,T,S1,T1,p,q,fuzz2,single);
    return S1.size() > 0;
  }
  
  Int lq=q.length();
  if(lq == 0 && exact) {
    std::vector<double> S1,T1;
    intersections(S1,T1,p,q.point(lq),fuzz);
    add(s,t,S,T,S1,T1,p,q,fuzz2,single);
    return S1.size() > 0;
  }
  
  triple maxp=p.max();
  triple minp=p.min();
  triple maxq=q.max();
  triple minq=q.min();
  
  if(maxp.getx()+fuzz >= minq.getx() &&
     maxp.gety()+fuzz >= minq.gety() && 
     maxp.getz()+fuzz >= minq.getz() && 
     maxq.getx()+fuzz >= minp.getx() &&
     maxq.gety()+fuzz >= minp.gety() &&
     maxq.getz()+fuzz >= minp.getz()) {
    // Overlapping bounding boxes

    --depth;
//    fuzz *= 2;

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
    
    path3 p1,p2;
    double pscale,poffset;
    
    std::vector<double> S1,T1;
    
//    fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);
    
    if(lp <= 1) {
      if(lp == 1) p.halve(p1,p2);
      if(lp == 0 || p1 == p || p2 == p) {
        intersections(T1,S1,q,p.point((Int) 0),fuzz);
        add(s,t,S,T,S1,T1,p,q,fuzz2,single);
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
      
    path3 q1,q2;
    double qscale,qoffset;
    
    if(lq <= 1) {
      if(lq == 1) q.halve(q1,q2);
      if(lq == 0 || q1 == q || q2 == q) {
        intersections(S1,T1,p,q.point((Int) 0),fuzz);
        add(s,t,S,T,S1,T1,p,q,fuzz2,single);
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
      add(s,t,S,T,S1,T1,pscale,qscale,0.0,0.0,p,q,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p1,q2,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,0.0,qoffset,p,q,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p2,q1,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,poffset,0.0,p,q,fuzz2,single);
      if(single || depth <= mindepth)
        return true;
      count += S1.size();
      if(Short && count > maxcount) return true;
    }
    
    S1.clear();
    T1.clear();
    if(intersections(s,t,S1,T1,p2,q2,fuzz,single,exact,depth)) {
      add(s,t,S,T,S1,T1,pscale,qscale,poffset,qoffset,p,q,fuzz2,single);
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

path3 concat(const path3& p1, const path3& p2)
{
  Int n1 = p1.length(), n2 = p2.length();

  if (n1 == -1) return p2;
  if (n2 == -1) return p1;
  triple a=p1.point(n1);
  triple b=p2.point((Int) 0);

  mem::vector<solvedKnot3> nodes(n1+n2+1);

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

  return path3(nodes, i+1);
}

path3 transformed(const array& t, const path3& p)
{
  Int n = p.size();
  mem::vector<solvedKnot3> nodes(n);

  for (Int i = 0; i < n; ++i) {
    nodes[i].pre = t * p.precontrol(i);
    nodes[i].point = t * p.point(i);
    nodes[i].post = t * p.postcontrol(i);
    nodes[i].straight = p.straight(i);
  }

  return path3(nodes, n, p.cyclic());
}

path3 transformed(const double* t, const path3& p)
{
  Int n = p.size();
  mem::vector<solvedKnot3> nodes(n);
  
  for(Int i=0; i < n; ++i) {
    nodes[i].pre=t*p.precontrol(i);
    nodes[i].point=t*p.point(i);
    nodes[i].post=t*p.postcontrol(i);
    nodes[i].straight=p.straight(i);
  }

  return path3(nodes, n, p.cyclic());
}

template<class T>
struct Split {
  T m0,m1,m2,m3,m4,m5;
  Split(T z0, T c0, T c1, T z1) {
    m0=0.5*(z0+c0);
    m1=0.5*(c0+c1);
    m2=0.5*(c1+z1);
    m3=0.5*(m0+m1);
    m4=0.5*(m1+m2);
    m5=0.5*(m3+m4);
  }
};
  
double cornerbound(double *P, double (*m)(double, double)) 
{
  double b=m(P[0],P[3]);
  b=m(b,P[12]);
  return m(b,P[15]);
}

double controlbound(double *P, double (*m)(double, double)) 
{
  double b=m(P[1],P[2]);
  b=m(b,P[4]);
  b=m(b,P[5]);
  b=m(b,P[6]);
  b=m(b,P[7]);
  b=m(b,P[8]);
  b=m(b,P[9]);
  b=m(b,P[10]);
  b=m(b,P[11]);
  b=m(b,P[13]);
  return m(b,P[14]);
}

double bound(double *P, double (*m)(double, double), double b,
             double fuzz, int depth)
{
  b=m(b,cornerbound(P,m));
  if(m(-1.0,1.0)*(b-controlbound(P,m)) >= -fuzz || depth == 0)
    return b;
  
  --depth;
  fuzz *= 2;

  Split<double> c0(P[0],P[1],P[2],P[3]);
  Split<double> c1(P[4],P[5],P[6],P[7]);
  Split<double> c2(P[8],P[9],P[10],P[11]);
  Split<double> c3(P[12],P[13],P[14],P[15]);

  Split<double> c4(P[12],P[8],P[4],P[0]);
  Split<double> c5(c3.m0,c2.m0,c1.m0,c0.m0);
  Split<double> c6(c3.m3,c2.m3,c1.m3,c0.m3);
  Split<double> c7(c3.m5,c2.m5,c1.m5,c0.m5);
  Split<double> c8(c3.m4,c2.m4,c1.m4,c0.m4);
  Split<double> c9(c3.m2,c2.m2,c1.m2,c0.m2);
  Split<double> c10(P[15],P[11],P[7],P[3]);

  // Check all 4 Bezier subpatches.
  double s0[]={c4.m5,c5.m5,c6.m5,c7.m5,c4.m3,c5.m3,c6.m3,c7.m3,
               c4.m0,c5.m0,c6.m0,c7.m0,P[12],c3.m0,c3.m3,c3.m5};
  b=bound(s0,m,b,fuzz,depth);
  double s1[]={P[0],c0.m0,c0.m3,c0.m5,c4.m2,c5.m2,c6.m2,c7.m2,
               c4.m4,c5.m4,c6.m4,c7.m4,c4.m5,c5.m5,c6.m5,c7.m5};
  b=bound(s1,m,b,fuzz,depth);
  double s2[]={c0.m5,c0.m4,c0.m2,P[3],c7.m2,c8.m2,c9.m2,c10.m2,
               c7.m4,c8.m4,c9.m4,c10.m4,c7.m5,c8.m5,c9.m5,c10.m5};
  b=bound(s2,m,b,fuzz,depth);
  double s3[]={c7.m5,c8.m5,c9.m5,c10.m5,c7.m3,c8.m3,c9.m3,c10.m3,
               c7.m0,c8.m0,c9.m0,c10.m0,c3.m5,c3.m4,c3.m2,P[15]};
  return bound(s3,m,b,fuzz,depth);
}
  
double cornerbound(triple *P, double (*m)(double, double),
                   double (*f)(const triple&)) 
{
  double b=m(f(P[0]),f(P[3]));
  b=m(b,f(P[12]));
  return m(b,f(P[15]));
}

// Return f evaluated at controlling vertex of bounding box of n control
// net points for similiar-triangle transform x'=x/z, y'=y/z, where z < 0.
double ratiobound(triple *P, double (*m)(double, double),
                  double (*f)(const triple&), int n)
{
  double MX=-P[0].getx();
  double MY=-P[0].gety();
  double Z=P[0].getz();
  double MZ=-Z;
  for(int i=1; i < n; ++i) {
    triple v=P[i];
    MX=m(MX,-v.getx());
    MY=m(MY,-v.gety());
    Z=m(Z,v.getz());
    MZ=m(MZ,-v.getz());
  }
  return m(f(triple(-MX,-MY,Z)),f(triple(-MX,-MY,-MZ)));
}

double controlbound(triple *P, double (*m)(double, double),
                    double (*f)(const triple&))
{
  double b=m(f(P[1]),f(P[2]));
  b=m(b,f(P[4]));
  b=m(b,f(P[5]));
  b=m(b,f(P[6]));
  b=m(b,f(P[7]));
  b=m(b,f(P[8]));
  b=m(b,f(P[9]));
  b=m(b,f(P[10]));
  b=m(b,f(P[11]));
  b=m(b,f(P[13]));
  return m(b,f(P[14]));
}

double bound(triple *P, double (*m)(double, double),
             double (*f)(const triple&), double b, double fuzz, int depth)
{
  b=m(b,cornerbound(P,m,f));
  if(m(-1.0,1.0)*(b-ratiobound(P,m,f,16)) >= -fuzz || depth == 0)
    return b;

  --depth;
  fuzz *= 2;
  
  Split<triple> c0(P[0],P[1],P[2],P[3]);
  Split<triple> c1(P[4],P[5],P[6],P[7]);
  Split<triple> c2(P[8],P[9],P[10],P[11]);
  Split<triple> c3(P[12],P[13],P[14],P[15]);

  Split<triple> c4(P[12],P[8],P[4],P[0]);
  Split<triple> c5(c3.m0,c2.m0,c1.m0,c0.m0);
  Split<triple> c6(c3.m3,c2.m3,c1.m3,c0.m3);
  Split<triple> c7(c3.m5,c2.m5,c1.m5,c0.m5);
  Split<triple> c8(c3.m4,c2.m4,c1.m4,c0.m4);
  Split<triple> c9(c3.m2,c2.m2,c1.m2,c0.m2);
  Split<triple> c10(P[15],P[11],P[7],P[3]);

  // Check all 4 Bezier subpatches.
  triple s0[]={c4.m5,c5.m5,c6.m5,c7.m5,c4.m3,c5.m3,c6.m3,c7.m3,
               c4.m0,c5.m0,c6.m0,c7.m0,P[12],c3.m0,c3.m3,c3.m5};
  b=bound(s0,m,f,b,fuzz,depth);
  triple s1[]={P[0],c0.m0,c0.m3,c0.m5,c4.m2,c5.m2,c6.m2,c7.m2,
               c4.m4,c5.m4,c6.m4,c7.m4,c4.m5,c5.m5,c6.m5,c7.m5};
  b=bound(s1,m,f,b,fuzz,depth);
  triple s2[]={c0.m5,c0.m4,c0.m2,P[3],c7.m2,c8.m2,c9.m2,c10.m2,
               c7.m4,c8.m4,c9.m4,c10.m4,c7.m5,c8.m5,c9.m5,c10.m5};
  b=bound(s2,m,f,b,fuzz,depth);
  triple s3[]={c7.m5,c8.m5,c9.m5,c10.m5,c7.m3,c8.m3,c9.m3,c10.m3,
               c7.m0,c8.m0,c9.m0,c10.m0,c3.m5,c3.m4,c3.m2,P[15]};
  return bound(s3,m,f,b,fuzz,depth);
}

template<class T>
struct Splittri {
  T l003,p102,p012,p201,p111,p021,r300,p210,p120,u030;
  T u021,u120;
  T p033,p231,p330;
  T p123;
  T l012,p312,r210,l102,p303,r201;
  T u012,u210,l021,p4xx,r120,px4x,pxx4,l201,r102;
  T l210,r012,l300;
  T r021,u201,r030;
  T u102,l120,l030;
  T l111,r111,u111,c111;
  
  Splittri(const T *p) {
    l003=p[0];
    p102=p[1];
    p012=p[2];
    p201=p[3];
    p111=p[4];
    p021=p[5];
    r300=p[6];
    p210=p[7];
    p120=p[8];
    u030=p[9];

    u021=0.5*(u030+p021);
    u120=0.5*(u030+p120);

    p033=0.5*(p021+p012);
    p231=0.5*(p120+p111);
    p330=0.5*(p120+p210);

    p123=0.5*(p012+p111);

    l012=0.5*(p012+l003);
    p312=0.5*(p111+p201);
    r210=0.5*(p210+r300);

    l102=0.5*(l003+p102);
    p303=0.5*(p102+p201);
    r201=0.5*(p201+r300);

    u012=0.5*(u021+p033);
    u210=0.5*(u120+p330);
    l021=0.5*(p033+l012);
    p4xx=0.5*p231+0.25*(p111+p102);
    r120=0.5*(p330+r210);
    px4x=0.5*p123+0.25*(p111+p210);
    pxx4=0.25*(p021+p111)+0.5*p312;
    l201=0.5*(l102+p303);
    r102=0.5*(p303+r201);

    l210=0.5*(px4x+l201); // = m120
    r012=0.5*(px4x+r102); // = m021
    l300=0.5*(l201+r102); // = r003 = m030

    r021=0.5*(pxx4+r120); // = m012
    u201=0.5*(u210+pxx4); // = m102
    r030=0.5*(u210+r120); // = u300 = m003

    u102=0.5*(u012+p4xx); // = m201
    l120=0.5*(l021+p4xx); // = m210
    l030=0.5*(u012+l021); // = u003 = m300

    l111=0.5*(p123+l102);
    r111=0.5*(p312+r210);
    u111=0.5*(u021+p231);
    c111=0.25*(p033+p330+p303+p111);
  }
};
  
// Return the extremum of the vertices of a Bezier triangle.
double cornerboundtri(double *P, double (*m)(double, double)) 
{
  double b=m(P[0],P[6]);
  return m(b,P[9]);
}

double cornerboundtri(triple *P, double (*m)(double, double),
                      double (*f)(const triple&)) 
{
  double b=m(f(P[0]),f(P[6]));
  return m(b,f(P[9]));
}

// Return the extremum of the non-vertex control points of a Bezier triangle.
double controlboundtri(double *P, double (*m)(double, double)) 
{
  double b=m(P[1],P[2]);
  b=m(b,P[3]);
  b=m(b,P[4]);
  b=m(b,P[5]);
  b=m(b,P[7]);
  return m(b,P[8]);
}

double controlboundtri(triple *P, double (*m)(double, double),
                       double (*f)(const triple&))
{
  double b=m(f(P[1]),f(P[2]));
  b=m(b,f(P[3]));
  b=m(b,f(P[4]));
  b=m(b,f(P[5]));
  b=m(b,f(P[7]));
  return m(b,f(P[8]));
}

// Return the global bound of a Bezier triangle.
double boundtri(double *P, double (*m)(double, double), double b,
                double fuzz, int depth)
{
  b=m(b,cornerboundtri(P,m));
  if(m(-1.0,1.0)*(b-controlboundtri(P,m)) >= -fuzz || depth == 0)
    return b;
  
  --depth;
  fuzz *= 2;

  Splittri<double> s(P);
  
  double l[]={s.l003,s.l102,s.l012,s.l201,s.l111,
              s.l021,s.l300,s.l210,s.l120,s.l030}; // left
  b=boundtri(l,m,b,fuzz,depth);
  
  double r[]={s.l300,s.r102,s.r012,s.r201,s.r111,
              s.r021,s.r300,s.r210,s.r120,s.r030}; // right
  b=boundtri(r,m,b,fuzz,depth);
  
  double u[]={s.l030,s.u102,s.u012,s.u201,s.u111,
              s.u021,s.r030,s.u210,s.u120,s.u030}; // up
  b=boundtri(u,m,b,fuzz,depth);
  
  double c[]={s.r030,s.u201,s.r021,s.u102,s.c111,
              s.r012,s.l030,s.l120,s.l210,s.l300}; // center
  return boundtri(c,m,b,fuzz,depth);
}

double boundtri(triple *P, double (*m)(double, double),
                double (*f)(const triple&), double b, double fuzz, int depth)
{
  b=m(b,cornerboundtri(P,m,f));
  if(m(-1.0,1.0)*(b-ratiobound(P,m,f,10)) >= -fuzz || depth == 0)
    return b;
  
  --depth;
  fuzz *= 2;

  Splittri<triple> s(P);
  
  triple l[]={s.l003,s.l102,s.l012,s.l201,s.l111,
              s.l021,s.l300,s.l210,s.l120,s.l030}; // left
  b=boundtri(l,m,f,b,fuzz,depth);
  
  triple r[]={s.l300,s.r102,s.r012,s.r201,s.r111,
              s.r021,s.r300,s.r210,s.r120,s.r030}; // right
  b=boundtri(r,m,f,b,fuzz,depth);
  
  triple u[]={s.l030,s.u102,s.u012,s.u201,s.u111,
              s.u021,s.r030,s.u210,s.u120,s.u030}; // up
  b=boundtri(u,m,f,b,fuzz,depth);
  
  triple c[]={s.r030,s.u201,s.r021,s.u102,s.c111,
              s.r012,s.l030,s.l120,s.l210,s.l300}; // center
  return boundtri(c,m,f,b,fuzz,depth);
}

inline void add(std::vector<double>& T, std::vector<double>& U,
                std::vector<double>& V, double t, double u, double v,
                const path3& p, double fuzz2)
{
  triple z=p.point(t);
  size_t n=T.size();
  for(size_t i=0; i < n; ++i)
    if((p.point(T[i])-z).abs2() <= fuzz2) return;
  T.push_back(t);
  U.push_back(u);
  V.push_back(v);
}
    
void add(std::vector<double>& T, std::vector<double>& U,
         std::vector<double>& V, std::vector<double>& T1,
         std::vector<double>& U1, std::vector<double>& V1,
         const path3& p, double tscale, double toffset,
         double uoffset, double voffset, double fuzz2)
{
  size_t n=T1.size();
  for(size_t i=0; i < n; ++i)
    add(T,U,V,tscale*T1[i]+toffset,0.5*U1[i]+uoffset,0.5*V1[i]+voffset,p,
        fuzz2);
}
    
void bounds(triple& Pmin, triple& Pmax, triple *P, double fuzz) 
{
  double Px[]={P[0].getx(),P[1].getx(),P[2].getx(),P[3].getx(),
               P[4].getx(),P[5].getx(),P[6].getx(),P[7].getx(),
               P[8].getx(),P[9].getx(),P[10].getx(),P[11].getx(),
               P[12].getx(),P[13].getx(),P[14].getx(),P[15].getx()};
  double bx=Px[0];
  double xmin=bound(Px,min,bx,fuzz,maxdepth);
  double xmax=bound(Px,max,bx,fuzz,maxdepth);
  
  double Py[]={P[0].gety(),P[1].gety(),P[2].gety(),P[3].gety(),
               P[4].gety(),P[5].gety(),P[6].gety(),P[7].gety(),
               P[8].gety(),P[9].gety(),P[10].gety(),P[11].gety(),
               P[12].gety(),P[13].gety(),P[14].gety(),P[15].gety()};
  double by=Py[0];
  double ymin=bound(Py,min,by,fuzz,maxdepth);
  double ymax=bound(Py,max,by,fuzz,maxdepth);
  
  double Pz[]={P[0].getz(),P[1].getz(),P[2].getz(),P[3].getz(),
               P[4].getz(),P[5].getz(),P[6].getz(),P[7].getz(),
               P[8].getz(),P[9].getz(),P[10].getz(),P[11].getz(),
               P[12].getz(),P[13].getz(),P[14].getz(),P[15].getz()};
  double bz=Pz[0];
  double zmin=bound(Pz,min,bz,fuzz,maxdepth);
  double zmax=bound(Pz,max,bz,fuzz,maxdepth);
  Pmin=triple(xmin,ymin,zmin);
  Pmax=triple(xmax,ymax,zmax);
}

inline double abs2(double x, double y, double z) 
{
  return x*x+y*y+z*z;
}

bool intersections(double& U, double& V, const triple& v, triple *P,
                   double fuzz, unsigned depth)
{
  if(errorstream::interrupt) throw interrupted();
  
  triple Pmin,Pmax;
  bounds(Pmin,Pmax,P,fuzz);
  
  double x=P[0].getx();
  double y=P[0].gety();
  double z=P[0].getz();
  double X=x, Y=y, Z=z;
  for(int i=1; i < 16; ++i) {
    triple v=P[i];
    double vx=v.getx();
    x=min(x,vx);
    X=max(X,vx);
    double vy=v.gety();
    y=min(y,vy);
    Y=max(Y,vy);
    double vz=v.getz();
    z=min(z,vz);
    Z=max(Z,vz);
  }
  
  if(X+fuzz >= v.getx() &&
     Y+fuzz >= v.gety() && 
     Z+fuzz >= v.getz() && 
     v.getx()+fuzz >= x &&
     v.gety()+fuzz >= y &&
     v.getz()+fuzz >= z) { // Overlapping bounding boxes
    
    --depth;
//    fuzz *= 2;

    if(abs2(X-x,Y-y,Z-z) <= fuzz*fuzz || depth == 0) {
      U=0.5;
      V=0.5;
      return true;
    }
    
// Compute the control points of the four subpatches obtained by splitting 
// the patch with control points P at u=v=1/2.
    Split<triple> c0(P[0],P[1],P[2],P[3]);
    Split<triple> c1(P[4],P[5],P[6],P[7]);
    Split<triple> c2(P[8],P[9],P[10],P[11]);
    Split<triple> c3(P[12],P[13],P[14],P[15]);

    Split<triple> c4(P[12],P[8],P[4],P[0]);
    Split<triple> c5(c3.m0,c2.m0,c1.m0,c0.m0);
    Split<triple> c6(c3.m3,c2.m3,c1.m3,c0.m3);
    Split<triple> c7(c3.m5,c2.m5,c1.m5,c0.m5);
    Split<triple> c8(c3.m4,c2.m4,c1.m4,c0.m4);
    Split<triple> c9(c3.m2,c2.m2,c1.m2,c0.m2);
    Split<triple> c10(P[15],P[11],P[7],P[3]);

    // Check all 4 Bezier subpatches.
  
    double U1,V1;
    triple Q0[]={P[0],c0.m0,c0.m3,c0.m5,c4.m2,c5.m2,c6.m2,c7.m2,
                 c4.m4,c5.m4,c6.m4,c7.m4,c4.m5,c5.m5,c6.m5,c7.m5};
    if(intersections(U1,V1,v,Q0,fuzz,depth)) {
      U=0.5*U1;
      V=0.5*V1;
      return true;
    }
  
    triple Q1[]={c0.m5,c0.m4,c0.m2,P[3],c7.m2,c8.m2,c9.m2,c10.m2,
                 c7.m4,c8.m4,c9.m4,c10.m4,c7.m5,c8.m5,c9.m5,c10.m5};
    if(intersections(U1,V1,v,Q1,fuzz,depth)) {
      U=0.5*U1;
      V=0.5*V1+0.5;
      return true;
    }
  
    triple Q2[]={c7.m5,c8.m5,c9.m5,c10.m5,c7.m3,c8.m3,c9.m3,c10.m3,
                 c7.m0,c8.m0,c9.m0,c10.m0,c3.m5,c3.m4,c3.m2,P[15]};
    if(intersections(U1,V1,v,Q2,fuzz,depth)) {
      U=0.5*U1+0.5;
      V=0.5*V1+0.5;
      return true;
    }
  
    triple Q3[]={c4.m5,c5.m5,c6.m5,c7.m5,c4.m3,c5.m3,c6.m3,c7.m3,
                 c4.m0,c5.m0,c6.m0,c7.m0,P[12],c3.m0,c3.m3,c3.m5};
    if(intersections(U1,V1,v,Q3,fuzz,depth)) {
      U=0.5*U1+0.5;
      V=0.5*V1;
      return true;
    }
  }
  return false;
}

bool intersections(std::vector<double>& T, std::vector<double>& U,
                   std::vector<double>& V, path3& p, triple *P,
                   double fuzz, bool single, unsigned depth)
{
  if(errorstream::interrupt) throw interrupted();
  
  double fuzz2=max(fuzzFactor*fuzz*fuzz,Fuzz2);
  
  triple pmin=p.min();
  triple pmax=p.max();
  
  double x=P[0].getx();
  double y=P[0].gety();
  double z=P[0].getz();
  double X=x, Y=y, Z=z;
  for(int i=1; i < 16; ++i) {
    triple v=P[i];
    double vx=v.getx();
    x=min(x,vx);
    X=max(X,vx);
    double vy=v.gety();
    y=min(y,vy);
    Y=max(Y,vy);
    double vz=v.getz();
    z=min(z,vz);
    Z=max(Z,vz);
  }
  
  if(X+fuzz >= pmin.getx() &&
     Y+fuzz >= pmin.gety() && 
     Z+fuzz >= pmin.getz() && 
     pmax.getx()+fuzz >= x &&
     pmax.gety()+fuzz >= y &&
     pmax.getz()+fuzz >= z) { // Overlapping bounding boxes
    
    --depth;
//    fuzz *= 2;

    if(((pmax-pmin).length()+sqrt(abs2(X-x,Y-y,Z-z)) <= fuzz) || depth == 0) {
      T.push_back(0.5);
      U.push_back(0.5);
      V.push_back(0.5);
      return true;
    }
    
    Int lp=p.length();

    path3 p0,p1;
    p.halve(p0,p1);
    
    std::vector<double> T1,U1,V1;
    double tscale,toffset;

    if(lp <= 1) {
      if(lp == 1) p.halve(p0,p1);
      if(lp == 0 || p0 == p || p1 == p) {
        double u,v;
        if(intersections(u,v,p.point((Int) 0),P,fuzz,depth)) {
          T1.push_back(0.0);
          U1.push_back(u);
          V1.push_back(v);
          add(T,U,V,T1,U1,V1,p,1.0,0.0,0.0,0.0,fuzz2);
        }
        return T1.size() > 0;
      }
      tscale=toffset=0.5;
    } else {
      Int tp=lp/2;
      p0=p.subpath(0,tp);
      p1=p.subpath(tp,lp);
      toffset=tp;
      tscale=1.0;
    }
      
    Split<triple> c0(P[0],P[1],P[2],P[3]);
    Split<triple> c1(P[4],P[5],P[6],P[7]);
    Split<triple> c2(P[8],P[9],P[10],P[11]);
    Split<triple> c3(P[12],P[13],P[14],P[15]);

    Split<triple> c4(P[12],P[8],P[4],P[0]);
    Split<triple> c5(c3.m0,c2.m0,c1.m0,c0.m0);
    Split<triple> c6(c3.m3,c2.m3,c1.m3,c0.m3);
    Split<triple> c7(c3.m5,c2.m5,c1.m5,c0.m5);
    Split<triple> c8(c3.m4,c2.m4,c1.m4,c0.m4);
    Split<triple> c9(c3.m2,c2.m2,c1.m2,c0.m2);
    Split<triple> c10(P[15],P[11],P[7],P[3]);

    static size_t maxcount=9;
    size_t count=0;
    
    bool Short=lp == 1;
    
    // Check all 4 Bezier subpatches against p0.
    triple Q0[]={P[0],c0.m0,c0.m3,c0.m5,c4.m2,c5.m2,c6.m2,c7.m2,
                 c4.m4,c5.m4,c6.m4,c7.m4,c4.m5,c5.m5,c6.m5,c7.m5};
    if(intersections(T1,U1,V1,p0,Q0,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,0.0,0.0,0.0,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    triple Q1[]={c0.m5,c0.m4,c0.m2,P[3],c7.m2,c8.m2,c9.m2,c10.m2,
                 c7.m4,c8.m4,c9.m4,c10.m4,c7.m5,c8.m5,c9.m5,c10.m5};
    if(intersections(T1,U1,V1,p0,Q1,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,0.0,0.0,0.5,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    triple Q2[]={c7.m5,c8.m5,c9.m5,c10.m5,c7.m3,c8.m3,c9.m3,c10.m3,
                 c7.m0,c8.m0,c9.m0,c10.m0,c3.m5,c3.m4,c3.m2,P[15]};
    if(intersections(T1,U1,V1,p0,Q2,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,0.0,0.5,0.5,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    triple Q3[]={c4.m5,c5.m5,c6.m5,c7.m5,c4.m3,c5.m3,c6.m3,c7.m3,
                 c4.m0,c5.m0,c6.m0,c7.m0,P[12],c3.m0,c3.m3,c3.m5};
    if(intersections(T1,U1,V1,p0,Q3,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,0.0,0.5,0.0,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    // Check all 4 Bezier subpatches against p1.
    T1.clear();
    U1.clear();
    V1.clear();
    if(intersections(T1,U1,V1,p1,Q0,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,toffset,0.0,0.0,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    if(intersections(T1,U1,V1,p1,Q1,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,toffset,0.0,0.5,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    if(intersections(T1,U1,V1,p1,Q2,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,toffset,0.5,0.5,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
  
    T1.clear();
    U1.clear();
    V1.clear();
    if(intersections(T1,U1,V1,p1,Q3,fuzz,single,depth)) {
      add(T,U,V,T1,U1,V1,p,tscale,toffset,0.5,0.0,fuzz2);
      if(single || depth <= mindepth)
        return true;
      count += T1.size();
      if(Short && count > maxcount) return true;
    }
    
    return T.size() > 0;
  }
  return false;
}

} //namespace camp
