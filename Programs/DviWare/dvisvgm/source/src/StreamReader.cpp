/*************************************************************************
** StreamReader.cpp                                                     **
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
#include "CRC32.h"
#include "StreamReader.h"
#include "macros.h"

using namespace std;


istream& StreamReader::replaceStream (istream &in) {
	istream &ret = *_is;
	_is = &in;
	return ret;
}


/** Reads an unsigned integer from assigned input stream.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @return read integer */
UInt32 StreamReader::readUnsigned (int bytes) {
	UInt32 ret = 0;
	for (bytes--; bytes >= 0 && !_is->eof(); bytes--) {
		UInt32 b = UInt32(_is->get());
		ret |= b << (8*bytes);
	}
	return ret;
}


/** Reads an unsigned integer from assigned input stream and updates the CRC32 checksum.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @param[in,out] crc32 checksum to be updated
 *  @return read integer */
UInt32 StreamReader::readUnsigned (int bytes, CRC32 &crc32) {
	UInt32 ret = readUnsigned(bytes);
	crc32.update(ret, bytes);
	return ret;
}


/** Reads an signed integer from assigned input stream.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @return read integer */
Int32 StreamReader::readSigned (int bytes) {
	Int32 ret = Int32(_is->get());
	if (ret & 128)        // negative value?
		ret |= 0xffffff00;
	for (bytes-=2; bytes >= 0 && !_is->eof(); bytes--)
		ret = (ret << 8) | _is->get();
	return ret;
}


/** Reads an signed integer from assigned input stream and updates the CRC32 checksum.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @param[in,out] crc32 checksum to be updated
 *  @return read integer */
Int32 StreamReader::readSigned (int bytes, CRC32 &crc32) {
	Int32 ret = readSigned(bytes);
	crc32.update(UInt32(ret), bytes);
	return ret;
}


/** Reads a string terminated by a 0-byte. */
string StreamReader::readString () {
	if (!_is)
		throw StreamReaderException("no stream assigned");
	string ret;
	while (!_is->eof() && _is->peek() > 0)
		ret += _is->get();
	_is->get();  // skip 0-byte
	return ret;
}


/** Reads a string terminated by a 0-byte and updates the CRC32 checksum.
 *  @param[in,out] crc32 checksum to be updated
 *  @param[in] finalZero consider final 0-byte in checksum
 *  @return the string read */
string StreamReader::readString (CRC32 &crc32, bool finalZero) {
	string ret = readString();
	crc32.update((const UInt8*)ret.c_str(), ret.length());
	if (finalZero)
		crc32.update(0, 1);
	return ret;
}


/** Reads a string of a given length.
 *  @param[in] length number of characters to read
 *  @return the string read */
string StreamReader::readString (int length) {
	if (!_is)
		throw StreamReaderException("no stream assigned");
	char *buf = new char[length+1];
	if (length <= 0)
		*buf = 0;
	else {
		_is->read(buf, length);  // reads 'length' bytes
		buf[length] = 0;
	}
	string ret = buf;
	delete [] buf;
	return ret;
}


/** Reads a string of a given length and updates the CRC32 checksum.
 *  @param[in] length number of characters to read
 *  @param[in,out] crc32 checksum to be updated
 *  @return the string read */
string StreamReader::readString (int length, CRC32 &crc32) {
	string ret = readString(length);
	crc32.update(ret.c_str());
	return ret;
}


vector<UInt8>& StreamReader::readBytes (int n, vector<UInt8> &bytes) {
	if (n > 0)
		_is->read((char*)&bytes[0], n);
	return bytes;
}


vector<UInt8>& StreamReader::readBytes (int n, vector<UInt8> &bytes, CRC32 &crc32) {
	readBytes(n, bytes);
	crc32.update(&bytes[0], bytes.size());
	return bytes;
}


int StreamReader::readByte (CRC32 &crc32) {
	int ret = readByte();
	if (ret >= 0) {
		const UInt8 c = UInt8(ret & 0xff);
		crc32.update(&c, 1);
	}
	return ret;
}
