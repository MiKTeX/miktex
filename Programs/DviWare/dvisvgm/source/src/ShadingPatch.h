/*************************************************************************
** ShadingPatch.h                                                       **
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

#ifndef DVISVGM_SHADINGPATCH_H
#define DVISVGM_SHADINGPATCH_H

#include "Color.h"
#include "GraphicsPath.h"
#include "MessageException.h"


class ShadingPatch
{
	public:
		struct Callback {
			virtual ~Callback () {}
			virtual void patchSegment (GraphicsPath<double> &path, const Color &color) =0;
		};

		typedef std::vector<DPair> PointVec;
		typedef std::vector<Color> ColorVec;

	public:
		ShadingPatch (Color::ColorSpace colorSpace) : _colorspace(colorSpace) {}
		virtual ~ShadingPatch () {}
		virtual int psShadingType () const =0;
		virtual void approximate (int gridsize, bool overlap, double delta, Callback &callback) const =0;
		virtual void getBBox (BoundingBox &bbox) const =0;
		virtual void getBoundaryPath (GraphicsPath<double> &path) const =0;
		virtual void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) =0;
		virtual void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch) =0;
		virtual int numPoints (int edgeflag) const =0;
		virtual int numColors (int edgeflag) const =0;
		virtual Color averageColor() const =0;
		Color::ColorSpace colorSpace () const {return _colorspace;}
		static ShadingPatch* create (int psShadingType, Color::ColorSpace cspace);

	protected:
		typedef void (Color::*ColorGetter)(std::valarray<double> &va) const;
		typedef void (Color::*ColorSetter)(const std::valarray<double> &va);
		void colorQueryFuncs (ColorGetter &getter, ColorSetter &setter) const;

	private:
		Color::ColorSpace _colorspace;  ///< color space used to compute the shading values
};


struct ShadingException : public MessageException
{
	ShadingException (const std::string &msg) : MessageException(msg) {}
};

#endif

