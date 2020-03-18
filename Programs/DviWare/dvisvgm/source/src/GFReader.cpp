/*************************************************************************
** GFReader.cpp                                                         **
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

#include <algorithm>
#include <sstream>
#include "FixWord.hpp"
#include "GFReader.hpp"
#include "Length.hpp"
#include "SignalHandler.hpp"

using namespace std;


struct GFCommand {
	void (GFReader::*method)(int);
	int numBytes;
};


/** Converts a scaled value to double */
static inline double scaled2double (int32_t scaled) {
	return double(scaled)/(1 << 16);
}


uint32_t GFReader::readUnsigned (int bytes) {
	uint32_t ret = 0;
	for (int i=bytes-1; i >= 0 && !_in.eof(); i--) {
		uint32_t b = _in.get();
		ret |= b << (8*i);
	}
	return ret;
}


int32_t GFReader::readSigned (int bytes) {
	uint32_t ret = _in.get();
	if (ret & 128)        // negative value?
		ret |= 0xffffff00;
	for (int i=bytes-2; i >= 0 && !_in.eof(); i--)
		ret = (ret << 8) | _in.get();
	return int32_t(ret);
}


string GFReader::readString (int bytes) {
	bytes = max(0, bytes);
	string str(bytes, '\0');
	_in.read(&str[0], bytes);
	return str;
}


/** Reads a single GF command from the current position of the input stream and calls the
 *  corresponding cmdFOO method.
 *  @return opcode of the executed command */
int GFReader::executeCommand () {
	SignalHandler::instance().check();
	/* Each cmdFOO command reads the necessary number of bytes from the stream so executeCommand
	doesn't need to know the exact GF command format. Some cmdFOO methods are used for multiple
	GF commands because they only differ in the size of their parameters. */
	static const GFCommand commands[] = {
		{&GFReader::cmdPaint, 1}, {&GFReader::cmdPaint, 2}, {&GFReader::cmdPaint, 3},                        // 64-66
		{&GFReader::cmdBoc, 0},   {&GFReader::cmdBoc1, 0},                                                   // 67-68
		{&GFReader::cmdEoc, 0},                                                                              // 69
		{&GFReader::cmdSkip, 0},  {&GFReader::cmdSkip, 1},  {&GFReader::cmdSkip, 2},{&GFReader::cmdSkip, 3}, // 70-73
		{&GFReader::cmdXXX, 1},   {&GFReader::cmdXXX, 2},   {&GFReader::cmdXXX, 3}, {&GFReader::cmdXXX, 4},  // 239-242
		{&GFReader::cmdYYY, 0},                                                                              // 243
		{&GFReader::cmdNop, 0},                                                                              // 244
		{&GFReader::cmdCharLoc, 0}, {&GFReader::cmdCharLoc0, 0},                                             // 245-246
		{&GFReader::cmdPre, 0},     {&GFReader::cmdPost, 0}, {&GFReader::cmdPostPost, 0}                     // 247-249
	};

	int opcode = _in.get();
	if (opcode < 0)  // at end of file?
		throw GFException("unexpected end of file");

	if (opcode >= 0 && opcode <= 63)
		cmdPaint0(opcode);
	else if (opcode >= 74 && opcode <= 238)
		cmdNewRow(opcode-74);
	else if (opcode >= 250)
		throw GFException("undefined GF command (opcode " + std::to_string(opcode) + ")");
	else {
		int offset = opcode <= 73 ? 64 : 239-(73-64+1);
		const GFCommand &cmd = commands[opcode-offset];
		if (cmd.method)
			(this->*cmd.method)(cmd.numBytes);
	}
	return opcode;
}


bool GFReader::executeChar (uint8_t c) {
	_in.clear();
	if (_charInfoMap.empty())
		executePostamble();          // read character info
	_in.clear();
	auto it = _charInfoMap.find(c);
	if (_in && it != _charInfoMap.end()) {
		_in.seekg(it->second.location);
		while (executeCommand() != 69);  // execute all commands until eoc is reached
		return true;
	}
	return false;
}


bool GFReader::executeAllChars () {
	_in.clear();
	if (_charInfoMap.empty())
		executePostamble();   // read character info
	_in.clear();
	if (_in) {
		_in.seekg(0);
		while (executeCommand() != 248); // execute all commands until postamble is reached
		return true;
	}
	return false;
}


bool GFReader::executePreamble () {
	_in.clear();
	if (!_in)
		return false;
	_in.seekg(0);
	executeCommand();
	return true;
}


bool GFReader::executePostamble () {
	_in.clear();
	if (!_in)
		return false;
	_in.seekg(-1, ios::end);
	while (_in.peek() == 223)        // skip fill bytes
		_in.seekg(-1, ios::cur);
	if (_in.peek() != 131)
		throw GFException("invalid identification byte in postpost");
	_in.seekg(-5, ios::cur);         // now on postpost
	if (_in.get() != 249)
		throw GFException("invalid GF file");
	uint32_t q = readUnsigned(4);    // pointer to begin of postamble
	_in.seekg(q);                    // now on begin of postamble
	while (executeCommand() != 249); // execute all commands until postpost is reached
	return true;
}


/** Returns the width of character c in PS point units */
double GFReader::getCharWidth (uint32_t c) const {
	auto it = _charInfoMap.find(c%256);
	return it == _charInfoMap.end() ? 0 : it->second.width*getDesignSize()/(1<<24);
}

///////////////////////


/** Reads the preamble. */
void GFReader::cmdPre (int) {
	uint32_t i = readUnsigned(1);
	if (i != 131)
		throw GFException("invalid identification byte in preamble");
	else {
		uint32_t k = readUnsigned(1);
		string s = readString(k);
		preamble(s);
	}
}


/** Reads the postamble. */
void GFReader::cmdPost (int) {
	readUnsigned(4);                // pointer to byte after final eoc
	_designSize = double(FixWord(readUnsigned(4)))*Length::pt2bp; // design size of font in points
	_checksum   = readUnsigned(4);  // checksum
	_hppp       = scaled2double(readUnsigned(4))/Length::pt2bp; // horizontal pixels per PS point
	_vppp       = scaled2double(readUnsigned(4))/Length::pt2bp; // vertical pixels per PS point
	_in.seekg(16, ios::cur);        // skip x and y bounds
	postamble();
}


/** Reads trailing bytes at end of stream. */
void GFReader::cmdPostPost (int) {
	readUnsigned(4);   // pointer to begin of postamble
	uint32_t i = readUnsigned(1);
	if (i != 131)
		throw GFException("invalid identification byte in postpost");
	while (readUnsigned(1) == 223); // skip fill bytes
}


/** Inverts "paint color" (black to white or vice versa) of n pixels
 *  and advances the cursor by n.
 *  @param[in] n number of pixels to be inverted */
void GFReader::cmdPaint0 (int n) {
	if (!_insideCharDef)
		throw GFException("character-related command outside BOC and EOC");
	if (_penDown)  // set pixels?
		_bitmap.setBits(_y, _x, n);
	_x += n;
	_penDown = !_penDown;  // invert pen state
}


/** Inverts "paint color" (black to white or vice versa) of n pixels
 *  and advances the cursor by n. The number n of pixels is read from
 *  the input stream.
 *  @param[in] len size of n in bytes */
void GFReader::cmdPaint (int len) {
	uint32_t pixels = readUnsigned(len);
	cmdPaint0(pixels);
}


/** Beginning of character (generic format). */
void GFReader::cmdBoc (int) {
	_currentChar = readSigned(4);
	readSigned(4);  // pointer to previous boc with same c mod 256
	_minX = readSigned(4);
	_maxX = readSigned(4);
	_minY = readSigned(4);
	_maxY = readSigned(4);
	_x = _minX;
	_y = _maxY;
	_penDown = false;
	_bitmap.resize(_minX, _maxX, _minY, _maxY);
	_insideCharDef = true;
	beginChar(_currentChar);
}


/** Beginning of character (compact format). */
void GFReader::cmdBoc1 (int) {
	_currentChar = readUnsigned(1);
	uint32_t dx = readUnsigned(1);
	_maxX = readUnsigned(1);
	_minX = _maxX - dx;
	uint32_t dy = readUnsigned(1);
	_maxY = readUnsigned(1);
	_minY = _maxY - dy;
	_x = _minX;
	_y = _maxY;
	_penDown = false;
	_bitmap.resize(_minX, _maxX, _minY, _maxY);
	_insideCharDef = true;
	beginChar(_currentChar);
}


/** End of character. */
void GFReader::cmdEoc (int) {
	if (!_insideCharDef)
		throw GFException("misplaced EOC");
	_insideCharDef = false;
	endChar(_currentChar);
}


/** Moves cursor to the beginning of a following row and sets
 *  paint color to white.
 *  @param[in] len if 0: move to next row, otherwise: number of bytes to read.
 *                 The read value denotes the number of rows to be skipped.  */
void GFReader::cmdSkip (int len) {
	if (!_insideCharDef)
		throw GFException("character-related command outside BOC and EOC");
	if (len == 0)
		_y--;
	else
		_y -= readUnsigned(len)+1;
	_x = _minX;
	_penDown = false;
}


/** Moves cursor to pixel number 'col' in the next row and sets
 *  the paint color to black.
 *  @param[in] col pixel/column number */
void GFReader::cmdNewRow (int col) {
	if (!_insideCharDef)
		throw GFException("character-related command outside BOC and EOC");
	_x = _minX + col ;
	_y--;
	_penDown = true;
}


void GFReader::cmdXXX (int len) {
	uint32_t n = readUnsigned(len);
	string str = readString(n);
	special(str);
}


void GFReader::cmdYYY (int) {
	int32_t y = readSigned(4);
	numspecial(y);
}


/** Does nothing. */
void GFReader::cmdNop (int) {
}


/** Reads character locator (part of postamble) */
void GFReader::cmdCharLoc0 (int) {
	uint8_t c  = readUnsigned(1); // character code mod 256
	uint8_t dm = readUnsigned(1); // = dx/65536
	int32_t w  = readSigned(4);   // (1<<24)*characterWidth/designSize
	int32_t p  = readSigned(4);   // pointer to begin of (last) character data
	int32_t dx = 65536*dm;        // horizontal escapement
	int32_t dy = 0;               // vertical escapement
	_charInfoMap.emplace(c, CharInfo(dx, dy, w, p));
}


/** Reads character locator (part of postamble) */
void GFReader::cmdCharLoc (int) {
	uint8_t c  = readUnsigned(1); // character code mod 256
	int32_t dx = readSigned(4);   // horizontal escapement (scaled pixel units)
	int32_t dy = readSigned(4);   // vertical escapement (scaled pixel units)
	int32_t w  = readSigned(4);   // (1<<24)*characterWidth/designSize
	int32_t p  = readSigned(4);   // pointer to begin of (last) character data
	_charInfoMap.emplace(c, CharInfo(dx, dy, w, p));
}
