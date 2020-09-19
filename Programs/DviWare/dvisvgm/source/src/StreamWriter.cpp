/*************************************************************************
** StreamWriter.cpp                                                     **
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

#include "HashFunction.hpp"
#include "StreamWriter.hpp"
#include "utility.hpp"

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
	writeUnsigned(uint32_t(val), n);
}


void StreamWriter::writeBytes (const std::vector<uint8_t> &bytes) {
	_os.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}


void StreamWriter::writeBytes (const uint8_t *buf, size_t bufsize) {
	_os.write(reinterpret_cast<const char*>(buf), bufsize);
}


void StreamWriter::writeBytes (int byte, size_t count) {
	while (count-- > 0)
		_os.put(byte);
}


/** Writes a string to the output stream.
 *  @param[in] str the string to write
 *  @param[in] finalZero if true, a final 0-byte is appended */
void StreamWriter::writeString (const string &str, bool finalZero) {
	for (char c : str)
		_os.put(c);
	if (finalZero)
		_os.put(0);
}


/** Writes an unsigned integer to the output stream.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered
 *  @param[in,out] hashfunc hash to update */
void StreamWriter::writeUnsigned (uint32_t val, int n, HashFunction &hashfunc) {
	writeUnsigned(val, n);
	hashfunc.update(util::bytes(val, n));
}


/** Writes a signed integer to the output stream and updates the hash value.
 *  @param[in] val the value to write
 *  @param[in] n number of bytes to be considered
 *  @param[in,out] hashfunc hash to update */
void StreamWriter::writeSigned (int32_t val, int n, HashFunction &hashfunc) {
	writeUnsigned(uint32_t(val), n, hashfunc);
}


/** Writes a string to the output stream and updates the hash value.
 *  @param[in] str the string to write
 *  @param[in,out] hashfunc hash to update
 *  @param[in] finalZero if true, a final 0-byte is appended */
void StreamWriter::writeString (const std::string &str, HashFunction &hashfunc, bool finalZero) {
	writeString(str, finalZero);
	hashfunc.update(str.data(), str.length() + (finalZero ? 1 : 0));
}
