/*************************************************************************
** Bezier.hpp                                                           **
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

#ifndef BEZIER_HPP
#define BEZIER_HPP

#include <vector>
#include "BoundingBox.hpp"
#include "Pair.hpp"

class Matrix;

class QuadBezier {
	public:
		QuadBezier ();
		QuadBezier (const DPair &p0, const DPair &p1, const DPair &p2);
		void setPoints (const DPair &p0, const DPair &p1, const DPair &p2);
		const DPair& point (int i) const {return _points[i];}
		DPair valueAt (double t) const;
		DPair derivativeAt (double t) const;
		double arclen (double t=1.0) const;

	private:
		DPair _points[3];
};


class CubicBezier {
	public:
		CubicBezier ();
		//CubicBezier (const DPair &p0, const DPair &p1, const DPair &p2);
		CubicBezier (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3);
		CubicBezier (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
		CubicBezier (const CubicBezier &source, double t0, double t1);
		explicit CubicBezier (const QuadBezier &qbezier);
		void setPoints (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3);
		void reverse ();
		DPair valueAt (double t) const;
		DPair blossomValue (double u, double v, double w) const;
		void subdivide (double t, CubicBezier *bezier1, CubicBezier *bezier2) const;
		CubicBezier& transform (const Matrix &matrix);
		int approximate (double delta, std::vector<DPair> &p, std::vector<double> *t=nullptr) const;
		const DPair& point (int i) const {return _points[i];}
		int reduceDegree (double delta, std::vector<DPair> &p) const;
		std::vector<QuadBezier> toQuadBeziers (double delta, std::vector<double> *startParams=nullptr) const;
		BoundingBox getBBox () const;

	protected:
		int approximate (double delta, double t0, double t1, std::vector<DPair> &p, std::vector<double> *t) const;
		void toQuadBeziers (double t0, double t1, double precision, std::vector<QuadBezier> &qbeziers, std::vector<double> *startParams) const;
		QuadBezier midpointApproximation () const;

	private:
		DPair _points[4];
};

#endif
