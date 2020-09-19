/*************************************************************************
** EPSToSVG.hpp                                                         **
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

#ifndef EPSTOSVG_HPP
#define EPSTOSVG_HPP

#include "EPSFile.hpp"
#include "ImageToSVG.hpp"

class PsSpecialHandler;

class EPSToSVG : public ImageToSVG {
	public:
		EPSToSVG (const std::string &fname, SVGOutputBase &out) : ImageToSVG(fname, out) {}
		bool isSinglePageFormat() const override {return true;}
		int totalPageCount() const override {return 1;}

	protected:
		std::string imageFormat () const override {return "EPS";}
		bool imageIsValid () const override {return EPSFile(filename()).hasValidHeader();}
		BoundingBox imageBBox () const override {return EPSFile(filename()).bbox();}
		std::string psSpecialCmd () const override {return "psfile=";}
};

#endif

