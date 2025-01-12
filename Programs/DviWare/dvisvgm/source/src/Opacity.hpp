/*************************************************************************
** Opacity.hpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef OPACITY_HPP
#define OPACITY_HPP

#include <string>

class OpacityAlpha {
	public:
		OpacityAlpha () =default;
		OpacityAlpha (double constalpha, double shapealpha) : _constalpha(constalpha), _shapealpha(shapealpha) {}
		explicit OpacityAlpha (double alpha) : _constalpha(alpha) {}
		void setConstAlpha (double alpha) { _constalpha = alpha;}
		void setShapeAlpha (double shapealpha) {_shapealpha = shapealpha;}
		double value () const {return _constalpha * _shapealpha;}
		bool operator == (const OpacityAlpha &alpha) const {return alpha._constalpha == _constalpha && alpha._shapealpha == _shapealpha;}
		bool operator != (const OpacityAlpha &alpha) const {return alpha._constalpha != _constalpha || alpha._shapealpha != _shapealpha;}
		bool isOpaque () const {return _constalpha == 1.0 && _shapealpha == 1.0;}

	private:
		double _constalpha=1.0;
		double _shapealpha=1.0;
};

class Opacity {
	public:
		enum BlendMode {
			BM_NORMAL, BM_MULTIPLY, BM_SCREEN, BM_OVERLAY,
			BM_SOFTLIGHT, BM_HARDLIGHT, BM_COLORDODGE, BM_COLORBURN,
			BM_DARKEN, BM_LIGHTEN, BM_DIFFERENCE, BM_EXCLUSION,
			BM_HUE, BM_SATURATION, BM_COLOR, BM_LUMINOSITY
		};

	public:
		Opacity () =default;
		Opacity (OpacityAlpha fillalpha, OpacityAlpha strokealpha, BlendMode bm) : _fillalpha(fillalpha), _strokealpha(strokealpha), _blendMode(bm) {}
		Opacity (OpacityAlpha fillalpha, OpacityAlpha strokealpha) : Opacity(fillalpha, strokealpha, BM_NORMAL) {}
		explicit Opacity (BlendMode bm) : _blendMode(bm) {}
		OpacityAlpha& fillalpha () {return _fillalpha;}
		OpacityAlpha& strokealpha () {return _strokealpha;}
		const OpacityAlpha& fillalpha () const {return _fillalpha;}
		const OpacityAlpha& strokealpha () const {return _strokealpha;}
		BlendMode blendMode () const {return _blendMode;}
		void setBlendMode (BlendMode mode) {_blendMode = mode;}
		std::string cssBlendMode () const {return cssBlendMode(_blendMode);}
		static std::string cssBlendMode (BlendMode bm);
		bool isFillDefault () const {return _fillalpha.isOpaque() && _blendMode == BM_NORMAL;}
		bool isStrokeDefault () const {return _strokealpha.isOpaque() && _blendMode == BM_NORMAL;}
		bool operator == (const Opacity &opacity) const;
		bool operator != (const Opacity &opacity) const;

		static BlendMode blendMode (const std::string &name);

	private:
		OpacityAlpha _fillalpha;
		OpacityAlpha _strokealpha;
		BlendMode _blendMode=BM_NORMAL;
};

#endif
