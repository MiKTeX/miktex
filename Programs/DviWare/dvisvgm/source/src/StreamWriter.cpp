/*************************************************************************
** StreamWriter.cpp                                                     **
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
#include "CRC32.hpp"
#include "StreamWriter.hpp"

using namespace std;


/** Writes an unsigned integer to the output stream.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered */
void StreamWriter::writeUnsigned (uint32_t val, int n) {
	for (n--; n >= 0; n--)
		_os.put(char((val >> (8*n)) & 0xff));
}


/** Writes a signed integer to the output stream.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered */
void StreamWriter::writeSigned (int32_t val, int n) {
	writeUnsigned((uint32_t)val, n);
}


/** Writes a signed integer to the output stream.
 *  @param[in] val the value to write
 *  @param[in] finalZero if true, a final 0-byte is appended */
void StreamWriter::writeString (const string &str, bool finalZero) {
	for (size_t i=0; i < str.length(); i++)
		_os.put(str[i]);
	if (finalZero)
		_os.put(0);
}


/** Writes an unsigned integer to the output stream.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered
 *  @param[in,out] crc32 checksum to be updated */
void StreamWriter::writeUnsigned (uint32_t val, int n, CRC32 &crc32) {
	writeUnsigned(val, n);
	crc32.update(val, n);
}


/** Writes a signed integer to the output stream and updates the CRC32 checksum.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered
 *  @param[in,out] crc32 checksum to be updated */
void StreamWriter::writeSigned (int32_t val, int n, CRC32 &crc32) {
	writeUnsigned((uint32_t)val, n, crc32);
}


/** Writes a string to the output stream and updates the CRC32 checksum.
 *  @param[in] str the string to write
 *  @param[in,out] crc32 checksum to be updated
 *  @param[in] finalZero if true, a final 0-byte is appended */
void StreamWriter::writeString (const std::string &str, CRC32 &crc32, bool finalZero) {
	writeString(str, finalZero);
	crc32.update((const uint8_t*)str.c_str(), str.length() + (finalZero ? 1 : 0));
}
