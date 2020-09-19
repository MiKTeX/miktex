/*************************************************************************
** DvisvgmSpecialHandler.cpp                                            **
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

#include <array>
#include <cstring>
#include <utility>
#include "Calculator.hpp"
#include "DvisvgmSpecialHandler.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "Length.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"

using namespace std;


DvisvgmSpecialHandler::DvisvgmSpecialHandler () :
	_currentMacro(_macros.end()),
	_defsParser(&SVGTree::appendToDefs, &SVGTree::pushDefsContext, &SVGTree::popDefsContext),
	_pageParser(&SVGTree::appendToPage, &SVGTree::pushPageContext, &SVGTree::popPageContext)
{
}


void DvisvgmSpecialHandler::preprocess (const string&, istream &is, SpecialActions&) {
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
	const string cmdstr = ir.getWord();
	auto it = find_if(commands.begin(), commands.end(), [&](const Command &cmd) {
		return cmd.name == cmdstr;
	});
	if (it != commands.end()) {
		ir.skipSpace();
		(this->*it->handler)(ir);
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
bool DvisvgmSpecialHandler::process (const string &prefix, istream &is, SpecialActions &actions) {
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
	const string cmdstr = ir.getWord();
	auto it = find_if(commands.begin(), commands.end(), [&](const Command &cmd) {
		return cmd.name == cmdstr;
	});
	if (it != commands.end()) {
		ir.skipSpace();
		(this->*it->handler)(ir, actions);
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
	const array<Constant, 5> constants {{
		{"x",      XMLString(actions.getX())},
		{"y",      XMLString(actions.getY())},
		{"color",  actions.getColor().svgColorString()},
		{"matrix", actions.getMatrix().toSVG()},
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


/** Evaluates substrings of the form {?(expr)} where 'expr' is a math expression,
 *  and replaces the substring by the computed value.
 *  @param[in,out] str string to scan for expressions */
static void evaluate_expressions (string &str, SpecialActions &actions) {
	size_t left = str.find("{?(");             // start position of expression macro
	while (left != string::npos) {
		size_t right = str.find(")}", left+2);  // end position of expression macro
		if (right == string::npos)
			break;
		Calculator calc;
		calc.setVariable("x", actions.getX());
		calc.setVariable("y", actions.getY());
		string expr = str.substr(left+3, right-left-3);  // math expression to evaluate
		if (util::normalize_space(expr).empty())         // no expression given, e.g. {?( )}
			str.erase(left, right-left+2);                // => replace with empty string
		else {
			try {
				double val = calc.eval(expr);
				str.replace(left, right-left+2, XMLString(val));
			}
			catch (CalculatorException &e) {
				throw SpecialException(string(e.what())+" in '{?("+expr+")}'");
			}
		}
		left = str.find("{?(", right+1);  // find next expression macro
	}
}


/** Processes raw SVG fragments from the input stream. The SVG data must represent
 *  a single or multiple syntactically complete XML parts, like opening/closing tags,
 *  comments, or CDATA blocks. These must not be split and distributed over several
 *  'raw' statements. Elements can be split but element tags can't.
 *  Example: "<g transform=" is invalid, "<g transform='scale(2,3)'>" is ok. */
void DvisvgmSpecialHandler::processRaw (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string xml = ir.getLine();
		if (!xml.empty()) {
			evaluate_expressions(xml, actions);
			expand_constants(xml, actions);
			_pageParser.parse(xml, actions);
		}
	}
}


void DvisvgmSpecialHandler::processRawDef (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string xml = ir.getLine();
		if (!xml.empty()) {
			expand_constants(xml, actions);
			_defsParser.parse(xml, actions);
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
				_pageParser.parse(def, actions);
			else {          // type == 'D'
				_defsParser.parse(def, actions);
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
 *  @param[in] transform if true, apply the current transformation matrix to the rectangle
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
static void update_bbox (Length w, Length h, Length d, bool transform, SpecialActions &actions) {
	double x = actions.getX();
	double y = actions.getY();
	BoundingBox bbox1(x, y, x+w.bp(), y-h.bp());
	BoundingBox bbox2(x, y, x+w.bp(), y+d.bp());
	if (transform) {
		bbox1.transform(actions.getMatrix());
		bbox2.transform(actions.getMatrix());
	}
	actions.embed(bbox1);
	actions.embed(bbox2);
}


/** Reads a length value including a trailing unit specifier and returns it. */
static Length read_length (InputReader &ir) {
	Length length;
	ir.skipSpace();
	if (!isalpha(ir.peek())) {
		double val = ir.getDouble();
		string unit = isalpha(ir.peek()) ? ir.getString(2) : "pt";
		length = Length(val, unit);
	}
	return length;
}


/** Evaluates the special dvisvgm:bbox.
 *  variant 1: dvisvgm:bbox [r[el]] <width> <height> [<depth>] [transform]
 *  variant 2: dvisvgm:bbox a[bs] <x1> <y1> <x2> <y2> [transform]
 *  variant 3: dvisvgm:bbox f[ix] <x1> <y1> <x2> <y2> [transform]
 *  variant 4: dvisvgm:bbox n[ew] <name>
 *  variant 5: dvisvgm:bbox lock | unlock */
void DvisvgmSpecialHandler::processBBox (InputReader &ir, SpecialActions &actions) {
	ir.skipSpace();
	if (ir.check("lock"))
		actions.bbox().lock();
	else if (ir.check("unlock"))
		actions.bbox().unlock();
	else {
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
					for (Length &len : lengths)
						len = read_length(ir);
					BoundingBox b(lengths[0], lengths[1], lengths[2], lengths[3]);
					ir.skipSpace();
					if (ir.check("transform"))
						b.transform(actions.getMatrix());
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
				ir.skipSpace();
				update_bbox(w, h, d, ir.check("transform"), actions);
			}
		}
		catch (const UnitException &e) {
			throw SpecialException(string("dvisvgm:bbox: ") + e.what());
		}
	}
}


void DvisvgmSpecialHandler::processImg (InputReader &ir, SpecialActions &actions) {
	try {
		Length w = read_length(ir);
		Length h = read_length(ir);
		string f = ir.getString();
		update_bbox(w, h, Length(0), false, actions);
		auto img = util::make_unique<XMLElement>("image");
		img->addAttribute("x", actions.getX());
		img->addAttribute("y", actions.getY());
		img->addAttribute("width", w.bp());
		img->addAttribute("height", h.bp());
		img->addAttribute("xlink:href", f);
		if (!actions.getMatrix().isIdentity())
			img->addAttribute("transform", actions.getMatrix().toSVG());
		actions.svgTree().appendToPage(std::move(img));
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


void DvisvgmSpecialHandler::dviEndPage (unsigned, SpecialActions &actions) {
	_defsParser.flush(actions);
	_pageParser.flush(actions);
	actions.bbox().unlock();
	for (auto &strvecpair : _macros) {
		StringVector &vec = strvecpair.second;
		for (string &str : vec) {
			// activate locked parts of a pattern again
			if (str[0] == 'L')
				str[0] = 'D';
		}
	}
}


vector<const char*> DvisvgmSpecialHandler::prefixes() const {
	vector<const char*> pfx {"dvisvgm:"};
	return pfx;
}

////////////////////////////////////////////////////////////////////////////////

/** Parses a fragment of XML code, creates corresponding XML nodes and adds them
 *  to the SVG tree. The code may be split and processed by several calls of this
 *  function. Incomplete chunks that can't be processed yet are stored and picked
 *  up again together with the next incoming XML fragment. If no further code should
 *  be appended, parameter 'finish' must be set.
 *  @param[in] xml XML fragment to parse
 *  @param[in] actions object providing the SVG tree functions
 *  @param[in] finish if true, no more XML is expected and parsing is finished */
void DvisvgmSpecialHandler::XMLParser::parse (const string &xml, SpecialActions &actions, bool finish) {
	// collect/extract an XML fragment that only contains complete tags
	// incomplete tags are held back
	_xmlbuf += xml;
	size_t left=0, right;
	while (left != string::npos) {
		right = _xmlbuf.find('<', left);
		if (left < right && left < _xmlbuf.length())  // plain text found?
			(actions.svgTree().*_append)(util::make_unique<XMLText>(_xmlbuf.substr(left, right-left)));
		if (right != string::npos) {
			left = right;
			if (_xmlbuf.compare(left, 9, "<![CDATA[") == 0) {
				right = _xmlbuf.find("]]>", left+9);
				if (right == string::npos) {
					if (finish)	throw SpecialException("expected ']]>' at end of CDATA block");
					break;
				}
				(actions.svgTree().*_append)(util::make_unique<XMLCData>(_xmlbuf.substr(left+9, right-left-9)));
				right += 2;
			}
			else if (_xmlbuf.compare(left, 4, "<!--") == 0) {
				right = _xmlbuf.find("-->", left+4);
				if (right == string::npos) {
					if (finish)	throw SpecialException("expected '-->' at end of comment");
					break;
				}
				(actions.svgTree().*_append)(util::make_unique<XMLComment>(_xmlbuf.substr(left+4, right-left-4)));
				right += 2;
			}
			else if (_xmlbuf.compare(left, 2, "<?") == 0) {
				right = _xmlbuf.find("?>", left+2);
				if (right == string::npos) {
					if (finish)	throw SpecialException("expected '?>' at end of processing instruction");
					break;
				}
				(actions.svgTree().*_append)(util::make_unique<XMLText>(_xmlbuf.substr(left, right-left+2)));
				right++;
			}
			else if (_xmlbuf.compare(left, 2, "</") == 0) {
				right = _xmlbuf.find('>', left+2);
				if (right == string::npos) {
					if (finish)	throw SpecialException("missing '>' at end of closing XML tag");
					break;
				}
				closeElement(_xmlbuf.substr(left+2, right-left-2), actions);
			}
			else {
				right = _xmlbuf.find('>', left+1);
				if (right == string::npos) {
					if (finish)	throw SpecialException("missing '>' or '/>' at end of opening XML tag");
					break;
				}
				openElement(_xmlbuf.substr(left+1, right-left-1), actions);
			}
		}
		left = right;
		if (right != string::npos)
			left++;
	}
	if (left == string::npos)
		_xmlbuf.clear();
	else
		_xmlbuf.erase(0, left);
}


/** Processes an opening element tag.
 *  @param[in] tag tag without leading and trailing angle brackets */
void DvisvgmSpecialHandler::XMLParser::openElement (const string &tag, SpecialActions &actions) {
	StringInputBuffer ib(tag);
	BufferInputReader ir(ib);
	string name = ir.getString("/ \t\n\r");
	ir.skipSpace();
	auto elemNode = util::make_unique<XMLElement>(name);
	map<string, string> attribs;
	if (ir.parseAttributes(attribs, true, "\"'")) {
		for (const auto &attrpair : attribs)
			elemNode->addAttribute(attrpair.first, attrpair.second);
	}
	ir.skipSpace();
	if (ir.peek() == '/')       // end of empty element tag
		(actions.svgTree().*_append)(std::move(elemNode));
	else if (ir.peek() < 0) {   // end of opening tag
		_nameStack.push_back(name);
		(actions.svgTree().*_pushContext)(std::move(elemNode));
	}
	else
		throw SpecialException("'>' or '/>' expected at end of opening tag <"+name);
}


/** Processes a closing element tag.
 *  @param[in] tag tag without leading and trailing angle brackets */
void DvisvgmSpecialHandler::XMLParser::closeElement (const string &tag, SpecialActions &actions) {
	StringInputBuffer ib(tag);
	BufferInputReader ir(ib);
	string name = ir.getString(" \t\n\r");
	ir.skipSpace();
	if (ir.peek() >= 0)
		throw SpecialException("'>' expected at end of closing tag </"+name);
	if (_nameStack.empty())
		throw SpecialException("spurious closing tag </" + name + ">");
	if (_nameStack.back() != name)
		throw SpecialException("expected </" + name + "> but found </" + _nameStack.back() + ">");
	(actions.svgTree().*_popContext)();
	_nameStack.pop_back();
}


/** Processes any remaining XML fragments, checks for missing closing tags,
 *  and resets the parser state. */
void DvisvgmSpecialHandler::XMLParser::flush (SpecialActions &actions) {
	if (!_xmlbuf.empty()) {
		parse("", actions, true);
		_xmlbuf.clear();
	}
	string tags;
	while (!_nameStack.empty()) {
		tags += "</"+_nameStack.back()+">, ";
		_nameStack.pop_back();
	}
	if (!tags.empty()) {
		tags.resize(tags.length()-2);
		throw SpecialException("missing closing tags: "+tags);
	}
}
