/*************************************************************************
** RedundantElementRemover.cpp                                          **
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

#include "DependencyGraph.hpp"
#include "RedundantElementRemover.hpp"
#include "../XMLNode.hpp"

using namespace std;

const char* RedundantElementRemover::info () const {
	return "remove redundant 'clipPath' elements";
}


/** Extracts the ID from a local URL reference like url(#abcde) */
static inline string extract_id_from_url (const string &url) {
	return url.substr(5, url.length()-6);
}


/** Removes elements present in the SVG tree that are not required.
 *  For now, only clipPath elements are removed. */
void RedundantElementRemover::execute (XMLElement *defs, XMLElement *context) {
	vector<XMLElement*> clipPathElements;
	if (!defs || !context || !defs->getDescendants("clipPath", nullptr, clipPathElements))
		return;

	// collect dependencies between clipPath elements in the defs section of the SVG tree
	DependencyGraph<string> idTree;
	for (const XMLElement *clip : clipPathElements) {
		if (const char *id = clip->getAttributeValue("id")) {
			if (const char *url = clip->getAttributeValue("clip-path"))
				idTree.insert(extract_id_from_url(url), id);
			else
				idTree.insert(id);
		}
	}
	// collect elements that reference a clipPath, i.e. have a clip-path attribute
	vector<XMLElement*> descendants;
	defs->getDescendants("!clipPath", "clip-path", descendants);
	context->getDescendants(nullptr, "clip-path", descendants);
	// remove referenced IDs and their dependencies from the dependency graph
	for (const XMLElement *elem : descendants) {
		string idref = extract_id_from_url(elem->getAttributeValue("clip-path"));
		idTree.removeDependencyPath(idref);
	}
	descendants.clear();
	for (const string &str : idTree.getKeys()) {
		XMLElement *node = defs->getFirstDescendant("clipPath", "id", str.c_str());
		XMLElement::remove(node);
	}
}
