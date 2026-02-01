/*************************************************************************
** SVGTree.hpp                                                          **
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

#ifndef SVGTREE_HPP
#define SVGTREE_HPP

#include <map>
#include <memory>
#include <set>
#include <stack>
#include <unordered_set>
#include "Color.hpp"
#include "FontWriter.hpp"
#include "GFGlyphTracer.hpp"
#include "Matrix.hpp"
#include "SVGCharHandler.hpp"
#include "SVGElement.hpp"
#include "XMLDocument.hpp"

class BoundingBox;
class Color;
class Font;
class Matrix;
class Opacity;
class PhysicalFont;

class SVGTree {
	public:
		SVGTree ();
		void reset ();
		bool write (std::ostream &os) const {return bool(_doc.write(os));}
		void newPage (int pageno);
		void appendToDefs (std::unique_ptr<XMLNode> node);
		void appendToPage (std::unique_ptr<XMLNode> node);
		void prependToPage (std::unique_ptr<XMLNode> node);
		void appendToDoc (std::unique_ptr<XMLNode> node)  {_doc.append(std::move(node));}
		void appendToRoot (std::unique_ptr<XMLNode> node) {_root->append(std::move(node));}
		void appendChar (int c, double x, double y) {_charHandler->appendChar(c, x, y);}
		void appendFontStyles (const std::unordered_set<const Font*> &fonts);
		void append (const PhysicalFont &font, const std::set<int> &chars, GFGlyphTracer::Callback *callback=nullptr);
		void pushDefsContext (std::unique_ptr<SVGElement> node);
		void popDefsContext ();
		void pushPageContext (std::unique_ptr<SVGElement> node);
		void popPageContext ();
		void setBBox (const BoundingBox &bbox) const;
		void setFont (int id, const Font &font) const;
		std::pair<int,const Font*> getFontPair () const;
		static bool setFontFormat (std::string formatstr);
		void setX (double x)                {_charHandler->notifyXAdjusted();}
		void setY (double y)                {_charHandler->notifyYAdjusted();}
		void setMatrix (const Matrix &m)    {_charHandler->setMatrix(m);}
		void setFillColor (const Color &c) const;
		void setStrokeColor (const Color &c) const;
		void setOpacity (const Opacity &op) {_charHandler->setOpacity(op);}
		void setVertical (bool state)       {_charHandler->setVertical(state);}
		void transformPage (const Matrix &m) const;
		Color getFillColor () const         {return _charHandler->getFillColor();}
		Color getStrokeColor () const       {return _charHandler->getStrokeColor();}
		const Opacity& getOpacity () const  {return _charHandler->getOpacity();}
		const Matrix& getMatrix () const    {return _charHandler->getMatrix();}
		XMLElement* rootNode () const       {return _root;}
		XMLElement* defsNode () const       {return _defs;}
		XMLElement* pageNode () const       {return _page;}

	protected:
		XMLCData* styleCDataNode ();

	public:
		static bool USE_FONTS;           ///< if true, create font references and don't draw paths directly
		static bool CREATE_CSS;          ///< define and use CSS classes to reference fonts?
		static bool CREATE_USE_ELEMENTS; ///< allow generation of <use/> elements?
		static FontWriter::FontFormat FONT_FORMAT;   ///< format of fonts to be embedded
		static bool RELATIVE_PATH_CMDS;  ///< relative path commands rather than absolute ones?
		static bool MERGE_CHARS;         ///< whether to merge chars with common properties into the same <text> tag
		static bool ADD_COMMENTS;        ///< add comments with additional information
		static double ZOOM_FACTOR;       ///< factor applied to width/height attribute
		static bool EMBED_BITMAP_DATA;   ///< if true, bitmaps are embedded into the SVG document

	private:
		XMLDocument _doc;
		SVGElement *_root=nullptr, *_page=nullptr, *_defs=nullptr;
		XMLCData *_styleCDataNode=nullptr;
		std::unique_ptr<SVGCharHandler> _charHandler;
		std::stack<SVGElement*> _defsContextStack;
		std::stack<SVGElement*> _pageContextStack;
};

#endif
