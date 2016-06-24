/*************************************************************************
** DVIToSVGActions.h                                                    **
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

#ifndef DVISVGM_DVITOSVGACTIONS_H
#define DVISVGM_DVITOSVGACTIONS_H

#include <map>
#include <set>
#include "BoundingBox.h"
#include "DVIActions.h"
#include "Matrix.h"
#include "SpecialActions.h"
#include "SpecialManager.h"
#include "SVGTree.h"


class  DVIToSVG;
struct FileFinder;
struct Font;
struct XMLNode;

class DVIToSVGActions : public DVIActions, public SpecialActions
{
	typedef std::map<const Font*, std::set<int> > CharMap;
	typedef std::set<const Font*> FontSet;
	typedef std::map<std::string,BoundingBox> BoxMap;

	public:
		DVIToSVGActions (DVIToSVG &dvisvg, SVGTree &svg);
		~DVIToSVGActions ();
		void reset ();
		void setChar (double x, double y, unsigned c, bool vertical, const Font *f);
		void setRule (double x, double y, double height, double width);
		void setBgColor (const Color &color);
		void setColor (const Color &color)             {_svg.setColor(color);}
		void setMatrix (const Matrix &m)               {_svg.setMatrix(m);}
		const Matrix& getMatrix () const               {return _svg.getMatrix();}
		void getPageTransform (Matrix &matrix) const   {_dvireader->getPageTransformation(matrix);}
		Color getColor () const                        {return _svg.getColor();}
		int getDVIStackDepth() const                   {return _dvireader->getStackDepth();}
		unsigned getCurrentPageNumber() const          {return _dvireader->getCurrentPageNumber();}
		void appendToPage (XMLNode *node)              {_svg.appendToPage(node);}
		void appendToDefs (XMLNode *node)              {_svg.appendToDefs(node);}
		void prependToPage (XMLNode *node)             {_svg.prependToPage(node);}
		void pushContextElement (XMLElementNode *node) {_svg.pushContextElement(node);}
		void popContextElement ()                      {_svg.popContextElement();}
		void setTextOrientation(bool vertical)         {_svg.setVertical(vertical);}
		void moveToX (double x);
		void moveToY (double y);
		void setFont (int num, const Font *font);
		void special (const std::string &spc, double dvi2bp, bool preprocessing=false);
		void beginPage (unsigned pageno, Int32 *c);
		void endPage (unsigned pageno);
		void progress (size_t current, size_t total, const char *id=0);
		void progress (const char *id);
		CharMap& getUsedChars () const        {return _usedChars;}
		const FontSet& getUsedFonts () const  {return _usedFonts;}
		void setDVIReader (BasicDVIReader &r) {_dvireader = &r;}
		void setPageMatrix (const Matrix &tm);
		double getX() const   {return _dvireader->getXPos();}
		double getY() const   {return _dvireader->getYPos();}
		void setX (double x)  {_dvireader->translateToX(x); _svg.setX(x);}
		void setY (double y)  {_dvireader->translateToY(y); _svg.setY(y);}
		void finishLine ()    {_dvireader->finishLine();}
		BoundingBox& bbox ()  {return _bbox;}
		BoundingBox& bbox (const std::string &name, bool reset=false);
		void embed (const BoundingBox &bbox);
		void embed (const DPair &p, double r=0);
		std::string getSVGFilename (unsigned pageno) const;
		std::string getBBoxFormatString () const;

	private:
		SVGTree &_svg;
		BasicDVIReader *_dvireader;
		BoundingBox _bbox;
		int _pageCount;
		int _currentFontNum;
		mutable CharMap _usedChars;
		FontSet _usedFonts;
		Matrix *_pageMatrix;  // transformation of whole page
		Color _bgcolor;
		BoxMap *_boxes;
};


#endif
