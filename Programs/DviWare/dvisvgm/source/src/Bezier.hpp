/*************************************************************************
** Bezier.hpp                                                           **
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

#ifndef BEZIER_HPP
#define BEZIER_HPP

#include <vector>
#include "BoundingBox.hpp"
#include "Pair.hpp"

class Matrix;

class Bezier {
	public:
		Bezier ();
		Bezier (const DPair &p0, const DPair &p1, const DPair &p2);
		Bezier (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3);
		Bezier (const Bezier &source, double t0, double t1);
		void setPoints (const DPair &p0, const DPair &p1, const DPair &p2, const DPair &p3);
		void reverse ();
		DPair valueAt (double t) const;
		DPair blossomValue (double u, double v, double w) const;
		void subdivide (double t, Bezier *bezier1, Bezier *bezier2) const;
		Bezier& transform (const Matrix &matrix);
		int approximate (double delta, std::vector<DPair> &p, std::vector<double> *t=nullptr) const;
		const DPair& point (int i) const {return _points[i];}
		int reduceDegree (double delta, std::vector<DPair> &p) const;
		BoundingBox getBBox () const;

	protected:
		int approximate (double delta, double t0, double t1, std::vector<DPair> &p, std::vector<double> *t) const;

	private:
		DPair _points[4];
};

#endif
