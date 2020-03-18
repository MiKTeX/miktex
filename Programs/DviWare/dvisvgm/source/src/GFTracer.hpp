/*************************************************************************
** GFTracer.hpp                                                         **
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

#ifndef GFTRACER_HPP
#define GFTRACER_HPP

#include <istream>
#include "GFReader.hpp"

class GFTracer : public GFReader {
	public:
		explicit GFTracer (std::istream &is) : GFReader(is) {}
		GFTracer (std::istream &is, double upp);
		virtual void moveTo (double x, double y) =0;
		virtual void lineTo (double x, double y) =0;
		virtual void curveTo (double c1x, double c1y, double c2x, double c2y, double x, double y) =0;
		virtual void closePath () =0;
		void beginChar (uint32_t c) override;
		void endChar (uint32_t c) override;

	protected:
		void unitsPerPoint(double upp) {_unitsPerPoint = upp;}

	private:
		double _unitsPerPoint=0; ///< target units per PS point
};

#endif
