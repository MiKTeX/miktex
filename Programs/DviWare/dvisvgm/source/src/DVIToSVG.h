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
#include "SVGTree.h"

struct DVIActions;
class Matrix;
struct SVGOutputBase;

class DVIToSVG : public DVIReader
{
	public:
		explicit DVIToSVG (std::istream &is, SVGOutputBase &out);
		~DVIToSVG ();
		void convert (const std::string &range, std::pair<int,int> *pageinfo=0);
		void setPageSize (const std::string &format)         {_bboxFormatString = format;}
		void setPageTransformation (const std::string &cmds) {_transCmds = cmds;}
		void getPageTransformation (Matrix &matrix) const;
		void translateToX (double x) {_tx = x-currState().h-_tx;}
		void translateToY (double y) {_ty = y-currState().v-_ty;}
		double getXPos() const       {return currState().h+_tx;}
		double getYPos() const       {return currState().v+_ty;}
		void finishLine ()           {_prevYPos = std::numeric_limits<double>::min();}

		std::string getSVGFilename (unsigned pageno) const;
		std::string getUserBBoxString () const  {return _bboxFormatString;}
		static void setProcessSpecials (const char *ignorelist=0, bool pswarning=false);

	public:
		static bool COMPUTE_PROGRESS;  ///< if true, an action to handle the progress ratio of a page is triggered
		static char TRACE_MODE;

	protected:
		DVIToSVG (const DVIToSVG&);
		void convert (unsigned firstPage, unsigned lastPage, std::pair<int,int> *pageinfo=0);
		int executeCommand ();
		void enterBeginPage (unsigned pageno, const std::vector<Int32> &c);
		void leaveEndPage (unsigned pageno);
		void embedFonts (XMLElementNode *svgElement);
		void moveRight (double dx);
		void moveDown (double dy);

		void dviPost (UInt16 stdepth, UInt16 pages, UInt32 pw, UInt32 ph, UInt32 mag, UInt32 num, UInt32 den, UInt32 lbopofs);
		void dviBop (const std::vector<Int32> &c, Int32 prevBopOffset);
		void dviEop ();
		void dviSetChar0 (UInt32 c, const Font *font);
		void dviSetChar (UInt32 c, const Font *font);
		void dviPutChar (UInt32 c, const Font *font);
		void dviSetRule (double h, double w);
		void dviPutRule (double h, double w);
		void dviPop ();
		void dviFontNum (UInt32 fontnum, SetFontMode mode, const Font *font);
		void dviDir (WritingMode dir);
		void dviXXX (const std::string &str);
		void dviXGlyphArray (std::vector<double> &dx, std::vector<double> &dy, std::vector<UInt16> &glyphs, const Font &font);
		void dviXGlyphString (std::vector<double> &dx, std::vector<UInt16> &glyphs, const Font &font);
		void dviXTextAndGlyphs (std::vector<double> &dx, std::vector<double> &dy, std::vector<UInt16> &chars, std::vector<UInt16> &glyphs, const Font &font);

	private:
		SVGTree _svg;
		SVGOutputBase &_out;
		DVIActions *_actions;
		std::string _bboxFormatString;  ///< bounding box size/format set by the user
		std::string _transCmds;         ///< page transformation commands set by the user
		double _pageHeight, _pageWidth; ///< global page height and width stored in the postamble
		double _tx, _ty;                ///< translation of cursor position
		double _prevYPos;               ///< previous vertical cursor position
		std::streampos _pageByte;       ///< position of the stream pointer relative to the preceding bop (in bytes)
};

#endif
