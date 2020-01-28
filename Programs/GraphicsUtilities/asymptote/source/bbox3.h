/*****
 * bbox3.h
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a rectangle that encloses a drawing object.
 *****/

#ifndef BBOX3_H
#define BBOX3_H

#include "triple.h"

// For CYGWIN
#undef near
#undef far

namespace camp {

// The box that encloses a path
struct bbox3 {
  bool empty;
  double left;
  double bottom;
  double near;
  double right;
  double top;
  double far;
  
  // Start bbox3 about the origin
  bbox3()
    : empty(true), left(0.0), bottom(0.0), near(0.0),
      right(0.0), top(0.0), far(0.0)
  {
  }

  bbox3(double left, double bottom, double near,
        double right, double top, double far)
    : empty(false), left(left), bottom(bottom), near(near),
      right(right), top(top), far(far)
  {
  }

  // Start a bbox3 with a point
  bbox3(double x, double y, double z)
    : empty(false), left(x), bottom(y), near(z), right(x), top(y), far(z)
  {
  }

  // Start a bbox3 with a point
  bbox3(const triple& v)
    : empty(false), left(v.getx()), bottom(v.gety()), near(v.getz()),
      right(v.getx()), top(v.gety()), far(v.getz())
  {
  }

  // Start a bbox3 with 2 points
  bbox3(const triple& m, const triple& M)
    : empty(false),
      left(m.getx()), bottom(m.gety()), near(m.getz()),
      right(M.getx()),    top(M.gety()), far(M.getz())
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
      near = far = z;
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
      if(z < near)
        near = z;
      else if(z > far)
        far = z;
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
    if(z < near)
      near = z;
    else if(z > far)
      far = z;
  }

  // Add (x,y) pair to a nonempty bbox3
  void addnonempty(pair v)
  {
    double x=v.getx();
    if(x < left)
      left = x;  
    else if(x > right)
      right = x;  
    double y=v.gety();
    if(y < bottom)
      bottom = y;
    else if(y > top)
      top = y;
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
    if(z < near) {
      near = z;
      times.near=t;
    }
    else if(z > far) {
      far = z;
      times.far=t;
    }
  }

  bbox3 operator+= (const triple& v)
  {
    add(v);
    return *this;
  }

  triple Min() const {
    return triple(left,bottom,near);
  }
  
  triple Max() const {
    return triple(right,top,far);
  }
  
  pair Min2() const {
    return pair(left,bottom);
  }
  
  pair Max2() const {
    return pair(right,top);
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
