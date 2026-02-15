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
  double leftBound;
  double bottomBound;
  double nearBound;
  double rightBound;
  double topBound;
  double farBound;

  // Start bbox3 about the origin
  bbox3()
    : empty(true), leftBound(0.0), bottomBound(0.0), nearBound(0.0),
        rightBound(0.0), topBound(0.0), farBound(0.0)
  {
  }

  bbox3(double left, double bottom, double near,
        double right, double top, double far)
    : empty(false), leftBound(left), bottomBound(bottom), nearBound(near),
        rightBound(right), topBound(top), farBound(far)
  {
  }

  // Start a bbox3 with a point
  bbox3(double x, double y, double z)
    : empty(false), leftBound(x), bottomBound(y), nearBound(z), rightBound(x), topBound(y), farBound(z)
  {
  }

  // Start a bbox3 with a point
  bbox3(const triple& v)
    : empty(false), leftBound(v.getx()), bottomBound(v.gety()), nearBound(v.getz()),
        rightBound(v.getx()), topBound(v.gety()), farBound(v.getz())
  {
  }

  // Start a bbox3 with 2 points
  bbox3(const triple& m, const triple& M)
    : empty(false),
        leftBound(m.getx()), bottomBound(m.gety()), nearBound(m.getz()),
        rightBound(M.getx()), topBound(M.gety()), farBound(M.getz())
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
      leftBound= rightBound= x;
      topBound= bottomBound= y;
      nearBound= farBound= z;
      empty = false;
    }
    else {
      if(x < leftBound)
        leftBound= x;
      else if(x > rightBound)
        rightBound= x;
      if(y < bottomBound)
        bottomBound= y;
      else if(y > topBound)
        topBound= y;
      if(z < nearBound)
        nearBound= z;
      else if(z > farBound)
        farBound= z;
    }
  }

  // Add a point to a nonempty bbox3
  void addnonempty(double x, double y, double z)
  {
    if(x < leftBound)
      leftBound= x;
    else if(x > rightBound)
      rightBound= x;
    if(y < bottomBound)
      bottomBound= y;
    else if(y > topBound)
      topBound= y;
    if(z < nearBound)
      nearBound= z;
    else if(z > farBound)
      farBound= z;
  }

  // Add (x,y) pair to a nonempty bbox3
  void addnonempty(pair v)
  {
    double x=v.getx();
    if(x < leftBound)
      leftBound= x;
    else if(x > rightBound)
      rightBound= x;
    double y=v.gety();
    if(y < bottomBound)
      bottomBound= y;
    else if(y > topBound)
      topBound= y;
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

    if(x < leftBound) {
      leftBound= x;
      times.leftBound= t;
    }
    else if(x > rightBound) {
      rightBound= x;
      times.rightBound= t;
    }
    if(y < bottomBound) {
      bottomBound= y;
      times.bottomBound= t;
    }
    else if(y > topBound) {
      topBound= y;
      times.topBound= t;
    }
    if(z < nearBound) {
      nearBound= z;
      times.nearBound=t;
    }
    else if(z > farBound) {
      farBound= z;
      times.farBound=t;
    }
  }

  bbox3 operator+= (const triple& v)
  {
    add(v);
    return *this;
  }

  triple Min() const {
    return triple(leftBound, bottomBound, nearBound);
  }

  triple Max() const {
    return triple(rightBound, topBound, farBound);
  }

  pair Min2() const {
    return pair(leftBound, bottomBound);
  }

  pair Max2() const {
    return pair(rightBound, topBound);
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
