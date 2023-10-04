/*************************************************************************
** TTFWriter.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2023 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <cmath>
#include <iterator>
#include <fstream>
#include <list>
#include <numeric>
#include <woff2/encode.h>
#include "TTFWriter.hpp"
#include "../Font.hpp"
#include "../utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;
using namespace ttf;

#ifdef TTFDEBUG
bool TTFWriter::CREATE_PS_GLYPH_OUTLINES;
#endif


TTFWriter::TTFWriter (const PhysicalFont &font, const set<int> &chars) :
	_font(font),
	_tracerCallback(),
	_tables({&_cmap, &_glyf, &_hmtx, &_hhea, &_loca, &_maxp, &_name, &_os2, &_post, &_head})  // mandatory tables
{
	if (font.verticalLayout()) {  // is font designed for vertical layout?
		_tables.push_back(&_vmtx);
		_tables.push_back(&_vhea);
	}
	for (TTFTable *table : _tables)
		table->setTTFWriter(this);
	for (uint32_t c : chars) {
		uint32_t codepoint = font.unicode(c);
		_unicodeCharMap.addRange(codepoint, codepoint, c);
	}
}


/** Pseudo table representing the TTF header (aka "offset table"). */
class TTFHeader : public TTFTable {
	public:
		explicit TTFHeader (uint16_t numTables) : _numTables(numTables) {}
		uint32_t tag () const override {return 0;}

		void write (ostream &os) const override {
			writeUInt32(os, 0x00010000);  // sfntVersion
			writeUInt16(os, _numTables);
			uint16_t entrySelector = util::ilog2(_numTables);
			uint16_t searchRange = (1 << entrySelector)*16;
			writeUInt16(os, searchRange);
			writeUInt16(os, entrySelector);
			writeUInt16(os, _numTables*16-searchRange);  // rangeShift
		}

	private:
		uint16_t _numTables;
};


/** Pseudo table representing the table records. */
class TTFTableRecords : public TTFTable {
	public:
		explicit TTFTableRecords (const list<TableBuffer> &buffers) : _buffers(buffers) {}
		uint32_t tag () const override {return 0;}

		void write (ostream &os) const override {
			int numTables = 0;
			for (const TableBuffer &buffer : _buffers)
				if (buffer.tag())
					numTables++;
			uint32_t offset = 12 + 16*numTables;
			for (const TableBuffer &buffer : _buffers) {
				if (buffer.tag()) {
					writeUInt32(os, buffer.tag());
					writeUInt32(os, buffer.checksum());
					writeUInt32(os, offset);
					writeUInt32(os, buffer.unpaddedSize());
					offset += buffer.paddedSize();
				}
			}
		}

	private:
		const list<TableBuffer> &_buffers;
};


/** Writes the font data in TrueType format to the given output stream. */
bool TTFWriter::writeTTF (ostream &os) {
	list<TableBuffer> buffers = createTableBuffers();
	// write TTF data
	for (; !buffers.empty(); buffers.pop_front())
		buffers.front().write(os);
	return true; // @@
}


/** Writes the font data in TrueType format to the given file. */
bool TTFWriter::writeTTF (const string &ttfname) {
#if defined(MIKTEX_WINDOWS)
	ofstream ofs(EXPATH_(ttfname), ios::binary);
#else
	ofstream ofs(ttfname, ios::binary);
#endif
	if (ofs)
		return writeTTF(ofs);
	return false;
}


/** Returns a list containing the binary TTF data segmented by the TTF tables.
 *  The first two list entries represent the TTF header (aka "offset table") and
 *  the table records, respectively. */
std::list<TableBuffer> TTFWriter::createTableBuffers () {
	list<TableBuffer> buffers;
	for (const TTFTable *table : _tables)
		buffers.emplace_back(table->createBuffer());
	// sort table buffers according to their tag/ID in ascending order
	buffers.sort([](const TableBuffer &buf1, const TableBuffer &buf2) {
		return buf1.tag() < buf2.tag();
	});
	TTFHeader header(_tables.size());
	TTFTableRecords records(buffers);
	buffers.emplace_front(records.createBuffer());
	buffers.emplace_front(header.createBuffer());
	// compute global checksum (checkSumAdjustment entry of head table)
	uint32_t checksum=0;
	for (const TableBuffer &buffer : buffers)
		checksum += buffer.checksum();
	checksum = 0xB1B0AFBA-checksum;
	// write checksum directly to the head table buffer
	auto headBufferIt = find_if(buffers.begin(), buffers.end(), [](const TableBuffer &buf) {
		return buf.tag() == TTFTable::name2id("head");
	});
	headBufferIt->setData(_head.offsetToChecksum(), checksum);
	return buffers;
}


/** Returns a factor c that scales the units per EM of the source font to
 *  a value which is a power of 2, i.e. log2(c*upem) is an integer. */
double TTFWriter::unitsPerEmFactor () const {
	double target_upem = targetUnitsPerEm();
	return target_upem > 0 ? (target_upem/_font.unitsPerEm()) : 0.0;
}


/** Returns the units per EM of the TTF font to be written.
 *  According to the OTF specification, the value should be a power of 2
 *  if the font contains TTF outlines. */
int TTFWriter::targetUnitsPerEm () const {
	int upem = _font.unitsPerEm();
	if (upem == 0)
		return 0;

	if (upem < 16)
		return 16;      // minimum allowed upem in TTF/OTF
	if (upem > 0x4000)
		return 0x4000;  // maximum allowed upem in TTF/OTF
	int log2_upem = util::ilog2(upem);
	if ((1 << log2_upem) == upem)  // is upem of source font a power of 2?
		return upem;
	return 1 << (log2_upem+1);
}


int16_t TTFWriter::hAdvance (uint16_t c) const {
	double scale = unitsPerEmFactor();
	double extend = _font.style() ? _font.style()->extend : 1;
	return int16_t(round(scale*extend*_font.hAdvance(c)));
}


void TTFWriter::updateGlobalBbox (uint32_t c, int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax) {
	if (xmin != 0 || ymin != 0 || xmax != 0 || ymax != 0)
		_hhea.updateXMinMax(c, xmin, xmax);
	if (xmin == 0 && xmax == 0)
		xmax = hAdvance(c);
	_head.updateGlobalBbox(xmin, ymin, xmax, ymax);
	_hmtx.updateWidthData(c, xmin, xmax);
	_os2.updateXMinMax(xmin, xmax);
	_os2.updateYMinMax(ymin, ymax);
	if (_vhea.active()) {
		_vhea.updateYMinMax(c, ymin, ymax);
		_vmtx.updateHeightData(c, ymin, ymax);
	}
}


/** Converts TTF data to WOFF2 and writes the result to an output stream.
 *  @param[in] buffer contents of TTF file
 *  @param[in,out] os WOFF2 output stream
 *  @return true on success */
static bool ttf_to_woff2 (const string &buffer, ostream &os) {
	const uint8_t* input_data = reinterpret_cast<const uint8_t*>(buffer.data());
	size_t output_size = woff2::MaxWOFF2CompressedSize(input_data, buffer.size());
	string output(output_size, 0);
	uint8_t* output_data = reinterpret_cast<uint8_t*>(&output[0]);
	woff2::WOFF2Params params;
	if (woff2::ConvertTTFToWOFF2(input_data, buffer.size(), output_data, &output_size, params)) {
		output.resize(output_size);
		copy(output.begin(), output.end(), ostream_iterator<uint8_t>(os));
		return true;
	}
	return false;
}


/** Writes the font data in WOFF2 format to the given output stream. */
bool TTFWriter::writeWOFF2 (ostream &os) {
	ostringstream oss;
	if (writeTTF(oss))
		return ttf_to_woff2(oss.str(), os);
	return false;
}


/** Writes the font data in WOFF2 format to the given file. */
bool TTFWriter::writeWOFF2 (const string &woff2name) {
#if defined(MIKTEX_WINDOWS)
	ofstream ofs(EXPATH_(woff2name), ios::binary);
#else
	ofstream ofs(woff2name, ios::binary);
#endif
	if (ofs)
		return writeWOFF2(ofs);
	return false;
}


/** Reads TTF data from an input stream, converts it to WOFF2, and
 *  writes the result to an output stream.
 *  @param[in,out] is TTF input stream
 *  @param[in,out] os WOFF2 output stream
 *  @return true on success */
bool TTFWriter::convertTTFToWOFF2 (istream &is, ostream &os) {
	ostringstream oss;
	oss << is.rdbuf();
	return ttf_to_woff2(oss.str(), os);
}


/** Reads TTF data from an input file, converts it to WOFF2, and
 *  writes the result to an output file.
 *  @param[in] ttfname name/path of TTF file
 *  @param[in] woff2name name/path of WOFF2 file
 *  @return true on success */
bool TTFWriter::convertTTFToWOFF2 (const string &ttfname, const string &woff2name) {
#if defined(MIKTEX_WINDOWS)
	ifstream ifs(EXPATH_(ttfname), ios::binary);
#else
	ifstream ifs(ttfname, ios::binary);
#endif
	if (ifs) {
#if defined(MIKTEX_WINDOWS)
		ofstream ofs(EXPATH_(woff2name), ios::binary);
#else
		ofstream ofs(woff2name, ios::binary);
#endif
		if (ofs)
			return convertTTFToWOFF2(ifs, ofs);
	}
	return false;
}


/** Pseudo table representing the WOFF header. */
class WOFFHeader : public TTFTable {
	public:
		WOFFHeader (uint16_t numTables, uint32_t ttfSize, uint32_t woffSize)
			: _numTables(numTables), _ttfSize(ttfSize), _woffSize(woffSize) {}

		uint32_t tag () const override {return 0;}

		void write (ostream &os) const override {
			writeUInt32(os, 0x774F4646);    // "WOFF"
			writeUInt32(os, 0x10000);       // version of contained TTF font (1.0)
			writeUInt32(os, _woffSize);     // total length of WOFF file
			writeUInt16(os, _numTables);    // number of tables
			writeUInt16(os, 0);             // reserved
			writeUInt32(os, _ttfSize);      // size of uncompressed TTF data
			writeUInt32(os, 0);             // WOFF version (not required)
			writeUInt32(os, 0);             // offset to metadata block
			writeUInt32(os, 0);             // compressed size of metadata block
			writeUInt32(os, 0);             // uncompressed size of metadata block
			writeUInt32(os, 0);             // offset to private data block
			writeUInt32(os, 0);             // size of private data block
		}

	private:
		uint16_t _numTables;  ///< number of tables in WOFF file
		uint32_t _ttfSize;    ///< total length of TTF file
		uint32_t _woffSize;   ///< total length of WOFF file
};


/** Pseudo table representing the WOFF table records. */
class WOFFTableRecords : public TTFTable {
	public:
		explicit WOFFTableRecords (const list<TableBuffer> &buffers) : _buffers(buffers) {}
		uint32_t tag () const override {return 0;}

		void write (ostream &os) const override {
			uint32_t offset = 44 + 20*_buffers.size();      // offset to first byte of table data
			for (const TableBuffer &buffer : _buffers) {
				if (buffer.tag()) {
					writeUInt32(os, buffer.tag());            // table tag/ID
					writeUInt32(os, offset);                  // offset to table from beginning of WOFF file
					writeUInt32(os, buffer.compressedSize()); // compressed length excluding padding
					writeUInt32(os, buffer.unpaddedSize());   // uncompressed length excluding padding
					writeUInt32(os, buffer.checksum());       // checksum of the uncompressed table
				}
				offset += buffer.paddedSize();
			}
		}

	private:
		const list<TableBuffer> &_buffers;
};


static bool ttf_to_woff (list<TableBuffer> &&buffers, ostream &os) {
	size_t ttfSize = std::accumulate(buffers.begin(), buffers.end(), size_t(0), [](size_t sum, const TableBuffer &buf) {
		return sum + buf.paddedSize();
	});
	buffers.pop_front();  // remove TTF header
	buffers.pop_front();  // remove TTF table records
	for (TableBuffer &buffer : buffers)
		buffer.compress();
	size_t woffSize = std::accumulate(buffers.begin(), buffers.end(), size_t(0), [](size_t sum, const TableBuffer &buf) {
		return sum + buf.paddedSize();
	});
	woffSize += 44 + 20*buffers.size();  // add size of header and table records
	WOFFHeader header(buffers.size(), ttfSize, woffSize);
	WOFFTableRecords records(buffers);
	buffers.emplace_front(records.createBuffer());
	buffers.emplace_front(header.createBuffer());
	for (; !buffers.empty(); buffers.pop_front())
		buffers.front().write(os);
	return true; // @@
}


/** Writes the font data in WOFF format to the given output stream. */
bool TTFWriter::writeWOFF (ostream &os) {
	return ttf_to_woff(createTableBuffers(), os);
}


/** Writes the font data in WOFF format to the given file. */
bool TTFWriter::writeWOFF (const string &woffname) {
#if defined(MIKTEX_WINDOWS)
	ofstream ofs(EXPATH_(woffname), ios::binary);
#else
	ofstream ofs(woffname, ios::binary);
#endif
	if (ofs)
		return writeWOFF(ofs);
	return false;
}


/** Reads TTF data from an input stream, converts it to WOFF, and
 *  writes the result to an output stream.
 *  @param[in,out] is TTF input stream
 *  @param[in,out] os WOFF output stream
 *  @return true on success */
bool TTFWriter::convertTTFToWOFF (istream &is, ostream &os) {
	list<TableBuffer> buffers;
	// read and process TTF header
	vector<char> bufvec(12, 0);
	is.read(&bufvec[0], 12);
	buffers.emplace_back(0, std::move(bufvec));
	int numTables = buffers.back().getUInt16(4);

	// read and process table records
	bufvec.clear();
	bufvec.resize(4 * 4 * numTables);
	is.read(&bufvec[0], 4 * 4 * numTables);
	buffers.emplace_back(0, std::move(bufvec));

	struct TableRecord {
		TableRecord (const TableBuffer &buf, size_t ofs)
			: tag(buf.getUInt32(ofs)), checksum(buf.getUInt32(ofs+4)), offset(buf.getUInt32(ofs+8)), length(buf.getUInt32(ofs+12)) {}
		uint32_t tag;
		uint32_t checksum;
		uint32_t offset;
		uint32_t length;
	};
	vector<TableRecord> tableRecords;
	tableRecords.reserve(numTables);
	const TableBuffer &recbuf = buffers.back();
	for (int i=0; i < numTables; i++)
		tableRecords.emplace_back(recbuf, 16*i);

	// read and process tables
	for (const TableRecord &record : tableRecords) {
		bufvec.clear();
		bufvec.resize(record.length);
		is.seekg(record.offset);
		is.read(&bufvec[0], record.length);
		if (record.tag == TTFTable::name2id("head"))
			buffers.emplace_back(record.tag, std::move(bufvec), HeadTable::offsetToChecksum());
		else
			buffers.emplace_back(record.tag, std::move(bufvec));
	}
	return ttf_to_woff(std::move(buffers), os);
}


/** Reads TTF data from an input file, converts it to WOFF, and
 *  writes the result to an output file.
 *  @param[in] ttfname name/path of TTF file
 *  @param[in] woffname name/path of WOFF file
 *  @return true on success */
bool TTFWriter::convertTTFToWOFF (const string &ttfname, const string &woffname) {
#if defined(MIKTEX_WINDOWS)
	ifstream ifs(EXPATH_(ttfname), ios::binary);
#else
	ifstream ifs(ttfname, ios::binary);
#endif
	if (ifs) {
#if defined(MIKTEX_WINDOWS)
		ofstream ofs(EXPATH_(woffname), ios::binary);
#else
		ofstream ofs(woffname, ios::binary);
#endif
		if (ofs)
			return convertTTFToWOFF(ifs, ofs);
	}
	return false;
}
