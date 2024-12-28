/*************************************************************************
** DvisvgmSpecialHandler.cpp                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <cstring>
#include <utility>
#include "Calculator.hpp"
#include "DvisvgmSpecialHandler.hpp"
#include "FileFinder.hpp"
#include "FileSystem.hpp"
#include "GraphicsPathParser.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "Length.hpp"
#include "Message.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"

using namespace std;


void SVGParser::assign (SVGTree &svg, Append append, PushContext pushContext, PopContext popContext) {
	_svg = &svg;
	_append = append;
	_pushContext = pushContext;
	_popContext = popContext;
	setRootElement(nullptr);
}


XMLElement* SVGParser::openElement (const std::string &tag) {
	XMLElement *elem = XMLParser::openElement(tag);
	if (elem->name() == "path" || elem->name() == "svg:path") {
		if (const char *d = elem->getAttributeValue("d")) {
			try {
				// parse and reformat path definition
				auto path = GraphicsPathParser<double>().parse(d);
				ostringstream oss;
				path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
				elem->addAttribute("d", oss.str());
			}
			catch (const GraphicsPathParserException &e) {
				throw XMLParserException(string("error in path data: ")+e.what());
			}
		}
	}
	return elem;
}


void SVGParser::appendNode (unique_ptr<XMLNode> node) {
	(_svg->*_append)(std::move(node));
}


XMLElement* SVGParser::finishPushContext (unique_ptr<XMLElement> elem) {
	unique_ptr<SVGElement> svgElement{static_cast<SVGElement*>(elem.release())};
	XMLElement *elemPtr = svgElement.get();
	(_svg->*_pushContext)(std::move(svgElement));
	return elemPtr;
}


void SVGParser::finishPopContext () {
	(_svg->*_popContext)();
}


XMLElement* SVGParser::createElementPtr (std::string name) const {
	return new SVGElement(std::move(name));
}

///////////////////////////////////////////////////////////////////////////

DvisvgmSpecialHandler::DvisvgmSpecialHandler () : _currentMacro(_macros.end())
{
}


void DvisvgmSpecialHandler::preprocess (const string&, istream &is, SpecialActions&) {
	constexpr struct Command {
		const char *name;
		void (DvisvgmSpecialHandler::*handler)(InputReader&);
	} commands[] = {
		{"raw",       &DvisvgmSpecialHandler::preprocessRaw},
		{"rawdef",    &DvisvgmSpecialHandler::preprocessRawDef},
		{"rawset",    &DvisvgmSpecialHandler::preprocessRawSet},
		{"endrawset", &DvisvgmSpecialHandler::preprocessEndRawSet},
		{"rawput",    &DvisvgmSpecialHandler::preprocessRawPut}
	};

	StreamInputReader ir(is);
	const string cmdstr = ir.getWord();
	auto it = find_if(begin(commands), end(commands), [&](const Command &cmd) {
		return cmd.name == cmdstr;
	});
	if (it != end(commands)) {
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
	pair<MacroMap::iterator, bool> state = _macros.emplace(id, StringVector());
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
	constexpr struct Command {
		const char *name;
		void (DvisvgmSpecialHandler::*handler)(InputReader&, SpecialActions&);
	} commands[] = {
		{"raw",          &DvisvgmSpecialHandler::processRaw},
		{"rawdef",       &DvisvgmSpecialHandler::processRawDef},
		{"rawset",       &DvisvgmSpecialHandler::processRawSet},
		{"endrawset",    &DvisvgmSpecialHandler::processEndRawSet},
		{"rawput",       &DvisvgmSpecialHandler::processRawPut},
		{"bbox",         &DvisvgmSpecialHandler::processBBox},
		{"img",          &DvisvgmSpecialHandler::processImg},
		{"currentcolor", &DvisvgmSpecialHandler::processCurrentColor},
		{"message",      &DvisvgmSpecialHandler::processMessage}
	};
	StreamInputReader ir(is);
	const string cmdstr = ir.getWord();
	auto it = find_if(begin(commands), end(commands), [&](const Command &cmd) {
		return cmd.name == cmdstr;
	});
	if (it != end(commands)) {
		ir.skipSpace();
		(this->*it->handler)(ir, actions);
	}
	return true;
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
			xml = actions.expandText(xml);
			_pageParser.parse(std::move(xml));
		}
	}
}


void DvisvgmSpecialHandler::processRawDef (InputReader &ir, SpecialActions &actions) {
	if (_nestingLevel == 0) {
		string xml = ir.getLine();
		if (!xml.empty()) {
			xml = actions.expandText(xml);
			_defsParser.parse(std::move(xml));
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
			def = actions.expandText(def);
			if (type == 'P')
				_pageParser.parse(std::move(def));
			else {          // type == 'D'
				_defsParser.parse(std::move(def));
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


inline string filename_suffix (const string &fname) {
	string ret;
	auto pos = fname.rfind('.');
	if (pos != string::npos)
		ret = util::tolower(fname.substr(pos+1));
	return ret;
}


void DvisvgmSpecialHandler::processImg (InputReader &ir, SpecialActions &actions) {
	try {
		const Length width = read_length(ir);
		const Length height = read_length(ir);
		const string fname = ir.getString();
		const string suffix = filename_suffix(fname);

		string pathstr;
		if (const char *path = FileFinder::instance().lookup(fname, false))
			pathstr = FileSystem::ensureForwardSlashes(path);
		if ((pathstr.empty() || !FileSystem::exists(pathstr)) && FileSystem::exists(fname))
			pathstr = fname;
		if (pathstr.empty())
			Message::wstream(true) << "file '" << fname << "' not found\n";

		update_bbox(width, height, Length(0), true, actions);
		auto imgageNode = util::make_unique<SVGElement>("image");
		imgageNode->addAttribute("x", actions.getX());
		imgageNode->addAttribute("y", actions.getY()-height.bp());
		imgageNode->addAttribute("width", width.bp());
		imgageNode->addAttribute("height", height.bp());

		string mimetype = util::mimetype(fname);
		if (SVGTree::EMBED_BITMAP_DATA && (mimetype == "image/jpeg" || mimetype == "image/png"))
			imgageNode->addAttribute("@@xlink:href", "data:"+mimetype+";base64,"+fname);
		else {
			string href = fname;
			// Only reference the image with an absolute path if either an absolute path was given by the user
			// or a given plain filename is not present in the current working directory but was found through
			// the FileFinder, i.e. it's usually located somewhere in the texmf tree.
			if (!FilePath::isAbsolute(fname) && (fname.find('/') != string::npos || FilePath(fname).exists()))
				href = FilePath(pathstr).relative(FilePath(actions.getSVGFilePath(1)));
			imgageNode->addAttribute("xlink:href", href);
		}
		imgageNode->setTransform(actions.getMatrix());
		actions.svgTree().appendToPage(std::move(imgageNode));
	}
	catch (const UnitException &e) {
		throw SpecialException(string("dvisvgm:img: ") + e.what());
	}
}


void DvisvgmSpecialHandler::processCurrentColor (InputReader &ir, SpecialActions &actions) {
	string param = ir.getString();
	Color color = actions.getFillColor();
	if (param.empty() || param == "on") {
		SVGElement::CURRENTCOLOR = color;
		SVGElement::USE_CURRENTCOLOR = true;
	}
	else if (param == "off") {
		if (SVGElement::USE_CURRENTCOLOR) {
			// force a color change to get the new currentColor setting recognized
			actions.setFillColor(Color{uint32_t(color)+1});
			actions.setFillColor(color);
			SVGElement::USE_CURRENTCOLOR = false;
		}
	}
	else
		throw SpecialException("currentcolor: unknown parameter '"+param+"'");
}


void DvisvgmSpecialHandler::processMessage (InputReader &ir, SpecialActions &actions) {
	string message = actions.expandText(ir.getLine());
	Message::ustream() << message << "\n";
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


void DvisvgmSpecialHandler::dviBeginPage (unsigned, SpecialActions &actions) {
	SVGTree &svg = actions.svgTree();
	_defsParser.assign(svg, &SVGTree::appendToDefs, &SVGTree::pushDefsContext, &SVGTree::popDefsContext);
	_pageParser.assign(svg, &SVGTree::appendToPage, &SVGTree::pushPageContext, &SVGTree::popPageContext);
}


void DvisvgmSpecialHandler::dviEndPage (unsigned, SpecialActions &actions) {
	_defsParser.finish();
	_pageParser.finish();
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


vector<const char*> DvisvgmSpecialHandler::prefixes () const {
	vector<const char*> pfx {"dvisvgm:"};
	return pfx;
}
