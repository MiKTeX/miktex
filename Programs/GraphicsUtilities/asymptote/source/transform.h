/*****
 * transform.h
 * Andy Hammerlindl 2002/05/22
 *
 * The transform datatype stores an affine transformation on the plane
 * The datamembers are x, y, xx, xy, yx, and yy.  A pair (x,y) is
 * transformed as
 *   x' = t.x + t.xx * x + t.xy * y
 *   y' = t.y + t.yx * x + t.yy * y
 *****/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <iostream>

#include "pair.h"

namespace camp {

class transform : public gc {
  double x;
  double y;
  double xx;
  double xy;
  double yx;
  double yy;

public:
  transform()
    : x(0.0), y(0.0), xx(1.0), xy(0.0), yx(0.0), yy(1.0) {}

  virtual ~transform() {}

  transform(double x, double y,
            double xx, double xy,
            double yx, double yy)
    : x(x), y(y), xx(xx), xy(xy), yx(yx), yy(yy) {}

  double getx() const { return x; }
  double gety() const { return y; }
  double getxx() const { return xx; }
  double getxy() const { return xy; }
  double getyx() const { return yx; }
  double getyy() const { return yy; }

  friend transform operator+ (const transform& t, const transform& s)
  {
    return transform(t.x + s.x, t.y + s.y,
                     t.xx + s.xx, t.xy + s.xy,
                     t.yx + s.yx, t.yy + s.yy);
  }

  friend transform operator- (const transform& t, const transform& s)
  {
    return transform(t.x - s.x, t.y - s.y,
                     t.xx - s.xx, t.xy - s.xy,
                     t.yx - s.yx, t.yy - s.yy);
  }

  friend transform operator- (const transform& t)
  {
    return transform(-t.x, -t.y,
                     -t.xx, -t.xy,
                     -t.yx, -t.yy);
  }

  friend pair operator* (const transform& t, const pair& z)
  {
    double x = z.getx(), y = z.gety();
    return pair(t.x + t.xx * x + t.xy * y, t.y + t.yx * x + t.yy * y);
  }

  // Calculates the composition of t and s, so for a pair, z,
  //   t * (s * z) == (t * s) * z
  // Can be thought of as matrix multiplication.
  friend transform operator* (const transform& t, const transform& s)
  {
    return transform(t.x + t.xx * s.x  + t.xy * s.y,
                     t.y + t.yx * s.x  + t.yy * s.y,
                     t.xx * s.xx + t.xy * s.yx,
                     t.xx * s.xy + t.xy * s.yy,
                     t.yx * s.xx + t.yy * s.yx,
                     t.yx * s.xy + t.yy * s.yy);
  }

  friend bool operator== (const transform& t1, const transform& t2)
  {
    return t1.x  == t2.x  && t1.y  == t2.y  &&
      t1.xx == t2.xx && t1.xy == t2.xy &&
      t1.yx == t2.yx && t1.yy == t2.yy;
  }

  friend bool operator!= (const transform& t1, const transform& t2)
  {
    return !(t1 == t2);
  }

  bool isIdentity() const
  {
    return x == 0.0 && y == 0.0 &&
      xx == 1.0 && xy == 0.0 && yx == 0.0 && yy == 1.0;
  }

  bool isNull() const
  {
    return x == 0.0 && y == 0.0 &&
      xx == 0.0 && xy == 0.0 && yx == 0.0 && yy == 0.0;
  }

  // Calculates the determinant, as if it were a matrix.
  friend double det(const transform& t)
  {
    return t.xx * t.yy - t.xy * t.yx;
  }

  // Tells if the transformation is invertible (bijective).
  bool invertible() const
  {
    return det(*this) != 0.0;
  }

  friend transform inverse(const transform& t)
  {
    double d = det(t);
    if (d == 0.0)
      reportError("inverting singular transform");

    d=1.0/d;
    return transform((t.xy * t.y - t.yy * t.x)*d,
                     (t.yx * t.x - t.xx * t.y)*d,
                     t.yy*d, -t.xy*d, -t.yx*d, t.xx*d);
  }

  friend ostream& operator<< (ostream& out, const transform& t)
  {
    return out << "(" << t.x  << ","
               << t.y  << ","
               << t.xx << ","
               << t.xy << ","
               << t.yx << ","
               << t.yy << ")";
  }
};

// The common transforms
static const transform identity;

inline transform shift(pair z)
{
  return transform (z.getx(), z.gety(), 1.0, 0.0, 0.0, 1.0);
}

inline transform xscale(double s)
{
  return transform (0.0, 0.0, s, 0.0, 0.0, 1.0);
}

inline transform yscale(double s)
{
  return transform (0.0, 0.0, 1.0, 0.0, 0.0, s);
}

inline transform scale(double s)
{
  return transform (0.0, 0.0, s, 0.0, 0.0, s);
}

inline transform scale(double x, double y)
{
  return transform (0.0, 0.0, x, 0.0, 0.0, y);
}

inline transform scale(pair z)
{
  // Equivalent to multiplication by z.
  double x = z.getx(), y = z.gety();
  return transform (0.0, 0.0, x, -y, y, x);
}

inline transform slant(double s)
{
  return transform (0.0, 0.0, 1.0, s, 0.0, 1.0);
}

inline transform rotate(double theta)
{
  double s = sin(theta), c = cos(theta);
  return transform (0.0, 0.0, c, -s, s, c);
}

// return rotate(angle(v)) if z != (0,0); otherwise return identity.
inline transform rotate(pair z)
{
  double d=z.length();
  if(d == 0.0) return identity;
  d=1.0/d;
  return transform (0.0, 0.0, d*z.getx(), -d*z.gety(), d*z.gety(), d*z.getx());
}

inline transform rotatearound(pair z, double theta)
{
  // Notice the operators are applied from right to left.
  // Could be optimized.
  return shift(z) * rotate(theta) * shift(-z);
}

inline transform reflectabout(pair z, pair w)
{
  if (z == w)
    reportError("points determining line to reflect about must be distinct");

  // Also could be optimized.
  transform basis = shift(z) * scale(w-z);
  transform flip = yscale(-1.0);

  return basis * flip * inverse(basis);
}

// Return the rotational part of t.
inline transform rotation(transform t)
{
  pair z(2.0*t.getxx()*t.getyy(),t.getyx()*t.getyy()-t.getxx()*t.getxy());
  if(t.getxx() < 0) z=-z;
  return rotate(atan2(z.gety(),z.getx()));
}

// Remove the x and y components, so that the new transform maps zero to zero.
inline transform shiftless(transform t)
{
  return transform(0, 0, t.getxx(), t.getxy(), t.getyx(), t.getyy());
}

// Return the translational component of t.
inline transform shift(transform t)
{
  return transform(t.getx(), t.gety(), 1.0, 0, 0, 1.0);
}

// Return the translational pair of t.
inline pair shiftpair(transform t)
{
  return pair(t.getx(), t.gety());
}

inline transform matrix(pair lb, pair rt)
{
  pair size=rt-lb;
  return transform(lb.getx(),lb.gety(),size.getx(),0,0,size.gety());
}

} //namespace camp

GC_DECLARE_PTRFREE(camp::transform);

#endif
