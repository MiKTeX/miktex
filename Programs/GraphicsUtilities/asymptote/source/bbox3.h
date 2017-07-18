/*****
 * bbox3.h
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a rectangle that encloses a drawing object.
 *****/

#ifndef BBOX3_H
#define BBOX3_H

#include "triple.h"

namespace camp {

// The box that encloses a path
struct bbox3 {
  bool empty;
  double left;
  double bottom;
  double lower;
  double right;
  double top;
  double upper;
  
  // Start bbox3 about the origin
  bbox3()
    : empty(true), left(0.0), bottom(0.0), lower(0.0),
      right(0.0), top(0.0), upper(0.0)
  {
  }

  bbox3(double left, double bottom, double lower,
        double right, double top, double upper)
    : empty(false), left(left), bottom(bottom), lower(lower),
      right(right), top(top), upper(upper)
  {
  }

  // Start a bbox3 with a point
  bbox3(double x, double y, double z)
    : empty(false), left(x), bottom(y), lower(z), right(x), top(y), upper(z)
  {
  }

  // Start a bbox3 with a point
  bbox3(const triple& v)
    : empty(false), left(v.getx()), bottom(v.gety()), lower(v.getz()),
      right(v.getx()), top(v.gety()), upper(v.getz())
  {
  }

  // Start a bbox3 with 2 points
  bbox3(const triple& m, const triple& M)
    : empty(false),
      left(m.getx()), bottom(m.gety()), lower(m.getz()),
      right(M.getx()),    top(M.gety()), upper(M.getz())
  {
  }
  
  // Add a point to a bbox3
  void add(const triple& v)
  {
    const double x = v.getx(), y = v.gety(), z = v.getz();
    add(x,y,z);
  }
  void add(double x, double y, double z)
  {
    if (empty) {
      left = right = x;
      top = bottom = y;
      lower = upper = z;
      empty = false;
    }
    else {
      if(x < left)
        left = x;  
      else if(x > right)
        right = x;  
      if(y < bottom)
        bottom = y;
      else if(y > top)
        top = y;
      if(z < lower)
        lower = z;
      else if(z > upper)
        upper = z;
    }
  }

  // Add a point to a nonempty bbox3
  void addnonempty(double x, double y, double z)
  {
    if(x < left)
      left = x;  
    else if(x > right)
      right = x;  
    if(y < bottom)
      bottom = y;
    else if(y > top)
      top = y;
    if(z < lower)
      lower = z;
    else if(z > upper)
      upper = z;
  }

  // Add a point to a nonempty bbox3
  void addnonempty(const triple& v)
  {
    addnonempty(v.getx(),v.gety(),v.getz());
  }

  // Add a point to a nonempty bbox, updating bounding times
  void addnonempty(const triple& v, bbox3& times, double t)
  {
    double x = v.getx(), y = v.gety(), z = v.getz();

    if(x < left) {
      left = x;  
      times.left = t;
    }
    else if(x > right) {
      right = x;  
      times.right = t;
    }
    if(y < bottom) {
      bottom = y;
      times.bottom = t;
    }
    else if(y > top) {
      top = y;
      times.top = t;
    }
    if(z < lower) {
      lower = z;
      times.lower=t;
    }
    else if(z > upper) {
      upper = z;
      times.upper=t;
    }
  }

  bbox3 operator+= (const triple& v)
  {
    add(v);
    return *this;
  }

  triple Min() const {
    return triple(left,bottom,lower);
  }
  
  triple Max() const {
    return triple(right,top,upper);
  }
  
  // transform bbox3 by 4x4 matrix
  void transform(const double* m)
  {
    const double xmin = left;
    const double ymin = bottom;
    const double zmin = lower;
    const double xmax = right;
    const double ymax = top;
    const double zmax = upper;
    
    empty = true;
    add(m*triple(xmin,ymin,zmin));
    addnonempty(m*triple(xmin,ymin,zmax));
    addnonempty(m*triple(xmin,ymax,zmin));
    addnonempty(m*triple(xmin,ymax,zmax));
    addnonempty(m*triple(xmax,ymin,zmin));
    addnonempty(m*triple(xmax,ymin,zmax));
    addnonempty(m*triple(xmax,ymax,zmin));
    addnonempty(m*triple(xmax,ymax,zmax));
  }
  
  friend ostream& operator << (ostream& out, const bbox3& b)
  {
    out << "Min " << b.Min() << " Max " << b.Max();
    return out;
  }
  
};

} // namespace camp

GC_DECLARE_PTRFREE(camp::bbox3);

#endif
