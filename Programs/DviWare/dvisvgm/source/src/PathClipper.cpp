/*************************************************************************
** PathClipper.cpp                                                      **
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
#include "Bezier.hpp"
#include "PathClipper.hpp"

using namespace std;
using namespace ClipperLib;

using Polygon = ClipperLib::Path;
using Polygons = ClipperLib::Paths;
using CurvedPath = PathClipper::CurvedPath;

const int SCALE_FACTOR = 1000;

inline cInt to_cInt (double x) {
	return static_cast<cInt>(lround(x*SCALE_FACTOR));
}


inline double to_double (cInt x) {
	return static_cast<double>(x)/SCALE_FACTOR;
}


inline DPair to_DPair (const IntPoint &p) {
	return DPair(to_double(p.X), to_double(p.Y));
}


/** In order to flatten a curved path, all path segements are processed sequentially.
 *  Depending on the type of the segment, one of the methods provided by this class
 *  is called. */
class FlattenActions : public CurvedPath::IterationActions {
	public:
		FlattenActions (vector<Bezier> &curves, Polygons &polygons, int &numLines)
			: _polygons(polygons), _curves(curves), _numLines(numLines) {}

		void moveto (const CurvedPath::Point &p) override {
			if (p == _currentPoint && !_currentPoly.empty())
				return;
			closepath();
			_currentPoly.emplace_back(IntPoint(to_cInt(p.x()), to_cInt(p.y()), 0));
			_currentPoint = _startPoint = p;
		}

		void lineto (const CurvedPath::Point &p) override {
			if (p == _currentPoint && !_currentPoly.empty())
				return;
			if (_currentPoly.empty()) // this shouldn't happen but in case it does...
				_currentPoly.emplace_back(IntPoint(0, 0, 0)); // ...add a start point first
			_numLines--;
			_currentPoly.back().Z.label2 = _numLines;
			_currentPoly.emplace_back(IntPoint(to_cInt(p.x()), to_cInt(p.y()), ZType(_numLines, 0)));
			_currentPoint = p;
		}

		void quadto (const CurvedPath::Point &p1, const CurvedPath::Point &p2) override {
			Bezier bezier(_currentPoint, p1, p2);
			addCurvePoints(bezier);
		}

		void cubicto (const CurvedPath::Point &p1, const CurvedPath::Point &p2, const CurvedPath::Point &p3) override {
			Bezier bezier(_currentPoint, p1, p2, p3);
			addCurvePoints(bezier);
		}

		void closepath () override {
			if (_currentPoly.empty())
				return;
			_numLines--;
			_currentPoly.back().Z.label2 = ZLabel(_numLines, 0);
			_currentPoly.front().Z.label1 = ZLabel(_numLines, 0);
			_polygons.push_back(_currentPoly);
			_currentPoly.clear();
		}

		void finished () override {
			closepath();
		}

	protected:
		void addCurvePoints (const Bezier &bezier) {
			if (_currentPoly.empty()) // this shouldn't happen but in case it does, ...
				_currentPoly.emplace_back(IntPoint(0, 0, 0)); // ...add a start point first
			vector<DPair> points;  // points of flattened curve
			vector<double> t;      // corresponding 'time' parameters
			bezier.approximate(0.01, points, &t);
			if (points.size() < 2)
				return;
			_curves.push_back(bezier);
			for (size_t i=1; i < points.size(); i++) {
				const DPair &p = points[i];
				if (p == _currentPoint)
					continue;
				_currentPoly.back().Z.label2 = ZLabel(_curves.size(), t[i-1]);
				ZLabel label(_curves.size(), t[i]);
				_currentPoly.emplace_back(IntPoint(to_cInt(p.x()), to_cInt(p.y()), ZType(label, label)));
				_currentPoint = p;
			}
		}

	private:
		CurvedPath::Point _startPoint, _currentPoint;
		Polygon _currentPoly;    ///< polygon being created
		Polygons &_polygons;     ///< all polygons created
		vector<Bezier> &_curves;
		int &_numLines;
};


/** Removes adjacent polygon vertices that equal their predecessor. */
static void remove_redundant_vertices (Polygon &polygon) {
	auto it1=polygon.begin();
	while (it1 != polygon.end()) {
		auto it2 = it1+1;
		if (it2 == polygon.end())
			it2 = polygon.begin();
		if (it1 == it2)
			return;

		if (*it1 != *it2)
			++it1;
		else {
			it1->Z.label2 = it2->Z.label2;
			polygon.erase(it2);
		}
	}
}


/** Approximates a curved path by a set of polygons and stores information
 *  to reconstruct the curved segments later. The z component of each
 *  polygon vertex holds two integers representing information about the two
 *  adjacent edges the vertex belongs to. This is required to identify the
 *  affected edges and thus the former (curve/line) segment of the path during
 *  the intersection process.
 *  @param[in] curvedPath curved path to be flattened
 *  @param[out] polygons the flattened path (set of polygons) */
void PathClipper::flatten (const CurvedPath &curvedPath, Polygons &polygons) {
	FlattenActions flattenActions(_curves, polygons, _numLines);
	curvedPath.iterate(flattenActions, false);
	for (Polygon &poly : polygons)
		remove_redundant_vertices(poly);
}


/** Returns the ID of the path segment the polygon edge defined by its start
 *  and end point belongs to. The z component of a polygon vertex holds a pair
 *  of labels that allows to identify the original path segments the point belongs to.
 *  Since always two adjacent segments share a point, each point gets two values assigned.
 *  Negative numbers denote line segments, positive ones Bézier curves.
 *  There are only these two segment types, so we don't need further flags in
 *  order to distinguish them. By comparing the labels of two adjacent polygon
 *  vertexes it's possible to identify the original path segment the corresponding
 *  edge belongs to.
 *  @param[in] p1 first of two adjacent vertices
 *  @param[in] p2 second of two adjacent vertices
 *  @param[out] t1 time parameter of p1
 *  @param[out] t2 time parameter of p2
 *  @return id of edge between p1 and p2, or 0 if it's not possible to identify the segment */
static int32_t segment_id (const IntPoint &p1, const IntPoint &p2, double &t1, double &t2) {
	const ZType &z1=p1.Z, &z2=p2.Z;
	if (z1 == z2 && z1.minLabel().id < 0) return z1.minLabel().id;
	if (z1.label1 == z2.label2) {t1=z1.label1.t; t2=z2.label2.t; return z1.label1.id;}
	if (z1.label2 == z2.label1) {t1=z1.label2.t; t2=z2.label1.t; return z1.label2.id;}
	if (z1.label1 == z2.label1) {t1=z1.label1.t; t2=z2.label1.t; return z1.label1.id;}
	if (z1.label2 == z2.label2) {t1=z1.label2.t; t2=z2.label2.t; return z1.label2.id;}
	// if we get here, it's not possible to identify the segment
	// => the edge is going to be handled as line segment
	return 0;
}


/** This function expects 3 colinear points p1, p2, and q, where q lies between p1 and p2,
 *  i.e. q divides the line \f$ \overline{p_1 p_2} \f$ somewhere. The function returns
 *  the corresponding division ratio. */
static double division_ratio (const IntPoint &p1, const IntPoint &p2, const IntPoint &q) {
	if (p1 == p2 || q == p1)
		return 0;
	if (q == p2)
		return 1;
	if (p1.X == p2.X)
		return double(q.Y-p1.Y)/(p2.Y-p1.Y);
	return double(q.X-p1.X)/(p2.X-p1.X);
}


/** Returns the label of point q that lies on the line between points p1 and p2. */
inline ZLabel division_label (const IntPoint &p1, const IntPoint &p2, const IntPoint &q) {
	double t1=0, t2=0;
	double s=0;
	int32_t id = segment_id(p1, p2, t1, t2);
	if (id > 0)
		s = t1+(t2-t1)*division_ratio(p1, p2, q);
	return ZLabel(id, s);
}


/** This method is called if the clipper library finds an intersection between two polygon edges.
 *  It populates the z coordinate of the intersection point with the idexes of the two edges.
 *  @param[in] e1bot first endpoint of edge 1
 *  @param[in] e1top second endpoint of edge 1
 *  @param[in] e2bot first endpoint of edge 2
 *  @param[in] e2top second endpoint of edge 2
 *  @param[in] ip intersection point of edge 1 and 2 */
void PathClipper::callback (IntPoint &e1bot, IntPoint &e1top, IntPoint &e2bot, IntPoint &e2top, IntPoint &ip) {
	ZLabel label1 = division_label(e1bot, e1top, ip);
	ZLabel label2 = division_label(e2bot, e2top, ip);
	ip.Z = ZType(label1, label2);
}


/** Iterates along the polygon edges until the endpoint of the current
 *  path segment is found and returns its vector index afterwards.
 *  @param[in] polygon the polygon to be processed
 *  @param[in] start index of the vertex where the iteration starts
 *  @param[out] label if not 0, retrieves the label of the endpoint
 *  @param[in] startLabel if true, the found endpoint is treated as start point and
 *             parameter 'label' gets the corresponding value */
static size_t find_segment_endpoint (const Polygon &polygon, size_t start, ZLabel *label=nullptr, bool startLabel=false) {
	if (polygon.empty())
		return 0;

	const size_t num_points = polygon.size();
	int i = start%num_points;
	double t1=0, t2=0; // time parameters of start and endpoint of current edge
	int32_t id1 = segment_id(polygon[i], polygon[(i+1)%num_points], t1, t2);
	int32_t id2 = id1;
	double t = t2; // time parameter of resulting endpoint
	for (size_t j=1; id1 == id2 && j < num_points; j++) {
		t = t2;
		i = (i+1)%num_points;
		if (id1 == 0)
			break;
		id2 = segment_id(polygon[i], polygon[(i+1)%num_points], t1, t2);
	}
	if (label) {
		*label = ZLabel(id1, id1 < 0 ? 0 : t);
		if (startLabel && id1 != 0)
			*label = polygon[i].Z.otherLabel(*label);
	}
	return i;
}


/** Reconstructs a curved path from the set of polygons.
 *  @param[in] polygons set of polygons to reconstruct
 *  @param[out] path the reconstructed curved path */
void PathClipper::reconstruct (const Polygons &polygons, CurvedPath &path) {
	for (const Polygon &polygon : polygons)
		reconstruct(polygon, path);
}


/** Reconstructs a curved path from a single polygon.
 *  @param[in] polygon polygon to reconstruct
 *  @param[out] path the reconstructed curved path */
void PathClipper::reconstruct (const Polygon &polygon, CurvedPath &path) {
	size_t num_points = polygon.size();
	if (num_points < 2)
		return;

	ZLabel label1, label2;  // labels of the current segment's start and endpoint
	int index1 = find_segment_endpoint(polygon, 0, &label1, true);
	int index2 = find_segment_endpoint(polygon, index1, &label2);
	int diff = (num_points+index2-index1)%num_points;
	path.moveto(to_DPair(polygon[index1]));
	for (size_t count = diff; count <= num_points; count += diff) {
		if (diff == 1 || label1.id <= 0)  // line segment?
			path.lineto(to_DPair(polygon[index2]));
		else {  // Bézier curve segment
			Bezier bezier(_curves[label1.id-1], label1.t, label2.t);
			if (label1.t > label2.t)
				bezier.reverse();
			path.cubicto(bezier.point(1), bezier.point(2), bezier.point(3));
		}
		if (label1.id == 0)
			find_segment_endpoint(polygon, index2, &label1, true);
		else
			label1 = polygon[index2].Z.otherLabel(label2);
		index1 = index2;
		index2 = find_segment_endpoint(polygon, index1, &label2);
		diff = (num_points+index2-index1)%num_points;
	}
	path.closepath();
}


inline PolyFillType polyFillType (CurvedPath::WindingRule wr) {
	return (wr == CurvedPath::WindingRule::NON_ZERO) ? pftNonZero : pftEvenOdd;
}


/** Computes the intersection of to curved paths.
 *  @param[in] p1 first curved path
 *  @param[in] p2 second curved path
 *  @param[out] result intersection of p1 and p2 */
void PathClipper::intersect (const CurvedPath &p1, const CurvedPath &p2, CurvedPath &result) {
	if (p1.size() < 2 || p2.size() < 2)
		return;
	Clipper clipper;
	Polygons polygons;
	flatten(p1, polygons);
	clipper.AddPaths(polygons, ptSubject, true);
	polygons.clear();
	flatten(p2, polygons);
	clipper.AddPaths(polygons, ptClip, true);
	clipper.ZFillFunction(callback);
	Polygons flattenedPath;
	clipper.Execute(ctIntersection, flattenedPath, polyFillType(p1.windingRule()), polyFillType(p2.windingRule()));
	reconstruct(flattenedPath, result);
}
