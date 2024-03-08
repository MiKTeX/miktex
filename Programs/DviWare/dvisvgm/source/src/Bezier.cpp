/*************************************************************************
** Bezier.cpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <algorithm>
#include <utility>
#include "Bezier.hpp"
#include "Matrix.hpp"
#include "utility.hpp"

using namespace std;

QuadBezier::QuadBezier () {
	_points[0] = _points[1] = _points[2]  = DPair(0, 0);
}


QuadBezier::QuadBezier (const DPair &p0, const DPair &p1, const DPair &p2) {
	setPoints(p0, p1, p2);
}


void QuadBezier::setPoints(const DPair &p0, const DPair &p1, const DPair &p2) {
	_points[0] = p0;
	_points[1] = p1;
	_points[2] = p2;
}


/** Returns the value (curve point) at t. */
DPair QuadBezier::valueAt (double t) const {
	const double s = 1-t;
	return _points[0]*s*s + _points[1]*2.0*s*t + _points[2]*t*t;
}


/** Returns the value of the first derivative of the curve at t. */
DPair QuadBezier::derivativeAt (double t) const {
	return _points[0]*(2*t-2) + _points[1]*(2-4*t) + _points[2]*(2*t);
}


/** Returns the arc length of the curve from 0 to t. */
double QuadBezier::arclen (double t) const {
	return math::integral(0, t, 20, [this](double t) -> double {
		DPair deriv = derivativeAt(t);
		return sqrt(deriv.x()*deriv.x() + deriv.y()*deriv.y());
	});
}


////////////////////////////////////////////////////////////////////////////////


CubicBezier::CubicBezier () {
	_points[0] = _points[1] = _points[2] = _points[3] = DPair(0, 0);
}


CubicBezier::CubicBezier (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3) {
	setPoints(p0, p1, p2, p3);
}


/** Creates a subcurve of a given Bézier curve.
 *  @param[in] source original curve to be clipped
 *  @param[in] t0 'time' parameter \f$\in[0,1]\f$ of source curve where the subcurve starts
 *  @param[in] t1 'time' parameter \f$\in[0,1]\f$ of source curve where the subcurve ends */
CubicBezier::CubicBezier (const CubicBezier &source, double t0, double t1) {
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
			CubicBezier subcurve;
			source.subdivide(t0, nullptr, &subcurve);
			subcurve.subdivide((t1-t0)/(1-t0), this, nullptr);
		}
	}
}


/** Creates a cubic Bézier from a quadratic one. */
CubicBezier::CubicBezier (const QuadBezier &qbezier) {
	const DPair &p0 = qbezier.point(0);
	const DPair &p1 = qbezier.point(1);
	const DPair &p2 = qbezier.point(2);
	setPoints(p0, p0+(p1-p0)*2.0/3.0, p2+(p1-p2)*2.0/3.0, p2);
}


void CubicBezier::setPoints(const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3) {
	_points[0] = p0;
	_points[1] = p1;
	_points[2] = p2;
	_points[3] = p3;
}


void CubicBezier::reverse() {
	swap(_points[0], _points[3]);
	swap(_points[1], _points[2]);
}


DPair CubicBezier::valueAt (double t) const {
	const double s = 1-t;
	return _points[0]*s*s*s + _points[1]*3.0*s*s*t + _points[2]*3.0*s*t*t + _points[3]*t*t*t;
}


/** Returns a value of the Bézier curve's blossom representation. */
DPair CubicBezier::blossomValue (double u, double v, double w) const {
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
void CubicBezier::subdivide (double t, CubicBezier *bezier1, CubicBezier *bezier2) const {
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
int CubicBezier::approximate (double delta, std::vector<DPair> &p, vector<double> *t) const {
	p.push_back(_points[0]);
	if (t)
		t->push_back(0);
	return approximate(delta, 0, 1, p, t);
}


int CubicBezier::approximate (double delta, double t0, double t1, vector<DPair> &p, vector<double> *t) const {
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
		CubicBezier b1, b2;
		subdivide(0.5, &b1, &b2);
		double tmid = (t0+t1)/2;
		b1.approximate(delta, t0, tmid, p, t);
		b2.approximate(delta, tmid, t1, p, t);
	}
	return static_cast<int>(p.size());
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
int CubicBezier::reduceDegree (double delta, vector<DPair> &p) const {
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
	return static_cast<int>(p.size()-1);
}


/** Approximates the cubic Bézier curve by a sequence of quadratic ones.
 *  @param[in] precision specifies the precision of the approximation
 *  @return map containing the split parameters t_n together with the qudratic curves */
vector<QuadBezier> CubicBezier::toQuadBeziers (double precision, vector<double> *splitParams) const {
	vector<QuadBezier> qbeziers;
	toQuadBeziers(0, 1, precision, qbeziers, splitParams);
	return qbeziers;
}


/** Returns the "mid-point approximation" of this cubic Bézier. */
QuadBezier CubicBezier::midpointApproximation () const {
	 DPair p0 = (_points[1]*3.0 - _points[0])/2.0;
	 DPair p1 = (_points[2]*3.0 - _points[3])/2.0;
	 return {_points[0], (p0+p1)/2.0, _points[3]};
}


/** Approximates a segment of a cubic Bézier curve by a sequence of quadratic curves.
 *  The quadratic segments are computed by adaptive subdivision of the cubic curve
 *  as described at http://www.caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
 *  @param[in] t0 curve parameter of the segment's start point
 *  @param[in] t1 curve parameter of the segment's end point
 *  @param[in] precision maximum allowed distance between the curve points b(t) and the
 *  	corresponding approximated point c(t) on the quadratic curve
 *  @param[out] qbeziers the resulting quadratic curves sorted in ascending order by their split points
 *  @param[out] startParams the start parameters t[k] of the curves qbeziers[k] relative to *this. */
void CubicBezier::toQuadBeziers (double t0, double t1, double precision, vector<QuadBezier> &qbeziers, vector<double> *startParams) const {
	// If -p0+3p1-3p2+p3 = 0, the degree of the curve is <= 2 and it's not necessary do any approximation.
	// In this case, the control point of the quadratic Bézier curve is (-p0+3p1)/2 = (3p2-p3)/2.
	// Otherwise, the distance d between q1:=(-p0+3p1)/2 and q2:=(3p2-p3)/2 is != 0.
	// Now we compute the quadratic Bézier with start point p0, end point p3, and control point (q1+q2)/2,
	// the "mid-point approximation" (MPA) of b.
	// The maximal distance between the points of the original curve b and the corresponding ones
	// on the MPA is d*sqrt(3)/18. The same computations can be done for the cubic curves we get
	// when subdividing b at a parameter t. The maximal distance of these curves to their MPA
	// is t^3*d*sqrt(3)/18.
	// Based on the formula for this distance we compute the split point tmax for a given precision
	// and check 3 cases:
	// * tmax >= 1:   curve is quadratic, return MPA of b
	// * tmax >= 0.5: split b at t=0.5, return the MPAs of both segments
	// * tmax < 0.5:  split b at tmax and 1-tmax, return the MPAs of the first and third segment,
	//                recurse the algorithm for the middle segment
	DPair q1 = (_points[1]*3.0 - _points[0])/2.0;
	DPair q2 = (_points[2]*3.0 - _points[3])/2.0;
	double dist = (q2-q1).length();
	double tmax3 = 18.0/sqrt(3.0)*precision/dist;  // the cube of tmax
	if (tmax3 >= 1.0) {
		// curve is already quadratic, no subdivision necessary, return MPA
		qbeziers.emplace_back(_points[0], (q1+q2)/2.0, _points[3]);
		if (startParams)
			startParams->push_back(t0);
	}
	else if (tmax3 >= 0.125) { // tmax >= 0.5
		// split the curve at t=0.5 and compute the MPA for both segments
		CubicBezier cbezier1, cbezier2;
		subdivide(0.5, &cbezier1, &cbezier2);
		qbeziers.emplace_back(cbezier1.midpointApproximation());
		qbeziers.emplace_back(cbezier2.midpointApproximation());
		if (startParams) {
			startParams->push_back(t0);
			startParams->push_back((t0+t1)/2);
		}
	}
	else { // tmax < 0.5
		double tmax = cbrt(tmax3);
		double smax = 1.0-tmax;
		double dt = t1-t0;
		// first segment can be approximated by its MPA
		qbeziers.emplace_back(CubicBezier(*this, 0, tmax).midpointApproximation());
		if (startParams)
			startParams->push_back(t0);
		// recurse for middle segment
		CubicBezier(*this, tmax, smax).toQuadBeziers(t0+tmax*dt, t0+smax*dt, precision, qbeziers, startParams);
		// third segment can be approximated by its MPA
		qbeziers.emplace_back(CubicBezier(*this, smax, 1).midpointApproximation());
		if (startParams)
			startParams->push_back(smax);
	}
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
BoundingBox CubicBezier::getBBox () const {
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


CubicBezier& CubicBezier::transform (const Matrix &matrix) {
	for (auto &point : _points)
		point = matrix * point;
	return *this;
}

