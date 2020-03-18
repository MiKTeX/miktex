/*************************************************************************
** WSNodeRemover.cpp                                                    **
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

#include "WSNodeRemover.hpp"
#include "../XMLNode.hpp"

const char* WSNodeRemover::info () const {
	return "remove redundant whitespace nodes";
}


void WSNodeRemover::execute (XMLElement *context) {
	if (!context)
		return;
	bool removeWS = context->name() != "text" && context->name() != "tspan";
	XMLNode *child = context->firstChild();
	while (child) {
		if (removeWS && child->toWSNode()) {
			XMLNode *next = child->next();
			XMLElement::remove(child);
			child = next;
			continue;
		}
		if (XMLElement *elem = child->toElement())
			execute(elem);
		child = child->next();
	}
}
