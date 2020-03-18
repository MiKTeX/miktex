/*************************************************************************
** ShadingPatch.hpp                                                     **
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

#ifndef SHADINGPATCH_HPP
#define SHADINGPATCH_HPP

#include <memory>
#include "Color.hpp"
#include "GraphicsPath.hpp"
#include "MessageException.hpp"


class ShadingPatch {
	public:
		struct Callback {
			virtual ~Callback () =default;
			virtual void patchSegment (GraphicsPath<double> &path, const Color &color) =0;
		};

		using PointVec = std::vector<DPair>;
		using ColorVec = std::vector<Color>;

	public:
		explicit ShadingPatch (Color::ColorSpace colorSpace) : _colorspace(colorSpace) {}
		virtual ~ShadingPatch () =default;
		virtual int psShadingType () const =0;
		virtual void approximate (int gridsize, bool overlap, double delta, Callback &callback) const =0;
		virtual BoundingBox getBBox () const =0;
		virtual GraphicsPath<double> getBoundaryPath () const =0;
		virtual void setPoints (const PointVec &points, int edgeflag, ShadingPatch *patch) =0;
		virtual void setColors (const ColorVec &colors, int edgeflag, ShadingPatch *patch) =0;
		virtual int numPoints (int edgeflag) const =0;
		virtual int numColors (int edgeflag) const =0;
		virtual Color averageColor() const =0;
		Color::ColorSpace colorSpace () const {return _colorspace;}
		static std::unique_ptr<ShadingPatch> create (int psShadingType, Color::ColorSpace cspace);

	protected:
		using ColorGetter = void (Color::*)(std::valarray<double> &va) const;
		using ColorSetter = void (Color::*)(const std::valarray<double> &va);
		void colorQueryFuncs (ColorGetter &getter, ColorSetter &setter) const;

	private:
		Color::ColorSpace _colorspace;  ///< color space used to compute the shading values
};


struct ShadingException : public MessageException {
	explicit ShadingException (const std::string &msg) : MessageException(msg) {}
};

#endif

