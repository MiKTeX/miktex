/*************************************************************************
** TriangularPatch.cpp                                                  **
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

#include "TriangularPatch.hpp"

using namespace std;

TriangularPatch::TriangularPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, TriangularPatch *patch)
	: ShadingPatch(cspace)
{
	setPoints(points, edgeflag, patch);
	setColors(colors, edgeflag, patch);
}


void TriangularPatch::setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) {
	auto triangularPatch = dynamic_cast<TriangularPatch*>(patch);
	if (edgeflag > 0 && !triangularPatch)
		throw ShadingException("missing preceding data in definition of triangular patch");
	if ((edgeflag == 0 && points.size() != 3) || (edgeflag > 0 && points.size() != 1))
		throw ShadingException("invalid number of vertices in triangular patch definition");

	_points[0] = points[0];
	switch (edgeflag) {
		case 0:
			_points[1] = points[1];
			_points[2] = points[2];
			break;
		case 1:
			_points[1] = triangularPatch->_points[1];
			_points[2] = triangularPatch->_points[2];
			break;
		case 2:
			_points[1] = triangularPatch->_points[2];
			_points[2] = triangularPatch->_points[0];
	}
}


void TriangularPatch::setPoints (const DPair &p1, const DPair &p2, const DPair &p3) {
	_points[0] = p1;
	_points[1] = p2;
	_points[2] = p3;
}


void TriangularPatch::setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch) {
	auto triangularPatch = dynamic_cast<TriangularPatch*>(patch);
	if (edgeflag > 0 && !triangularPatch)
		throw ShadingException("missing preceding data in definition of triangular patch");
	if ((edgeflag == 0 && colors.size() != 3) || (edgeflag > 0 && colors.size() != 1))
		throw ShadingException("invalid number of colors in triangular patch definition");

	_colors[0] = colors[0];
	switch (edgeflag) {
		case 0:
			_colors[1] = colors[1];
			_colors[2] = colors[2];
			break;
		case 1:
			_colors[1] = triangularPatch->_colors[1];
			_colors[2] = triangularPatch->_colors[2];
			break;
		case 2:
			_colors[1] = triangularPatch->_colors[2];
			_colors[2] = triangularPatch->_colors[0];
	}
}


void TriangularPatch::setColors (const Color &c1, const Color &c2, const Color &c3) {
	_colors[0] = c1;
	_colors[1] = c2;
	_colors[2] = c3;
}


/** Returns the Cartesian coordinates for the barycentric coordinates \f$(u, v, 1-u-v)\f$
 *  of a point of the triangle, where \f$u, v \in [0,1]\f$ and \f$u+v \le 1\f$.
 *  The relation between the vertices of the triangle and their barycentric coordinates
 *  is as follows: \f$(1,0,0)=p_1, (0,1,0)=p_2, (0,0,1)=p_0\f$. */
DPair TriangularPatch::valueAt (double u, double v) const {
	return _points[0] + (_points[1]-_points[0])*u + (_points[2]-_points[0])*v;
}


/** Returns the color at a given point of the triangle. The point must be given
 *  in barycentric coordinates \f$(u, v, 1-u-v)\f$, where \f$u, v \in [0,1]\f$
 *  and \f$u+v \le 1\f$.
 *  The relation between the vertices of the triangle and their barycentric coordinates
 *  is as follows: \f$(1,0,0)=p_1, (0,1,0)=p_2, (0,0,1)=p_0\f$. */
Color TriangularPatch::colorAt (double u, double v) const {
	ColorGetter getComponents;
	ColorSetter setComponents;
	colorQueryFuncs(getComponents, setComponents);
	valarray<double> comp[3];
	for (int i=0; i < 3; i++)
		(_colors[i].*getComponents)(comp[i]);
	Color color;
	(color.*setComponents)(comp[0]*(1-u-v) + comp[1]*u + comp[2]*v);
	return color;
}


Color TriangularPatch::averageColor () const {
	return averageColor(_colors[0], _colors[1], _colors[2]);
}


/** Compute the average of three given colors depending on the assigned color space. */
Color TriangularPatch::averageColor (const Color &c1, const Color &c2, const Color &c3) const {
	ColorGetter getComponents;
	ColorSetter setComponents;
	colorQueryFuncs(getComponents, setComponents);
	valarray<double> va1, va2, va3;
	(c1.*getComponents)(va1);
	(c2.*getComponents)(va2);
	(c3.*getComponents)(va3);
	Color averageColor;
	(averageColor.*setComponents)((va1+va2+va3)/3.0);
	return averageColor;
}


/** Snaps value x to the interval [0,1]. Values lesser than or near 0 are mapped to 0, values
 *  greater than or near 1 are mapped to 1. */
static inline double snap (double x) {
	if (abs(x) < 0.001)
		return 0;
	if (abs(1-x) < 0.001)
		return 1;
	return x;
}


/** Approximate the patch by dividing it into a grid of triangular segments that are filled
 *  with the average color of the corresponding region. In order to prevent visual gaps between
 *  adjacent segments due to anti-aliasing, the flag 'overlap' can be set. It enlarges the
 *  segments so that they overlap with their right and bottom neighbors (which are drawn on
 *  top of the overlapping regions).
 *  @param[in] gridsize number of segments per row/column
 *  @param[in] overlap if true, enlarge each segment to overlap with its right and bottom neighbors
 *  @param[in] delta reduce level of detail if the segment size is smaller than the given value
 *  @param[in] callback object notified */
void TriangularPatch::approximate (int gridsize, bool overlap, double delta, Callback &callback) const {
	if (_colors[0] == _colors[1] && _colors[1] == _colors[2]) {
		GraphicsPath<double> path = getBoundaryPath();
		callback.patchSegment(path, _colors[0]);
	}
	else {
		const double inc = 1.0/gridsize;
		for (double u1=0; u1 < 1; u1=snap(u1+inc)) {
			double u2 = snap(u1+inc);
			double ou2 = (overlap && snap(u2+inc) <= 1 ? snap(u2+inc) : u2);
			for (double v1=0; snap(u1+v1) < 1; v1=snap(v1+inc)) {
				double v2 = snap(v1+inc);
				double ov2 = (overlap && snap(v2+inc) <= 1 ? snap(v2+inc) : v2);
				if (!overlap || (snap(u1+ov2) <= 1 && snap(ou2+v1) <= 1)) {
					// create triangular segments pointing in the same direction as the whole patch
					GraphicsPath<double> path;
					path.moveto(valueAt(u1, v1));
					path.lineto(valueAt(ou2, v1));
					path.lineto(valueAt(u1, ov2));
					path.closepath();
					callback.patchSegment(path, averageColor(colorAt(u1, v1), colorAt(u2, v1), colorAt(u1, v2)));
					if (snap(u2+v2) <= 1 && (!overlap || inc > delta)) {
						// create triangular segments pointing in the opposite direction as the whole patch
						path.clear();
						path.moveto(valueAt(u1, v2));
						path.lineto(valueAt(u2, v1));
						path.lineto(valueAt(u2, v2));
						path.closepath();
						callback.patchSegment(path, averageColor(colorAt(u1, v2), colorAt(u2, v1), colorAt(u2, v2)));
					}
				}
			}
		}
	}
}


GraphicsPath<double> TriangularPatch::getBoundaryPath () const {
	GraphicsPath<double> path;
	path.clear();
	path.moveto(_points[0]);
	path.lineto(_points[1]);
	path.lineto(_points[2]);
	path.closepath();
	return path;
}


BoundingBox TriangularPatch::getBBox () const {
	BoundingBox bbox;
	bbox.embed(_points[0]);
	bbox.embed(_points[1]);
	bbox.embed(_points[2]);
	return bbox;
}
