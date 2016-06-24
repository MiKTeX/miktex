/*************************************************************************
** TriangularPatch.h                                                    **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef DVISVGM_TRIANGULARPATCH_H
#define DVISVGM_TRIANGULARPATCH_H

#include "Color.h"
#include "Pair.h"
#include "ShadingPatch.h"

class TriangularPatch : public ShadingPatch
{
   public:
      TriangularPatch ();
		TriangularPatch (Color::ColorSpace cspace) : ShadingPatch(cspace) {}
		TriangularPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, TriangularPatch *patch);
		int psShadingType() const {return 4;}
		DPair valueAt (double u, double v) const;
		Color colorAt (double u, double v) const;
		Color averageColor() const;
		void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch);
		void setPoints (const DPair &p1, const DPair &p2, const DPair &p3);
		void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch);
		void setColors (const Color &c1, const Color &c2, const Color &c3);
		void approximate (int gridsize, bool overlap, double delta, Callback &listener) const;
		void getBBox (BoundingBox &bbox) const;
		void getBoundaryPath(GraphicsPath<double> &path) const;
		int numPoints (int edgeflag) const {return edgeflag == 0 ? 3 : 1;}
		int numColors (int edgeflag) const {return edgeflag == 0 ? 3 : 1;}

	protected:
		Color averageColor (const Color &c1, const Color &c2, const Color &c3) const;

   private:
		DPair _points[3];
		Color _colors[3];
};


class LatticeTriangularPatch : public TriangularPatch
{
	public:
		LatticeTriangularPatch (Color::ColorSpace cspace) : TriangularPatch(cspace) {}
		int psShadingType() const {return 5;}
};

#endif
