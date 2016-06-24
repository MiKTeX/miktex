/*************************************************************************
** DVIReader.cpp                                                        **
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

#include <config.h>
#include <algorithm>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include "Color.h"
#include "DVIActions.h"
#include "DVIReader.h"
#include "Font.h"
#include "FontManager.h"
#include "SignalHandler.h"
#include "VectorStream.h"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;

bool DVIReader::COMPUTE_PROGRESS = false;


DVIReader::DVIReader (istream &is, DVIActions *a) : BasicDVIReader(is), _actions(a)
{
	_inPage = false;
	_pageHeight = _pageWidth = 0;
	_dvi2bp = 0.0;
	_tx = _ty = 0;    // no cursor translation
	_prevYPos = numeric_limits<double>::min();
	_inPostamble = false;
	_currFontNum = 0;
	_currPageNum = 0;
	_pagePos = 0;
	_mag = 1;
	executePreamble();
	collectBopOffsets();
	executePostamble();
}


DVIActions* DVIReader::replaceActions (DVIActions *a) {
	DVIActions *prev_actions = _actions;
	_actions = a;
	return prev_actions;
}


/** Reads a single DVI command from the current position of the input stream and calls the
 *  corresponding cmdFOO method.
 *  @return opcode of the command executed */
int DVIReader::executeCommand () {
	SignalHandler::instance().check();
	CommandHandler handler;
	int param; // parameter of handler
	const streampos cmdpos = tell();
	int opcode = evalCommand(handler, param);
	(this->*handler)(param);
	if (_dviState.v+_ty != _prevYPos) {
		_tx = _ty = 0;
		_prevYPos = _dviState.v;
	}
	if (COMPUTE_PROGRESS && _inPage && _actions) {
		size_t pagelen = numberOfPageBytes(_currPageNum-1);
		// ensure progress() is called at 0%
		if (opcode == 139)  // bop?
			_actions->progress(0, pagelen);
		// ensure progress() is called at 100%
		if (peek() == 140)  // eop reached?
			_pagePos = pagelen;
		else
			_pagePos += tell()-cmdpos;
		_actions->progress(_pagePos, pagelen);
	}
	return opcode;
}


/** Executes all DVI commands read from the input stream. */
void DVIReader::executeAll () {
	int opcode = 0;
	while (!eof() && opcode >= 0) {
		try {
			opcode = executeCommand();
		}
		catch (const InvalidDVIFileException &e) {
			// end of stream reached
			opcode = -1;
		}
	}
}


/** Reads and executes the commands of a single page.
 *  This methods stops reading after the page's eop command has been executed.
 *  @param[in] n number of page to be executed
 *  @returns true if page was read successfully */
bool DVIReader::executePage (unsigned n) {
	clearStream();    // reset all status bits
	if (!isStreamValid())
		throw DVIException("invalid DVI file");
	if (n < 1 || n > numberOfPages())
		return false;

	seek(_bopOffsets[n-1]);           // goto bop of n-th page
	_inPostamble = false;             // not in postamble
	_currPageNum = n;
	while (executeCommand() != 140);  // 140 == eop
	return true;
}


void DVIReader::executePreamble () {
	clearStream();
	if (isStreamValid()) {
		seek(0);
		if (readByte() == 247) {
			cmdPre(0);
			return;
		}
	}
	throw DVIException("invalid DVI file");
}


/** Moves stream pointer to begin of postamble */
static void to_postamble (StreamReader &reader) {
	reader.clearStream();
	if (!reader.isStreamValid())
		throw DVIException("invalid DVI file");

	reader.seek(-1, ios::end);          // stream pointer to last byte
	int count=0;
	while (reader.peek() == 223) {
		reader.seek(-1, ios::cur);       // skip fill bytes
		count++;
	}
	if (count < 4)  // the standard requires at least 4 trailing fill bytes
		throw DVIException("missing fill bytes at end of file");

	reader.seek(-4, ios::cur);          // now on first byte of q (pointer to begin of postamble)
	UInt32 q = reader.readUnsigned(4);  // pointer to begin of postamble
	reader.seek(q);                     // now on begin of postamble
}


/** Reads and executes the commands of the postamble. */
void DVIReader::executePostamble () {
	to_postamble(*this);
	while (executeCommand() != 249);  // executes all commands until post_post (= 249) is reached
}


/** Collects and records the file offsets of all bop commands. */
void DVIReader::collectBopOffsets () {
	to_postamble(*this);
	_bopOffsets.push_back(tell());     // also add offset of postamble
	readByte();                        // skip post command
	UInt32 offset = readUnsigned(4);   // offset of final bop
	while ((Int32)offset > 0) {        // not yet on first bop?
		_bopOffsets.push_back(offset);  // record offset
		seek(offset+41);                // skip bop command and the 10 \count values => now on offset of previous bop
		offset = readUnsigned(4);
	}
	reverse(_bopOffsets.begin(), _bopOffsets.end());
}


/** Returns the current x coordinate in PS point units.
 *  This is the horizontal position where the next output would be placed. */
double DVIReader::getXPos () const {
	return _dviState.h+_tx;
}


/** Returns the current y coordinate in PS point units.
 *  This is the vertical position where the next output would be placed. */
double DVIReader::getYPos () const {
	return _dviState.v+_ty;
}


/////////////////////////////////////

/** Reads and executes DVI preamble command.
 *  Format: pre ver[1] num[4] den[4] mag[4] cmtlen[1] cmt[cmtlen] */
void DVIReader::cmdPre (int) {
	setDVIFormat((DVIFormat)readUnsigned(1)); // identification number
	UInt32 num = readUnsigned(4);  // numerator units of measurement
	UInt32 den = readUnsigned(4);  // denominator units of measurement
	if (den == 0)
		throw DVIException("denominator of measurement unit is zero");
	_mag = readUnsigned(4);        // magnification
	UInt32 k   = readUnsigned(1);  // length of following comment
	string cmt = readString(k);    // comment
	// 1 dviunit * num/den == multiples of 0.0000001m
	// 1 dviunit * _dvibp: length of 1 dviunit in PS points * _mag/1000
	_dvi2bp = num/254000.0*72.0/den*_mag/1000.0;
	if (_actions)
		_actions->preamble(cmt);
}


/** Reads and executes DVI postamble command.
 *  Format: post p[4] num[4] den[4] mag[4] ph[4] pw[4] sd[2] np[2] */
void DVIReader::cmdPost (int) {
	readUnsigned(4);  // skip pointer to previous bop
	UInt32 num = readUnsigned(4);
	UInt32 den = readUnsigned(4);
	if (den == 0)
		throw DVIException("denominator of measurement unit is zero");
	_mag = readUnsigned(4);
	_pageHeight = readUnsigned(4); // height of tallest page in dvi units
	_pageWidth  = readUnsigned(4); // width of widest page in dvi units
	readUnsigned(2);               // skip max. stack depth
	if (readUnsigned(2) != numberOfPages())
		throw DVIException("page count entry doesn't match actual number of pages");

	// 1 dviunit * num/den == multiples of 0.0000001m
	// 1 dviunit * _dvi2bp: length of 1 dviunit in PS points * _mag/1000
	_dvi2bp = num/254000.0*72.0/den*_mag/1000.0;
	_pageHeight *= _dvi2bp;   // to pt units
	_pageWidth *= _dvi2bp;
	_inPostamble = true;
	if (_actions)
		_actions->postamble();
}


/** Reads and executes DVI post_post command.
 *  Format: post_post q[4] i[1] 223[>=4] */
void DVIReader::cmdPostPost (int) {
	_inPostamble = false;
	readUnsigned(4);   // pointer to begin of postamble
	setDVIFormat((DVIFormat)readUnsigned(1));  // identification byte
	while (readUnsigned(1) == 223);  // skip fill bytes (223), eof bit should be set now
}


/** Reads and executes Begin-Of-Page command.
 *  Format: bop c0[+4] ... c9[+4] p[+4] */
void DVIReader::cmdBop (int) {
	Int32 c[10];
	for (int i=0; i < 10; i++)
		c[i] = readSigned(4);
	readSigned(4);        // pointer to peceeding bop (-1 in case of first page)
	_dviState.reset();    // set all DVI registers to 0
	while (!_stateStack.empty())
		_stateStack.pop();
	_currFontNum = 0;
	_inPage = true;
	_pagePos = 0;
	beginPage(_currPageNum, c);
	if (_actions)
		_actions->beginPage(_currPageNum, c);
}


/** Reads and executes End-Of-Page command. */
void DVIReader::cmdEop (int) {
	if (!_stateStack.empty())
		throw DVIException("stack not empty at end of page");
	_inPage = false;
	endPage(_currPageNum);
	if (_actions)
		_actions->endPage(_currPageNum);
}


/** Reads and executes push command. */
void DVIReader::cmdPush (int) {
	_stateStack.push(_dviState);
}


/** Reads and executes pop command (restores pushed position information). */
void DVIReader::cmdPop (int) {
	if (_stateStack.empty())
		throw DVIException("stack empty at pop command");
	else {
		DVIState prevState = _dviState;
		_dviState = _stateStack.top();
		_stateStack.pop();
		if (_actions) {
			if (prevState.h != _dviState.h)
				_actions->moveToX(_dviState.h + _tx);
			if (prevState.v != _dviState.v)
				_actions->moveToY(_dviState.v + _ty);
			if (prevState.d != _dviState.d)
				_actions->setTextOrientation(_dviState.d != WMODE_LR);
		}
	}
}


/** Helper function that actually sets/puts a charater. It is called by the
 *  cmdSetChar and cmdPutChar methods.
 * @param[in] c character to typeset
 * @param[in] moveCursor if true, register h is increased by the character width
 * @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::putChar (UInt32 c, bool moveCursor) {
	if (!_inPage)
		throw DVIException("set_char/put_char outside of page");

	FontManager &fm = FontManager::instance();
	Font *font = fm.getFont(_currFontNum);
	if (!font)
		throw DVIException("no font selected");

	if (VirtualFont *vf = dynamic_cast<VirtualFont*>(font)) {    // is current font a virtual font?
		vector<UInt8> *dvi = const_cast<vector<UInt8>*>(vf->getDVI(c)); // get DVI snippet that describes character c
		if (dvi) {
			DVIState pos = _dviState;        // save current cursor position
			_dviState.x = _dviState.y = _dviState.w = _dviState.z = 0;
			int save_fontnum = _currFontNum; // save current font number
			fm.enterVF(vf);                  // new font number context
			cmdFontNum0(fm.vfFirstFontNum(vf));
			double save_scale = _dvi2bp;
			// DVI units in virtual fonts are multiples of 1^(-20) times the scaled size of the VF
			_dvi2bp = vf->scaledSize()/(1 << 20);

			VectorInputStream<UInt8> vis(*dvi);
			istream &is = replaceStream(vis);
			try {
				executeAll();            // execute DVI fragment
			}
			catch (const DVIException &e) {
				//					Message::estream(true) << "invalid dvi in vf: " << e.getMessage() << endl; // @@
			}
			replaceStream(is);          // restore previous input stream
			_dvi2bp = save_scale;       // restore previous scale factor
			fm.leaveVF();               // restore previous font number context
			cmdFontNum0(save_fontnum);  // restore previous font number
			_dviState = pos;            // restore previous cursor position
		}
	}
	else if (_actions)
		_actions->setChar(_dviState.h+_tx, _dviState.v+_ty, c, _dviState.d != WMODE_LR, font);

	if (moveCursor) {
		double dist = font->charWidth(c) * font->scaleFactor() * _mag/1000.0;
		switch (_dviState.d) {
			case WMODE_LR: _dviState.h += dist; break;
			case WMODE_TB: _dviState.v += dist; break;
			case WMODE_BT: _dviState.v -= dist; break;
		}
	}
}


/** Reads and executes set_char_x command. Puts a character at the current
 *  position and advances the cursor.
 *  @param[in] c character to set
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdSetChar0 (int c) {
	putChar(c, true);
}


/** Reads and executes setx command. Puts a character at the current
 *  position and advances the cursor.
 *  @param[in] len number of parameter bytes (possible values: 1-4)
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdSetChar (int len) {
	// According to the dvi specification all character codes are unsigned
	// except len == 4. At the moment all char codes are treated as unsigned...
	UInt32 c = readUnsigned(len); // if len == 4 c may be signed
	putChar(c, true);
}


/** Reads and executes putx command. Puts a character at the current
 *  position but doesn't change the cursor position.
 *  @param[in] len number of parameter bytes (possible values: 1-4)
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdPutChar (int len) {
	// According to the dvi specification all character codes are unsigned
	// except len == 4. At the moment all char codes are treated as unsigned...
	Int32 c = readUnsigned(len);
	putChar(c, false);
}


/** Reads and executes set_rule command. Puts a solid rectangle at the current
 *  position and updates the cursor position.
 *  Format: set_rule h[+4] w[+4]
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdSetRule (int) {
	if (!_inPage)
		throw DVIException("set_rule outside of page");
	double height = _dvi2bp*readSigned(4);
	double width  = _dvi2bp*readSigned(4);
	if (_actions && height > 0 && width > 0)
		_actions->setRule(_dviState.h+_tx, _dviState.v+_ty, height, width);
	moveRight(width);
}


/** Reads and executes set_rule command. Puts a solid rectangle at the current
 *  position but leaves the cursor position unchanged.
 *  Format: put_rule h[+4] w[+4]
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdPutRule (int) {
	if (!_inPage)
		throw DVIException("put_rule outside of page");
	double height = _dvi2bp*readSigned(4);
	double width  = _dvi2bp*readSigned(4);
	if (_actions && height > 0 && width > 0)
		_actions->setRule(_dviState.h+_tx, _dviState.v+_ty, height, width);
}


void DVIReader::moveRight (double dx) {
	switch (_dviState.d) {
		case WMODE_LR: _dviState.h += dx; break;
		case WMODE_TB: _dviState.v += dx; break;
		case WMODE_BT: _dviState.v -= dx; break;
	}
	if (_actions) {
		if (_dviState.d == WMODE_LR)
			_actions->moveToX(_dviState.h+_tx);
		else
			_actions->moveToY(_dviState.v+_ty);
	}
}


void DVIReader::moveDown (double dy) {
	switch (_dviState.d) {
		case WMODE_LR: _dviState.v += dy; break;
		case WMODE_TB: _dviState.h -= dy; break;
		case WMODE_BT: _dviState.h += dy; break;
	}
	if (_actions) {
		if (_dviState.d == WMODE_LR)
			_actions->moveToY(_dviState.v+_ty);
		else
			_actions->moveToX(_dviState.h+_tx);
	}
}


void DVIReader::cmdRight (int len) {moveRight(_dvi2bp*readSigned(len));}
void DVIReader::cmdDown (int len)  {moveDown(_dvi2bp*readSigned(len));}
void DVIReader::cmdX0 (int)        {moveRight(_dviState.x);}
void DVIReader::cmdY0 (int)        {moveDown(_dviState.y);}
void DVIReader::cmdW0 (int)        {moveRight(_dviState.w);}
void DVIReader::cmdZ0 (int)        {moveDown(_dviState.z);}
void DVIReader::cmdX (int len)     {_dviState.x = _dvi2bp*readSigned(len); cmdX0(0);}
void DVIReader::cmdY (int len)     {_dviState.y = _dvi2bp*readSigned(len); cmdY0(0);}
void DVIReader::cmdW (int len)     {_dviState.w = _dvi2bp*readSigned(len); cmdW0(0);}
void DVIReader::cmdZ (int len)     {_dviState.z = _dvi2bp*readSigned(len); cmdZ0(0);}
void DVIReader::cmdNop (int)       {}


/** Sets the text orientation (horizontal, vertical).
 *  This command is only available in DVI files of format 3 (created by pTeX) */
void DVIReader::cmdDir (int) {
	UInt8 wmode = readUnsigned(1);
	if (wmode == 4)  // yoko mode (4) equals default LR mode (0)
		wmode = 0;
	if (wmode == 2 || wmode > 3) {
		ostringstream oss;
		oss << "invalid writing mode value " << wmode << " (0, 1, or 3 expected)";
		throw DVIException(oss.str());
	}
	_dviState.d = (WritingMode)wmode;
	if (_actions)
		_actions->setTextOrientation(_dviState.d != WMODE_LR);
}


void DVIReader::cmdXXX (int len) {
	if (!_inPage)
		throw DVIException("special outside of page");
	UInt32 numBytes = readUnsigned(len);
	string s = readString(numBytes);
	if (_actions)
		_actions->special(s, _dvi2bp);
}


/** Selects a previously defined font by its number.
 * @param[in] num font number
 * @throw DVIException if font number is undefined */
void DVIReader::cmdFontNum0 (int num) {
	if (Font *font = FontManager::instance().getFont(num)) {
		_currFontNum = num;
		if (_actions && !dynamic_cast<VirtualFont*>(font))
			_actions->setFont(FontManager::instance().fontID(num), font);  // all fonts get a recomputed ID
	}
	else {
		ostringstream oss;
		oss << "undefined font number " << num;
		throw DVIException(oss.str());
	}
}


/** Selects a previously defined font.
 * @param[in] len size of font number variable (in bytes)
 * @throw DVIException if font number is undefined */
void DVIReader::cmdFontNum (int len) {
	UInt32 num = readUnsigned(len);
	cmdFontNum0(num);
}


/** Helper function to handle a font definition.
 *  @param[in] fontnum local font number
 *  @param[in] name font name
 *  @param[in] cs checksum to be compared with TFM checksum
 *  @param[in] ds design size in PS point units
 *  @param[in] ss scaled size in PS point units */
void DVIReader::defineFont (UInt32 fontnum, const string &name, UInt32 cs, double ds, double ss) {
	if (!_inPostamble)  // only process font definitions collected in the postamble; skip all others
		return;

	FontManager &fm = FontManager::instance();
	int id = fm.registerFont(fontnum, name, cs, ds, ss);
	Font *font = fm.getFontById(id);
	if (VirtualFont *vf = dynamic_cast<VirtualFont*>(font)) {
		// read vf file, register its font and character definitions
		fm.enterVF(vf);
#if defined(MIKTEX_WINDOWS)
                ifstream ifs(UW_(vf->path()), ios::binary);
#else
		ifstream ifs(vf->path(), ios::binary);
#endif
		VFReader vfReader(ifs);
		vfReader.replaceActions(this);
		vfReader.executeAll();
		fm.leaveVF();
	}
	if (_actions)
		_actions->defineFont(id, font);
}


/** Defines a new font.
 * @param[in] len size of font number variable (in bytes) */
void DVIReader::cmdFontDef (int len) {
	UInt32 fontnum  = readUnsigned(len);   // font number
	UInt32 checksum = readUnsigned(4);     // font checksum (to be compared with corresponding TFM checksum)
	UInt32 ssize    = readUnsigned(4);     // scaled size of font in DVI units
	UInt32 dsize    = readUnsigned(4);     // design size of font in DVI units
	UInt32 pathlen  = readUnsigned(1);     // length of font path
	UInt32 namelen  = readUnsigned(1);     // length of font name
	readString(pathlen);                   // skip font path
	string fontname = readString(namelen);

	defineFont(fontnum, fontname, checksum, dsize*_dvi2bp, ssize*_dvi2bp);
}


/** This template method is called by the VFReader after reading a font definition from a VF file.
 *  @param[in] fontnum local font number
 *  @param[in] path path to font file
 *  @param[in] name font name
 *  @param[in] checksum checksum to be compared with TFM checksum
 *  @param[in] dsize design size in PS point units
 *  @param[in] ssize scaled size in PS point units */
void DVIReader::defineVFFont (UInt32 fontnum, string path, string name, UInt32 checksum, double dsize, double ssize) {
	if (VirtualFont *vf = FontManager::instance().getVF())
		defineFont(fontnum, name, checksum, dsize, ssize * vf->scaleFactor());
}


/** This template method is called by the VFReader after reading a character definition from a VF file.
 *  @param[in] c character number
 *  @param[in] dvi DVI fragment describing the character */
void DVIReader::defineVFChar (UInt32 c, vector<UInt8> *dvi) {
	FontManager::instance().assignVfChar(c, dvi);
}


/** XDV extension: includes image or pdf file.
 *  parameters: box[1] matrix[4][6] p[2] len[2] path[l] */
void DVIReader::cmdXPic (int) {
	// just skip the parameters
	readUnsigned(1);           // box
	for (int i=0; i < 6; i++)  // matrix
		readSigned(4);
	readSigned(2);             // page number
	UInt16 len = readUnsigned(2);
	readString(len);           // path to image/pdf file
}


/** XDV extension: defines a native font */
void DVIReader::cmdXFontDef (int) {
	Int32 fontnum = readSigned(4);
	double ptsize = _dvi2bp*readUnsigned(4);
	UInt16 flags = readUnsigned(2);
	UInt8 psname_len = readUnsigned(1);
	UInt8 fmname_len = getDVIFormat() == DVI_XDVOLD ? readUnsigned(1) : 0;
	UInt8 stname_len = getDVIFormat() == DVI_XDVOLD ? readUnsigned(1) : 0;
	string fontname = readString(psname_len);
	UInt32 fontIndex=0;
	if (getDVIFormat() == DVI_XDVOLD)
		seek(fmname_len+stname_len, ios::cur);
	else
		fontIndex = readUnsigned(4);
	FontStyle style;
	Color color;
	if (flags & 0x0100) { // vertical?
	}
	if (flags & 0x0200) { // colored?
		// The font color must not interfere with color specials. If the font color is not black,
		// all color specials should be ignored, i.e. glyphs of a non-black fonts have a fixed color
		// that can't be changed by color specials.
		UInt32 rgba = readUnsigned(4);
		color.setRGB(UInt8(rgba >> 24), UInt8((rgba >> 16) & 0xff), UInt8((rgba >> 8) & 0xff));
	}
	if (flags & 0x1000)   // extend?
		style.extend = _dvi2bp*readSigned(4);
	if (flags & 0x2000)   // slant?
		style.slant = _dvi2bp*readSigned(4);
	if (flags & 0x4000)   // embolden?
		style.bold = _dvi2bp*readSigned(4);
	if ((flags & 0x0800) && (getDVIFormat() == DVI_XDVOLD)) { // variations?
		UInt16 num_variations = readSigned(2);
		for (int i=0; i < num_variations; i++)
			readUnsigned(4);
	}
	if (_inPage)
		FontManager::instance().registerFont(fontnum, fontname, fontIndex, ptsize, style, color);
}


/** XDV extension: prints an array of characters where each character
 *  can take independent x and y coordinates.
 *  parameters: w[4] n[2] x[4][n] y[4][n] c[2][n] */
void DVIReader::cmdXGlyphArray (int) {
	putGlyphArray(false);
}


/** XDV extension: prints an array/string of characters where each character
 *  can take independent x coordinates whereas all share a single y coordinate.
 *  parameters: w[4] n[2] x[4][n] y[4] c[2][n] */
void DVIReader::cmdXGlyphString (int) {
	putGlyphArray(true);
}


/** Implements the common functionality of cmdXGlyphA and cmdXGlyphS.
 *  @param[in] xonly indicates if the characters share a single y coordinate (xonly==true) */
void DVIReader::putGlyphArray (bool xonly) {
	double strwidth = _dvi2bp*readSigned(4);
	UInt16 num_glyphs = readUnsigned(2);
	vector<Int32> x(num_glyphs);
	vector<Int32> y(num_glyphs);
	for (int i=0; i < num_glyphs; i++) {
		x[i] = readSigned(4);
		y[i] = xonly ? 0 : readSigned(4);
	}
	if (!_actions)
		seek(2*num_glyphs, ios::cur);
	else {
		if (Font *font = FontManager::instance().getFont(_currFontNum)) {
			for (int i=0; i < num_glyphs; i++) {
				UInt16 glyph_index = readUnsigned(2);
				double xx = _dviState.h + x[i]*_dvi2bp + _tx;
				double yy = _dviState.v + y[i]*_dvi2bp + _ty;
				_actions->setChar(xx, yy, glyph_index, false, font);
			}
		}
	}
	moveRight(strwidth);
}
