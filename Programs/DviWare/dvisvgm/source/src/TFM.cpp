/*************************************************************************
** TFM.cpp                                                              **
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
#include <fstream>
#include <vector>
#include "FileFinder.h"
#include "Message.h"
#include "StreamReader.h"
#include "TFM.h"

using namespace std;


/** Reads a sequence of n TFM words (4 Bytes each).
 *  @param[in]  sr the TFM data is read from this object
 *  @param[out] v the read words
 *  @param[in]  n number of words to be read */
template <typename T>
static void read_words (StreamReader &sr, vector<T> &v, unsigned n) {
	v.clear();
	v.resize(n);
	for (unsigned i=0; i < n; i++)
		v[i] = sr.readUnsigned(4);
}


/** Converts a TFM fix point value to double (PS point units). */
static double fix2double (FixWord fix) {
	const double pt2bp = 72/72.27;
	return double(fix)/(1 << 20)*pt2bp;
}


TFM::TFM (istream &is) {
	is.seekg(0);
	StreamReader sr(is);
	UInt16 lf = UInt16(sr.readUnsigned(2)); // length of entire file in 4 byte words
	UInt16 lh = UInt16(sr.readUnsigned(2)); // length of header in 4 byte words
	_firstChar= UInt16(sr.readUnsigned(2)); // smallest character code in font
	_lastChar = UInt16(sr.readUnsigned(2)); // largest character code in font
	UInt16 nw = UInt16(sr.readUnsigned(2)); // number of words in width table
	UInt16 nh = UInt16(sr.readUnsigned(2)); // number of words in height table
	UInt16 nd = UInt16(sr.readUnsigned(2)); // number of words in depth table
	UInt16 ni = UInt16(sr.readUnsigned(2)); // number of words in italic corr. table
	UInt16 nl = UInt16(sr.readUnsigned(2)); // number of words in lig/kern table
	UInt16 nk = UInt16(sr.readUnsigned(2)); // number of words in kern table
	UInt16 ne = UInt16(sr.readUnsigned(2)); // number of words in ext. char table
	UInt16 np = UInt16(sr.readUnsigned(2)); // number of font parameter words

	if (6+lh+(_lastChar-_firstChar+1)+nw+nh+nd+ni+nl+nk+ne+np != lf)
		throw FontMetricException("inconsistent length values");
	if (_firstChar >= _lastChar || _lastChar > 255 || ne > 256)
		throw FontMetricException("character codes out of range");

	readHeader(sr);
	is.seekg(24+lh*4);  // move to char info table
	readTables(sr, nw, nh, nd, ni);
}


void TFM::readHeader (StreamReader &sr) {
	_checksum = sr.readUnsigned(4);
	_designSize = sr.readUnsigned(4);
}


void TFM::readTables (StreamReader &sr, int nw, int nh, int nd, int ni) {
	read_words(sr, _charInfoTable, _lastChar-_firstChar+1);
	read_words(sr, _widthTable, nw);
	read_words(sr, _heightTable, nh);
	read_words(sr, _depthTable, nd);
	read_words(sr, _italicTable, ni);
}


/** Returns the design size of this font in PS point units. */
double TFM::getDesignSize () const {
	return fix2double(_designSize);
}


/** Returns the index to the entry of the character info table that describes the metric of a given character.
 *  @param[in] c character whose index is retrieved
 *  @return table index for character c, or -1 if there's no entry */
int TFM::charIndex (int c) const {
	if (c < _firstChar || c > _lastChar || size_t(c-_firstChar) >= _charInfoTable.size())
		return -1;
	return c-_firstChar;
}


// the char info word for each character consists of 4 bytes holding the following information:
// width index w, height index (h), depth index (d), italic correction index (it),
// tag (tg) and a remainder:
//
// byte 1   | byte 2    | byte 3    | byte 4
// xxxxxxxx | xxxx xxxx | xxxxxx xx | xxxxxxxx
// w        | h    d    | it     tg | remainder

/** Returns the width of char c in PS point units. */
double TFM::getCharWidth (int c) const {
	int index = charIndex(c);
	if (index < 0)
		return 0;
	index = (_charInfoTable[index] >> 24) & 0xFF;
	return fix2double(_widthTable[index]) * fix2double(_designSize);
}


/** Returns the height of char c in PS point units. */
double TFM::getCharHeight (int c) const {
	int index = charIndex(c);
	if (index < 0)
		return 0;
	index = (_charInfoTable[index] >> 20) & 0x0F;
	return fix2double(_heightTable[index]) * fix2double(_designSize);
}


/** Returns the depth of char c in PS point units. */
double TFM::getCharDepth (int c) const {
	int index = charIndex(c);
	if (index < 0)
		return 0;
	index = (_charInfoTable[index] >> 16) & 0x0F;
	return fix2double(_depthTable[index]) * fix2double(_designSize);
}


/** Returns the italic correction of char c in PS point units. */
double TFM::getItalicCorr (int c) const {
	int index = charIndex(c);
	if (index < 0)
		return 0;
	index = (_charInfoTable[index] >> 10) & 0x3F;
	return fix2double(_italicTable[index]) * fix2double(_designSize);
}
