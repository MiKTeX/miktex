/*************************************************************************
** TTFTable.cpp                                                         **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <vectorstream.hpp>
#include <zlib.h>
#include "TTFTable.hpp"

using namespace std;
using namespace ttf;


TableBuffer::TableBuffer (uint32_t tag, vector<char> data)
	: _tag(tag), _unpaddedSize(data.size()), _compressedSize(_unpaddedSize), _data(std::move(data))
{
	_data.resize((_unpaddedSize+3) & ~3, 0);  // ensure data.size() % 4 == 0
	for (size_t i=0; i < paddedSize(); i+=4) {
		auto *bytes = reinterpret_cast<const uint8_t*>(_data.data());
		_checksum += bytes[i]*0x1000000 + bytes[i+1]*0x010000 + bytes[i+2]*0x0100 + bytes[i+3];
	}
}


TableBuffer::TableBuffer (uint32_t tag, vector<char> data, size_t checksumExcludeOffset)
	: TableBuffer(tag, std::move(data))
{
	if (checksumExcludeOffset+4 < _data.size())
		_checksum -= getUInt32(checksumExcludeOffset);
}


string TableBuffer::name () const {
	string ret;
	for (int i=3; i >= 0; i--) {
		char c = char(_tag >> (8*i));
		ret += (c >= 32 && c <= 126 ? c : '*');
	}
	return ret;
}


/** Tries to compress the buffer data. If the size of the compressed buffer data is
 *  greater or equal than the size of the uncompressed data, the buffer stays uncompressed.
 *  @returns true if the data was compressed */
void TableBuffer::compress () {
	if (_data.size() < 16)
		return;
	uLong compressedSize = compressBound(_data.size());
	vector<char> compressedData(compressedSize, 0);
	// Compress with zlib for now. We could also use zopfli for slightly better but much slower compression.
	// Only use the compressed data if it actually leads to a size reduction. Otherwise, use the original table data.
	auto source = reinterpret_cast<const Bytef*>(_data.data());
	auto target = reinterpret_cast<Bytef*>(&compressedData[0]);
	if (compress2(target, &compressedSize, source, _unpaddedSize, Z_BEST_COMPRESSION) == Z_OK && compressedSize < _unpaddedSize) {
		_compressedSize = compressedSize;
		_data = std::move(compressedData);
		_data.resize((compressedSize+3) & ~3, 0);  // reduce buffer to padded compressed size
	}
}

////////////////////////////////////////////////////////////////////////////////////

/** Creates a buffer object containing the binary data represented by this table. */
TableBuffer TTFTable::createBuffer () const {
	ovectorstream<vector<char>> vecstream;
	write(vecstream);
	vector<char> vec;
	vecstream.swap_vector(vec);  // move data from vector stream to vec
	return TableBuffer(tag(), std::move(vec));
}


string TTFTable::name () const {
	string ret;
	for (int i=3; i >= 0; i--)
		ret += char(tag() >> (8*i));
	return ret;
}
