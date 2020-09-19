/*************************************************************************
** TensorProductPatch.cpp                                               **
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

#include <valarray>
#include "TensorProductPatch.hpp"

using namespace std;


TensorProductPatch::TensorProductPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, TensorProductPatch *patch)
	: ShadingPatch(cspace)
{
	setPoints(points, edgeflag, patch);
	setColors(colors, edgeflag, patch);
}


void TensorProductPatch::setFirstMatrixColumn (const DPair source[4], bool reverse) {
	for (int i=0; i < 4; i++)
		_points[i][0] = source[reverse ? 3-i : i];
}


void TensorProductPatch::setFirstMatrixColumn (DPair source[4][4], int col, bool reverse) {
	for (int i=0; i < 4; i++)
		_points[i][0] = source[reverse ? 3-i : i][col];
}


/*void TensorProductPatch::setPoints (const DPair points[4][4]) {
	for (int i=0; i < 4; i++)
		for (int j=0; j < 4; j++)
			_points[i][j] = points[i][j];
}*/


/** Sets the control points defining the structure of the patch. If the edge flag is 0,
 *  the point vector must contain all 16 control points of the 4x4 matrix in "spiral" order:
 *    0 11 10  9
 *    1 12 15  8
 *    2 13 14  7
 *    3  4  5  6
 *  If the edge flag is 1,2, or 3, the points of the first matrix collumn
 *  are omitted, and taken from a reference patch instead.
 *  @param[in] points the control points in "spiral" order as described in the PS reference, p. 286
 *  @param[in] edgeflag defines how to connect this patch with another one
 *  @param[in] patch reference patch required if edgeflag > 0 */
void TensorProductPatch::setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) {
	auto tpPatch = dynamic_cast<TensorProductPatch*>(patch);
	if (edgeflag > 0 && !tpPatch)
		throw ShadingException("missing preceding data in definition of tensor-product patch");
	if ((edgeflag == 0 && points.size() != 16) || (edgeflag > 0 && points.size() != 12))
		throw ShadingException("invalid number of control points in tensor-product patch definition");

	// assign the 12 control points that are invariant for all edge flag values
	int i = (edgeflag == 0 ? 4 : 0);
	_points[3][1] = points[i++];
	_points[3][2] = points[i++];
	_points[3][3] = points[i++];
	_points[2][3] = points[i++];
	_points[1][3] = points[i++];
	_points[0][3] = points[i++];
	_points[0][2] = points[i++];
	_points[0][1] = points[i++];
	_points[1][1] = points[i++];
	_points[2][1] = points[i++];
	_points[2][2] = points[i++];
	_points[1][2] = points[i];
	// populate the first column of the control point matrix
	switch (edgeflag) {
		case 0: setFirstMatrixColumn(&points[0], false); break;
		case 1: setFirstMatrixColumn(tpPatch->_points[3], false); break;
		case 2: setFirstMatrixColumn(tpPatch->_points, 3, true); break;
		case 3: setFirstMatrixColumn(tpPatch->_points[0], true); break;
	}
}


/** Sets the vertex colors of the patch. If the edge flag is 0,
 *  the color vector must contain all 4 colors in the following order:
 *  c00, c30, c33, c03, where cXY belongs to the vertex pXY of the control
 *  point matrix.
 *  c00 ---- c03
 *   |        |
 *   |        |
 *  c30 ---- c33
 *  If the edge flag is 1,2, or 3, the colors c00 and c30 are omitted,
 *  and taken from a reference patch instead.
 *  @param[in] points the color values in the order c00, c30, c33, c03
 *  @param[in] edgeflag defines how to connect this patch with another one
 *  @param[in] patch reference patch required if edgeflag > 0 */
void TensorProductPatch::setColors(const ColorVec &colors, int edgeflag, ShadingPatch* patch) {
	auto tpPatch = dynamic_cast<TensorProductPatch*>(patch);
	if (edgeflag > 0 && !tpPatch)
		throw ShadingException("missing preceding data in definition of tensor-product patch");
	if ((edgeflag == 0 && colors.size() != 4) || (edgeflag > 0 && colors.size() != 2))
		throw ShadingException("invalid number of colors in tensor-product patch definition");

	int i = (edgeflag == 0 ? 2 : 0);
	_colors[3] = colors[i];
	_colors[1] = colors[i+1];
	switch (edgeflag) {
		case 0: _colors[0] = colors[0]; _colors[2] = colors[1]; break;
		case 1: _colors[0] = tpPatch->_colors[2]; _colors[2] = tpPatch->_colors[3]; break;
		case 2: _colors[0] = tpPatch->_colors[3]; _colors[2] = tpPatch->_colors[1]; break;
		case 3: _colors[0] = tpPatch->_colors[1]; _colors[2] = tpPatch->_colors[0]; break;
	}
}


/** Returns the point P(u,v) of the patch. */
DPair TensorProductPatch::valueAt (double u, double v) const {
	// check if we can return one of the vertices
	if (u == 0) {
		if (v == 0)
			return _points[0][0];
		else if (v == 1)
			return _points[3][0];
	}
	else if (u == 1) {
		if (v == 0)
			return _points[0][3];
		else if (v == 1)
			return _points[3][3];
	}
	// compute tensor product
	DPair p[4];
	for (int i=0; i < 4; i++) {
		Bezier bezier(_points[i][0], _points[i][1], _points[i][2], _points[i][3]);
		p[i] = bezier.valueAt(u);
	}
	Bezier bezier(p[0], p[1], p[2], p[3]);
	return bezier.valueAt(v);
}


/** Returns the color at point P(u,v) which is bilinearly interpolated from
 *  the colors assigned to vertices of the patch. */
Color TensorProductPatch::colorAt (double u, double v) const {
	// check if we can return one of the vertex colors
	if (u == 0) {
		if (v == 0)
			return _colors[0];
		else if (v == 1)
			return _colors[2];
	}
	else if (u == 1) {
		if (v == 0)
			return _colors[1];
		else if (v == 1)
			return _colors[3];
	}
	// interpolate color
	ColorGetter getComponents;
	ColorSetter setComponents;
	colorQueryFuncs(getComponents, setComponents);
	valarray<double> comp[4];
	for (int i=0; i < 4; i++)
		(_colors[i].*getComponents)(comp[i]);
	Color color;
	(color.*setComponents)((1-u)*(1-v)*comp[0] + u*(1-v)*comp[1] + (1-u)*v*comp[2] + u*v*comp[3]);
	return color;
}


Color TensorProductPatch::averageColor () const {
	return averageColor(_colors[0], _colors[1], _colors[2], _colors[3]);
}


/** Compute the average of four given colors depending on the assigned color space. */
Color TensorProductPatch::averageColor (const Color &c1, const Color &c2, const Color &c3, const Color &c4) const {
	ColorGetter getComponents;
	ColorSetter setComponents;
	colorQueryFuncs(getComponents, setComponents);
	valarray<double> va1, va2, va3, va4;
	(c1.*getComponents)(va1);
	(c2.*getComponents)(va2);
	(c3.*getComponents)(va3);
	(c4.*getComponents)(va4);
	Color averageColor;
	(averageColor.*setComponents)((va1+va2+va3+va4)/4.0);
	return averageColor;
}


GraphicsPath<double> TensorProductPatch::getBoundaryPath () const {
	// Simple approach: Use the outer curves as boundary path. This doesn't always lead
	// to correct results since, depending on the control points, P(u,v) might exceed
	// the simple boundary.
	GraphicsPath<double> path;
	path.moveto(_points[0][0]);
	path.cubicto(_points[0][1], _points[0][2], _points[0][3]);
	path.cubicto(_points[1][3], _points[2][3], _points[3][3]);
	path.cubicto(_points[3][2], _points[3][1], _points[3][0]);
	path.cubicto(_points[2][0], _points[1][0], _points[0][0]);
	path.closepath();
	return path;
}


/** Computes the bicubically interpolated isoparametric Bézier curve P(u,t) that
 *  runs "vertically" from P(u,0) to P(u,1) through the patch P.
 *  @param[in] u "horizontal" parameter in the range from 0 to 1
 *  @param[out] bezier the resulting Bézier curve */
void TensorProductPatch::verticalCurve (double u, Bezier &bezier) const {
	// check for simple cases (boundary curves) first
	if (u == 0)
		bezier.setPoints(_points[0][0], _points[1][0], _points[2][0], _points[3][0]);
	else if (u == 1)
		bezier.setPoints(_points[0][3], _points[1][3], _points[2][3], _points[3][3]);
	else {
		// compute "inner" curve
		DPair p[4];
		for (int i=0; i < 4; i++) {
			Bezier bezier(_points[i][0], _points[i][1], _points[i][2], _points[i][3]);
			p[i] = bezier.valueAt(u);
		}
		bezier.setPoints(p[0], p[1], p[2], p[3]);
	}
}


/** Computes the bicubically interpolated isoparametric Bézier curve P(t,v) that
 *  runs "horizontally" from P(0,v) to P(1,v) through the patch P.
 *  @param[in] v "vertical" parameter in the range from 0 to 1
 *  @param[out] bezier the resulting Bézier curve */
void TensorProductPatch::horizontalCurve (double v, Bezier &bezier) const {
	// check for simple cases (boundary curves) first
	if (v == 0)
		bezier.setPoints(_points[0][0], _points[0][1], _points[0][2], _points[0][3]);
	else if (v == 1)
		bezier.setPoints(_points[3][0], _points[3][1], _points[3][2], _points[3][3]);
	else {
		// compute "inner" curve
		DPair p[4];
		for (int i=0; i < 4; i++) {
			Bezier bezier(_points[0][i], _points[1][i], _points[2][i], _points[3][i]);
			p[i] = bezier.valueAt(v);
		}
		bezier.setPoints(p[0], p[1], p[2], p[3]);
	}
}


/** Computes the sub-patch that maps the unit square [0,1]x[0,1] to
 *  the area P([u1,u2],[v1,v2]) of patch P. The control points of the sub-patch
 *  can easily be calculated using the tensor product blossom of patch P.
 *  See G. Farin: Curves and Surfaces for CAGD, p. 259 for example. */
void TensorProductPatch::subpatch (double u1, double u2, double v1, double v2, TensorProductPatch &patch) const {
	if (u1 > u2) swap(u1, u2);
	if (v1 > v2) swap(v1, v2);
	// compute control points
	double u[] = {u1, u1, u1, 0};    // blossom parameters of the "horizontal" domain (plus dummy value 0)
	for (int i=0; i < 4; i++) {
		u[3-i] = u2;
		double v[] = {v1, v1, v1, 0}; // blossom parameters of the "vertical" domain (plus dummy value 0)
		for (int j=0; j < 4; j++) {
			v[3-j] = v2;
			patch._points[i][j] = blossomValue(u, v);
		}
	}
	// assign color values
	patch._colors[0] = colorAt(u1, v1);
	patch._colors[1] = colorAt(u2, v1);
	patch._colors[2] = colorAt(u1, v2);
	patch._colors[3] = colorAt(u2, v2);
}


/** Computes the value b(u1,u2,u3;v1,v2,v3) where b is tensor product blossom of the patch. */
DPair TensorProductPatch::blossomValue (double u1, double u2, double u3, double v1, double v2, double v3) const {
	DPair p[4];
	for (int i=0; i < 4; i++) {
		Bezier bezier(_points[i][0], _points[i][1], _points[i][2], _points[i][3]);
		p[i] = bezier.blossomValue(u1, u2, u3);
	}
	Bezier bezier(p[0], p[1], p[2], p[3]);
	return bezier.blossomValue(v1, v2, v3);
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


/** Computes a single row of segments approximating the patch region between v1 and v1+inc. */
void TensorProductPatch::approximateRow (double v1, double inc, bool overlap, double delta, const vector<Bezier> &vbeziers, Callback &callback) const {
	double v2 = snap(v1+inc);
	double ov2 = (overlap && v2 < 1) ? snap(v2+inc) : v2;
	Bezier hbezier1, hbezier2;
	horizontalCurve(v1, hbezier1);
	horizontalCurve(ov2, hbezier2);
	double u1 = 0;
	for (size_t i=1; i < vbeziers.size(); i++) {
		double u2 = snap(u1+inc);
		double ou2 = (overlap && u2 < 1) ? snap(u2+inc) : u2;
		// compute segment boundaries
		Bezier b1(hbezier1, u1, ou2);
		Bezier b2(vbeziers[i + (overlap && i < vbeziers.size()-1 ? 1 : 0)], v1, ov2);
		Bezier b3(hbezier2, u1, ou2);
		Bezier b4(vbeziers[i-1], v1, ov2);
		GraphicsPath<double> path;
		path.moveto(b1.point(0));
		if (inc > delta) {
			path.cubicto(b1.point(1), b1.point(2), b1.point(3));
			path.cubicto(b2.point(1), b2.point(2), b2.point(3));
			path.cubicto(b3.point(2), b3.point(1), b3.point(0));
			path.cubicto(b4.point(2), b4.point(1), b4.point(0));
		}
		else {
			path.lineto(b1.point(3));
			path.lineto(b2.point(3));
			path.lineto(b3.point(0));
		}
		path.closepath();
		callback.patchSegment(path, averageColor(colorAt(u1, v1), colorAt(u2, v1), colorAt(u1, v2), colorAt(u2, v2)));
		u1 = u2;
	}
}


/** Approximate the patch by dividing it into a grid of segments that are filled with the
 *  average color of the corresponding region. The boundary of each segment consists of
 *  four Bézier curves, too. In order to prevent visual gaps between neighbored segments due
 *  to anti-aliasing, the flag 'overlap' can be set. It enlarges the segments so that they overlap
 *  with their right and bottom neighbors (which are drawn on top of the overlapping regions).
 *  @param[in] gridsize number of segments per row/column
 *  @param[in] overlap if true, enlarge each segment to overlap with its right and bottom neighbors
 *  @param[in] delta reduce level of detail if the segment size is smaller than the given value
 *  @param[in] callback object notified */
void TensorProductPatch::approximate (int gridsize, bool overlap, double delta, Callback &callback) const {
	if (_colors[0] == _colors[1] && _colors[1] == _colors[2] && _colors[2] == _colors[3]) {
		// simple case: monochromatic patch
		GraphicsPath<double> path = getBoundaryPath();
		callback.patchSegment(path, _colors[0]);
	}
	else {
		const double inc = 1.0/gridsize;
		// collect curves dividing the patch into several columns (curved vertical stripes)
		vector<Bezier> vbeziers(gridsize+1);
		double u=0;
		for (int i=0; i <= gridsize; i++) {
			verticalCurve(u, vbeziers[i]);
			u = snap(u+inc);
		}
		// compute the segments row by row
		double v=0;
		for (int i=0; i < gridsize; i++) {
			approximateRow(v, inc, overlap, delta, vbeziers, callback);
			v = snap(v+inc);
		}
	}
}


BoundingBox TensorProductPatch::getBBox () const {
	BoundingBox bbox;
	Bezier bezier;
	for (int i=0; i <= 1; i++) {
		horizontalCurve(i, bezier);
		bbox.embed(bezier.getBBox());
		verticalCurve(i, bezier);
		bbox.embed(bezier.getBBox());
	}
	return bbox;
}


#if 0
void TensorProductPatch::approximate (int gridsize, Callback &callback) const {
	const double inc = 1.0/gridsize;
	Bezier ubezier0; verticalCurve(0, ubezier0);
	Bezier ubezier1; verticalCurve(1, ubezier1);
	Bezier vbezier0; horizontalCurve(0, vbezier0);
	Bezier vbezier1; horizontalCurve(1, vbezier1);
	for (double v1=0; v1 < 1; v1=snap(v1+inc)) {
		double v2 = snap(v1+inc);
		DPair p0 = valueAt(0, v1);
		DPair p2 = valueAt(0, v2);
		Color c0 = colorAt(0, v1);
		Color c2 = colorAt(0, v2);
		double u1 = 0;
		for (double u2=inc; u2 <= 1; u2=snap(u2+inc)) {
			DPair p1 = valueAt(u2, v1);
			DPair p3 = valueAt(u2, v2);
			Color c1 = colorAt(u2, v1);
			Color c3 = colorAt(u2, v2);
			// Compute a single patch segment. Only those segment edges that lay on the
			// patch boundary are drawn as Bézier curves, all other edges are approximated
			// with straight lines. This ensures a smooth outline and reduces the number of
			// time consuming computations.
			GraphicsPath<double> path;
			path.moveto(p0);
			if (v1 > 0)
				path.lineto(p1);
			else {
				Bezier bezier(vbezier0, u1, u2);
				path.cubicto(bezier.point(1), bezier.point(2), bezier.point(3));
			}
			if (u2 < 1)
				path.lineto(p3);
			else {
				Bezier bezier(ubezier1, v1, v2);
				path.cubicto(bezier.point(1), bezier.point(2), bezier.point(3));
			}
			if (v2 < 1)
				path.lineto(p2);
			else {
				Bezier bezier(vbezier1, u1, u2);
				path.cubicto(bezier.point(2), bezier.point(1), bezier.point(0));
			}
			if (u1 > 0)
				path.closepath();
			else {
				Bezier bezier(ubezier0, v1, v2);
				path.cubicto(bezier.point(2), bezier.point(1), bezier.point(0));
				path.closepath();
			}
			callback.patchSegment(path, averageColor(c0, c1, c2, c3));
			p0 = p1;
			p2 = p3;
			c0 = c1;
			c2 = c3;
			u1 = u2;
		}
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////////////


CoonsPatch::CoonsPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, CoonsPatch *patch)
	: TensorProductPatch(cspace)
{
	setPoints(points, edgeflag, patch);
	setColors(colors, edgeflag, patch);
}


DPair CoonsPatch::valueAt (double u, double v) const {
	// Compute the value of P(u,v) using the Coons equation rather than the
	// tensor product since the "inner" control points of the tensor matrix
	// might not be set yet.
	Bezier bezier1(_points[3][0], _points[3][1], _points[3][2], _points[3][3]);
	Bezier bezier2(_points[0][0], _points[0][1], _points[0][2], _points[0][3]);
	Bezier bezier3(_points[3][0], _points[2][0], _points[1][0], _points[0][0]);
	Bezier bezier4(_points[3][3], _points[2][3], _points[1][3], _points[0][3]);
	DPair ph = bezier1.valueAt(u)*(1-v) + bezier2.valueAt(u)*v;
	DPair pv = bezier3.valueAt(v)*(1-u) + bezier4.valueAt(v)*u;
	DPair pc = (_points[3][0]*(1-u) + _points[3][3]*u)*(1-v) + (_points[0][0]*(1-u) + _points[0][3]*u)*v;
	return ph+pv-pc;
}


/** Sets the 12 control points defining the geometry of the coons patch. The points
 *  must be given in the following order:
 *  3  4  5  6
 *  2        7
 *  1        8
 *  0 11 10  9
 *  where each edge of the square represents the four control points of a cubic Bézier curve.
 *  If the edge flag is 1, 2, or 3, the points 0 to 3 are omitted, and taken from a reference
 *  patch instead.
 *  @param[in] points the control points in cyclic order as described in the PS reference, p. 281
 *  @param[in] edgeflag defines how to connect this patch to another one
 *  @param[in] patch reference patch required if edgeflag > 0 */
void CoonsPatch::setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) {
	auto coonsPatch = dynamic_cast<CoonsPatch*>(patch);
	if (edgeflag > 0 && !coonsPatch)
		throw ShadingException("missing preceding data in definition of relative Coons patch");
	if ((edgeflag == 0 && points.size() != 12) || (edgeflag > 0 && points.size() != 8))
		throw ShadingException("invalid number of control points in Coons patch definition");

	// Since a Coons patch is a special tensor product patch, we only have to reorder the
	// control points and compute the additional "inner" points of the 4x4 point tensor matrix.

	// set outer control points of the tensor matrix except those of the first column
	// because these points depend on the edge flag
	int i = (edgeflag == 0 ? 4 : 0);
	_points[3][1] = points[i++];
	_points[3][2] = points[i++];
	_points[3][3] = points[i++];
	_points[2][3] = points[i++];
	_points[1][3] = points[i++];
	_points[0][3] = points[i++];
	_points[0][2] = points[i++];
	_points[0][1] = points[i];

	// set control points of first matrix column
	switch (edgeflag) {
		case 0: setFirstMatrixColumn(&points[0], false); break;
		case 1: setFirstMatrixColumn(coonsPatch->_points[3], false); break;
		case 2: setFirstMatrixColumn(coonsPatch->_points, 3, true); break;
		case 3: setFirstMatrixColumn(coonsPatch->_points[0], true); break;
	}
	// compute inner control points of the tensor matrix
	_points[1][1] = valueAt(1.0/3.0, 2.0/3.0);
	_points[1][2] = valueAt(2.0/3.0, 2.0/3.0);
	_points[2][1] = valueAt(1.0/3.0, 1.0/3.0);
	_points[2][2] = valueAt(2.0/3.0, 1.0/3.0);
}


void CoonsPatch::setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch) {
	auto coonsPatch = dynamic_cast<CoonsPatch*>(patch);
	if (edgeflag > 0 && !coonsPatch)
		throw ShadingException("missing preceding data in definition of relative Coons patch");
	if ((edgeflag == 0 && colors.size() != 4) || (edgeflag > 0 && colors.size() != 2))
		throw ShadingException("invalid number of colors in Coons patch definition");

	int i = (edgeflag == 0 ? 2 : 0);
	_colors[3] = colors[i];
	_colors[1] = colors[i+1];
	switch (edgeflag) {
		case 0: _colors[0] = colors[0]; _colors[2] = colors[1]; break;
		case 1: _colors[0] = coonsPatch->_colors[2]; _colors[2] = coonsPatch->_colors[3]; break;
		case 2: _colors[0] = coonsPatch->_colors[3]; _colors[2] = coonsPatch->_colors[1]; break;
		case 3: _colors[0] = coonsPatch->_colors[1]; _colors[2] = coonsPatch->_colors[0]; break;
	}
}

