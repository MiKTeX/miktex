/*************************************************************************
** DvisvgmSpecialHandler.cpp                                            **
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

#include <config.h>
#include <cstring>
#include <utility>
#include "DvisvgmSpecialHandler.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "Length.hpp"
#include "SpecialActions.hpp"
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
	} commands[] = {
		{"raw",       &DvisvgmSpecialHandler::preprocessRaw},
		{"rawdef",    &DvisvgmSpecialHandler::preprocessRawDef},
		{"rawset",    &DvisvgmSpecialHandler::preprocessRawSet},
		{"endrawset", &DvisvgmSpecialHandler::preprocessEndRawSet},
		{"rawput",    &DvisvgmSpecialHandler::preprocessRawPut},
	};

	StreamInputReader ir(is);
	string cmd = ir.getWord();
	for (size_t i=0; i < sizeof(commands)/sizeof(Command); i++) {
		if (commands[i].name == cmd) {
			ir.skipSpace();
			(this->*commands[i].handler)(ir);
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
		_currentMacro->second.emplace_back(string("P")+str);
}


void DvisvgmSpecialHandler::preprocessRawDef (InputReader &ir) {
	if (_currentMacro == _macros.end())
		return;
	string str = ir.getLine();
	if (!str.empty())
		_currentMacro->second.emplace_back(string("D")+str);
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
	} commands[] = {
		{"raw",       &DvisvgmSpecialHandler::processRaw},
		{"rawdef",    &DvisvgmSpecialHandler::processRawDef},
		{"rawset",    &DvisvgmSpecialHandler::processRawSet},
		{"endrawset", &DvisvgmSpecialHandler::processEndRawSet},
		{"rawput",    &DvisvgmSpecialHandler::processRawPut},
		{"bbox",      &DvisvgmSpecialHandler::processBBox},
		{"img",       &DvisvgmSpecialHandler::processImg},
	};
	StreamInputReader ir(is);
	string cmd = ir.getWord();
	for (size_t i=0; i < sizeof(commands)/sizeof(Command); i++) {
		if (commands[i].name == cmd) {
			ir.skipSpace();
			(this->*commands[i].handler)(ir, actions);
			return true;
		}
	}
	return true;
}


/** Replaces constants of the form {?name} by their corresponding value.
 *  @param[in,out] str text to expand
 *  @param[in] actions interfcae to the world outside the special handler */
static void expand_constants (string &str, SpecialActions &actions) {
	struct Constant {
		const char *name;
		string val;
	}
	constants[] = {
		{"x", XMLString(actions.getX())},
		{"y", XMLString(actions.getY())},
		{"color", actions.getColor().svgColorString()},
		{"nl", "\n"},
		{0, ""}
	};
	bool repl_bbox = true;
	while (repl_bbox) {
		size_t pos = str.find(string("{?bbox "));
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
	for (const Constant *p=constants; p->name; p++) {
		const string pattern = string("{?")+p->name+"}";
		size_t pos = str.find(pattern);
		while (pos != string::npos) {
			str.replace(pos, strlen(p->name)+3, p->val);
			pos = str.find(pattern, pos+p->val.length());  // look for further matches
		}
	}
}


void DvisvgmSpecialHandler::processRaw (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string str = ir.getLine();
		if (!str.empty()) {
			expand_constants(str, actions);
			actions.appendToPage(new XMLTextNode(str));
		}
	}
}


void DvisvgmSpecialHandler::processRawDef (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string str = ir.getLine();
		if (!str.empty()) {
			expand_constants(str, actions);
			actions.appendToDefs(new XMLTextNode(str));
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
	MacroMap::iterator it = _macros.find(id);
	if (it == _macros.end())
		throw SpecialException("undefined SVG fragment '" + id + "' referenced");

	StringVector &defvector = it->second;
	for (string &defstr : defvector) {
		char &type = defstr[0];
		string def = defstr.substr(1);
		if ((type == 'P' || type == 'D') && !def.empty()) {
			expand_constants(def, actions);
			if (type == 'P')
				actions.appendToPage(new XMLTextNode(def));
			else {          // type == 'D'
				actions.appendToDefs(new XMLTextNode(def));
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
static void update_bbox (double w, double h, double d, SpecialActions &actions) {
	double x = actions.getX();
	double y = actions.getY();
	actions.embed(BoundingBox(x, y, x+w, y-h));
	actions.embed(BoundingBox(x, y, x+w, y+d));
}


/** Evaluates the special dvisvgm:bbox.
 *  variant 1: dvisvgm:bbox [r[el]] <width> <height> [<depth>]
 *  variant 2: dvisvgm:bbox a[bs] <x1> <y1> <x2> <y2>
 *  variant 3: dvisvgm:bbox f[ix] <x1> <y1> <x2> <y2>
 *  variant 4: dvisvgm:bbox n[ew] <name> */
void DvisvgmSpecialHandler::processBBox (InputReader &ir, SpecialActions &actions) {
	ir.skipSpace();
	int c = ir.peek();
	if (isalpha(c)) {
		while (!isspace(ir.peek()))  // skip trailing characters
			ir.get();
		if (c == 'n') {
			ir.skipSpace();
			string name;
			while (isalnum(ir.peek()))
				name += char(ir.get());
			ir.skipSpace();
			if (!name.empty() && ir.eof())
				actions.bbox(name, true); // create new user box
		}
		else if (c == 'a' || c == 'f') {
			double p[4];
			for (int i=0; i < 4; i++)
				p[i] = ir.getDouble()*Length::pt2bp;
			BoundingBox b(p[0], p[1], p[2], p[3]);
			if (c == 'a')
				actions.embed(b);
			else {
				actions.bbox() = b;
				actions.bbox().lock();
			}
		}
	}
	else
		c = 'r';   // no mode specifier => relative box parameters

	if (c == 'r') {
		double w = ir.getDouble()*Length::pt2bp;
		double h = ir.getDouble()*Length::pt2bp;
		double d = ir.getDouble()*Length::pt2bp;
		update_bbox(w, h, d, actions);
	}
}


void DvisvgmSpecialHandler::processImg (InputReader &ir, SpecialActions &actions) {
	double w = ir.getDouble()*Length::pt2bp;
	double h = ir.getDouble()*Length::pt2bp;
	string f = ir.getString();
	update_bbox(w, h, 0, actions);
	XMLElementNode *img = new XMLElementNode("image");
	img->addAttribute("x", actions.getX());
	img->addAttribute("y", actions.getY());
	img->addAttribute("width", w);
	img->addAttribute("height", h);
	img->addAttribute("xlink:href", f);
	if (!actions.getMatrix().isIdentity())
		img->addAttribute("transform", actions.getMatrix().getSVG());
	actions.appendToPage(img);
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


const char** DvisvgmSpecialHandler::prefixes () const {
	static const char *pfx[] = {"dvisvgm:", 0};
	return pfx;
}
