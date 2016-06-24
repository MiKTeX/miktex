/*************************************************************************
** CMapReader.cpp                                                       **
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
#include <fstream>
#include <sstream>
#include "CMap.h"
#include "CMapManager.h"
#include "CMapReader.h"
#include "FileFinder.h"
#include "InputReader.h"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;


CMapReader::CMapReader () : _cmap(0), _inCMap(false)
{
}


/** Reads a cmap file and returns the corresponding CMap object.
 *  @param fname[in] name/path of cmap file
 *  @return CMap object representing the read data, or 0 if file could not be read */
CMap* CMapReader::read (const string &fname) {
	if (const char *path = FileFinder::lookup(fname.c_str(), "cmap", false)) {
#if defined(MIKTEX_WINDOWS)
          ifstream ifs(UW_(path));
#else
		ifstream ifs(path);
#endif
		if (ifs)
			return read(ifs, fname);
	}
	_tokens.clear();
	return 0;
}


/** Reads cmap data from a given stream and returns the corresponding CMap object.
 *  @param is[in] cmap data input stream
 *  @param is[in] name name of CMap to be read
 *  @return CMap object representing the read data, or 0 if file could not be read */
CMap* CMapReader::read (std::istream& is, const string &name) {
	_tokens.clear();
	_cmap = new SegmentedCMap(name);
	StreamInputReader ir(is);
	try {
		while (ir) {
			Token token(ir);
			if (token.type() == Token::TT_EOF)
				break;
			if (_inCMap) {
				if (token.type() == Token::TT_OPERATOR)
					executeOperator(token.strvalue(), ir);
				else
					_tokens.push_back(token);
			}
			else if (token.type() == Token::TT_OPERATOR && token.strvalue() == "begincmap")
				_inCMap = true;
		}
	}
	catch (CMapReaderException &e) {
		delete _cmap;
		_cmap = 0;
		throw;
	}
	return _cmap;
}


void CMapReader::executeOperator (const string &op, InputReader &ir) {
	const struct Operator {
		const char *name;
		void (CMapReader::*handler)(InputReader&);
	} operators[] = {
		{"beginbfchar",   &CMapReader::op_beginbfchar},
		{"beginbfrange",  &CMapReader::op_beginbfrange},
		{"begincidrange", &CMapReader::op_begincidrange},
		{"def",           &CMapReader::op_def},
		{"endcmap",       &CMapReader::op_endcmap},
		{"usecmap",       &CMapReader::op_usecmap},
	};

	for (size_t i=0; i < sizeof(operators)/sizeof(Operator); i++) {
		if (operators[i].name == op) {
			(this->*operators[i].handler)(ir);
			break;
		}
	}
	_tokens.clear();
}


void CMapReader::op_def (InputReader&) {
	size_t size = _tokens.size();
	if (size >= 2) {
		const string val = popToken().strvalue();
		const string name = popToken().strvalue();
		if (name == "CMapName") {
			if (val != _cmap->_name)
				throw CMapReaderException("CMapName doesn't match filename");
		}
		else if (name == "WMode") {
			if (val == "0" || val == "1")
				_cmap->_vertical = (val == "1");
			else
				throw CMapReaderException("invalid WMode (0 or 1 expected)");
		}
		else if (name == "Registry")
			_cmap->_registry = val;
		else if (name == "Ordering")
			_cmap->_ordering = val;
	}
}


void CMapReader::op_endcmap (InputReader &) {
	_inCMap = false;
}


void CMapReader::op_usecmap (InputReader &) {
	if (_tokens.empty())
		throw CMapReaderException("stack underflow while processing usecmap");
	else {
		const string name = popToken().strvalue();
		if ((_cmap->_basemap = CMapManager::instance().lookup(name)) == 0)
			throw CMapReaderException("CMap file '"+name+"' not found");
	}
}


static UInt32 parse_hexentry (InputReader &ir) {
	ir.skipSpace();
	if (ir.get() != '<')
		throw CMapReaderException("invalid range entry ('<' expected)");
	int val;
	if (!ir.parseInt(16, val))
		throw CMapReaderException("invalid range entry (hexadecimal value expected)");
	if (ir.get() != '>')
		throw CMapReaderException("invalid range entry ('>' expected)");
	return UInt32(val);
}


void CMapReader::op_begincidrange (InputReader &ir) {
	if (!_tokens.empty() && _tokens.back().type() == Token::TT_NUMBER) {
		ir.skipSpace();
		int num_entries = static_cast<int>(popToken().numvalue());
		while (num_entries > 0 && ir.peek() == '<') {
			UInt32 first = parse_hexentry(ir);
			UInt32 last =  parse_hexentry(ir);
			UInt32 cid;
			ir.skipSpace();
			if (!ir.parseUInt(cid))
				throw CMapReaderException("invalid range entry (decimal value expected)");
			_cmap->addCIDRange(first, last, cid);
			ir.skipSpace();
		}
	}
}


void CMapReader::op_beginbfrange (InputReader &ir) {
	if (!_tokens.empty() && _tokens.back().type() == Token::TT_NUMBER) {
		ir.skipSpace();
		int num_entries = static_cast<int>(popToken().numvalue());
		while (num_entries > 0 && ir.peek() == '<') {
			UInt32 first = parse_hexentry(ir);
			UInt32 last =  parse_hexentry(ir);
			UInt32 chrcode = parse_hexentry(ir);
			_cmap->addBFRange(first, last, chrcode);
			ir.skipSpace();
		}
		_cmap->_mapsToCID = false;
	}
}


void CMapReader::op_beginbfchar (InputReader &ir) {
	if (!_tokens.empty() && _tokens.back().type() == Token::TT_NUMBER) {
		ir.skipSpace();
		int num_entries = static_cast<int>(popToken().numvalue());
		while (num_entries > 0 && ir.peek() == '<') {
			UInt32 cid = parse_hexentry(ir);
			ir.skipSpace();
			if (ir.peek() == '/')
				throw CMapReaderException("mapping of named characters is not supported");
			UInt32 chrcode = parse_hexentry(ir);
			_cmap->addBFRange(cid, cid, chrcode);
			ir.skipSpace();
		}
		_cmap->_mapsToCID = false;
	}
}

////////////////////////////////////////////////////////////////////////////////////

CMapReader::Token::Token (InputReader &ir) {
	scan(ir);
}


/** Reads the next characters from the input stream to create a token. */
void CMapReader::Token::scan (InputReader &ir) {
	ir.skipSpace();
	while (ir.peek() == '%') {   // comment?
		while (ir.peek() != '\n') // => skip until end of line
			ir.get();
		ir.skipSpace();
	}
	ir.skipSpace();
	if (ir.eof())
		_type = TT_EOF;
	else if (ir.peek() == '/') { // PS name?
		ir.get();
		while (!strchr("[]{}<>", ir.peek()) && !isspace(ir.peek()))
			_value += ir.get();
		_type = TT_NAME;
	}
	else if (ir.peek() == '(') { // string?
		ir.get();
		int level=0;
		while (ir.peek() != ')' || level > 0) {
			if (ir.peek() == '(')
				level++;
			else if (ir.peek() == ')' && level > 0)
				level--;
			_value += ir.get();
		}
		ir.get();  // skip ')'
		_type = TT_STRING;
	}
	else if (strchr("[]{}<>", ir.peek())) {  // PS delimiter
		_value = ir.get();
		_type = TT_DELIM;
	}
	else if (isdigit(ir.peek())) {  // number?
		double val;
		if (ir.parseDouble(val)) {
			ostringstream oss;
			oss << val;
			_value = oss.str();
			_type = TT_NUMBER;
		}
	}
	else {
		while (!strchr("[]{}<>", ir.peek()) && !isspace(ir.peek()))
			_value += ir.get();
		_type = TT_OPERATOR;
	}
}
