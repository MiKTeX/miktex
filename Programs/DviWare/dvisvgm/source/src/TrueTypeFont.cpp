/*************************************************************************
** TrueTypeFont.cpp                                                     **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <iterator>
#include <sstream>
#include <woff2/encode.h>
#include <zlib.h>
#include "StreamReader.hpp"
#include "StreamWriter.hpp"
#include "TrueTypeFont.hpp"
#include "utility.hpp"

using namespace std;


/** Reads the font data from a TTF file.
 *  OTF specification: https://www.microsoft.com/typography/otspec/otff.htm
 *  @param[in] fname name/path of file to read
 *  @return true on success */
bool TrueTypeFont::read (const string &fname) {
	_tableRecords.clear();
	_buffer = util::read_file_contents(fname);
	if (_buffer.empty())
		return false;
	istringstream iss(_buffer);
	StreamReader reader(iss);
	_version = reader.readUnsigned(4);
	uint16_t numTables = reader.readUnsigned(2);
	if (numTables == 0 || _buffer.size() <= 12u+16u*numTables)
		return false;

	// read table records
	reader.seek(12);  // skip searchRange, entrySelector, and rangeShift
	for (uint16_t i=0; i < numTables; i++) {
		uint32_t tag = reader.readUnsigned(4);
		uint32_t checksum = reader.readUnsigned(4);
		uint32_t offset = reader.readUnsigned(4);
		uint32_t length = reader.readUnsigned(4);
		if ((offset | length) > _buffer.size() || offset+length > _buffer.size())
			return false;
		TTFTableRecord record = {tag, checksum, length, reinterpret_cast<const uint8_t*>(_buffer.data())+offset};
		_tableRecords.emplace_back(std::move(record));
	}
	return true;
}


/** Compresses the data of the referenced TTF table and updated the record data accordingly.
 *  @return true if the data was compressed */
bool TrueTypeFont::WOFFTableRecord::compressTableData () {
	if (ttfRecord.size < 16)
		return false;
	uLong compressedSize = compressBound(ttfRecord.size);
	compressedData.resize(compressedSize);
	// Compress with zlib for now. We could also use zopfli for slightly better but much slower compression.
	// Only use the compressed data if it actually leads to a size reduction. Otherwise, use the original table data.
	if (compress2(compressedData.data(), &compressedSize, ttfRecord.data, ttfRecord.size, Z_BEST_COMPRESSION) != Z_OK
			|| compressedSize >= ttfRecord.size) {
		compressedData.clear();
		return false;
	}
	size = compressedSize;
	data = compressedData.data();
	return true;
}


/** Writes font data in WOFF format to a given output stream.
	 WOFF specifiction: https://www.w3.org/TR/WOFF */
void TrueTypeFont::writeWOFF (ostream &os) const {
	// compute WOFF table records first
	vector<WOFFTableRecord> woffRecords;
	uint32_t woffSize = static_cast<uint32_t>(44 + 20*_tableRecords.size());
	uint32_t ttfSize = static_cast<uint32_t>(12 + 16*_tableRecords.size());
	for (const TTFTableRecord &ttfRecord : _tableRecords) {
		WOFFTableRecord woffRecord(woffSize, ttfRecord);
		woffRecord.compressTableData();
		woffSize += woffRecord.paddedSize();
		ttfSize += ttfRecord.paddedSize();
		woffRecords.emplace_back(std::move(woffRecord));
	}
	// write WOFF header
	StreamWriter writer(os);
	writer.writeUnsigned(0x774F4646, 4);              // "WOFF"
	writer.writeUnsigned(_version, 4);                // version of contained TTF font
	writer.writeUnsigned(woffSize, 4);                // total length of WOFF file
	writer.writeUnsigned(_tableRecords.size(), 2);    // number of tables
	writer.writeBytes(0, 2);                          // reserved
	writer.writeUnsigned(ttfSize, 4);                 // size of uncompressed TTF data
	writer.writeBytes(0, 4);                          // WOFF version (not required)
	writer.writeBytes(0, 12);                         // offset, compressed and uncompressed size of metadata block
	writer.writeBytes(0, 8);                          // offset and size of private data block

	// write WOFF table directory
	for (const WOFFTableRecord &woffRecord : woffRecords) {
		writer.writeUnsigned(woffRecord.ttfRecord.tag, 4);
		writer.writeUnsigned(woffRecord.offset, 4);
		writer.writeUnsigned(woffRecord.size, 4);
		writer.writeUnsigned(woffRecord.ttfRecord.size, 4);
		writer.writeUnsigned(woffRecord.ttfRecord.checksum, 4);
	}

	// write WOFF tables
	for (const WOFFTableRecord &woffRecord : woffRecords) {
		writer.writeBytes(woffRecord.data, woffRecord.size);
		size_t padding = woffRecord.paddedSize()-woffRecord.size;
		writer.writeBytes(0, padding);
	}
}


void TrueTypeFont::writeWOFF (const string &fname) const {
	ofstream ofs(fname.c_str(), ios::binary);
	writeWOFF(ofs);
}


/** Puts the TrueType data in a WOFF2 container and writes the resulting data to a given stream.
 *  @param[out] os stream to write the WOFF2 data to
 *  @return true on success */
bool TrueTypeFont::writeWOFF2 (ostream &os) const {
	const uint8_t* input_data = reinterpret_cast<const uint8_t*>(_buffer.data());
	size_t output_size = woff2::MaxWOFF2CompressedSize(input_data, _buffer.size());
	string output(output_size, 0);
	uint8_t* output_data = reinterpret_cast<uint8_t*>(&output[0]);
	woff2::WOFF2Params params;
	if (woff2::ConvertTTFToWOFF2(input_data, _buffer.size(), output_data, &output_size, params)) {
		output.resize(output_size);
		copy(output.begin(), output.end(), ostream_iterator<uint8_t>(os));
		return true;
	}
	return false;
}


bool TrueTypeFont::writeWOFF2 (const string &fname) const {
	ofstream ofs(fname.c_str(), ios::binary);
	return writeWOFF2(ofs);
}


/** Converts the numeric table tag (which represents a four-character ID) to a string. */
string TrueTypeFont::TTFTableRecord::name () const {
	string ret;
	for (int i=24; i >= 0; i-= 8)
		ret += static_cast<char>(tag >> i);
	return ret;
}


uint32_t TrueTypeFont::TTFTableRecord::computeChecksum () const {
	uint32_t sum=0;
	const uint32_t *startptr = reinterpret_cast<const uint32_t*>(data);
	const uint32_t *endptr = startptr + paddedSize()/sizeof(uint32_t);
	while (startptr < endptr)
		 sum += *startptr++;
	return sum;
}