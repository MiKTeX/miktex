/*************************************************************************
** CRC32.cpp                                                            **
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
#include <cstring>
#include "CRC32.h"

using namespace std;


CRC32::CRC32 () : _crc32(0xFFFFFFFF)
{
	const UInt32 poly = 0xEDB88320;
	for (int i = 0; i < 256; i++) {
		UInt32 crc=i;
		for (int j=8; j > 0; j--) {
			if (crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
		_tab[i] = crc;
	}
}


/** Resets CRC32 sum to 0. */
void CRC32::reset () {
	_crc32 = 0xFFFFFFFF;
}


/** Appends string bytes to the previous data and computes the resulting checksum.
 *  @param[in] data string to update the checksum with */
void CRC32::update (const char *data) {
	update((const UInt8*)data, strlen(data));
}


/** Appends a single value to the previous data and computes the resulting checksum.
 *  @param[in] n value to update the checksum with
 *  @param[in] bytes number of bytes to consider (0-4) */
void CRC32::update (UInt32 n, int bytes) {
	for (int i=bytes-1; i >= 0; --i) {
		UInt8 byte = UInt8((n >> (8*i)) & 0xff);
		update(&byte, 1);
	}
}


/** Appends a sequence of bytes to the previous data and computes the resulting checksum.
 *  @param[in] bytes pointer to array of bytes
 *  @param[in] len number of bytes in array */
void CRC32::update (const UInt8 *bytes, size_t len) {
	for (size_t i=0; i < len; ++i)
		_crc32 = ((_crc32 >> 8) & 0x00FFFFFF) ^ _tab[(_crc32 ^ *bytes++) & 0xFF];
}


void CRC32::update (istream &is) {
	char buf [4096];
	while (is) {
		is.read(buf, 4096);
		update((UInt8*)buf, is.gcount());
	}
}


/** Returns the checksum computed from values added with the update functions. */
UInt32 CRC32::get () const {
	return _crc32 ^ 0xFFFFFFFF;
}


/** Computes the CRC32 checksum of a sequence of bytes.
 *  @param[in] bytes pointer to array of bytes
 *  @param[in] len number of bytes in array
 *  @return CRC32 checksum */
UInt32 CRC32::compute (const UInt8 *bytes, size_t len) {
	CRC32 crc32;
	crc32.update(bytes, len);
	return crc32.get();
}


/** Computes the CRC32 checksum of a string. */
UInt32 CRC32::compute (const char *str) {
	return compute((const UInt8*)str, strlen(str));
}


UInt32 CRC32::compute (istream &is) {
	CRC32 crc32;
	crc32.update(is);
	return crc32.get();
}
