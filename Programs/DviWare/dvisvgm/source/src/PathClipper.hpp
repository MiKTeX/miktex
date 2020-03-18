/*************************************************************************
** PathClipper.hpp                                                      **
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

#ifndef PATHCLIPPER_HPP
#define PATHCLIPPER_HPP

#include <clipper.hpp>
#include <string>
#include <vector>
#include "Bezier.hpp"
#include "GraphicsPath.hpp"
#include "MessageException.hpp"


using ClipperLib::IntPoint;

class PathClipper {
	public:
		using CurvedPath = GraphicsPath<double>;

	public:
		void intersect (const CurvedPath &p1, const CurvedPath &p2, CurvedPath &result);

	protected:
		void flatten (const CurvedPath &gp, ClipperLib::Paths &polygons);
		void reconstruct (const ClipperLib::Path &polygon, CurvedPath &path);
		void reconstruct (const ClipperLib::Paths &polygons, CurvedPath &path);
		static void callback (IntPoint &e1bot, IntPoint &e1top, IntPoint &e2bot, IntPoint &e2top, IntPoint &ip);

	private:
		std::vector<Bezier> _curves;
		int _numLines=0;  ///< negative number of straight line segments in path been processed
};

#endif
