/*************************************************************************
** DvisvgmSpecialHandler.cpp                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <array>
#include <cstring>
#include <utility>
#include "DvisvgmSpecialHandler.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "Length.hpp"
#include "SpecialActions.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"

using namespace std;


DvisvgmSpecialHandler::DvisvgmSpecialHandler ()
	: _currentMacro(_macros.end()), _nestingLevel(0)
{
}


void DvisvgmSpecialHandler::preprocess (const char*, istream &is, SpecialActions&) {
	struct Command {
		const char *name;
		void (DvisvgmSpecialHandler::*handler)(InputReader&);
	};
	constexpr array<Command, 5> commands {{
		{"raw",       &DvisvgmSpecialHandler::preprocessRaw},
		{"rawdef",    &DvisvgmSpecialHandler::preprocessRawDef},
		{"rawset",    &DvisvgmSpecialHandler::preprocessRawSet},
		{"endrawset", &DvisvgmSpecialHandler::preprocessEndRawSet},
		{"rawput",    &DvisvgmSpecialHandler::preprocessRawPut}
	}};

	StreamInputReader ir(is);
	string cmdstr = ir.getWord();
	for (const Command &command : commands) {
		if (command.name == cmdstr) {
			ir.skipSpace();
			(this->*command.handler)(ir);
			return;
		}
	}
}


void DvisvgmSpecialHandler::preprocessRawSet (InputReader &ir) {
	_nestingLevel++;
	string id = ir.getString();
	if (id.empty())
		throw SpecialException("definition of unnamed SVG fragment");
	if (_nestingLevel > 1)
		throw SpecialException("nested definition of SVG fragment '" + id + "'");

	_currentMacro = _macros.find(id);
	if (_currentMacro != _macros.end()) {
		_currentMacro = _macros.end();
		throw SpecialException("redefinition of SVG fragment '" + id + "'");
	}
	pair<string, StringVector> entry(id, StringVector());
	pair<MacroMap::iterator, bool> state = _macros.emplace(move(entry));
	_currentMacro = state.first;
}


void DvisvgmSpecialHandler::preprocessEndRawSet (InputReader&) {
	if (_nestingLevel > 0 && --_nestingLevel == 0)
		_currentMacro = _macros.end();
}


void DvisvgmSpecialHandler::preprocessRaw (InputReader &ir) {
	if (_currentMacro == _macros.end())
		return;
	string str = ir.getLine();
	if (!str.empty())
		_currentMacro->second.emplace_back("P"+str);
}


void DvisvgmSpecialHandler::preprocessRawDef (InputReader &ir) {
	if (_currentMacro == _macros.end())
		return;
	string str = ir.getLine();
	if (!str.empty())
		_currentMacro->second.emplace_back("D"+str);
}


void DvisvgmSpecialHandler::preprocessRawPut (InputReader &ir) {
	if (_currentMacro != _macros.end())
		throw SpecialException("dvisvgm:rawput not allowed inside rawset/endrawset");
}


/** Evaluates and executes a dvisvgm special statement.
 *  @param[in] prefix special prefix read by the SpecialManager
 *  @param[in] is the special statement is read from this stream
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
bool DvisvgmSpecialHandler::process (const char *prefix, istream &is, SpecialActions &actions) {
	struct Command {
		const char *name;
		void (DvisvgmSpecialHandler::*handler)(InputReader&, SpecialActions&);
	};
	constexpr array<Command, 7> commands {{
		{"raw",       &DvisvgmSpecialHandler::processRaw},
		{"rawdef",    &DvisvgmSpecialHandler::processRawDef},
		{"rawset",    &DvisvgmSpecialHandler::processRawSet},
		{"endrawset", &DvisvgmSpecialHandler::processEndRawSet},
		{"rawput",    &DvisvgmSpecialHandler::processRawPut},
		{"bbox",      &DvisvgmSpecialHandler::processBBox},
		{"img",       &DvisvgmSpecialHandler::processImg}
	}};
	StreamInputReader ir(is);
	string cmdstr = ir.getWord();
	for (const Command &command : commands) {
		if (command.name == cmdstr) {
			ir.skipSpace();
			(this->*command.handler)(ir, actions);
			return true;
		}
	}
	return true;
}


/** Replaces constants of the form {?name} by their corresponding value.
 *  @param[in,out] str text to expand
 *  @param[in] actions interfcae to the world outside the special handler */
static void expand_constants (string &str, SpecialActions &actions) {
	bool repl_bbox = true;
	while (repl_bbox) {
		size_t pos = str.find("{?bbox ");
		if (pos == string::npos)
			repl_bbox = false;
		else {
			size_t endpos = pos+7;
			while (endpos < str.length() && isalnum(str[endpos]))
				++endpos;
			if (str[endpos] == '}') {
				BoundingBox &box=actions.bbox(str.substr(pos+7, endpos-pos-7));
				str.replace(pos, endpos-pos+1, box.toSVGViewBox());
			}
			else
				repl_bbox = false;
		}
	}
	struct Constant {
		const char *name;
		string val;
	};
	const array<Constant, 4> constants {{
		{"x",     XMLString(actions.getX())},
		{"y",     XMLString(actions.getY())},
		{"color", actions.getColor().svgColorString()},
		{"nl",    "\n"},
	}};
	for (const Constant &constant : constants) {
		const string pattern = string("{?")+constant.name+"}";
		size_t pos = str.find(pattern);
		while (pos != string::npos) {
			str.replace(pos, strlen(constant.name)+3, constant.val);
			pos = str.find(pattern, pos+constant.val.length());  // look for further matches
		}
	}
}


void DvisvgmSpecialHandler::processRaw (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string str = ir.getLine();
		if (!str.empty()) {
			expand_constants(str, actions);
			actions.appendToPage(util::make_unique<XMLTextNode>(str));
		}
	}
}


void DvisvgmSpecialHandler::processRawDef (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string str = ir.getLine();
		if (!str.empty()) {
			expand_constants(str, actions);
			actions.appendToDefs(util::make_unique<XMLTextNode>(str));
		}
	}
}


void DvisvgmSpecialHandler::processRawSet (InputReader&, SpecialActions&) {
	_nestingLevel++;
}


void DvisvgmSpecialHandler::processEndRawSet (InputReader&, SpecialActions&) {
	if (_nestingLevel > 0)
		_nestingLevel--;
}


void DvisvgmSpecialHandler::processRawPut (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel > 0)
		return;
	string id = ir.getString();
	auto it = _macros.find(id);
	if (it == _macros.end())
		throw SpecialException("undefined SVG fragment '" + id + "' referenced");

	StringVector &defvector = it->second;
	for (string &defstr : defvector) {
		char &type = defstr[0];
		string def = defstr.substr(1);
		if ((type == 'P' || type == 'D') && !def.empty()) {
			expand_constants(def, actions);
			if (type == 'P')
				actions.appendToPage(util::make_unique<XMLTextNode>(def));
			else {          // type == 'D'
				actions.appendToDefs(util::make_unique<XMLTextNode>(def));
				type = 'L';  // locked
			}
		}
	}
}


/** Embeds the virtual rectangle (x, y ,w , h) into the current bounding box,
 *  where (x,y) is the lower left vertex composed of the current DVI position.
 *  @param[in] w width of the rectangle in PS point units
 *  @param[in] h height of the rectangle in PS point units
 *  @param[in] d depth of the rectangle in PS point units
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
static void update_bbox (Length w, Length h, Length d, SpecialActions &actions) {
	double x = actions.getX();
	double y = actions.getY();
	actions.embed(BoundingBox(x, y, x+w.bp(), y-h.bp()));
	actions.embed(BoundingBox(x, y, x+w.bp(), y+d.bp()));
}


/** Reads a length value including a trailing unit specifier and returns it. */
static Length read_length (InputReader &ir) {
	ir.skipSpace();
	double val = ir.getDouble();
	string unit = isalpha(ir.peek()) ? ir.getString(2) : "pt";
	return Length(val, unit);
}


/** Evaluates the special dvisvgm:bbox.
 *  variant 1: dvisvgm:bbox [r[el]] <width> <height> [<depth>]
 *  variant 2: dvisvgm:bbox a[bs] <x1> <y1> <x2> <y2>
 *  variant 3: dvisvgm:bbox f[ix] <x1> <y1> <x2> <y2>
 *  variant 4: dvisvgm:bbox n[ew] <name> */
void DvisvgmSpecialHandler::processBBox (InputReader &ir, SpecialActions &actions) {
	ir.skipSpace();
	int c = ir.peek();
	try {
		if (!isalpha(c))
			c = 'r';   // no mode specifier => relative box parameters
		else {
			while (!isspace(ir.peek()))  // skip trailing characters
				ir.get();
			if (c == 'n') {   // "new": create new local bounding box
				ir.skipSpace();
				string name;
				while (isalnum(ir.peek()))
					name += char(ir.get());
				ir.skipSpace();
				if (!name.empty() && ir.eof())
					actions.bbox(name, true); // create new user box
			}
			else if (c == 'a' || c == 'f') {  // "abs" or "fix"
				Length lengths[4];
				for (int i=0; i < 4; i++)
					lengths[i] = read_length(ir);
				BoundingBox b(lengths[0], lengths[1], lengths[2], lengths[3]);
				if (c == 'a')
					actions.embed(b);
				else {
					actions.bbox() = b;
					actions.bbox().lock();
				}
			}
		}
		if (c == 'r') {
			Length w = read_length(ir);
			Length h = read_length(ir);
			Length d = read_length(ir);
			update_bbox(w, h, d, actions);
		}
	}
	catch (const UnitException &e) {
		throw SpecialException(string("dvisvgm:bbox: ") + e.what());
	}
}


void DvisvgmSpecialHandler::processImg (InputReader &ir, SpecialActions &actions) {
	try {
		Length w = read_length(ir);
		Length h = read_length(ir);
		string f = ir.getString();
		update_bbox(w, h, 0, actions);
		auto img = util::make_unique<XMLElementNode>("image");
		img->addAttribute("x", actions.getX());
		img->addAttribute("y", actions.getY());
		img->addAttribute("width", w.bp());
		img->addAttribute("height", h.bp());
		img->addAttribute("xlink:href", f);
		if (!actions.getMatrix().isIdentity())
			img->addAttribute("transform", actions.getMatrix().getSVG());
		actions.appendToPage(std::move(img));
	}
	catch (const UnitException &e) {
		throw SpecialException(string("dvisvgm:img: ") + e.what());
	}
}


void DvisvgmSpecialHandler::dviPreprocessingFinished () {
	string id;
	if (_currentMacro != _macros.end())
		id = _currentMacro->first;
	// ensure all pattern definitions are closed after pre-processing the whole DVI file
	_currentMacro = _macros.end();
	_nestingLevel = 0;
	if (!id.empty())
		throw SpecialException("missing dvisvgm:endrawset for SVG fragment '" + id + "'");
}


void DvisvgmSpecialHandler::dviEndPage (unsigned, SpecialActions&) {
	for (auto &strvecpair : _macros) {
		StringVector &vec = strvecpair.second;
		for (string &str : vec) {
			// activate locked parts of a pattern again
			if (str[0] == 'L')
				str[0] = 'D';
		}
	}
}


const vector<const char*> DvisvgmSpecialHandler::prefixes () const {
	const vector<const char*> pfx {"dvisvgm:"};
	return pfx;
}
