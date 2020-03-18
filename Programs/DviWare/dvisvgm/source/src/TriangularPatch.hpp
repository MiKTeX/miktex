/*************************************************************************
** TriangularPatch.hpp                                                  **
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

#ifndef TRIANGULARPATCH_HPP
#define TRIANGULARPATCH_HPP

#include "Color.hpp"
#include "Pair.hpp"
#include "ShadingPatch.hpp"

class TriangularPatch : public ShadingPatch {
	public:
		TriangularPatch ();
		explicit TriangularPatch (Color::ColorSpace cspace) : ShadingPatch(cspace) {}
		TriangularPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, TriangularPatch *patch);
		int psShadingType() const override {return 4;}
		DPair valueAt (double u, double v) const;
		Color colorAt (double u, double v) const;
		Color averageColor() const override;
		void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) override;
		void setPoints (const DPair &p1, const DPair &p2, const DPair &p3);
		void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch) override;
		void setColors (const Color &c1, const Color &c2, const Color &c3);
		void approximate (int gridsize, bool overlap, double delta, Callback &listener) const override;
		BoundingBox getBBox () const override;
		GraphicsPath<double> getBoundaryPath () const override;
		int numPoints (int edgeflag) const override {return edgeflag == 0 ? 3 : 1;}
		int numColors (int edgeflag) const override {return edgeflag == 0 ? 3 : 1;}

	protected:
		Color averageColor (const Color &c1, const Color &c2, const Color &c3) const;

	private:
		DPair _points[3];
		Color _colors[3];
};


class LatticeTriangularPatch : public TriangularPatch {
	public:
		explicit LatticeTriangularPatch (Color::ColorSpace cspace) : TriangularPatch(cspace) {}
		int psShadingType() const override {return 5;}
};

#endif
