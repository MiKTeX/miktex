/*************************************************************************
** Subfont.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include "FileFinder.hpp"
#include "Message.hpp"
#include "Subfont.hpp"
#include "utility.hpp"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;

// helper functions

static int skip_mapping_data (istream &is);
static bool scan_line (const char *line, int lineno, vector<uint16_t> &mapping, const string &fname, long &pos);


/** Constructs a new SubfontDefinition object.
 *  @param[in] name name of subfont definition
 *  @param[in] fpath path to corresponding .sfd file*/
SubfontDefinition::SubfontDefinition (const string &name, const char *fpath) : _sfname(name) {
	// read all subfont IDs from the .sfd file but skip the mapping data
#if defined(MIKTEX_WINDOWS)
        ifstream is(UW_(fpath));
#else
	ifstream is(fpath);
#endif
	while (is) {
		if (is.peek() == '#')                            // comment line?
			is.ignore(numeric_limits<int>::max(), '\n');  // => skip it
		else if (isspace(is.peek()))
			is.get();
		else {
			string id;
			while (is && !isspace(is.peek()))
				id += is.get();
			if (!id.empty()) {
				auto state = _subfonts.emplace(pair<string,unique_ptr<Subfont>>(id, unique_ptr<Subfont>()));
				if (state.second) // id was not present in map already
					state.first->second = unique_ptr<Subfont>(new Subfont(*this, state.first->first));
				skip_mapping_data(is);
			}
		}
	}
}


/** Looks for a subfont definition of the given name and returns the corresponding object.
 *  All objects are only created once for a given name and stored in an internal cache.
 *  @param[in] name name of subfont definition to lookup
 *  @return pointer to subfont definition object or 0 if it doesn't exist */
SubfontDefinition* SubfontDefinition::lookup (const std::string &name) {
	static unordered_map<string,unique_ptr<SubfontDefinition>> sfdMap;
	auto it = sfdMap.find(name);
	if (it != sfdMap.end())
		return it->second.get();
	SubfontDefinition *sfd=nullptr;
	if (const char *path = FileFinder::instance().lookup(name+".sfd", false)) {
		sfd = new SubfontDefinition(name, path);
		sfdMap[name] = unique_ptr<SubfontDefinition>(sfd);
	}
	return sfd;
}


/** Returns the full path to the corresponding .sfd file or 0 if it can't be found. */
const char* SubfontDefinition::path () const {
	return FileFinder::instance().lookup(filename(), false);
}


/** Returns the subfont with the given ID, or 0 if it doesn't exist. */
Subfont* SubfontDefinition::subfont (const string &id) const {
	auto it = _subfonts.find(id);
	if (it != _subfonts.end())
		return it->second.get();
	return nullptr;
}


/** Returns all subfonts defined in this SFD. */
int SubfontDefinition::subfonts (vector<Subfont*> &sfs) const {
	for (const auto &strsfpair : _subfonts)
		sfs.push_back(strsfpair.second.get());
	return int(sfs.size());
}

//////////////////////////////////////////////////////////////////////


/** Reads the character mappings for a given subfont ID.
 *  Format of subfont definition (sfd) files:
 *  sfd ::= (ID entries | '#' <string> '\n')*
 *  ID ::= <string without whitespace>
 *  entries ::= (integer | integer ':' | integer '_' integer)*
 *  The mapping data for a subfont is given as a sequence of 256 16-bit values where
 *  value v at position c defines the (global) character code that is assigned to the
 *  local subfont character c. The sequence v,v+1,v+2,...,v+n can be abbreviated with
 *  v '_' v+n, e.g. 10_55. In order to continue the sequence at a different position,
 *  the syntax number ':' can be used. Example: 10: 5 6 7 assigns the values v=5, 6, 7
 *  to c=10, 11 and 12, respectively.
 *  @return true if the data has been read successfully */
bool Subfont::read () {
	if (!_mapping.empty())  // if there's already a mapping assigned, we're finished here
		return true;
	if (const char *p = _sfd.path()) {
#if defined(MIKTEX_WINDOWS)
                ifstream is(UW_(p));
#else
		ifstream is(p);
#endif
		if (!is)
			return false;
		int lineno=1;
		while (is) {
			if (is.peek() == '#' || is.peek() == '\n') {
				is.ignore(numeric_limits<int>::max(), '\n');  // skip comment and empty line
				lineno++;
			}
			else if (isspace(is.peek()))
				is.get();
			else {
				string id;
				while (is && !isspace(is.peek()))
					id += is.get();
				if (id != _id)
					lineno += skip_mapping_data(is);
				else {
					// build mapping array
					_mapping.resize(256, 0);
					long pos=0;
					char buf[1024];
					bool complete=false;
					while (!complete) {
						is.getline(buf, 1024);
						complete = scan_line(buf, lineno, _mapping, _sfd.filename(), pos);
					}
					return true;
				}
			}
		}
	}
	return false;
}


/** Returns the global character code of the target font for a
 *  (local) character code of the subfont.
 *  @param[in] c local character code relative to the subfont
 *  @return character code of the target font */
uint16_t Subfont::decode (unsigned char c) {
	if (_mapping.empty() && !read())
		return 0;
	return _mapping[c];
}


//////////////////////////////////////////////////////////////////////


/** Skips the mapping data of a subfont entry.
 *  @param[in] is stream to read from
 *  @return number of lines skipped */
static int skip_mapping_data (istream &is) {
	char buf[1024];
	bool complete=false;
	int lines=0;
	while (is && !complete) {
		is.getline(buf, 1024);
		if (is.gcount() > 1)
			lines++;
		const char *p = buf+is.gcount()-2;
		while (p >= buf && isspace(*p))
			p--;
		complete = (p < buf || *p != '\\');  // line doesn't end with backslash
	}
	return lines;
}


/** Scans a single line of mapping data and stores the values in the given array.
 *  @param[in] line the line of text to be scanned
 *  @param[in] lineno line number used in exception messages
 *  @param[in,out] mapping the mapping data
 *  @param[in] fname name of the mapfile being scanned
 *  @param[in,out] offset position/index of next mapping value
 *  @return true if the line is the last one the current mapping sequence, i.e. the line doesn't end with a backslash */
static bool scan_line (const char *line, int lineno, vector<uint16_t> &mapping, const string &fname, long &offset) {
	const char *p=line;
	char *q;
	for (; *p && isspace(*p); p++);
	while (*p) {
		if (*p == '\\') {
			while (*++p)
				if (!isspace(*p))
					throw SubfontException("unexpected backslash in mapping table", fname, lineno);
		}
		else {
			long val1 = strtol(p, &q, 0); // first value of range
			long val2 = -1;               // last value of range
			switch (*q) {
				case ':':
					if (val1 < 0 || val1 > 255)
						throw SubfontException("offset value "+to_string(val1)+" out of range (0-255)", fname, lineno);
					offset = val1;
					val1 = -1;
					q++;
					break;
				case '_':
					p = q+1;
					val2 = strtol(p, &q, 0);
					if (val1 < 0 || val1 > 0xffffL)
						throw SubfontException("table value "+to_string(val1)+" out of range", fname, lineno);
					if (val2 < 0 || val2 > 0xffffL)
						throw SubfontException("table value "+to_string(val2)+" out of range", fname, lineno);
					if (p == q || (!isspace(*q) && *q != '\\' && *q))
						throw SubfontException("unexpected character '"+to_string(*q)+"'", fname, lineno);
					break;
				default:
					if (p == q || (!isspace(*q) && *q != '\\' && *q))
						throw SubfontException("unexpected character '"+to_string(*q)+"'", fname, lineno);
					if (val1 < 0 || val1 > 0xffffL)
						throw SubfontException("invalid character code", fname, lineno);
					val2 = val1;
			}
			if (val1 >= 0) {
				if (val1 > val2 || offset+val2-val1 > 255)
					throw SubfontException("invalid range in mapping table: "+util::tohex(val1)+"_"+util::tohex(val2), fname, lineno);
				for (long v=val1; v <= val2; v++) {
					if (mapping[offset])
						throw SubfontException("mapping of character "+to_string(offset)+" already defined", fname, lineno);
					mapping[offset++] = static_cast<uint16_t>(v);
				}
			}
			for (p=q; *p && isspace(*p); p++);
		}
	}
	for (p--; p >= line && isspace(*p); p--);
	return p < line || *p != '\\';
}

