/*************************************************************************
** DVIActions.h                                                         **
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

#ifndef DVISVGM_DVIACTIONS_H
#define DVISVGM_DVIACTIONS_H

#include <string>
#include "Message.h"
#include "types.h"

class  BoundingBox;
struct Font;
class  SpecialManager;


struct DVIActions
{
	virtual ~DVIActions () {}
	virtual void setChar (double x, double y, unsigned c, bool vertical, const Font *f) {}
	virtual void setRule (double x, double y, double height, double width) {}
	virtual void setTextOrientation (bool vertical) {}
	virtual void moveToX (double x) {}
	virtual void moveToY (double y) {}
	virtual void defineFont (int num, const Font *font) {}
	virtual void setFont (int num, const Font *font) {}
	virtual void special (const std::string &s, double dvi2bp, bool preprocessing=false) {}
	virtual void preamble (const std::string &cmt) {}
	virtual void postamble () {}
	virtual void beginPage (unsigned pageno, Int32 *c) {}
	virtual void endPage (unsigned pageno) {}
	virtual BoundingBox& bbox () =0;
	virtual void progress (size_t current, size_t total, const char *id=0) {}
};

#endif
