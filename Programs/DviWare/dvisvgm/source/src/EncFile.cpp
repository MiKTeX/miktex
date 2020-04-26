/*************************************************************************
** EncFile.cpp                                                          **
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

#include <fstream>
#include "Font.hpp"
#include "EncFile.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "FileFinder.hpp"
#include "Message.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

static string read_entry (InputReader &in);
static bool valid_name_char (int c);


EncFile::EncFile (string encname) : _encname(std::move(encname))
{
	read();
}


const char* EncFile::path () const {
	return FileFinder::instance().lookup(_encname+".enc", false);
}


/** Search for suitable enc-file and read its encoding information.
 *  The file contents must be a valid PostScript vector with 256 entries. */
void EncFile::read () {
	if (const char *p = path()) {
#if defined(MIKTEX_WINDOWS)
                ifstream ifs(EXPATH_(p));
#else
		ifstream ifs(p);
#endif
		read(ifs);
	}
	else
		Message::wstream(true) << "encoding file '" << _encname << ".enc' not found\n";
}


/** Read encoding information from stream. */
void EncFile::read (istream &is) {
	StreamInputBuffer ib(is, 256);
	BufferInputReader in(ib);
	_table.resize(256);

	// find beginning of vector
	while (!in.eof()) {
		in.skipSpace();
		if (in.peek() == '%')
			in.skipUntil("\n");
		else
			if (in.get() == '[')
				break;
	}

	// read vector entries
	int n=0;
	while (!in.eof()) {
		in.skipSpace();
		if (in.peek() == '%')
			in.skipUntil("\n");
		else if (in.peek() == ']') {
			in.get();
			break;
		}
		else {
			string entry = read_entry(in);
			if (entry == ".notdef")
				entry.clear();
			if (n < 256)
				_table[n++] = entry;
		}
	}
	// remove trailing .notdef names
	for (n--; n > 0 && _table[n].empty(); n--);
	_table.resize(n+1);
}


static string read_entry (InputReader &in) {
	string entry;
	bool accept_slashes=true;
	while (!in.eof() && ((in.peek() == '/' && accept_slashes) || valid_name_char(in.peek()))) {
		if (in.peek() != '/')
			accept_slashes = false;
		entry += char(in.get());
	}
	if (entry.length() > 1) {
		// strip leading slashes
		// According to the PostScript specification, a single slash without further
		// following characters is a valid name.
		size_t n=0;
		while (n < entry.length() && entry[n] == '/')
			n++;
		entry = entry.substr(n);
	}
	return entry;
}


static bool valid_name_char (int c) {
	const char *delimiters = "<>(){}[]/~%";
	return isprint(c) && !isspace(c) && !strchr(delimiters, c);
}


/** Returns an entry of the encoding table.
 * @param[in] c character code
 * @return character name assigned to character code c*/
const char* EncFile::charName (uint32_t c) const {
	if (c < _table.size())
		return !_table[c].empty() ? _table[c].c_str() : nullptr;
	return nullptr;
}
