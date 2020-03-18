/*************************************************************************
** DVIToSVG.hpp                                                         **
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

#ifndef DVITOSVG_HPP
#define DVITOSVG_HPP

#include <set>
#include <string>
#include <utility>
#include "DVIReader.hpp"
#include "FilePath.hpp"
#include "SVGTree.hpp"

struct DVIActions;
struct SVGOutputBase;
class HashFunction;

class DVIToSVG : public DVIReader {
	public:
		class HashSettings {
			public:
				enum Parameter {P_LIST, P_REPLACE};
				void setParameters (const std::string &paramstr);
				void setOptionHash (const std::string &optHash) {_optHash = optHash;}
				std::string algorithm () const {return _algo;}
				std::string optionsHash () const {return _optHash;}
				bool isSet (Parameter param) {return _params.find(param) != _params.end();}

			private:
				std::string _algo;
				std::string _optHash;
				std::set<Parameter> _params;
		};

	public:
		explicit DVIToSVG (std::istream &is, SVGOutputBase &out);
		DVIToSVG (const DVIToSVG&) =delete;
		void convert (const std::string &range, std::pair<int,int> *pageinfo=nullptr);
		void setPageSize (const std::string &format)         {_bboxFormatString = format;}
		void setPageTransformation (const std::string &cmds) {_transCmds = cmds;}
		Matrix getPageTransformation () const override;
		void translateToX (double x) override {_tx = x-dviState().h-_tx;}
		void translateToY (double y) override {_ty = y-dviState().v-_ty;}
		double getXPos() const override       {return dviState().h+_tx;}
		double getYPos() const override       {return dviState().v+_ty;}
		void finishLine () override           {_prevYPos = std::numeric_limits<double>::min();}
		void listHashes (const std::string &rangestr, std::ostream &os);

		FilePath getSVGFilePath (unsigned pageno) const;
		std::string getUserBBoxString () const  {return _bboxFormatString;}
		static void setProcessSpecials (const char *ignorelist=nullptr, bool pswarning=false);

	public:
		static bool COMPUTE_PROGRESS;  ///< if true, an action to handle the progress ratio of a page is triggered
		static char TRACE_MODE;
		static HashSettings PAGE_HASH_SETTINGS;

	protected:
		void convert (unsigned firstPage, unsigned lastPage, HashFunction *hashFunc);
		int executeCommand () override;
		void enterBeginPage (unsigned pageno, const std::vector<int32_t> &c);
		void leaveEndPage (unsigned pageno);
		void embedFonts (XMLElement *svgElement);
		void moveRight (double dx, MoveMode mode) override;
		void moveDown (double dy, MoveMode mode) override;

		void dviPost (uint16_t stdepth, uint16_t pages, uint32_t pw, uint32_t ph, uint32_t mag, uint32_t num, uint32_t den, uint32_t lbopofs) override;
		void dviBop (const std::vector<int32_t> &c, int32_t prevBopOffset) override;
		void dviEop () override;
		void dviSetChar0 (uint32_t c, const Font *font) override;
		void dviSetChar (uint32_t c, const Font *font) override;
		void dviPutChar (uint32_t c, const Font *font) override;
		void dviSetRule (double h, double w) override;
		void dviPutRule (double h, double w) override;
		void dviPop () override;
		void dviFontNum (uint32_t fontnum, SetFontMode mode, const Font *font) override;
		void dviDir (WritingMode dir) override;
		void dviXXX (const std::string &str) override;
		void dviXGlyphArray (std::vector<double> &dx, std::vector<double> &dy, std::vector<uint16_t> &glyphs, const Font &font) override;
		void dviXGlyphString (std::vector<double> &dx, std::vector<uint16_t> &glyphs, const Font &font) override;
		void dviXTextAndGlyphs (std::vector<double> &dx, std::vector<double> &dy, std::vector<uint16_t> &chars, std::vector<uint16_t> &glyphs, const Font &font) override;

	private:
		SVGTree _svg;
		SVGOutputBase &_out;
		std::unique_ptr<DVIActions> _actions;
		std::string _bboxFormatString;  ///< bounding box size/format set by the user
		std::string _transCmds;         ///< page transformation commands set by the user
		double _pageHeight, _pageWidth; ///< global page height and width stored in the postamble
		double _tx, _ty;                ///< translation of cursor position
		double _prevXPos, _prevYPos;    ///< previous cursor position
		WritingMode _prevWritingMode;   ///< previous writing mode
		std::streampos _pageByte;       ///< position of the stream pointer relative to the preceding bop (in bytes)
};

#endif
