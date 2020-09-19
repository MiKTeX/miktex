/*************************************************************************
** EllipticalArc.cpp                                                    **
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

#include <cmath>
#include "EllipticalArc.hpp"
#include "utility.hpp"

using namespace std;


/** Constructs an elliptical arc from end point parameterization.
 *  @param[in] start start point of arc
 *  @param[in] rx length of semi-major axis
 *  @param[in] ry length of semi-minor axis
 *  @param[in] angle rotation of ellipse around its center (in radians)
 *  @param[in] laf if true, choose the larger arc between start and end point
 *  @param[in] sweep if true, arc is drawn in the direction of increasing angles
 *  @param[in] end end point of the arc */
EllipticalArc::EllipticalArc (const DPair &start, double rx, double ry, double angle, bool laf, bool sweep, const DPair &end)
	: _rx(abs(rx)), _ry(abs(ry)), _rotationAngle(math::normalize_angle(angle, math::PI)),
	  _largeArc(laf), _sweepPositive(sweep), _startPoint(start), _endPoint(end)
{
	if (!isStraightLine()) {
		// fix out-of-range radii according to section F.6.6.3 in
		// https://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
		double c = cos(_rotationAngle);
		double s = sin(_rotationAngle);
		DPair p = (_startPoint-_endPoint)/2.0;
		p = DPair(c*p.x()+s*p.y(), c*p.y()-s*p.x());
		double lambda = (p.x()*p.x())/(_rx*_rx) + (p.y()*p.y())/(_ry*_ry);
		if (lambda > 1) {
			lambda = sqrt(lambda);
			_rx *= lambda;
			_ry *= lambda;
		}
	}
}


/** Constructs an elliptical arc from center parameterization
 *  @param[in] center absolute coordinates of the center of the ellipse
 *  @param[in] rx length of semi-major axis
 *  @param[in] ry length of semi-minor axis
 *  @param[in] rot rotation of ellipse around its center (in radians)
 *  @param[in] startAngle angle between major axis and vector from center to start point
 *  @param[in] deltaAngle angle between the vectors from center to start and end point, respectively */
EllipticalArc::EllipticalArc (const DPair &center, double rx, double ry, double rot, double startAngle, double deltaAngle)
	: _rx(rx), _ry(ry), _rotationAngle(math::normalize_angle(rot, math::TWO_PI)),
	  _largeArc(abs(deltaAngle) > math::PI), _sweepPositive(deltaAngle > 0)
{
	// https://www.w3.org/TR/SVG/implnote.html#ArcConversionCenterToEndpoint
	double c = cos(_rotationAngle);
	double s = sin(_rotationAngle);
	double c1 = cos(startAngle);
	double s1 = sin(startAngle);
	double c2 = cos(startAngle+deltaAngle);
	double s2 = sin(startAngle+deltaAngle);
	_startPoint = DPair(c*rx*c1*c - s*ry*s1, s*rx*c1 + c*ry*s1) + center;
	_endPoint   = DPair(c*rx*c2*c - s*ry*s2, s*rx*c2 + c*ry*s2) + center;
}


/** Returns the angle between (1, 0) and a given vector.
 *  The angle is normalized to the range [0, 2pi). */
static inline double angle (const DPair &p) {
	return math::normalize_0_2pi(atan2(p.y(), p.x()));
}


/** Computes the center parameterization of the arc. */
EllipticalArc::CenterParams EllipticalArc::getCenterParams () const {
	EllipticalArc::CenterParams params;
	if (isStraightLine()) {
		params.center = (_endPoint-_startPoint)/2.0;
		params.startAngle = params.deltaAngle = 0;
	}
	else {
		// https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
		double c = cos(_rotationAngle);
		double s = sin(_rotationAngle);
		DPair p = (_startPoint-_endPoint)/2.0;
		p = DPair(c*p.x()+s*p.y(), c*p.y()-s*p.x());
		double rx2 = _rx*_rx, ry2 = _ry*_ry;
		double px2 = p.x()*p.x(), py2 = p.y()*p.y();
		double radicand = rx2*ry2 - rx2*py2 - ry2*px2;
		if (radicand < 0)  // should not happen if out-of-range radii were fixed correctly
			radicand = 0;
		else
			radicand /= rx2*py2 + ry2*px2;
		double root = sqrt(radicand) * (_largeArc == _sweepPositive ? -1 : 1);
		DPair cp(root*p.y()*_rx/_ry, -root*p.x()*_ry/_rx);
		DPair mid = (_startPoint+_endPoint)/2.0;
		params.center = DPair(c*cp.x() - s*cp.y() + mid.x(), s*cp.x() + c*cp.y() + mid.y());
		DPair q1((p.x() - cp.x())/_rx, (p.y() - cp.y())/_ry);
		DPair q2(-(p.x() + cp.x())/_rx, -(p.y() + cp.y())/_ry);
		params.startAngle = angle(q1);
		params.deltaAngle = angle(q2) - params.startAngle;
		if (_sweepPositive && params.deltaAngle < 0)
			params.deltaAngle += math::TWO_PI;
		else if (!_sweepPositive && params.deltaAngle > 0)
			params.deltaAngle -= math::TWO_PI;
	}
	return params;
}


/** Applies the affine transformation described by a given matrix to the arc. */
void EllipticalArc::transform (const Matrix &matrix) {
	double c = cos(_rotationAngle);
	double s = sin(_rotationAngle);
	Matrix ellipse({_rx*c, -_ry*s, 0, _rx*s, _ry*c});  // E := rotate(xrot)*scale(rx, ry)
	ellipse.lmultiply(matrix);                         // E':= M*E
	// Compute the singular value decomposition of the transformed ellipse shape:
	// E' = rotate(phi)*scale(sx, sy)*rotate(theta)
	// The initial, right-hand rotation can be ignored because it rotates the unit circle
	// around the origin, i.e. rotate(theta) maps the circle to itself.
	// The signs of sx and sy don't matter either. They just flip the yet unrotated
	// ellipse on the x- and/or y-axis. Thus, |sx| and |sy| are the new radii,
	// and phi the new rotation angle.
	auto vec = math::svd({{ellipse.get(0,0), ellipse.get(0,1)}, {ellipse.get(1,0), ellipse.get(1,1)}});
	if (std::abs(vec[1]-vec[2]) < 1e-7) {  // circle?
		_rx = _ry = vec[1];   // always >= 0
		_rotationAngle = 0;
	}
	else {
		_rx = vec[1];         // always >= 0
		_ry = abs(vec[2]);    // ensure >= 0
		_rotationAngle = math::normalize_angle(vec[0], math::HALF_PI);
	}
	// change drawing direction (clockwise vs. counter-clockwise) if 'matrix'
	// flipped the ellipse horizontally or vertically but not both
	if ((matrix.get(0, 0) < 0) != (matrix.get(1, 1) < 0))
		_sweepPositive = !_sweepPositive;
	_startPoint = matrix * _startPoint;
	_endPoint = matrix * _endPoint;
}


/** Approximates an arc of the unit circle by a single cubic Bézier curve.
 *  @param[in] phi start angle of the arc in radians
 *  @param[in] delta length of the arc */
static Bezier approx_unit_arc (double phi, double delta) {
	double c = 0.551915024494;  // see http://spencermortensen.com/articles/bezier-circle
	if (abs(delta + math::HALF_PI) < 1e-7)
		c = -c;
	else
		c = 4.0/3*tan(delta/4);
	DPair p1(cos(phi), sin(phi));
	DPair p4(cos(phi+delta), sin(phi+delta));
	DPair p2(p1.x()-c*p1.y(), p1.y()+c*p1.x());
	DPair p3(p4.x()+c*p4.y(), p4.y()-c*p4.x());
	return Bezier(p1, p2, p3, p4);
}


/** Approximates the arc by a sequence of cubic Bézier curves. */
vector<Bezier> EllipticalArc::approximate () const {
	vector<Bezier> beziers;
	if (_startPoint != _endPoint) {
		if (isStraightLine()) {
			DPair dir = (_endPoint - _startPoint);
			dir /= dir.length()/3.0;
			beziers.emplace_back(Bezier(_startPoint, _startPoint+dir, _endPoint-dir, _endPoint));
		}
		else {
			CenterParams cparams = getCenterParams();
			int numCurves = ceil(cparams.deltaAngle/math::HALF_PI);
			double remainder = abs(fmod(cparams.deltaAngle, math::HALF_PI));
			if (remainder < 1e-7)
				numCurves--;
			else if (math::HALF_PI-remainder < 1e-7)
				numCurves++;
			if (numCurves > 0) {
				double c = cos(_rotationAngle);
				double s = sin(_rotationAngle);
				Matrix ellipse = {_rx*c, -_ry*s, cparams.center.x(), _rx*s, _ry*c, cparams.center.y()};
				double angle = cparams.startAngle;
				double diff = cparams.deltaAngle/numCurves;
				while (numCurves-- > 0) {
					beziers.emplace_back(approx_unit_arc(angle, diff).transform(ellipse));
					angle += diff;
				}
			}
		}
	}
	return beziers;
}


static inline bool is_angle_between (double t, double angle1, double angle2) {
	if (angle1 < angle2)
		return angle1 < t && t < angle2;
	return angle2 > t || t > angle1;
}


/** Returns the tight bounding box of the arc. */
BoundingBox EllipticalArc::getBBox () const {
	BoundingBox bbox;
	bbox.embed(_startPoint);
	bbox.embed(_endPoint);
	if (!isStraightLine()) {
		// compute extremes of ellipse centered at the origin
		double c = cos(_rotationAngle);
		double s = sin(_rotationAngle);
		double tx1 = math::normalize_0_2pi(-atan2(_ry*s, _rx*c));  // position of vertical tangent, d/dt E(tx1)=(0, y)
		double tx2 = math::normalize_0_2pi(math::PI+tx1);          // position of second vertical tangent
		double ct = cos(tx1);
		double st = sin(tx1);
		DPair pv1(_rx*c*ct - _ry*s*st, _rx*s*ct + _ry*c*st);       // E(tx1), 1st point on ellipse with vertical tangent
		DPair pv2 = -pv1;                                          // E(tx2), 2nd point on ellipse with vertical tangent

		double ty1 = math::normalize_0_2pi(atan2(_ry*c, _rx*s));   // position of horizontal tangent, d/dt E(ty1)=(x, 0)
		double ty2 = math::normalize_0_2pi(math::PI+ty1);          // position of second horizontal tangent
		ct = cos(ty1);
		st = sin(ty1);
		DPair ph1(_rx*c*ct - _ry*s*st, _rx*s*ct + _ry*c*st);       // E(ty1), 1st point on ellipse with horizontal tangent
		DPair ph2 = -ph1;                                          // E(ty2), 2nd point on ellipse with horizontal tangent

		// translate extreme points to actual coordinates
		CenterParams cparams = getCenterParams();
		pv1 += cparams.center;
		pv2 += cparams.center;
		ph1 += cparams.center;
		ph2 += cparams.center;

		double angle1 = cparams.startAngle;
		double angle2 = math::normalize_0_2pi(angle1+cparams.deltaAngle);
		if (!_sweepPositive)
			swap(angle1, angle2);

		// only consider extreme points located on the arc
		if (is_angle_between(tx1, angle1, angle2))
			bbox.embed(pv1);
		if (is_angle_between(tx2, angle1, angle2))
			bbox.embed(pv2);
		if (is_angle_between(ty1, angle1, angle2))
			bbox.embed(ph1);
		if (is_angle_between(ty2, angle1, angle2))
			bbox.embed(ph2);
	}
	return bbox;
}
