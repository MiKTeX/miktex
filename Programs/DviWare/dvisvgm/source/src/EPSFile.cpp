/*************************************************************************
** EPSFile.cpp                                                          **
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
#include <istream>
#include <limits>
#include "EPSFile.h"
#include "InputBuffer.h"
#include "InputReader.h"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;


/** Reads a little-endian 32-bit integer from the given input stream. */
static UInt32 getUInt32 (istream &is) {
	UInt32 value=0;
	char buf[4];
	is.read(buf, 4);
	for (int i=0; i < 4; i++)
		value |= ((buf[i] & 255) << (8*i));
	return value;
}


static size_t getline (istream &is, char *line, size_t n) {
	char buf[512];
	is.get(buf, min(n, (size_t)512)-1);
	n = is.gcount();
	size_t linelen=0;
	for (size_t i=0; i < n; i++)
		if (isprint(buf[i]))
			line[linelen++] = buf[i];
	line[linelen] = 0;
	if (is.peek() == '\n')
		is.get();
	else
		is.ignore(numeric_limits<size_t>::max(), '\n');
	return linelen;
}


#if defined(MIKTEX_WINDOWS)
EPSFile::EPSFile(const std::string& fname) : _ifs(UW_(fname.c_str()), ios::binary), _headerValid(false), _offset(0), _pslength(0)
#else
EPSFile::EPSFile (const std::string& fname) : _ifs(fname.c_str(), ios::binary), _headerValid(false), _offset(0), _pslength(0)
#endif
{
	if (_ifs) {
		if (getUInt32(_ifs) != 0xC6D3D0C5)  // no binary header present?
			_ifs.seekg(0);                   // go back to the first byte
		else {
			_offset = getUInt32(_ifs);       // stream offset where PS part of the file begins
			_pslength = getUInt32(_ifs);     // length of PS section in bytes
			_ifs.seekg(_offset);             // continue reading at the beginning of the PS section
		}
		string str;
		str += _ifs.get();
		str += _ifs.get();
		_headerValid = (str == "%!");
		_ifs.seekg(0);
	}
}


/** Returns an input stream for the EPS file. The stream pointer is automatically moved
 *  to the beginning of the ASCII (PostScript) part of the file. */
istream& EPSFile::istream () const {
	_ifs.clear();
	_ifs.seekg(_offset);
	return _ifs;
}


/** Extracts the bounding box information from the DSC header/footer (if present)
 *  @param[out] box the extracted bounding box
 *  @return true if %%BoundingBox data could be read successfully */
bool EPSFile::bbox (BoundingBox &box) const {
	std::istream &is = EPSFile::istream();
	if (is) {
		char buf[64];
		while (is) {
			size_t linelen = getline(is, buf, 64);
			if (strncmp(buf, "%%BoundingBox:", 14) == 0) {
				CharInputBuffer ib(buf, linelen);
				BufferInputReader ir(ib);
				ir.skip(14);
				ir.skipSpace();
				if (!ir.check("(atend)", true)) {
					int val[4];
					for (int i=0; i < 4; i++) {
						ir.skipSpace();
						ir.parseInt(val[i]);
					}
					box = BoundingBox(val[0], val[1], val[2], val[3]);
					return true;
				}
			}
		}
	}
	return false;
}
