/*************************************************************************
** DVIToSVGActions.hpp                                                  **
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

#ifndef DVITOSVGACTIONS_HPP
#define DVITOSVGACTIONS_HPP

#include <unordered_map>
#include <unordered_set>
#include "BoundingBox.hpp"
#include "DVIActions.hpp"
#include "Matrix.hpp"
#include "SpecialActions.hpp"
#include "SpecialManager.hpp"
#include "SVGTree.hpp"


class DVIToSVG;
class FileFinder;
class Font;
class XMLNode;

class DVIToSVGActions : public DVIActions, public SpecialActions {
	using CharMap = std::unordered_map<const Font*, std::set<int>>;
	using FontSet = std::unordered_set<const Font*>;
	using BoxMap = std::unordered_map<std::string,BoundingBox>;

	public:
		DVIToSVGActions (DVIToSVG &dvisvg, SVGTree &svg) : _svg(svg), _dvireader(&dvisvg) {}
		void reset () override;
		void setChar (double x, double y, unsigned c, bool vertical, const Font &f) override;
		void setRule (double x, double y, double height, double width) override;
		void setBgColor (const Color &color) override;
		void setColor (const Color &color) override             {_svg.setColor(color);}
		void setMatrix (const Matrix &m) override               {_svg.setMatrix(m);}
		const Matrix& getMatrix () const override               {return _svg.getMatrix();}
		Matrix getPageTransformation () const override          {return _dvireader->getPageTransformation();}
		Color getColor () const override                        {return _svg.getColor();}
		int getDVIStackDepth () const override                  {return _dvireader->stackDepth();}
		unsigned getCurrentPageNumber () const override         {return _dvireader->currentPageNumber();}
		void setTextOrientation (bool vertical) override        {_svg.setVertical(vertical);}
		void moveToX (double x, bool forceSVGMove) override;
		void moveToY (double y, bool forceSVGMove) override;
		void setFont (int num, const Font &font) override;
		void special (const std::string &spc, double dvi2bp, bool preprocessing=false) override;
		void beginPage (unsigned pageno, const std::vector<int32_t> &c) override;
		void endPage (unsigned pageno) override;
		void progress (size_t current, size_t total, const char *id=nullptr) override;
		void progress (const char *id) override;
		double getX() const override  {return _dvireader->getXPos();}
		double getY() const override  {return _dvireader->getYPos();}
		void setX (double x) override {_dvireader->translateToX(x); _svg.setX(x);}
		void setY (double y) override {_dvireader->translateToY(y); _svg.setY(y);}
		void finishLine () override   {_dvireader->finishLine();}
		void lockOutput () override   {_outputLocked = true;}
		void unlockOutput () override {_outputLocked = false;}
		bool outputLocked () const override       {return _outputLocked;}
		const SVGTree& svgTree () const override  {return _svg;}
		BoundingBox& bbox () override {return _bbox;}
		BoundingBox& bbox (const std::string &name, bool reset=false) override;
		void embed (const BoundingBox &bbox) override;
		void embed (const DPair &p, double r=0) override;
		FilePath getSVGFilePath (unsigned pageno) const override;
		std::string getBBoxFormatString () const override;
		CharMap& getUsedChars () const        {return _usedChars;}
		const FontSet& getUsedFonts () const  {return _usedFonts;}
		void setDVIReader (BasicDVIReader &r) {_dvireader = &r;}

	private:
		SVGTree &_svg;
		BasicDVIReader *_dvireader;
		BoundingBox _bbox;
		int _pageCount=0;
		int _currentFontNum=-1;
		mutable CharMap _usedChars;
		FontSet _usedFonts;
		Color _bgcolor=Color::TRANSPARENT;
		BoxMap _boxes;
		bool _outputLocked=false;
};


#endif
