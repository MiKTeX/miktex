/*************************************************************************
** HtmlSpecialHandler.cpp                                               **
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

#include "HtmlSpecialHandler.hpp"
#include "HyperlinkManager.hpp"
#include "InputReader.hpp"
#include "SpecialActions.hpp"

using namespace std;


void HtmlSpecialHandler::preprocess (const string&, istream &is, SpecialActions &actions) {
	StreamInputReader ir(is);
	ir.skipSpace();
	// collect page number and ID of named anchors
	map<string,string> attribs;
	if (ir.check("<a ") && ir.parseAttributes(attribs, true, "\"") > 0) {
		map<string,string>::iterator it;
		if ((it = attribs.find("name")) != attribs.end())
			HyperlinkManager::instance().addNameAchor(it->second, actions.getCurrentPageNumber());
		else if ((it = attribs.find("href")) != attribs.end())
			HyperlinkManager::instance().addHrefAnchor(it->second);
	}
}


bool HtmlSpecialHandler::process (const string&, istream &is, SpecialActions &actions) {
	_active = true;
	StreamInputReader ir(is);
	ir.skipSpace();
	map<string,string> attribs;
	map<string,string>::iterator it;
	if (ir.check("<a ") && ir.parseAttributes(attribs, true, "\"") > 0) {
		if ((it = attribs.find("href")) != attribs.end())   // <a href="URI">
			HyperlinkManager::instance().createLink(it->second, actions);
		else if ((it = attribs.find("name")) != attribs.end())  // <a name="ID">
			HyperlinkManager::instance().setActiveNameAnchor(it->second, actions);
		else
			return false;  // none or only invalid attributes
	}
	else if (ir.check("</a>"))
		HyperlinkManager::instance().closeAnchor(actions);
	else if (ir.check("<img src=")) {
	}
	else if (ir.check("<base ") && ir.parseAttributes(attribs, true, "\"") > 0 && (it = attribs.find("href")) != attribs.end())
		HyperlinkManager::instance().setBaseUrl(it->second);
	return true;
}


/** This method is called every time the DVI position changes. */
void HtmlSpecialHandler::dviMovedTo (double x, double y, SpecialActions &actions) {
	if (_active)
		HyperlinkManager::instance().checkNewLine(actions);
}


void HtmlSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (_active) {
		HyperlinkManager::instance().createViews(pageno, actions);
		_active = false;
	}
}


vector<const char*> HtmlSpecialHandler::prefixes() const {
	vector<const char*> pfx {"html:"};
	return pfx;
}
