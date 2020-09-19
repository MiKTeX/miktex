/*************************************************************************
** StreamReader.cpp                                                     **
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

#include <algorithm>
#include <vector>
#include "HashFunction.hpp"
#include "StreamReader.hpp"
#include "utility.hpp"

using namespace std;


istream& StreamReader::replaceStream (istream &in) {
	istream &ret = *_is;
	_is = &in;
	return ret;
}


/** Reads an unsigned integer from assigned input stream.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @return read integer */
uint32_t StreamReader::readUnsigned (int bytes) {
	uint32_t ret = 0;
	for (bytes--; bytes >= 0 && !_is->eof(); bytes--) {
		uint32_t b = uint32_t(_is->get());
		ret |= b << (8*bytes);
	}
	return ret;
}


/** Reads an unsigned integer from assigned input stream and updates the hash value.
 *  @param[in] n number of bytes to read (max. 4)
 *  @param[in,out] hashfunc hash to update
 *  @return read integer */
uint32_t StreamReader::readUnsigned (int n, HashFunction &hashfunc) {
	uint32_t ret = readUnsigned(n);
	hashfunc.update(util::bytes(ret, n));
	return ret;
}


/** Reads an signed integer from assigned input stream.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @return read integer */
int32_t StreamReader::readSigned (int bytes) {
	uint32_t ret = uint32_t(_is->get());
	if (ret & 128)        // negative value?
		ret |= 0xffffff00;
	for (bytes-=2; bytes >= 0 && !_is->eof(); bytes--)
		ret = (ret << 8) | _is->get();
	return int32_t(ret);
}


/** Reads an signed integer from assigned input stream and updates the hash value.
 *  @param[in] bytes number of bytes to read (max. 4)
 *  @param[in,out] hashfunc hash to update
 *  @return read integer */
int32_t StreamReader::readSigned (int n, HashFunction &hashfunc) {
	int32_t ret = readSigned(n);
	hashfunc.update(util::bytes(ret, n));
	return ret;
}


/** Reads a string terminated by a 0-byte. */
string StreamReader::readString () {
	if (!_is)
		throw StreamReaderException("no stream assigned");
	string ret;
	while (!_is->eof() && _is->peek() > 0)
		ret += char(_is->get());
	_is->get();  // skip 0-byte
	return ret;
}


/** Reads a string terminated by a 0-byte and updates the hash value.
 *  @param[in,out] hashfunc hash to update
 *  @param[in] finalZero consider final 0-byte in checksum
 *  @return the string read */
string StreamReader::readString (HashFunction &hashfunc, bool finalZero) {
	string ret = readString();
	hashfunc.update(ret.data(), ret.length());
	if (finalZero)
		hashfunc.update(0, 1);
	return ret;
}


/** Reads a string of a given length.
 *  @param[in] length number of characters to read
 *  @return the string read */
string StreamReader::readString (int length) {
	if (!_is)
		throw StreamReaderException("no stream assigned");
	length = max(0, length);
	string str(length, '\0');
	_is->read(&str[0], length);  // read 'length' bytes and append '\0'
	return str;
}


/** Reads a string of a given length and updates the hash value.
 *  @param[in] length number of characters to read
 *  @param[in,out] hashfunc hash to update
 *  @return the string read */
string StreamReader::readString (int length, HashFunction &hashfunc) {
	string ret = readString(length);
	hashfunc.update(ret.data(), length);
	return ret;
}


vector<uint8_t> StreamReader::readBytes (int n) {
	vector<uint8_t> bytes(n);
	if (n > 0)
		_is->read(reinterpret_cast<char*>(bytes.data()), n);
	return bytes;
}


vector<uint8_t> StreamReader::readBytes (int n, HashFunction &hashfunc) {
	vector<uint8_t> bytes = readBytes(n);
	hashfunc.update(bytes);
	return bytes;
}


int StreamReader::readByte (HashFunction &hashfunc) {
	int ret = readByte();
	if (ret >= 0) {
		char c = ret & 0xff;
		hashfunc.update(&c, 1);
	}
	return ret;
}
