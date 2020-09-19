/*************************************************************************
** TrueTypeFont.hpp                                                     **
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

#ifndef TRUETYPEFONT_HPP
#define TRUETYPEFONT_HPP

#include <istream>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include "MessageException.hpp"


class TrueTypeFont {
	struct TTFTableRecord {
		std::string name () const;
		uint32_t paddedSize () const {return (size+3) & ~3;}
		uint32_t computeChecksum () const;
		uint32_t tag;         ///< table tag
		uint32_t checksum;    ///< checksum of the table data
		uint32_t size;        ///< number of bytes in the table
		const uint8_t *data;  ///< points to the begin of the table
	};

	struct WOFFTableRecord {
		WOFFTableRecord (uint32_t off, const TTFTableRecord &rec) : offset(off), size(rec.size), data(rec.data), ttfRecord(rec) {}
		size_t paddedSize () const {return (size+3) & ~3;}
		bool compressTableData ();
		uint32_t offset;                        ///< file offset to first byte of the table
		uint32_t size;                          ///< number of bytes in the table
		const uint8_t *data;                    ///< points to the begin of the WOFF table
		std::vector<uint8_t> compressedData;    ///< compressed table data
		const TTFTableRecord &ttfRecord;        ///< corresponding TTF table record
	};

	public:
		TrueTypeFont () : _version(0) {}
		explicit TrueTypeFont (const std::string &fname) {read(fname);}
		bool read (const std::string &fname);
		void writeWOFF (std::ostream &os) const;
		void writeWOFF (const std::string &fname) const;
		bool writeWOFF2 (std::ostream &os) const;
		bool writeWOFF2 (const std::string &fname) const;

	private:
		uint32_t _version;        ///< TTF version
		std::string _buffer;      ///< contents of TTF file
		std::vector<TTFTableRecord> _tableRecords;
};


struct TrueTypeFontException : public MessageException {
	explicit TrueTypeFontException (const std::string &msg) : MessageException(msg) {}
};

#endif
