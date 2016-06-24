/*************************************************************************
** DVIToSVG.h                                                           **
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

#ifndef DVISVGM_DVITOSVG_H
#define DVISVGM_DVITOSVG_H

#include <string>
#include <utility>
#include "DVIReader.h"
#include "SpecialManager.h"
#include "SVGTree.h"

class Matrix;
struct SVGOutputBase;

class DVIToSVG : public DVIReader
{
	public:
		DVIToSVG (std::istream &is, SVGOutputBase &out);
		~DVIToSVG ();
		void convert (const std::string &range, std::pair<int,int> *pageinfo=0);
		void setPageSize (const std::string &format)         {_bboxFormatString = format;}
		void setPageTransformation (const std::string &cmds) {_transCmds = cmds;}
		void getPageTransformation (Matrix &matrix) const;
		std::string getSVGFilename (unsigned pageno) const;
		std::string getUserBBoxString () const               {return _bboxFormatString;}
		static void setProcessSpecials (const char *ignorelist=0, bool pswarning=false);

	public:
		static char TRACE_MODE;

	protected:
		DVIToSVG (const DVIToSVG &);
		void convert (unsigned firstPage, unsigned lastPage, std::pair<int,int> *pageinfo=0);
		void beginPage (unsigned pageno, Int32 *c);
		void endPage (unsigned pageno);
		void embedFonts (XMLElementNode *svgElement);

	private:
		SVGTree _svg;
		SVGOutputBase &_out;
		std::string _bboxFormatString;  ///< bounding box size/format set by the user
		std::string _transCmds;         ///< page transformation commands set by the user
};

#endif
