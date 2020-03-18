/*************************************************************************
** PdfSpecialHandler.cpp                                                **
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
#include <unordered_map>
#include <utility>
#include "Color.hpp"
#include "HyperlinkManager.hpp"
#include "InputReader.hpp"
#include "MapLine.hpp"
#include "PdfSpecialHandler.hpp"
#include "FontMap.hpp"
#include "Message.hpp"
#include "PapersizeSpecialHandler.hpp"
#include "PDFParser.hpp"
#include "SpecialActions.hpp"
#include "SpecialManager.hpp"

using namespace std;

using CmdHandler = void (PdfSpecialHandler::*)(StreamInputReader&, SpecialActions&);


void PdfSpecialHandler::preprocess (const string&, istream &is, SpecialActions &actions) {
	StreamInputReader ir(is);
	ir.skipSpace();
	const string cmdstr = ir.getWord();
	static unordered_map<string, CmdHandler> commands = {
		{"bann",     &PdfSpecialHandler::preprocessBeginAnn},
		{"bannot",   &PdfSpecialHandler::preprocessBeginAnn},
		{"beginann", &PdfSpecialHandler::preprocessBeginAnn},
		{"dest",     &PdfSpecialHandler::preprocessDest},
		{"pagesize", &PdfSpecialHandler::preprocessPagesize}
	};
	auto it = commands.find(cmdstr);
	if (it != commands.end())
		(this->*it->second)(ir, actions);
}


bool PdfSpecialHandler::process (const string&, istream &is, SpecialActions &actions) {
	_active = true;
	StreamInputReader ir(is);
	ir.skipSpace();
	const string cmdstr = ir.getWord();
	ir.skipSpace();
	// dvipdfm(x) specials currently supported
	static unordered_map<string, CmdHandler> commands = {
		{"bann",     &PdfSpecialHandler::processBeginAnn},
		{"bannot",   &PdfSpecialHandler::processBeginAnn},
		{"beginann", &PdfSpecialHandler::processBeginAnn},
		{"eann",     &PdfSpecialHandler::processEndAnn},
		{"eannot",   &PdfSpecialHandler::processEndAnn},
		{"endann",   &PdfSpecialHandler::processEndAnn},
		{"dest",     &PdfSpecialHandler::processDest},
		{"mapfile",  &PdfSpecialHandler::processMapfile},
		{"mapline",  &PdfSpecialHandler::processMapline}
	};
	auto it = commands.find(cmdstr);
	if (it != commands.end())
		(this->*it->second)(ir, actions);
	return true;
}


static char prepare_mode (InputReader &ir) {
	// read mode selector ('+', '-', or '=')
	char modechar = '+';           // default mode (append if new, do not replace existing mapping)
	if (strchr("=+-", ir.peek()))  // leading modifier given?
		modechar = static_cast<char>(ir.get());
	return modechar;
}


/** Sets the page size. This command is similar to the papersize special.
 *  Syntax pdf:pagesize (<dimname> <dimlenhth>)+ */
void PdfSpecialHandler::preprocessPagesize (StreamInputReader &ir, SpecialActions &actions) {
	// add page sizes to collection of paper sizes in order to handle them equally
	SpecialHandler *handler = SpecialManager::instance().findHandlerByName("papersize");
	if (auto papersizeHandler = dynamic_cast<PapersizeSpecialHandler*>(handler)) {
		try {
			Length width, height;
			// parse parameter sequence of the form (name length)+
			while (!ir.eof()) {
				string dimname = ir.getWord();
				string lenstr = ir.getString(" \t");
				// only consider width and height settings
				if (dimname == "width" && !lenstr.empty())
					width.set(lenstr);
				else if (dimname == "height" && !lenstr.empty())
					height.set(lenstr);
			}
			papersizeHandler->storePaperSize(actions.getCurrentPageNumber(), width, height);
		}
		catch (UnitException &e) { // ignore invalid length units for now
		}
	}
}


void PdfSpecialHandler::processMapfile (StreamInputReader &ir, SpecialActions&) {
	char modechar = prepare_mode(ir);
	string fname = ir.getString();
	if (!FontMap::instance().read(fname, modechar))
		Message::wstream(true) << "can't open map file " << fname << '\n';
}


void PdfSpecialHandler::processMapline (StreamInputReader &ir, SpecialActions&) {
	char modechar = prepare_mode(ir);
	try {
		MapLine mapline(ir.getStream());
		FontMap::instance().apply(mapline, modechar);
	}
	catch (const MapLineException &ex) {
		Message::wstream(true) << "pdf:mapline: " << ex.what() << '\n';
	}
}


/** Defines a named destination, e.g. a link target.
 * Syntax: dest PDFString PDFDest */
void PdfSpecialHandler::preprocessDest (StreamInputReader &ir, SpecialActions &actions) {
	PDFParser parser;
	vector<PDFObject> objects = parser.parse(ir, [&](const string &opname, vector<PDFObject> &objects) {
		if (!opname.empty() && opname[0] == '@') {
			if (opname == "@thispage")
				objects.emplace_back(static_cast<int>(actions.getCurrentPageNumber()));
			else if (opname == "@xpos")
				objects.emplace_back(actions.getX());
			else if (opname == "@ypos")
				objects.emplace_back(actions.getY());
		}
	});
	if (objects.size() < 2)
		return;
	auto name = objects[0].get<string>();
	auto dest = objects[1].get<PDFArray>();
	// get target info from array [pageno /XYZ xpos ypos zpos]
	if (name && dest && dest->size() >= 4 && dest->at(0).get<int>()) {
		int pageno = *dest->at(0).get<int>();
		HyperlinkManager::instance().addNameAchor(*name, pageno);
	}
}


/** Extracts the URI from a PDF annotation dictionary. "GoTo" targets (named anchors)
 *  are prefixed with a '#'.
 *  @param[in] annotDict annotation dictionary containing the target URI
 *  @return the URI if one was found, "" otherwise */
static string get_uri (const PDFDict &annotDict) {
	// At the moment, we only support link annotations
	const PDFObject *type = annotDict.get("Type");
	if (type && string(*type) == "Annot") {
		const PDFObject *subtype = annotDict.get("Subtype");
		if (subtype && string(*subtype) == "Link") {
			const PDFObject *dict = annotDict.get("A");
			if (const PDFDict *actionDict = dict->get<PDFDict>()) {
				if (const PDFObject *s = actionDict->get("S")) {
					if (string(*s) == "GoTo") {
						if (const PDFObject *dest = actionDict->get("D"))
							return "#" + string(*dest);
					}
					else if (string(*s) == "URI") {
						if (const PDFObject *uri = actionDict->get("URI"))
							return string(*uri);
					}
				}
			}
		}
	}
	return "";
}


void PdfSpecialHandler::preprocessBeginAnn (StreamInputReader &ir, SpecialActions&) {
	PDFParser parser;
	vector<PDFObject> pdfobjs = parser.parse(ir);
	if (pdfobjs.empty() || !pdfobjs[0].get<PDFDict>())
		return;
	const PDFDict &annotDict = *pdfobjs[0].get<PDFDict>();
	string uri = get_uri(annotDict);
	if (!uri.empty())
		HyperlinkManager::instance().addHrefAnchor(uri);
}


/** Converts a PDFObject to a Color, where a single number denotes a gray value.
 *  Number arrays are treated as gray, RGB, or CMYK colors depending on the
 *  number of components. */
static Color to_color (const PDFObject &obj) {
	Color color;
	if (obj.get<int>() || obj.get<double>())
		color.setGray(double(obj));
	else if (auto colorArray = obj.get<PDFArray>()) {
		size_t size = min(size_t(4), colorArray->size());
		valarray<double> colorComps(size);
		for (size_t i=0; i < size; i++)
			colorComps[i] = double(colorArray->at(i));
		switch (size) {
			case 1: color.setGray(colorComps); break;
			case 3: color.setRGB(colorComps); break;
			case 4: color.setCMYK(colorComps); break;
		}
	}
	return color;
}


/** Begins a breakable annotation, e.g. a hyperlink.
 *  Syntax: beginann PDFDict */
void PdfSpecialHandler::processBeginAnn (StreamInputReader &ir, SpecialActions &actions) {
	PDFParser parser;
	vector<PDFObject> pdfobjs = parser.parse(ir);
	if (pdfobjs.empty() || !pdfobjs[0].get<PDFDict>())
		return;
	const PDFDict &annotDict = *pdfobjs[0].get<PDFDict>();
	string uri = get_uri(annotDict);
	if (uri.empty())
		return;

	// check presence of entry /Border [hr vr bw] defining the horizontal/vertical
	// corner radius and the border width
	auto it = annotDict.find("Border");
	if (it != annotDict.end() && it->second.get<PDFArray>() && it->second.get<PDFArray>()->size() > 2)
		HyperlinkManager::instance().setLineWidth(double(it->second.get<PDFArray>()->at(2)));

	// check presence of entry /C defining the border color
	it = annotDict.find("C");
	if (it != annotDict.end())
		HyperlinkManager::setDefaultLinkColor(to_color(it->second));
	HyperlinkManager::instance().createLink(uri, actions);
}


/** Terminates the preceding breakable annotation.
 *  Syntax: endann */
void PdfSpecialHandler::processEndAnn (StreamInputReader&, SpecialActions &actions) {
	HyperlinkManager::instance().closeAnchor(actions);
}


void PdfSpecialHandler::processDest (StreamInputReader &ir, SpecialActions &actions) {
	PDFParser parser;
	vector<PDFObject> objects = parser.parse(ir);
	if (!objects.empty()) {
		if (auto name = objects[0].get<string>())
			HyperlinkManager::instance().setActiveNameAnchor(*name, actions);
	}
}


/** This method is called every time the DVI position changes. */
void PdfSpecialHandler::dviMovedTo (double x, double y, SpecialActions &actions) {
	if (_active)
		HyperlinkManager::instance().checkNewLine(actions);
}


void PdfSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (_active) {
		HyperlinkManager::instance().createViews(pageno, actions);
		_active = false;
	}
}


vector<const char*> PdfSpecialHandler::prefixes() const {
	vector<const char*> pfx {"pdf:"};
	return pfx;
}
