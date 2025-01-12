/*************************************************************************
** TTFTable.hpp                                                         **
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

#pragma once
#include <cstdint>
#include <ostream>
#include <sstream>
#include <vector>

namespace ttf {

/** Class to handle the binary TTF table data. */
class TableBuffer {
	friend class TTFWriter;
	public:
		TableBuffer (uint32_t tag, std::vector<char> data);
		TableBuffer (uint32_t tag, std::vector<char> data, size_t checksumExcludeOffset);
		uint32_t tag () const {return _tag;}
		uint32_t unpaddedSize () const {return _unpaddedSize;}
		uint32_t paddedSize () const {return uint32_t(_data.size());}
		uint32_t compressedSize () const {return _compressedSize;}
		uint32_t checksum () const {return _checksum;}
		void compress ();
		std::string name () const;

		uint8_t getUInt8 (size_t offs) const {return _data[offs];}
		uint16_t getUInt16 (size_t offs) const {return getUInt8(offs)*0x100 + getUInt8(offs+1);}
		uint32_t getUInt32 (size_t offs) const {return getUInt16(offs)*0x10000 + getUInt16(offs+2);}

		/** Writes the n bytes of a given value in big-endian order to the buffer.
		 *  The buffer must be big enough to take the value at the desired position.
		 *  It is not resized automatically.
		 *  @param[in] offset offset/index the first byte is written to
		 *  @param[in] val the value to write */
		template <typename T>
		void setData (size_t offset, const T val) {
			if (offset+sizeof(T) <= _data.size()) {
				typename std::make_unsigned<T>::type uval = val;
				for (int i = sizeof(T)-1; i >= 0; i--)	{
					_data[offset+i] = uint8_t(uval & 0xff);
					uval >>= 8;
				}
			}
		}

		/** Writes the buffer data to the given output stream. */
		void write (std::ostream &os) const {
			os.write(_data.data(), _data.size());
		}

	private:
		uint32_t _tag;            ///< tag/ID of corresponding TTF table
		uint32_t _unpaddedSize;   ///< size of the buffer excluding pad bytes
		uint32_t _compressedSize; ///< sizeof compressed buffer excluding pad bytes
		uint32_t _checksum=0;     ///< checksum of uncompressed data
		std::vector<char> _data;  ///< the (compressed/uncompressed) table data including pad bytes
};


class TTFWriter;

/** Abstract base class for all TTF tables. */
class TTFTable {
	friend class TTFWriter;
	public:
		virtual ~TTFTable () =default;
		virtual uint32_t tag () const =0;
		virtual void write (std::ostream &os) const =0;
		bool active () const {return _ttfWriter != nullptr;}
		TableBuffer createBuffer () const;
		std::string name () const;

		static constexpr uint32_t name2id (const char *str) {
			return (str[0] << 24) | (str[1] << 16) | (str[2] << 8) | str[3];
		}

	protected:
		void setTTFWriter (TTFWriter *ttfWriter) {_ttfWriter = ttfWriter;}
		TTFWriter* ttfWriter () const {return _ttfWriter;}

		static void writeUInt8 (std::ostream &os, uint8_t val)   {os.put(val);}
		static void writeUInt16 (std::ostream &os, uint16_t val) {os.put(val >> 8); os.put(val & 0xff);}
		static void writeUInt32 (std::ostream &os, uint32_t val) {writeUInt16(os, val >> 16); writeUInt16(os, val & 0xffff);}
		static void writeInt8 (std::ostream &os, int8_t val)     {writeUInt8(os, uint8_t(val));}
		static void writeInt16 (std::ostream &os, int16_t val)   {writeUInt16(os, uint16_t(val));}
		static void writeInt32 (std::ostream &os, int32_t val)   {writeUInt32(os, uint32_t(val));}

	private:
		TTFWriter *_ttfWriter=nullptr;
};

} // namespace ttf
