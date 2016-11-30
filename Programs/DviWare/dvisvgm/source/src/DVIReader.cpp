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
#include "Color.hpp"
#include "DVIActions.hpp"
#include "DVIReader.hpp"
#include "Font.hpp"
#include "FontManager.hpp"
#include "VectorStream.hpp"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;

DVIReader::DVIReader (istream &is) : BasicDVIReader(is)
{
	_inPage = false;
	_dvi2bp = 0.0;
	_inPostamble = false;
	_currFontNum = 0;
	_currPageNum = 0;
	_mag = 1;
	executePreamble();
	collectBopOffsets();
	executePostamble();
}


int DVIReader::executeCommand () {
	int opcode = BasicDVIReader::executeCommand();
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

	seek(_bopOffsets[n-1]); // goto bop of n-th page
	_inPostamble = false;   // not in postamble
	_currPageNum = n;
	while (executeCommand() != OP_EOP);
	return true;
}


void DVIReader::executePreamble () {
	clearStream();
	if (isStreamValid()) {
		seek(0);
		if (readByte() == OP_PRE) {
			cmdPre(0);
			return;
		}
	}
	throw DVIException("invalid DVI file");
}


/** Moves stream pointer to begin of postamble */
void DVIReader::goToPostamble () {
	clearStream();
	if (!isStreamValid())
		throw DVIException("invalid DVI file");

	seek(-1, ios::end);  // stream pointer to last byte
	int count=0;
	while (peek() == DVI_FILL) {   // skip fill bytes
		seek(-1, ios::cur);
		count++;
	}
	if (count < 4)  // the standard requires at least 4 trailing fill bytes
		throw DVIException("missing fill bytes at end of file");

	seek(-4, ios::cur);            // now at first byte of q (pointer to begin of postamble)
	uint32_t q = readUnsigned(4);  // pointer to begin of postamble
	seek(q);                       // now at begin of postamble
}


/** Reads and executes the commands of the postamble. */
void DVIReader::executePostamble () {
	goToPostamble();
	while (executeCommand() != OP_POSTPOST);  // executes all commands until post_post (= 249) is reached
}


/** Collects and records the file offsets of all bop commands. */
void DVIReader::collectBopOffsets () {
	goToPostamble();
	_bopOffsets.push_back(tell());      // also add offset of postamble
	readByte();                         // skip post command
	uint32_t offset = readUnsigned(4);  // offset of final bop
	while ((int32_t)offset > 0) {       // not yet on first bop?
		_bopOffsets.push_back(offset);   // record offset
		seek(offset+41);                 // skip bop command and the 10 \count values => now on offset of previous bop
		offset = readUnsigned(4);
	}
	reverse(_bopOffsets.begin(), _bopOffsets.end());
}

/////////////////////////////////////

/** Reads and executes DVI preamble command.
 *  Format: pre ver[1] num[4] den[4] mag[4] cmtlen[1] cmt[cmtlen] */
void DVIReader::cmdPre (int) {
	uint8_t id = readUnsigned(1);
	setDVIVersion(DVIVersion(id));     // identification number
	uint32_t numer = readUnsigned(4);  // numerator units of measurement
	uint32_t denom = readUnsigned(4);  // denominator units of measurement
	if (denom == 0)
		throw DVIException("denominator of measurement unit is zero");
	_mag = readUnsigned(4);        // magnification
	uint32_t k = readUnsigned(1);  // length of following comment
	string comment = readString(k);
	// 1 dviunit * num/den == multiples of 0.0000001m
	// 1 dviunit * _dvibp: length of 1 dviunit in PS points * _mag/1000
	_dvi2bp = numer/254000.0*72.0/denom*_mag/1000.0;
	dviPre(id, numer, denom, _mag, comment);
}


/** Reads and executes DVI postamble command.
 *  Format: post p[4] num[4] den[4] mag[4] ph[4] pw[4] sd[2] np[2] */
void DVIReader::cmdPost (int) {
	uint32_t prevBopOffset = readUnsigned(4);
	uint32_t numer = readUnsigned(4);
	uint32_t denom = readUnsigned(4);
	if (denom == 0)
		throw DVIException("denominator of measurement unit is zero");
	_mag = readUnsigned(4);
	uint32_t pageHeight = readUnsigned(4); // height of tallest page in dvi units
	uint32_t pageWidth  = readUnsigned(4); // width of widest page in dvi units
	uint16_t stackDepth = readUnsigned(2); // max. stack depth required
	uint16_t numPages = readUnsigned(2);
	if (numPages != numberOfPages())
		throw DVIException("page count in postamble doesn't match actual number of pages");

	// 1 dviunit * num/den == multiples of 0.0000001m
	// 1 dviunit * _dvi2bp: length of 1 dviunit in PS points * _mag/1000
	_dvi2bp = numer/254000.0*72.0/denom*_mag/1000.0;
	_inPostamble = true;
	dviPost(stackDepth, numPages, pageWidth*_dvi2bp, pageHeight*_dvi2bp, _mag, numer, denom, prevBopOffset);
}


/** Reads and executes DVI post_post command.
 *  Format: post_post q[4] i[1] 223[>=4] */
void DVIReader::cmdPostPost (int) {
	_inPostamble = false;
	uint32_t postOffset = readUnsigned(4);   // pointer to begin of postamble
	uint8_t id = readUnsigned(1);
	setDVIVersion(DVIVersion(id));   // identification byte
	while (readUnsigned(1) == DVI_FILL);  // skip fill bytes (223), eof bit should be set now
	dviPostPost(DVIVersion(id), postOffset);
}


/** Reads and executes Begin-Of-Page command.
 *  Format: bop c0[+4] ... c9[+4] p[+4] */
void DVIReader::cmdBop (int) {
	vector<int32_t> c(10);
	for (int i=0; i < 10; i++)
		c[i] = readSigned(4);
	int32_t prevBopOffset = readSigned(4);  // pointer to peceeding bop (-1 in case of first page)
	_dviState.reset();    // set all DVI registers to 0
	while (!_stateStack.empty())
		_stateStack.pop();
	_currFontNum = 0;
	_inPage = true;
	dviBop(c, prevBopOffset);
}


/** Reads and executes End-Of-Page command. */
void DVIReader::cmdEop (int) {
	if (!_stateStack.empty())
		throw DVIException("stack not empty at end of page");
	_inPage = false;
	dviEop();
}


/** Reads and executes push command. */
void DVIReader::cmdPush (int) {
	_stateStack.push(_dviState);
	dviPush();
}


/** Reads and executes pop command (restores pushed position information). */
void DVIReader::cmdPop (int) {
	if (_stateStack.empty())
		throw DVIException("stack empty at pop command");
	_dviState = _stateStack.top();
	_stateStack.pop();
	dviPop();
}


/** Helper function that handles charaters from virtual fonts (VF).
 *  It is called by the cmdSetChar and cmdPutChar methods.
 *  @param[in] font current font (corresponding to _currFontNum)
 *  @param[in] c character to typeset */
void DVIReader::putVFChar (Font *font, uint32_t c) {
	if (VirtualFont *vf = dynamic_cast<VirtualFont*>(font)) { // is current font a virtual font?
		if (const vector<uint8_t> *dvi = vf->getDVI(c)) { // try to get DVI snippet that represents character c
			FontManager &fm = FontManager::instance();
			DVIState savedState = _dviState;  // save current cursor position
			_dviState.x = _dviState.y = _dviState.w = _dviState.z = 0;
			int savedFontNum = _currFontNum; // save current font number
			fm.enterVF(vf);                  // enter VF font number context
			setFont(fm.vfFirstFontNum(vf), SetFontMode::VF_ENTER);
			double savedScale = _dvi2bp;
			// DVI units in virtual fonts are multiples of 1^(-20) times the scaled size of the VF
			_dvi2bp = vf->scaledSize()/(1 << 20);
			VectorInputStream<uint8_t> vis(*dvi);
			istream &is = replaceStream(vis);
			try {
				executeAll();  // execute DVI fragment
			}
			catch (const DVIException &e) {
				// Message::estream(true) << "invalid dvi in vf: " << e.getMessage() << endl; // @@
			}
			replaceStream(is);     // restore previous input stream
			_dvi2bp = savedScale;  // restore previous scale factor
			fm.leaveVF();          // restore previous font number context
			setFont(savedFontNum, SetFontMode::VF_LEAVE);  // restore previous font number
			_dviState = savedState;  // restore previous cursor position
		}
	}
}


/** Reads and executes set_char_x command. Puts a character at the current
 *  position and advances the cursor.
 *  @param[in] c character to set
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdSetChar0 (int c) {
	if (!_inPage)
		throw DVIException("setchar outside of page");
	Font *font = FontManager::instance().getFont(_currFontNum);
	dviSetChar0(c, font); // template method that may trigger further actions
	putVFChar(font, c);   // further character processing if current font is a virtual font
	moveRight(font->charWidth(c)*font->scaleFactor()*_mag/1000.0);
}


/** Reads and executes setx command. Puts a character at the current
 *  position and advances the cursor.
 *  @param[in] len number of parameter bytes (possible values: 1-4)
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdSetChar (int len) {
	if (!_inPage)
		throw DVIException("setchar outside of page");
	// According to the dvi specification all character codes are unsigned
	// except len == 4. At the moment all char codes are treated as unsigned...
	uint32_t c = readUnsigned(len); // if len == 4 c may be signed
	Font *font = FontManager::instance().getFont(_currFontNum);
	dviSetChar(c, font); // template method that may trigger further actions
	putVFChar(font, c);  // further character processing if current font is a virtual font
	moveRight(font->charWidth(c)*font->scaleFactor()*_mag/1000.0);
}


/** Reads and executes putx command. Puts a character at the current
 *  position but doesn't change the cursor position.
 *  @param[in] len number of parameter bytes (possible values: 1-4)
 *  @throw DVIException if method is called ouside a bop/eop pair */
void DVIReader::cmdPutChar (int len) {
	if (!_inPage)
		throw DVIException("putchar outside of page");
	// According to the dvi specification all character codes are unsigned
	// except len == 4. At the moment all char codes are treated as unsigned...
	int32_t c = readUnsigned(len);
	Font *font = FontManager::instance().getFont(_currFontNum);
	dviPutChar(c, font);
	putVFChar(font, c);
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
	dviSetRule(height, width);
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
	dviPutRule(height, width);
}


void DVIReader::moveRight (double dx) {
	switch (_dviState.d) {
		case WritingMode::LR: _dviState.h += dx; break;
		case WritingMode::TB: _dviState.v += dx; break;
		case WritingMode::BT: _dviState.v -= dx; break;
	}
}


void DVIReader::moveDown (double dy) {
	switch (_dviState.d) {
		case WritingMode::LR: _dviState.v += dy; break;
		case WritingMode::TB: _dviState.h -= dy; break;
		case WritingMode::BT: _dviState.h += dy; break;
	}
}


void DVIReader::cmdRight (int len) {
	double dx = _dvi2bp*readSigned(len);
	moveRight(dx);
	dviRight(dx);
}


void DVIReader::cmdDown (int len) {
	double dy = _dvi2bp*readSigned(len);
	moveDown(dy);
	dviDown(dy);
}


void DVIReader::cmdNop (int) {dviNop();}
void DVIReader::cmdX0 (int)  {moveRight(_dviState.x); dviX0();}
void DVIReader::cmdY0 (int)  {moveDown(_dviState.y); dviY0();}
void DVIReader::cmdW0 (int)  {moveRight(_dviState.w); dviW0();}
void DVIReader::cmdZ0 (int)  {moveDown(_dviState.z); dviZ0();}


void DVIReader::cmdX (int len) {
	double dx = _dvi2bp*readSigned(len);
	_dviState.x = dx;
	moveRight(dx);
	dviX(dx);
}


void DVIReader::cmdY (int len) {
	double dy = _dvi2bp*readSigned(len);
	_dviState.y = dy;
	moveDown(dy);
	dviY(dy);
}


void DVIReader::cmdW (int len) {
	double dx = _dvi2bp*readSigned(len);
	_dviState.w = dx;
	moveRight(dx);
	dviW(dx);
}


void DVIReader::cmdZ (int len) {
	double dy = _dvi2bp*readSigned(len);
	_dviState.z = dy;
	moveDown(dy);
	dviZ(dy);
}


/** Sets the text orientation (horizontal, vertical).
 *  This command is only available in DVI version 3 (created by pTeX) */
void DVIReader::cmdDir (int) {
	uint8_t wmode = readUnsigned(1);
	if (wmode == 4)  // yoko mode (4) equals default LR mode (0)
		wmode = 0;
	if (wmode == 2 || wmode > 3) {
		ostringstream oss;
		oss << "invalid writing mode value " << wmode << " (0, 1, 3, or 4 expected)";
		throw DVIException(oss.str());
	}
	_dviState.d = (WritingMode)wmode;
	dviDir(_dviState.d);
}


void DVIReader::cmdXXX (int len) {
	if (!_inPage)
		throw DVIException("special outside of page");
	uint32_t numBytes = readUnsigned(len);
	string str = readString(numBytes);
	dviXXX(str);
}


/** Change the current font.
 *  @param[in] fontnum local number of font to select
 *  @param[in] mode info on command that triggered the font change
 *  @throw DVIException if font number is undefined */
void DVIReader::setFont (int fontnum, SetFontMode mode) {
	if (const Font *font = FontManager::instance().getFont(fontnum)) {
		_currFontNum = fontnum;
		dviFontNum(uint32_t(fontnum), mode, font);
	}
	else {
		ostringstream oss;
		oss << "undefined font number " << fontnum;
		throw DVIException(oss.str());
	}
}


/** Selects a previously defined font by its number.
 *  @param[in] fontnum font number
 *  @throw DVIException if font number is undefined */
void DVIReader::cmdFontNum0 (int fontnum) {
	setFont(fontnum, SetFontMode::SF_SHORT);
}


/** Selects a previously defined font.
 *  @param[in] len size of font number variable (in bytes)
 *  @throw DVIException if font number is undefined */
void DVIReader::cmdFontNum (int len) {
	uint32_t fontnum = readUnsigned(len);
	setFont(fontnum, SetFontMode::SF_LONG);
}


/** Helper function to handle a font definition.
 *  @param[in] fontnum local font number
 *  @param[in] name font name
 *  @param[in] cs checksum to be compared with TFM checksum
 *  @param[in] ds design size in PS point units
 *  @param[in] ss scaled size in PS point units */
const Font* DVIReader::defineFont (uint32_t fontnum, const string &name, uint32_t cs, double ds, double ss) {
	FontManager &fm = FontManager::instance();
	Font *font = fm.getFont(fontnum);
	if (!font) {
		int id = fm.registerFont(fontnum, name, cs, ds, ss);
		font = fm.getFontById(id);
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
	}
	return font;
}


/** Defines a new font.
 *  @param[in] len size of font number variable (in bytes) */
void DVIReader::cmdFontDef (int len) {
	uint32_t fontnum  = readUnsigned(len);    // font number
	uint32_t checksum = readUnsigned(4);      // font checksum (to be compared with corresponding TFM checksum)
	uint32_t ssize    = readUnsigned(4);      // scaled size of font in DVI units
	uint32_t dsize    = readUnsigned(4);      // design size of font in DVI units
	uint32_t pathlen  = readUnsigned(1);      // length of font path
	uint32_t namelen  = readUnsigned(1);      // length of font name
	string path       = readString(pathlen);  // path to font file
	string fontname   = readString(namelen);
	const Font *font  = defineFont(fontnum, fontname, checksum, dsize*_dvi2bp, ssize*_dvi2bp);
	dviFontDef(fontnum, checksum, font);
}


/** This template method is called by the VFReader after reading a font definition from a VF file.
 *  @param[in] fontnum local font number
 *  @param[in] path path to font file
 *  @param[in] name font name
 *  @param[in] checksum checksum to be compared with TFM checksum
 *  @param[in] dsize design size in PS point units
 *  @param[in] ssize scaled size in PS point units */
void DVIReader::defineVFFont (uint32_t fontnum, const string &path, const string &name, uint32_t checksum, double dsize, double ssize) {
	if (const VirtualFont *vf = FontManager::instance().getVF())
		defineFont(fontnum, name, checksum, dsize, ssize * vf->scaleFactor());
}


/** This template method is called by the VFReader after reading a character definition from a VF file.
 *  @param[in] c character number
 *  @param[in] dvi DVI fragment describing the character */
void DVIReader::defineVFChar (uint32_t c, vector<uint8_t> &&dvi) {
	FontManager::instance().assignVFChar(c, std::move(dvi));
}


/** XDV extension: includes image or pdf file.
 *  parameters: box[1] matrix[4][6] p[2] len[2] path[l] */
void DVIReader::cmdXPic (int) {
	uint8_t box = readUnsigned(1);    // box
	vector<int32_t> matrix(6);
	for (int i=0; i < 6; i++)       // matrix
		matrix[i] = readSigned(4);
	int16_t page = readSigned(2);     // page number
	uint16_t len = readUnsigned(2);
	string path = readString(len);  // path to image/pdf file
	dviXPic(box, matrix, page, path);
}


/** XDV extension: defines a native font */
void DVIReader::cmdXFontDef (int) {
	int32_t fontnum = readSigned(4);
	double ptsize = _dvi2bp*readUnsigned(4);
	uint16_t flags = readUnsigned(2);
	uint8_t psname_len = readUnsigned(1);
	uint8_t fmname_len = getDVIVersion() == DVI_XDV5 ? readUnsigned(1) : 0;  // length of family name
	uint8_t stname_len = getDVIVersion() == DVI_XDV5 ? readUnsigned(1) : 0;  // length of style name
	string fontname = readString(psname_len);
	uint32_t fontIndex=0;
	if (getDVIVersion() == DVI_XDV5)
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
		uint32_t rgba = readUnsigned(4);
		color.setRGB(uint8_t(rgba >> 24), uint8_t((rgba >> 16) & 0xff), uint8_t((rgba >> 8) & 0xff));
	}
	if (flags & 0x1000)   // extend?
		style.extend = _dvi2bp*readSigned(4);
	if (flags & 0x2000)   // slant?
		style.slant = _dvi2bp*readSigned(4);
	if (flags & 0x4000)   // embolden?
		style.bold = _dvi2bp*readSigned(4);
	if ((flags & 0x0800) && (getDVIVersion() == DVI_XDV5)) { // variations?
		uint16_t num_variations = readSigned(2);
		for (int i=0; i < num_variations; i++)
			readUnsigned(4);
	}
	const Font *font = FontManager::instance().getFont(fontnum);
	if (!font) {
		FontManager::instance().registerFont(fontnum, fontname, fontIndex, ptsize, style, color);
		font = FontManager::instance().getFont(fontnum);
	}
	dviXFontDef(fontnum, dynamic_cast<const NativeFont*>(font));
}


/** XDV extension: prints an array of characters where each character
 *  can take independent x and y coordinates.
 *  parameters: w[4] n[2] (dx,dy)[(4+4)n] glyphs[2n] */
void DVIReader::cmdXGlyphArray (int) {
	vector<double> dx, dy;
	vector<uint16_t> glyphs;
	double width = putGlyphArray(false, dx, dy, glyphs);
	if (Font *font = FontManager::instance().getFont(_currFontNum))
		dviXGlyphArray(dx, dy, glyphs, *font);
	else
		throw DVIException("missing setfont prior to xglypharray");
	moveRight(width);
}


/** XDV extension: prints an array/string of characters where each character
 *  can take independent x coordinates whereas all share a single y coordinate.
 *  parameters: w[4] n[2] dx[4n] dy[4] glyphs[2n] */
void DVIReader::cmdXGlyphString (int) {
	vector<double> dx, dy;
	vector<uint16_t> glyphs;
	double width = putGlyphArray(true, dx, dy, glyphs);
	if (Font *font = FontManager::instance().getFont(_currFontNum))
		dviXGlyphString(dx, glyphs, *font);
	else
		throw DVIException("missing setfont prior to xglyphstring");
	moveRight(width);
}


/** XDV extension: Same as cmdXGlyphArray plus a leading array of UTF-16 characters
 *  that specify the "actual text" represented by the glyphs to be printed. It usually
 *  contains the text with special characters (like ligatures) expanded so that it
 *  can be used for text search, plain text copy & paste etc. This XDV command was
 *  introduced with XeTeX 0.99995 and can be triggered by <tt>\\XeTeXgenerateactualtext1</tt>.
 *  parameters: l[2] chars[2l] w[4] n[2] (dx,dy)[8n] glyphs[2n] */
void DVIReader::cmdXTextAndGlyphs (int) {
	uint16_t l = readUnsigned(2);  // number of chars
	vector<uint16_t> chars(l);
	for (int i=0; i < l; i++)
		chars[i] = readUnsigned(2);
	vector<double> x, y;
	vector<uint16_t> glyphs;
	double width = putGlyphArray(false, x, y, glyphs);
	if (Font *font = FontManager::instance().getFont(_currFontNum))
		dviXTextAndGlyphs(x, y, chars, glyphs, *font);
	else
		throw DVIException("missing setfont prior to xtextandglyphs");
	moveRight(width);
}


/** Implements the common functionality of cmdXGlyphArray, cmdXGlyphString, and cmdXTextAndGlyphs.
 *  @param[in] xonly indicates if the characters share a single y coordinate (xonly==true)
 *  @param[out] dx relative horizontal positions of each glyph
 *  @param[out] dy relative vertical positions of each glyph
 *  @param[out] glyphs FreeType indices of the glyphs to typeset
 *  @return total width of the glyph array */
double DVIReader::putGlyphArray (bool xonly, vector<double> &dx, vector<double> &dy, vector<uint16_t> &glyphs) {
	double strwidth = _dvi2bp*readSigned(4);
	uint16_t num_glyphs = readUnsigned(2);
	dx.resize(num_glyphs);
	dy.resize(num_glyphs);
	glyphs.resize(num_glyphs);
	for (int i=0; i < num_glyphs; i++) {
		dx[i] = readSigned(4)*_dvi2bp;
		dy[i] = xonly ? 0 : readSigned(4)*_dvi2bp;
	}
	for (int i=0; i < num_glyphs; i++)
		glyphs[i] = readUnsigned(2);
	return strwidth;
}
