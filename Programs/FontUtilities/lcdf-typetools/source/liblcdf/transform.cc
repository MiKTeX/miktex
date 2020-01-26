// -*- related-file-name: "../include/lcdf/transform.hh" -*-

/* transform.{cc,hh} -- planar affine transformations
 *
 * Copyright (c) 2000-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/transform.hh>
#include <lcdf/straccum.hh>
#include <math.h>

Transform::Transform()
{
    _m[0] = _m[3] = 1;
    _m[1] = _m[2] = _m[4] = _m[5] = 0;
    _null = true;
}

Transform::Transform(const double m[6])
{
    _m[0] = m[0];
    _m[1] = m[1];
    _m[2] = m[2];
    _m[3] = m[3];
    _m[4] = m[4];
    _m[5] = m[5];
    check_null(0);
}

Transform::Transform(double m0, double m1, double m2,
		     double m3, double m4, double m5)
{
    _m[0] = m0;
    _m[1] = m1;
    _m[2] = m2;
    _m[3] = m3;
    _m[4] = m4;
    _m[5] = m5;
    check_null(0);
}

void
Transform::check_null(double tolerance)
{
    _null = (fabs(_m[0] - 1) < tolerance && fabs(_m[1]) < tolerance
	     && fabs(_m[2]) < tolerance && fabs(_m[3] - 1) < tolerance
	     && fabs(_m[4]) < tolerance && fabs(_m[5]) < tolerance);
}


void
Transform::scale(double x, double y)
{
    _m[0] *= x;
    _m[1] *= x;
    _m[2] *= y;
    _m[3] *= y;

    if (x != 1 || y != 1)
	_null = false;
}

void
Transform::rotate(double r)
{
    double c = cos(r);
    double s = sin(r);

    double a = _m[0], b = _m[2];
    _m[0] = a*c + b*s;
    _m[2] = b*c - a*s;

    a = _m[1], b = _m[3];
    _m[1] = a*c + b*s;
    _m[3] = b*c - a*s;

    if (r != 0)
	_null = false;
}

void
Transform::translate(double x, double y)
{
    _m[4] += _m[0]*x + _m[2]*y;
    _m[5] += _m[1]*x + _m[3]*y;

    if (x != 0 || y != 0)
	_null = false;
}

void Transform::raw_translate(double x, double y) {
    _m[4] += x;
    _m[5] += y;

    if (x != 0 || y != 0)
        _null = false;
}

void
Transform::shear(double s)
{
    *this *= Transform(1, 0, s, 1, 0, 0);
}

Transform& Transform::operator*=(const Transform& x) {
    if (x.null())
        /* do nothing */;
    else if (null())
        memcpy(_m, x._m, sizeof(_m));
    else {
        double m[6];
        m[0] = _m[0] * x._m[0] + _m[2] * x._m[1];
        m[1] = _m[1] * x._m[0] + _m[3] * x._m[1];
        m[2] = _m[0] * x._m[2] + _m[2] * x._m[3];
        m[3] = _m[1] * x._m[2] + _m[3] * x._m[3];
        m[4] = _m[0] * x._m[4] + _m[2] * x._m[5] + _m[4];
        m[5] = _m[1] * x._m[4] + _m[3] * x._m[5] + _m[5];
        memcpy(_m, m, sizeof(_m));
    }
    return *this;
}


void
Transform::real_apply_to(Point &p) const
{
    double x = p.x;
    p.x = x*_m[0] + p.y*_m[2] + _m[4];
    p.y = x*_m[1] + p.y*_m[3] + _m[5];
}

Point
Transform::real_apply(const Point &p) const
{
    return Point(p.x*_m[0] + p.y*_m[2] + _m[4],
		 p.x*_m[1] + p.y*_m[3] + _m[5]);
}

Bezier &
operator*=(Bezier &b, const Transform &t)
{
    if (!t.null()) {
	b.mpoint(0) *= t;
	b.mpoint(1) *= t;
	b.mpoint(2) *= t;
	b.mpoint(3) *= t;
    }
    return b;
}

Bezier
operator*(const Bezier &b, const Transform &t)
{
    return (t.null()
	    ? b
	    : Bezier(b.point(0) * t, b.point(1) * t, b.point(2) * t, b.point(3) * t));
}

String
Transform::unparse() const
{
    StringAccum sa;
    sa << '[';
    for (int i = 0; i < 6; i++) {
	if (i)
	    sa << ',' << ' ';
	sa << _m[i];
    }
    sa << ']';
    return sa.take_string();
}
