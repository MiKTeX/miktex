// -*- related-file-name: "../include/lcdf/bezier.hh" -*-

/* bezier.{cc,hh} -- cubic Bezier curves
 *
 * Copyright (c) 1998-2019 Eddie Kohler
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
#include <lcdf/bezier.hh>

//
// bounding box
//

void
Bezier::make_bb() const noexcept
{
    _bb = 0;
    for (int i = 1; i < 4; i++) {
	if (_p[i].x > bb_right_x())
	    _bb = (_bb & ~0x03) | (i << 0);
	else if (_p[i].x < bb_left_x())
	    _bb = (_bb & ~0x0C) | (i << 2);
	if (_p[i].y > bb_top_x())
	    _bb = (_bb & ~0x30) | (i << 4);
	else if (_p[i].y < bb_bottom_x())
	    _bb = (_bb & ~0xC0) | (i << 6);
    }
}


//
// is_flat, eval
//

bool
Bezier::is_flat(double t) const noexcept
{
    return (_p[2].on_segment(_p[0], _p[3], t)
	    && _p[1].on_segment(_p[0], _p[3], t));
}

static Point
eval_bezier(Point *b_in, int degree, double u)
{
    assert(degree < 4);
    Point b[4];
    for (int i = 0; i <= degree; i++)
	b[i] = b_in[i];

    double m = 1.0 - u;
    for (int i = 1; i <= degree; i++)
	for (int j = 0; j <= degree - i; j++)
	    b[j] = b[j]*m + b[j+1]*u;
    return b[0];
}

Point
Bezier::eval(double u) const noexcept
{
    Bezier b = *this;
    double m = 1.0 - u;
    for (int i = 1; i < 4; i++)
	for (int j = 0; j < 4 - i; j++)
	    b._p[j] = m * b._p[j] + u * b._p[j+1];
    return b._p[0];
}


//
// halve
//

void
Bezier::halve(Bezier &l, Bezier &r) const noexcept
{
    Point half = Point::midpoint(_p[1], _p[2]);
    l._p[0] = _p[0];
    l._p[1] = Point::midpoint(_p[0], _p[1]);
    l._p[2] = Point::midpoint(l._p[1], half);
    r._p[3] = _p[3];
    r._p[2] = Point::midpoint(_p[2], _p[3]);
    r._p[1] = Point::midpoint(r._p[2], half);
    r._p[0] = l._p[3] = Point::midpoint(l._p[2], r._p[1]);
}


//
// hit testing
//

bool
Bezier::in_bb(const Point &p, double tolerance) const noexcept
{
    ensure_bb();
    if (bb_right() + tolerance < p.x
	|| bb_left() - tolerance > p.x
	|| bb_top() + tolerance < p.y
	|| bb_bottom() - tolerance > p.y)
	return false;
    else
	return true;
}

double
Bezier::hit_recurse(const Point &p, double tolerance, double leftd,
		    double rightd, double leftt, double rightt) const noexcept
{
    Bezier left, right;
    double middled, resultt;

    if (is_flat(tolerance)) {
	if (p.on_segment(_p[0], _p[3], tolerance))
	    return (leftt + rightt) / 2;
	else
	    return -1;
    }

    if (leftd < tolerance * tolerance)
	return leftt;
    if (rightd < tolerance * tolerance)
	return rightt;

    if (!in_bb(p, tolerance))
	return -1;

    halve(left, right);
    middled = (right._p[0] - p).squared_length();
    resultt = left.hit_recurse
	(p, tolerance, leftd, middled, leftt, (leftt + rightt) / 2);
    if (resultt >= 0)
	return resultt;

    return right.hit_recurse
	(p, tolerance, middled, rightd, (leftt + rightt) / 2, rightt);
}

bool
Bezier::hit(const Point &p, double tolerance) const noexcept
{
    double leftd = (_p[0] - p).squared_length();
    double rightd = (_p[3] - p).squared_length();
    double resultt = hit_recurse(p, tolerance, leftd, rightd, 0, 1);
    return resultt >= 0;
}


//
// segmentize to list of points
//
// uses recursive subdivision
//

void
Bezier::segmentize(Vector<Point> &v, bool first) const
{
    if (is_flat(0.5)) {
	if (first)
	    v.push_back(_p[0]);
	v.push_back(_p[3]);
    } else {
	Bezier left, right;
	halve(left, right);
	left.segmentize(v, first);
	right.segmentize(v, false);
    }
}


//
// curve fitting
//
// code after Philip J. Schneider's algorithm described, with code, in the
// first Graphics Gems
//

static void
chord_length_parameterize(const Point *d, int nd, Vector<double> &result)
{
    assert(result.size() == 0);
    result.reserve(nd);
    result.push_back(0);
    for (int i = 1; i < nd; i++)
	result.push_back(result.back() + Point::distance(d[i-1], d[i]));
    double last_dist = result.back();
    for (int i = 1; i < nd; i++)
	result[i] /= last_dist;
}

static inline double
B0(double u)
{
    double m = 1.0 - u;
    return m*m*m;
}

static inline double
B1(double u)
{
    double m = 1.0 - u;
    return 3*m*m*u;
}

static inline double
B2(double u)
{
    double m = 1.0 - u;
    return 3*m*u*u;
}

static inline double
B3(double u)
{
    return u*u*u;
}

static Bezier
generate_bezier(const Point *d, int nd, const Vector<double> &parameters,
		const Point &left_tangent, const Point &right_tangent)
{
    Point *a0 = new Point[nd];
    Point *a1 = new Point[nd];

    for (int i = 0; i < nd; i++) {
	a0[i] = left_tangent * B1(parameters[i]);
	a1[i] = right_tangent * B2(parameters[i]);
    }

    double c[2][2], x[2];
    c[0][0] = c[0][1] = c[1][0] = c[1][1] = x[0] = x[1] = 0.0;

    int last = nd - 1;
    for (int i = 0; i < nd; i++) {
	c[0][0] += Point::dot(a0[i], a0[i]);
	c[0][1] += Point::dot(a0[i], a1[i]);
	c[1][1] += Point::dot(a1[i], a1[i]);

	Point tmp = d[i] - (d[0] * (B0(parameters[i]) + B1(parameters[i]))
			    + d[last] * (B2(parameters[i]) + B3(parameters[i])));
	x[0] += Point::dot(a0[i], tmp);
	x[1] += Point::dot(a1[i], tmp);
    }
    c[1][0] = c[0][1];

    // compute determinants
    double det_c0_c1 = c[0][0]*c[1][1] - c[1][0]*c[0][1];
    double det_c0_x = c[0][0]*x[1] - c[0][1]*x[0];
    double det_x_c1 = x[0]*c[1][1] - x[1]*c[0][1];

    // finally, derive alpha values
    if (det_c0_c1 == 0.0)
	det_c0_c1 = c[0][0]*c[1][1] * 10e-12;
    double alpha_l = det_x_c1 / det_c0_c1;
    double alpha_r = det_c0_x / det_c0_c1;

    // if alpha negative, use the Wu/Barsky heuristic
    if (alpha_l < 0.0 || alpha_r < 0.0) {
	double distance = Point::distance(d[0], d[last]) / 3;
	return Bezier(d[0], d[0] + left_tangent*distance,
		      d[last] + right_tangent*distance, d[last]);
    } else
	return Bezier(d[0], d[0] + left_tangent*alpha_l,
		      d[last] + right_tangent*alpha_r, d[last]);
}

static double
newton_raphson_root_find(const Bezier &b, const Point &p, double u)
{
    const Point *b_pts = b.points();

    Point b_det[3];
    for (int i = 0; i < 3; i++)
	b_det[i] = (b_pts[i+1] - b_pts[i]) * 3;

    Point b_det_det[2];
    for (int i = 0; i < 2; i++)
	b_det_det[i] = (b_det[i+1] - b_det[i]) * 2;

    Point b_u = b.eval(u);
    Point b_det_u = eval_bezier(b_det, 2, u);
    Point b_det_det_u = eval_bezier(b_det_det, 1, u);

    double numerator = Point::dot(b_u - p, b_det_u);
    double denominator = Point::dot(b_det_u, b_det_u) +
	Point::dot(b_u - p, b_det_det_u);

    return u - numerator/denominator;
}

static void
reparameterize(const Point *d, int nd, Vector<double> &parameters,
	       const Bezier &b)
{
    for (int i = 0; i < nd; i++)
	parameters[i] = newton_raphson_root_find(b, d[i], parameters[i]);
}

static double
compute_max_error(const Point *d, int nd, const Bezier &b,
		  const Vector<double> &parameters, int *split_point)
{
    *split_point = nd/2;
    double max_dist = 0.0;
    for (int i = 1; i < nd - 1; i++) {
	double dist = (b.eval(parameters[i]) - d[i]).squared_length();
	if (dist >= max_dist) {
	    max_dist = dist;
	    *split_point = i;
	}
    }
    return max_dist;
}

static void
fit0(const Point *d, int nd, Point left_tangent, Point right_tangent,
     double error, Vector<Bezier> &result)
{
    // Use a heuristic for small regions (only two points)
    if (nd == 2) {
	double dist = Point::distance(d[0], d[1]) / 3;
	result.push_back(Bezier(d[0],
				d[0] + dist*left_tangent,
				d[1] + dist*right_tangent,
				d[1]));
	return;
    }

    // Parameterize points and attempt to fit curve
    Vector<double> parameters;
    chord_length_parameterize(d, nd, parameters);
    Bezier b = generate_bezier(d, nd, parameters, left_tangent, right_tangent);

    // find max error
    int split_point;
    double max_error = compute_max_error(d, nd, b, parameters, &split_point);
    if (max_error < error) {
	result.push_back(b);
	return;
    }

    // if error not too large, try iteration and reparameterization
    if (max_error < error*error)
	for (int i = 0; i < 4; i++) {
	    reparameterize(d, nd, parameters, b);
	    b = generate_bezier(d, nd, parameters, left_tangent, right_tangent);
	    max_error = compute_max_error(d, nd, b, parameters, &split_point);
	    if (max_error < error) {
		result.push_back(b);
		return;
	    }
	}

    // fitting failed -- split at max error point and fit again
    Point center_tangent = ((d[split_point-1] - d[split_point+1])/2).normal();
    fit0(d, split_point+1, left_tangent, center_tangent, error, result);
    fit0(d+split_point, nd-split_point, -center_tangent, right_tangent, error, result);
}

void
Bezier::fit(const Vector<Point> &points, double error, Vector<Bezier> &result)
{
    int npoints = points.size();
    Point left_tangent = (points[1] - points[0]).normal();
    Point right_tangent = (points[npoints-2] - points[npoints-1]).normal();
    fit0(&points[0], npoints, left_tangent, right_tangent, error, result);
}
