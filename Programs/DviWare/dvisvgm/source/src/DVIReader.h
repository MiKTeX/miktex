/*************************************************************************
** DVIReader.h                                                          **
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

#ifndef DVISVGM_DVIREADER_H
#define DVISVGM_DVIREADER_H

#include <limits>
#include <map>
#include <stack>
#include <string>
#include "BasicDVIReader.h"
#include "Color.h"
#include "VFActions.h"
#include "types.h"

class Font;
class FontStyle;
class NativeFont;
class VirtualFont;

class DVIReader : public BasicDVIReader, public VFActions
{
	protected:
		enum WritingMode {WMODE_LR=0, WMODE_TB=1, WMODE_BT=3};
		enum SetFontMode {SF_SHORT, SF_LONG, VF_ENTER, VF_LEAVE};

		struct DVIState
		{
			double h, v;        ///< horizontal and vertical cursor position
			double x, w, y, z;  ///< additional registers to store horizontal (x, w) and vertical (y, z) positions
			WritingMode d;      ///< direction: 0: horizontal, 1: vertical(top->bottom), 3: vertical (bottom->top)
			DVIState ()   {reset();}
			void reset () {h = v = x = w = y = z = 0.0; d=WMODE_LR;}
		};

	public:
		explicit DVIReader (std::istream &is);
		bool executeDocument ();
		void executeAll ();
		void executePreamble ();
		void executePostamble ();
		bool executePage (unsigned n);
		bool inPostamble () const            {return _inPostamble;}
		double getXPos () const              {return _currDviState.h;}
		double getYPos () const              {return _currDviState.v;}
		int stackDepth () const              {return _stateStack.size();}
		int currentFontNumber () const       {return _currFontNum;}
		unsigned currentPageNumber () const  {return _currPageNum;}
		unsigned numberOfPages () const      {return _bopOffsets.empty() ? 0 : _bopOffsets.size()-1;}

	protected:
		int executeCommand ();
		void collectBopOffsets ();
		size_t numberOfPageBytes (int n) const {return _bopOffsets.size() > 1 ? _bopOffsets[n+1]-_bopOffsets[n] : 0;}
		virtual void moveRight (double dx);
		virtual void moveDown (double dy);
		void putVFChar (Font *font, UInt32 c);
		void putGlyphArray (bool xonly, std::vector<double> &dx, std::vector<double> &dy, std::vector<UInt16> &glyphs);
		const Font* defineFont (UInt32 fontnum, const std::string &name, UInt32 cs, double ds, double ss);
		void setFont (int num, SetFontMode mode);
		const DVIState& currState() const {return _currDviState;}
		const DVIState& prevState() const {return _prevDviState;}
		double dvi2bp () const {return _dvi2bp;}
		bool inPage () const   {return _inPage;}

		// VFAction methods
		void defineVFFont (UInt32 fontnum, std::string path, std::string name, UInt32 checksum, double dsize, double ssize);
		void defineVFChar (UInt32 c, std::vector<UInt8> *dvi);

		// The following template methods provide higher-level access to the DVI commands.
		// In contrast to their cmdXXX pendants, they don't require any handling of the input stream.
		// All command arguments are delivered as function parameters. These methods are called after
		// the DVI registers have been updated, i.e. currState() represents the state after executing
		// the command. If the previous register state is required, prevState() can be used.
		virtual void dviPre (UInt8 id, UInt32 numer, UInt32 denom, UInt32 mag, const std::string &comment) {}
		virtual void dviPost (UInt16 stdepth, UInt16 pages, UInt32 pw, UInt32 ph, UInt32 mag, UInt32 num, UInt32 den, UInt32 lbopofs) {}
		virtual void dviPostPost (UInt8 id, UInt32 postOffset) {}
		virtual void dviBop (const std::vector<Int32> &c, Int32 prevBopOffset) {}
		virtual void dviEop () {}
		virtual void dviSetChar0 (UInt32 c, const Font *font) {}
		virtual void dviSetChar (UInt32 c, const Font *font) {}
		virtual void dviPutChar (UInt32 c, const Font *font) {}
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
		virtual void dviFontDef (UInt32 fontnum, UInt32 checksum, const Font *font) {}
		virtual void dviFontNum (UInt32 fontnum, SetFontMode mode, const Font *font) {}
		virtual void dviDir (WritingMode dir) {}
		virtual void dviXXX (const std::string &str) {}
		virtual void dviXPic (UInt8 box, const std::vector<Int32> &matrix, Int16 page, const std::string &path) {}
		virtual void dviXFontDef (UInt32 fontnum, const NativeFont *font) {}
		virtual void dviXGlyphArray (std::vector<double> &dx, std::vector<double> &dy, std::vector<UInt16> &glyphs, const Font &font) {}
		virtual void dviXGlyphString (std::vector<double> &dx, std::vector<UInt16> &glyphs, const Font &font) {}
		virtual void dviXTextAndGlyphs (std::vector<double> &dx, std::vector<double> &dy, std::vector<UInt16> &chars, std::vector<UInt16> &glyphs, const Font &font) {}

	private:
		// The following low-level methods evaluate the DVI commands read from
		// the input stream and call the corresponding dviXXX methods.
		void cmdSetChar0 (int c);
		void cmdSetChar (int len);
		void cmdPutChar (int len);
		void cmdSetRule (int len);
		void cmdPutRule (int len);
		void cmdNop (int len);
		void cmdBop (int len);
		void cmdEop (int len);
		void cmdPush (int len);
		void cmdPop (int len);
		void cmdDir (int len);
		void cmdRight (int len);
		void cmdDown (int len);
		void cmdX0 (int len);
		void cmdY0 (int len);
		void cmdW0 (int len);
		void cmdZ0 (int len);
		void cmdX (int len);
		void cmdY (int len);
		void cmdW (int len);
		void cmdZ (int len);
		void cmdFontDef (int len);
		void cmdFontNum0 (int n);
		void cmdFontNum (int len);
		void cmdXXX (int len);
		void cmdPre (int len);
		void cmdPost (int len);
		void cmdPostPost (int len);
		void cmdXPic (int len);
		void cmdXFontDef (int len);
		void cmdXGlyphArray (int len);
		void cmdXGlyphString (int len);
		void cmdXTextAndGlyphs (int len);

	private:
		bool _inPage;            ///< true if stream pointer is between bop and eop
		unsigned _currPageNum;   ///< current page number (1 is first page)
		int _currFontNum;        ///< current font number
		double _dvi2bp;          ///< factor to convert dvi units to PS points
		UInt32 _mag;             ///< magnification factor * 1000
		bool _inPostamble;       ///< true if stream pointer is inside the postamble
		DVIState _currDviState;  ///< current state of the DVI registers
		DVIState _prevDviState;  ///< previous state of the DVI registers
		std::stack<DVIState> _stateStack;
		std::vector<UInt32> _bopOffsets;
};

#endif
