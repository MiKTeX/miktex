/*************************************************************************
** MapLine.cpp                                                          **
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

#include <cstring>
#include <sstream>
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "MapLine.hpp"
#include "Subfont.hpp"

using namespace std;


/** Constructs a MapLine object by parsing a single map line from the given stream. */
MapLine::MapLine (istream &is) : MapLine() {
	char buf[256];
	is.getline(buf, 256);
	parse(buf);
}


MapLine::MapLine (string str) : MapLine() {
	size_t pos = str.rfind('\n');
	if (pos != string::npos)
		str = str.substr(0, pos);
	parse(str.c_str());
}


// Some of the following functions have been derived from the dvipdfmx source file fontmap.c:
// http://cvs.ktug.or.kr/viewcvs/dvipdfmx/src/fontmap.c?revision=1.43&view=markup


/** Returns true if the given string is in dvips mapline format, and false if it's in dvipdfm format.
	 @param[in] line string to check */
bool MapLine::isDVIPSFormat (const char *line) const {
	if (strchr(line, '"') || strchr(line, '<'))  // these chars are only present in dvips maps
		return true;
	char prevchar = ' ';
	int entry_count=0;
	for (const char *p=line; *p; ++p) {
		if (isspace(prevchar)) {
			if (*p == '-') // options starting with '-' are only present in dvipdfm map files
				return false;
			if (!isspace(*p))
				entry_count++;
		}
		prevchar = *p;
	}
	// tfm_name and ps_name only => dvips map
	return entry_count == 2;
}


/** Separates main font name and subfont definition name from a given combined name.
 *  Example: "basename@sfdname@10" => {"basename10", "sfdname"}
 *  @param[in,out] fontname complete fontname; after separation: main fontname only
 *  @param[out] sfdname name of subfont definition
 *  @return true on success */
static bool split_fontname (string &fontname, string &sfdname) {
	size_t pos1;    // index of first '@'
	if ((pos1 = fontname.find('@')) != string::npos && pos1 > 0) {
		size_t pos2; // index of second '@'
		if ((pos2 = fontname.find('@', pos1+1)) != string::npos && pos2 > pos1+1) {
			sfdname = fontname.substr(pos1+1, pos2-pos1-1);
			fontname = fontname.substr(0, pos1) + fontname.substr(pos2+1);
			return true;
		}
	}
	return false;
}


/** Parses a single mapline and stores the scanned data in member variables.
 *  The line may either be given in dvips or dvipdfmx mapfile format.
 *  @param[in] line the mapline */
void MapLine::parse (const char *line) {
	if (line) {
		CharInputBuffer ib(line, strlen(line));
		BufferInputReader ir(ib);
		_texname = ir.getString();
		string sfdname;
		split_fontname(_texname, sfdname);
		if (!sfdname.empty())
			_sfd = SubfontDefinition::lookup(sfdname);
		if (isDVIPSFormat(line))
			parseDVIPSLine(ir);
		else
			parseDVIPDFMLine(ir);
	}
}


/** Parses a single line in dvips mapfile format.
 *  @param[in] ir the input stream must be assigned to this reader */
void MapLine::parseDVIPSLine (InputReader &ir) {
	ir.skipSpace();
	if (ir.peek() != '<' && ir.peek() != '"')
		_psname = ir.getString();
	ir.skipSpace();
	while (ir.peek() == '<' || ir.peek() == '"') {
		if (ir.peek() == '<') {
			ir.get();
			if (ir.peek() == '[' || ir.peek() == '<')
				ir.get();  // skip second char of "<[" and "<<"
			string name = ir.getString();
			if (name.length() > 4 && name.substr(name.length()-4) == ".enc")
				_encname = name.substr(0, name.length()-4);
			else
				_fontfname = name;
		}
		else {  // ir.peek() == '"' => list of PS font operators
			string options = ir.getQuotedString("\"");
			StringInputBuffer sib(options);
			BufferInputReader sir(sib);
			while (!sir.eof()) {
				double number;
				if (sir.parseDouble(number)) {
					// operator with preceding numeric parameter (value opstr)
					string opstr = sir.getString();
					if (opstr == "SlantFont")
						_slant = number;
					else if (opstr == "ExtendFont")
						_extend = number;
				}
				else {
					// operator without parameter => skip for now
					sir.getString();
				}
			}
		}
		ir.skipSpace();
	}
}


static void throw_number_expected (char opt, bool integer_only=false) {
	ostringstream oss;
	oss << "option -" << opt << ": " << (integer_only ? "integer" : "floating point") << " value expected";
	throw MapLineException(oss.str());
}


/** Parses a single line in dvipdfmx mapfile format.
 *  @param[in] ir the input stream must be assigned to this reader */
void MapLine::parseDVIPDFMLine (InputReader &ir) {
	ir.skipSpace();
	if (ir.peek() != '-') {
		_encname = ir.getString();
		if (_encname == "default" || _encname == "none")
			_encname.clear();
	}
	ir.skipSpace();
		if (ir.peek() != '-')
		_fontfname = ir.getString();
	if (!_fontfname.empty()) {
		parseFilenameOptions(_fontfname);
	}
	ir.skipSpace();
	while (ir.peek() == '-') {
		ir.get();
		int option = ir.get();
		if (!isprint(option))
			throw MapLineException("option character expected");
		ir.skipSpace();
		switch (option) {
			case 's': // slant
				if (!ir.parseDouble(_slant))
					throw_number_expected('s');
				break;
			case 'e': // extend
				if (!ir.parseDouble(_extend))
					throw_number_expected('e');
				break;
			case 'b': // bold
				if (!ir.parseDouble(_bold))
					throw_number_expected('b');
				break;
			case 'r': //remap (deprecated)
				break;
			case 'i': // ttc index
				if (!ir.parseInt(_fontindex, false))
					throw_number_expected('i', true);
				break;
			case 'p': // UCS plane
				int dummy;
				if (!ir.parseInt(dummy, false))
					throw_number_expected('p', true);
				break;
			case 'u': // to unicode
				ir.getString();
				break;
			case 'v': // stemV
				int stemv;
				if (!ir.parseInt(stemv, true))
					throw_number_expected('v', true);
				break;
			case 'm': // map single chars
				ir.skipUntil("-");
				break;
			case 'w': // writing mode (horizontal=0, vertical=1)
				int vertical;
				if (!ir.parseInt(vertical, false))
					throw_number_expected('w', true);
				break;
			default:
				throw MapLineException("invalid option: -" + string(1, option));
		}
		ir.skipSpace();
	}
}


/** [:INDEX:][!]FONTNAME[/CSI][,VARIANT] */
void MapLine::parseFilenameOptions (string fname) {
	_fontfname = fname;
	StringInputBuffer ib(fname);
	BufferInputReader ir(ib);
	if (ir.peek() == ':' && isdigit(ir.peek(1))) {  // index given?
		ir.get();
		_fontindex = ir.getInt();  // font index of file with multiple fonts
		if (ir.peek() == ':')
			ir.get();
		else
			_fontindex = 0;
	}
	if (ir.peek() == '!')  // no embedding
		ir.get();

	bool csi_given=false, style_given=false;
	int pos;
	if ((pos = ir.find('/')) >= 0) {  // csi delimiter
		csi_given = true;
		_fontfname = ir.getString(pos);
	}
	else if ((pos = ir.find(',')) >= 0) {
		style_given = true;
		_fontfname = ir.getString(pos);
	}
	else
		_fontfname = ir.getString();

	if (csi_given) {
		if ((pos = ir.find(',')) >= 0) {
			style_given = true;
			ir.getString(pos);  // charcoll
		}
		else if (ir.eof())
			throw MapLineException("CSI specifier expected");
		else
			ir.getString();  // charcoll
	}
	if (style_given) {
		ir.get();  // skip ','
		if (ir.check("BoldItalic")) {
		}
		else if (ir.check("Bold")) {
		}
		else if (ir.check("Italic")) {
		}
		if (!ir.eof())
			throw MapLineException("invalid style given");
	}
}
