/*************************************************************************
** TensorProductPatch.h                                                 **
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

#ifndef TENSORPRODUCTPATCH_H
#define TENSORPRODUCTPATCH_H

#include <map>
#include <ostream>
#include <vector>
#include "Bezier.h"
#include "Color.h"
#include "MessageException.h"
#include "Pair.h"
#include "ShadingPatch.h"


/** This class represents a single tensor product patch P which is defined by 16 control points
 *  and 4 color values. The mapping of the unit square to the patch surface is defined as the sum
 *  \f[P(u,v):=\sum_{i=0}^3\sum_{j=0}^3 p_{ij} B_i(u) B_j(v)\f]
 *  where \f$B_k(t)={3\choose k}(1-t)^k t^k\f$ and \f$u,v \in [0,1]\f$. The four colors assigned
 *  to the vertices are interpolated bilinearily over the unit square. */
class TensorProductPatch : public ShadingPatch
{
	friend class CoonsPatch;

	public:
		TensorProductPatch () : ShadingPatch(Color::RGB_SPACE) {}
		TensorProductPatch (Color::ColorSpace cspace) : ShadingPatch(cspace) {}
		TensorProductPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, TensorProductPatch *patch);
		int psShadingType() const {return 7;}
		void setPoints (const DPair points[4][4], int edgeflag, TensorProductPatch *patch);
		void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch);
		void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch);
		virtual DPair valueAt (double u, double v) const;
		Color colorAt (double u, double v) const;
		Color averageColor () const;
		void horizontalCurve (double v, Bezier &bezier) const;
		void verticalCurve (double u, Bezier &bezier) const;
		void getBoundaryPath (GraphicsPath<double> &path) const;
		void subpatch (double u1, double u2, double v1, double v2, TensorProductPatch &patch) const;
		DPair blossomValue (double u1, double u2, double u3, double v1, double v2, double v3) const;
		DPair blossomValue (double u[3], double v[3]) const {return blossomValue(u[0], u[1], u[2], v[0], v[1], v[2]);}
		void approximate (int gridsize, bool overlap, double delta, Callback &callback) const;
		void getBBox (BoundingBox &bbox) const;
		int numPoints (int edgeflag) const {return edgeflag == 0 ? 16 : 12;}
		int numColors (int edgeflag) const {return edgeflag == 0 ? 4 : 2;}

	protected:
		Color averageColor (const Color &c1, const Color &c2, const Color &c3, const Color &c4) const;
		void approximateRow (double v1, double inc, bool overlap, double delta, const std::vector<Bezier> &beziers, Callback &callback) const;
		void setFirstMatrixColumn (const DPair source[4], bool reverse);
		void setFirstMatrixColumn (DPair source[4][4], int col, bool reverse);

	private:
		DPair _points[4][4];  ///< control point matrix defining the patch surface
		Color _colors[4];     ///< vertex colors cK (c0->p00, c1->p03, c2->p30, c3->p33)
};


/** Coons patches are special tensor product patches where the four "inner" control points
 *  depend on the outer ones, i.e. they are computed automatically and can't be set by the user.
 *  Thus, a Coons patch is defined by 12 control points, 4 vertex colors and a corresponding
 *  color space. */
class CoonsPatch : public TensorProductPatch
{
	public:
		CoonsPatch () {}
		CoonsPatch (Color::ColorSpace cspace) : TensorProductPatch(cspace) {}
		CoonsPatch (const PointVec &points, const ColorVec &colors, Color::ColorSpace cspace, int edgeflag, CoonsPatch *patch);
		int psShadingType() const {return 6;}
		virtual void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch);
		virtual void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch);
		virtual DPair valueAt (double u, double v) const;
		int numPoints (int edgeflag) const {return edgeflag == 0 ? 12 : 8;}
		int numColors (int edgeflag) const {return edgeflag == 0 ? 4 : 2;}
};

#endif
