/*************************************************************************
** Bezier.cpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#include <algorithm>
#include <utility>
#include "Bezier.hpp"
#include "Matrix.hpp"

using namespace std;

Bezier::Bezier () {
	_points[0] = _points[1] = _points[2] = _points[3] = DPair(0);
}


/** Creates a quadratic Bézier curve. internally, it's represented as a cubic one. */
Bezier::Bezier (const DPair &p0, const DPair &p1, const DPair &p2) {
	setPoints(p0, p0+(p1-p0)*2.0/3.0, p2+(p1-p2)*2.0/3.0, p2);
}


Bezier::Bezier (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3) {
	setPoints(p0, p1, p2, p3);
}


/** Creates a subcurve of a given Bézier curve.
 *  @param[in] source original curve to be clipped
 *  @param[in] t0 'time' parameter \f$\in[0,1]\f$ of source curve where the subcurve starts
 *  @param[in] t1 'time' parameter \f$\in[0,1]\f$ of source curve where the subcurve ends */
Bezier::Bezier (const Bezier &source, double t0, double t1) {
	if (t0 == t1)
		_points[0] = _points[1] = _points[2] = _points[3] = source.valueAt(t0);
	else {
		if (t0 > t1)
			swap(t0, t1);
		if (t0 == 0)
			source.subdivide(t1, this, nullptr);
		else if (t1 == 1)
			source.subdivide(t0, nullptr, this);
		else {
			Bezier subcurve;
			source.subdivide(t0, nullptr, &subcurve);
			subcurve.subdivide((t1-t0)/(1-t0), this, nullptr);
		}
	}
}


void Bezier::setPoints(const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3) {
	_points[0] = p0;
	_points[1] = p1;
	_points[2] = p2;
	_points[3] = p3;
}


void Bezier::reverse() {
	swap(_points[0], _points[3]);
	swap(_points[1], _points[2]);
}


DPair Bezier::valueAt (double t) const {
	const double s = 1-t;
	return _points[0]*s*s*s + _points[1]*3.0*s*s*t + _points[2]*3.0*s*t*t + _points[3]*t*t*t;
}


/** Returns a value of the Bézier curve's blossom representation. */
DPair Bezier::blossomValue (double u, double v, double w) const {
	const double uv = u*v;
	const double uw = u*w;
	const double vw = v*w;
	const double uvw = u*v*w;
	return _points[0]*(1.0-u-v-w+uv+uw+vw-uvw)
		+_points[1]*(u+v+w-2.0*(uv+uw+vw)+3.0*uvw)
		+_points[2]*(uv+uw+vw-3.0*uvw)
		+_points[3]*uvw;
}


/** Splits the curve at t into two sub-curves. */
void Bezier::subdivide (double t, Bezier *bezier1, Bezier *bezier2) const {
	const double s = 1-t;
	DPair p01   = _points[0]*s + _points[1]*t;
	DPair p12   = _points[1]*s + _points[2]*t;
	DPair p23   = _points[2]*s + _points[3]*t;
	DPair p012  = p01*s + p12*t;
	DPair p123  = p12*s + p23*t;
	DPair p0123 = p012*s + p123*t;
	if (bezier1)
		bezier1->setPoints(_points[0], p01, p012, p0123);
	if (bezier2)
		bezier2->setPoints(p0123, p123, p23, _points[3]);
}


/** Approximates the current Bézier curve by a sequence of line segments.
 *  This is done by subdividing the curve several times using De Casteljau's algorithm.
 *  If a sub-curve is almost flat, i.e. \f$\sum\limits_{k=0}^2 |p_{k+1}-p_k| - |p_3-p_0| < \delta\f$,
 *  the curve is not further subdivided.
 *  @param[in] delta threshold where to stop further subdivisions (see description above)
 *  @param[out] p the resulting sequence of points defining the start/end points of the line segments
 *  @param[out] t corresponding curve parameters of the approximated points p: \f$ b(t_i)=p_i \f$
 *  @return number of points in vector p */
int Bezier::approximate (double delta, std::vector<DPair> &p, vector<double> *t) const {
	p.push_back(_points[0]);
	if (t)
		t->push_back(0);
	return approximate(delta, 0, 1, p, t);
}


int Bezier::approximate (double delta, double t0, double t1, vector<DPair> &p, vector<double> *t) const {
	// compute distance of adjacent control points
	const double l01 = (_points[1]-_points[0]).length();
	const double l12 = (_points[2]-_points[1]).length();
	const double l23 = (_points[3]-_points[2]).length();
	const double l03 = (_points[3]-_points[0]).length();
	if (l01+l12+l23-l03 < delta) { // is curve flat enough?
		p.push_back(_points[3]);    // => store endpoint
		if (t)
			t->push_back(t1);
	}
	else {
		// subdivide curve at b(0.5) and approximate the resulting parts separately
		Bezier b1, b2;
		subdivide(0.5, &b1, &b2);
		double tmid = (t0+t1)/2;
		b1.approximate(delta, t0, tmid, p, t);
		b2.approximate(delta, tmid, t1, p, t);
	}
	return p.size();
}


/** Returns the signed area of the triangle (p1, p2, p3). */
static inline double signed_area (const DPair &p1, const DPair &p2, const DPair &p3) {
	return ((p2.x()-p1.x())*(p3.y()-p1.y()) - (p3.x()-p1.x())*(p2.y()-p1.y()))/2.0;
}


static inline double dot_prod (const DPair &p1, const DPair &p2) {
	return p1.x()*p2.x() + p1.y()*p2.y();
}


/** Returns true if p3 is located between p1 and p2, i.e. p3 lays almost on the line
 *  between p1 and p2. */
static bool between (const DPair &p1, const DPair &p2, const DPair &p3, double delta) {
	double sqr_dist = dot_prod(p2-p1, p2-p1);
	double factor = sqr_dist == 0.0 ? 1.0 : sqr_dist;
	double area2 = abs(signed_area(p1, p2, p3));
	return area2*area2/factor < delta    // does p3 lay almost on the line through p1 and p2...
		&& min(p1.x(), p2.x()) <= p3.x()  // ...and on or inside the rectangle spanned by p1 and p2?
		&& max(p1.x(), p2.x()) >= p3.x()
		&& min(p1.y(), p2.y()) <= p3.y()
		&& max(p1.y(), p2.y()) >= p3.y();
}


static inline bool near (const DPair &p1, const DPair &p2, double delta) {
	DPair diff = p2-p1;
	return abs(diff.x()) < delta && abs(diff.y()) < delta;
}


/** Tries to reduce the degree of the Bézier curve. This only works if the number of
 *  control points can be reduces without changing the shape of the curve significantly.
 *  @param[in] delta deviation tolerance
 *  @param[in] p control points of the reduced curve
 *  @return degree of the reduced curve */
int Bezier::reduceDegree (double delta, vector<DPair> &p) const {
	p.clear();
	if (near(_points[0], _points[1], delta) && near(_points[0], _points[2], delta) && near(_points[0], _points[3], delta))
		p.push_back(_points[0]);
	else if (between(_points[0], _points[3], _points[1], delta) && between(_points[0], _points[3], _points[2], delta)) {
		p.push_back(_points[0]);
		p.push_back(_points[3]);
	}
	else if (near((_points[1]-_points[0])*1.5+_points[0], (_points[2]-_points[3])*1.5+_points[3], delta)) {
		p.push_back(_points[0]);
		p.push_back((_points[1]-_points[0])*1.5 + _points[0]);
		p.push_back(_points[3]);
	}
	else {
		p.resize(4);
		for (int i=0; i < 4; i++)
			p[i] = _points[i];
	}
	return p.size()-1;
}


/** Try to solve the quadratic equation ax^2 + bx + c = 0. */
static bool solve_quadratic_equation (double a, double b, double c, double &x1, double &x2) {
	if (a == 0) {
		if (b == 0)
			return false;
		x1 = x2 = -c/b;
	}
	else {
		double discr = b*b - 4*a*c;
		if (discr < 0)
			return false;
		double p = -b/a/2;
		double r = sqrt(discr)/a/2;
		x1 = p+r;
		x2 = p-r;
	}
	return true;
}


/** Returns a tight bounding box parallel to the x- and y-axis. */
BoundingBox Bezier::getBBox () const {
	BoundingBox bbox;
	// coefficients of the derivative
	DPair pa = _points[3] - _points[2]*3.0 + _points[1]*3.0 - _points[0];
	DPair pb = (_points[2]-_points[1]*2.0+_points[0])*2.0;
	DPair pc = _points[1]-_points[0];

	// compute extrema for t > 0 and t < 1
	double t1, t2;
	if (solve_quadratic_equation(pa.x(), pb.x(), pc.x(), t1, t2)) {
		if (t1 > 0.001 && t1 < 0.999)
			bbox.embed(valueAt(t1));
		if (t1 != t2 && t2 > 0.001 && t2 < 0.999)
			bbox.embed(valueAt(t2));
	}
	if (solve_quadratic_equation(pa.y(), pb.y(), pc.y(), t1, t2)) {
		if (t1 > 0.001 && t1 < 0.999)
			bbox.embed(valueAt(t1));
		if (t1 != t2 && t2 > 0.001 && t2 < 0.999)
			bbox.embed(valueAt(t2));
	}
	bbox.embed(_points[0]);
	bbox.embed(_points[3]);
	return bbox;
}


Bezier& Bezier::transform (const Matrix &matrix) {
	for (int i=0; i < 4; i++)
		_points[i] = matrix*_points[i];
	return *this;
}
