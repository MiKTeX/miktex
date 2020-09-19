/*************************************************************************
** DVIReader.hpp                                                        **
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

#ifndef DVIREADER_HPP
#define DVIREADER_HPP

#include <limits>
#include <map>
#include <stack>
#include <string>
#include "BasicDVIReader.hpp"
#include "Color.hpp"
#include "VFActions.hpp"

class Font;
class FontStyle;
class HashFunction;
class NativeFont;
class VirtualFont;

class DVIReader : public BasicDVIReader, public VFActions {
	protected:
		enum class WritingMode {LR=0, TB=1, BT=3};
		enum class SetFontMode {SF_SHORT, SF_LONG, VF_ENTER, VF_LEAVE};
		enum class MoveMode {SETCHAR, CHANGEPOS};

		struct DVIState {
			double h, v;        ///< horizontal and vertical cursor position
			double x, w, y, z;  ///< additional registers to store horizontal (x, w) and vertical (y, z) positions
			WritingMode d;      ///< direction: 0: horizontal, 1: vertical(top->bottom), 3: vertical (bottom->top)
			DVIState ()   {reset();}
			void reset () {h = v = x = w = y = z = 0.0; d=WritingMode::LR;}
		};

	public:
		explicit DVIReader (std::istream &is);
		bool executeDocument ();
		void executeAll ();
		void executePreamble ();
		void executePostamble ();
		bool executePage (unsigned n);
		bool inPostamble () const                    {return _inPostamble;}
		double getXPos () const override             {return _dviState.h;}
		double getYPos () const override             {return _dviState.v;}
		int stackDepth () const override             {return _stateStack.size();}
		int currentFontNumber () const               {return _currFontNum;}
		unsigned currentPageNumber () const override {return _currPageNum;}
		unsigned numberOfPages () const              {return _bopOffsets.empty() ? 0 : _bopOffsets.size()-1;}

	protected:
		int executeCommand () override;
		void collectBopOffsets ();
		size_t numberOfPageBytes (int n) const {return _bopOffsets.size() > 1 ? _bopOffsets[n+1]-_bopOffsets[n] : 0;}
		bool computePageHash (size_t pageno, HashFunction &hashFunc);
		void goToPostamble ();
		virtual void moveRight (double dx, MoveMode mode);
		virtual void moveDown (double dy, MoveMode mode);
		void putVFChar (Font *font, uint32_t c);
		double putGlyphArray (bool xonly, std::vector<double> &dx, std::vector<double> &dy, std::vector<uint16_t> &glyphs);
		const Font* defineFont (uint32_t fontnum, const std::string &name, uint32_t cs, double ds, double ss);
		void setFont (int num, SetFontMode mode);
		const DVIState& dviState() const {return _dviState;}
		double dvi2bp () const {return _dvi2bp;}
		bool inPage () const   {return _inPage;}

		// VFAction methods
		void defineVFFont (uint32_t fontnum, const std::string &path, const std::string &name, uint32_t checksum, double dsize, double ssize) override;
		void defineVFChar (uint32_t c, std::vector<uint8_t> &&dvi) override;

		// The following template methods provide higher-level access to the DVI commands.
		// In contrast to their cmdXXX pendants, they don't require any handling of the input stream.
		// All command arguments are delivered as function parameters. These methods are called after
		// the DVI registers have been updated, i.e. currState() represents the state after executing
		// the command. If the previous register state is required, prevState() can be used.
		virtual void dviPre (uint8_t id, uint32_t numer, uint32_t denom, uint32_t mag, const std::string &comment) {}
		virtual void dviPost (uint16_t stdepth, uint16_t pages, uint32_t pw, uint32_t ph, uint32_t mag, uint32_t num, uint32_t den, uint32_t lbopofs) {}
		virtual void dviPostPost (uint8_t id, uint32_t postOffset) {}
		virtual void dviBop (const std::vector<int32_t> &c, int32_t prevBopOffset) {}
		virtual void dviEop () {}
		virtual void dviSetChar0 (uint32_t c, const Font *font) {}
		virtual void dviSetChar (uint32_t c, const Font *font) {}
		virtual void dviPutChar (uint32_t c, const Font *font) {}
		virtual void dviSetRule (double h, double w) {}
		virtual void dviPutRule (double h, double w) {}
		virtual void dviNop () {}
		virtual void dviPush () {}
		virtual void dviPop () {}
		virtual void dviRight (double dx) {}
		virtual void dviDown (double dy) {}
		virtual void dviX0 () {}
		virtual void dviY0 () {}
		virtual void dviW0 () {}
		virtual void dviZ0 () {}
		virtual void dviX (double x) {}
		virtual void dviY (double y) {}
		virtual void dviW (double w) {}
		virtual void dviZ (double z) {}
		virtual void dviFontDef (uint32_t fontnum, uint32_t checksum, const Font *font) {}
		virtual void dviFontNum (uint32_t fontnum, SetFontMode mode, const Font *font) {}
		virtual void dviDir (WritingMode dir) {}
		virtual void dviXXX (const std::string &str) {}
		virtual void dviXPic (uint8_t box, const std::vector<int32_t> &matrix, int16_t page, const std::string &path) {}
		virtual void dviXFontDef (uint32_t fontnum, const NativeFont *font) {}
		virtual void dviXGlyphArray (std::vector<double> &dx, std::vector<double> &dy, std::vector<uint16_t> &glyphs, const Font &font) {}
		virtual void dviXGlyphString (std::vector<double> &dx, std::vector<uint16_t> &glyphs, const Font &font) {}
		virtual void dviXTextAndGlyphs (std::vector<double> &dx, std::vector<double> &dy, std::vector<uint16_t> &chars, std::vector<uint16_t> &glyphs, const Font &font) {}

	private:
		// The following low-level methods evaluate the DVI commands read from
		// the input stream and call the corresponding dviXXX methods.
		void cmdSetChar0 (int c) override;
		void cmdSetChar (int len) override;
		void cmdPutChar (int len) override;
		void cmdSetRule (int len) override;
		void cmdPutRule (int len) override;
		void cmdNop (int len) override;
		void cmdBop (int len) override;
		void cmdEop (int len) override;
		void cmdPush (int len) override;
		void cmdPop (int len) override;
		void cmdDir (int len) override;
		void cmdRight (int len) override;
		void cmdDown (int len) override;
		void cmdX0 (int len) override;
		void cmdY0 (int len) override;
		void cmdW0 (int len) override;
		void cmdZ0 (int len) override;
		void cmdX (int len) override;
		void cmdY (int len) override;
		void cmdW (int len) override;
		void cmdZ (int len) override;
		void cmdFontDef (int len) override;
		void cmdFontNum0 (int n) override;
		void cmdFontNum (int len) override;
		void cmdXXX (int len) override;
		void cmdPre (int len) override;
		void cmdPost (int len) override;
		void cmdPostPost (int len) override;
		void cmdXPic (int len) override;
		void cmdXFontDef (int len) override;
		void cmdXGlyphArray (int len) override;
		void cmdXGlyphString (int len) override;
		void cmdXTextAndGlyphs (int len) override;

	private:
		bool _inPage;            ///< true if stream pointer is between bop and eop
		unsigned _currPageNum;   ///< current page number (1 is first page)
		int _currFontNum;        ///< current font number
		double _dvi2bp;          ///< factor to convert dvi units to PS points
		uint32_t _mag;           ///< magnification factor * 1000
		bool _inPostamble;       ///< true if stream pointer is inside the postamble
		DVIState _dviState;      ///< current state of the DVI registers
		std::stack<DVIState> _stateStack;
		std::vector<uint32_t> _bopOffsets;
};

#endif
