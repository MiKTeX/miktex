/*************************************************************************
** EPSToSVG.h                                                           **
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

#ifndef DVISVGM_EPSTOSVG_H
#define DVISVGM_EPSTOSVG_H

#include <string>
#include "SpecialActions.h"
#include "SVGTree.h"

struct SVGOutputBase;

class EPSToSVG : protected SpecialActions
{
	public:
		EPSToSVG (const std::string &fname, SVGOutputBase &out) : _fname(fname), _out(out), _x(0), _y(0) {}
		void convert ();
		void setTransformation (const Matrix &m);
		void setPageSize (const std::string &name);

	protected:
		// implement abstract base class SpecialActions
		double getX () const                           {return _x;}
		double getY () const                           {return _y;}
		void setX (double x)                           {_x = x; _svg.setX(x);}
		void setY (double y)                           {_y = y; _svg.setY(y);}
		void finishLine ()                             {}
		void setColor (const Color &color)             {_svg.setColor(color);}
		Color getColor () const                        {return _svg.getColor();}
		void setMatrix (const Matrix &m)               {_svg.setMatrix(m);}
		const Matrix& getMatrix () const               {return _svg.getMatrix();}
		void getPageTransform (Matrix &matrix) const   {}
		void setBgColor (const Color &color)           {}
		void appendToPage (XMLNode *node)              {_svg.appendToPage(node);}
		void appendToDefs (XMLNode *node)              {_svg.appendToDefs(node);}
		void prependToPage (XMLNode *node)             {_svg.prependToPage(node);}
		void pushContextElement (XMLElementNode *node) {_svg.pushContextElement(node);}
		void popContextElement ()                      {_svg.popContextElement();}
		void embed (const BoundingBox &bbox)           {_bbox.embed(bbox);}
		void embed (const DPair &p, double r=0)        {if (r==0) _bbox.embed(p); else _bbox.embed(p, r);}
		void progress (const char *id);
		unsigned getCurrentPageNumber() const          {return 0;}
		BoundingBox& bbox ()                           {return _bbox;}
		BoundingBox& bbox (const std::string &name, bool reset=false) {return _bbox;}
		std::string getSVGFilename (unsigned pageno) const;
		std::string getBBoxFormatString () const       {return "";}

	private:
		std::string _fname;   ///< name of EPS file
		SVGTree _svg;
		SVGOutputBase &_out;
		double _x, _y;
		BoundingBox _bbox;
};

#endif
