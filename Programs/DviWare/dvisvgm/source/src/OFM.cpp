/*************************************************************************
** OFM.cpp                                                              **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <cstdint>
#include <string>
#include "Length.hpp"
#include "OFM.hpp"

using namespace std;


static uint32_t read_uint32 (StreamReader &reader, const string &name) {
	uint32_t quad = reader.readUnsigned(4);
	if (quad & 0x80000000)
		throw FontMetricException("negative '"+name+"' value not permitted");
	return quad;
}

void OFM::FileDimensions::read (StreamReader &reader) {
	lf = read_uint32(reader, "lf");
	lh = read_uint32(reader, "lh");
	bc = read_uint32(reader, "bc");
	ec = read_uint32(reader, "ec");
	nw = read_uint32(reader, "nw");
	nh = read_uint32(reader, "nh");
	nd = read_uint32(reader, "nd");
	ni = read_uint32(reader, "ni");
	nl = read_uint32(reader, "nl");
	nk = read_uint32(reader, "nk");
	ne = read_uint32(reader, "ne");
	np = read_uint32(reader, "np");
	dir = read_uint32(reader, "font-dir");
}


/** Reads the common beginning of OFM level 0 and 1 files, and initializes
 *  the corresponding members.
 *  @param[in] is reads from this stream
 *  @param[in] maxNumWidths maximum number of width entries
 *  @return the collected data (file offsets, table sizes, writing direction) */
OFM::FileDimensions OFM::read (istream &is, uint32_t maxNumWidths) {
	OFM::FileDimensions dim;
	if (is) {
		is.seekg(0);
		StreamReader sr(is);
		uint32_t ofmLevel = sr.readUnsigned(4);
		if (ofmLevel > 1)
			throw FontMetricException("OFM level "+to_string(ofmLevel)+" not supported");
		if (int(ofmLevel) != level())
			throw FontMetricException("OFM level "+to_string(level())+" expected but found level "+to_string(ofmLevel));
		dim.read(sr);
		if (dim.nw > maxNumWidths)
			throw FontMetricException("invalid size of width table");
		_firstChar = dim.bc;  // smallest character code in font
		_lastChar = dim.ec;   // largest character code in font
		_vertical = (dim.dir > 0);
	}
	return dim;
}


/** Reads and stores the required data from the OFM header. */
void OFM::readHeader (StreamReader &reader) {
	_checksum = reader.readUnsigned(4);
	_designSize = double(FixWord(reader.readSigned(4)))*Length::pt2bp;
}


/** Reads a sequence of values and stores them in a vector.
 *  @param[in] reader the TFM data is read from this object
 *  @param[out] words vector taking the read words
 *  @param[in] n number of entries to read */
static void read_words (StreamReader &reader, vector<FixWord> &words, size_t n) {
	words.resize(n);
	for (size_t i=0; i < n; i++)
		words[i] = reader.readSigned(4);
}


/** Reads and stores the required OFM tables (characters widths, heights, depths, ...). */
void OFM::readTables (StreamReader &reader, const FileDimensions &dim) {
	read_words(reader, _widthTable, dim.nw);
	read_words(reader, _heightTable, dim.nh);
	read_words(reader, _depthTable, dim.nd);
	read_words(reader, _italicTable, dim.ni);
	reader.skip(8*dim.nl + 4*dim.nk + 8*dim.ne);
	read_words(reader, _params, min(dim.np, 7u));  // we only need params 0-6
	_params.resize(7, 0);   // ensure 7 parameters
	for (FixWord h : _heightTable)
		_ascent = max(_ascent, h);
	for (FixWord d : _depthTable)
		_descent = max(_descent, d);
}


/** Returns the character index for a given character or -1 if
 *  the OFM file doesn't contain information on the character. */
size_t OFM::charIndex (int c) const {
	uint32_t chr=c;
	if (chr < _firstChar || chr > _lastChar || size_t(chr-_firstChar) >= numCharInfos())
		return -1;
	return chr-_firstChar;
}


/** Returns the width of a character in bp units. */
double OFM::getCharWidth (int c) const {
	auto cindex = charIndex(c);
	if (cindex == size_t(-1))
		return 0;
	return double(_widthTable[widthIndex(cindex)])*_designSize;
}


/** Returns the height of a character in bp units. */
double OFM::getCharHeight (int c) const {
	auto cindex = charIndex(c);
	if (cindex == size_t(-1))
		return 0;
	return double(_heightTable[heightIndex(cindex)])*_designSize;
}


/** Returns the depth of a character in bp units. */
double OFM::getCharDepth (int c) const {
	auto cindex = charIndex(c);
	if (cindex == size_t(-1))
		return 0;
	return double(_depthTable[depthIndex(cindex)])*_designSize;
}


/** Returns the italic correction width of a character in bp units. */
double OFM::getItalicCorr (int c) const {
	auto cindex = charIndex(c);
	if (cindex == size_t(-1))
		return 0;
	return double(_italicTable[italicIndex(cindex)])*_designSize;
}

/** Returns the optimal space width between words in bp units. */
double OFM::getSpace () const {
	return _params.empty() ? 0 : double(_params[1])*_designSize;
}


/** Returns the amount of glue stretching between words in bp units. */
double OFM::getSpaceStretch () const {
	return _params.empty() ? 0 : double(_params[2])*_designSize;
}


/** Returns the amount of glue shrinking between words in bp units. */
double OFM::getSpaceShrink () const {
	return _params.empty() ? 0 : double(_params[3])*_designSize;
}


/** Returns the size of one EM unit in bp units. */
double OFM::getQuad () const {
	if (_params.empty() || _params[5] == 0)
		return _designSize;
	return double(_params[5])*_designSize;
}

/////////////////////////////////////////////////////////////////////////////

/** Reads and stores the required data from an OFM level 0 data stream. */
void OFM0::read (std::istream &is) {
	auto dim = OFM::read(is, 0xFFFF);
	if (_firstChar >= _lastChar || _lastChar > 0xFFFF)
		throw FontMetricException("character codes out of range");
	if (14 + dim.lh + 2*(dim.ec-dim.bc+1) + dim.nw + dim.nh + dim.nd + dim.ni +2*dim.nl + dim.nk + 2*dim.ne + dim.np != dim.lf)
		throw FontMetricException("inconsistent table size values");
	StreamReader sr(is);
	readHeader(sr);
	sr.seek(56 + dim.lh*4);  // move to char info table
	readCharInfos(sr);
	readTables(sr, dim);
}


/** Reads and stores the required data from the char-info section.
 *  Expects the stream pointer to be located on the first byte of the first char-info entry. */
void OFM0::readCharInfos (StreamReader &reader) {
	auto numChars = _lastChar-_firstChar+1;
	_charInfos.resize(numChars);
	for (size_t i=0; i < numChars; i++) {
		_charInfos[i] = uint64_t(reader.readUnsigned(4)) << 32;
		_charInfos[i] |= reader.readUnsigned(4);
	}
}

////////////////////////////////////////////////////////////////////////////////

/** Reads and stores the required data from an OFM level 1 data stream. */
void OFM1::read (std::istream &is) {
	auto dim = OFM::read(is, 0x110000);
	if (_firstChar >= _lastChar || _lastChar > 0x10FFFF)
		throw FontMetricException("character codes out of range");
	StreamReader sr(is);
	uint32_t nco = sr.readUnsigned(4);  // OFM1: offset of char-info table (in 4-byte words)
	_ncw = sr.readUnsigned(4);  // total number of character info words in OFM file
	_npc = sr.readUnsigned(4);  // number of parameters per character
	// check validity of number entries for new (OFM level 1) table types
	size_t sum=0, numTables=0;
	for (int i=0; i < 12; i++) {
		uint32_t num = sr.readUnsigned(4);
		sum += num;
		if (i%2 == 0)
			numTables += num;
	}
	if (numTables > 32)
		throw FontMetricException("too many OFM level 1 tables");
	if (29 + dim.lh + _ncw + dim.nw + dim.nh + dim.nd + dim.ni +2*dim.nl + dim.nk + 2*dim.ne + dim.np + sum != dim.lf)
		throw FontMetricException("inconsistent table size values");
	readHeader(sr);
	sr.seek(4*nco);  // move to char info table
	readCharInfos(sr);
	readTables(sr, dim);
}


/** Reads and stores the required data from the char-info section.
 *  Expects the stream pointer to be located on the first byte of the first char-info entry. */
void OFM1::readCharInfos (StreamReader &reader) {
	auto numCharInfos = _ncw/(3 + _npc/2);  // number of char info entries in file
	auto numChars = _lastChar-_firstChar+1;
	size_t numCharsRead=0;
	_charInfos.reserve(numChars);
	while (numCharsRead <= numChars && numCharInfos > 0) {
		CharInfo info;
		info.widthIndex = uint16_t(reader.readUnsigned(2));
		info.heightIndex = uint8_t(reader.readUnsigned(1));
		info.depthIndex = uint8_t(reader.readUnsigned(1));
		info.italicIndex = uint8_t(reader.readUnsigned(1));
		reader.skip(3);  // skip RFU, ext-tag, tag, and remainder
		auto repeats = uint16_t(reader.readUnsigned(2));
		if (numCharsRead + repeats > numChars)
			throw FontMetricException("number of characters exceeded due to too many char-info repeats");
		reader.skip(_npc*2);  // skip parameters
		if (_npc%2 == 0)
			reader.skip(2);    // skip padding bytes
		for (int j=0; j <= repeats; j++)
			_charInfos.push_back(info);
		numCharsRead += repeats+1;
		numCharInfos--;
	}
	if (numCharInfos > 0)
		throw FontMetricException("more char-info entries present than characters");
	if (numCharsRead < numChars)
		throw FontMetricException("not all characters covered by char-info entries");
}
