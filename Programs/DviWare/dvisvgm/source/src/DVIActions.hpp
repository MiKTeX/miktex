/*************************************************************************
** DVIActions.hpp                                                       **
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

#ifndef DVIACTIONS_HPP
#define DVIACTIONS_HPP

#include <string>
#include <vector>

class BoundingBox;
class Font;

struct DVIActions {
	virtual ~DVIActions () =default;
	virtual void reset () {}
	virtual void setChar (double x, double y, unsigned c, bool vertical, const Font &f) {}
	virtual void setRule (double x, double y, double height, double width) {}
	virtual void setTextOrientation (bool vertical) {}
	virtual void moveToX (double x, bool forceSVGMove) {}
	virtual void moveToY (double y, bool forceSVGMove) {}
	virtual void setFont (int num, const Font &font) {}
	virtual void special (const std::string &s, double dvi2bp, bool preprocessing=false) {}
	virtual void beginPage (unsigned pageno, const std::vector<int32_t> &c) {}
	virtual void endPage (unsigned pageno) {}
	virtual BoundingBox& bbox () =0;
	virtual void progress (size_t current, size_t total, const char *id=nullptr) {}
};

#endif
