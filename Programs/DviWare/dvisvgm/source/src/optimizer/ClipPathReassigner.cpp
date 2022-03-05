/*************************************************************************
** ClipPathReassigner.cpp                                               **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <algorithm>
#include <set>
#include <sstream>
#include "ClipPathReassigner.hpp"
#include "../XMLNode.hpp"
#include "../XXHashFunction.hpp"

using namespace std;

const char* ClipPathReassigner::info () const {
	return "remove duplicate clip paths and update the references";
}


/** Returns a hash value for an XML element where the id attribute is not taken into account.
 *  Two elements that differ only by their id attribute get the same hash value. */
static uint64_t hash_value (XMLElement *elem) {
	const char *id = elem->getAttributeValue("id");
	elem->removeAttribute("id");
	ostringstream oss;
	elem->write(oss);
	uint64_t value = XXH64HashFunction(oss.str().data(), oss.str().length()).digestValue();
	if (id)
		elem->addAttribute("id", id);
	return value;
}


/** Extracts the ID from a local URL reference like url(#abcde) */
static inline string extract_id_from_url (const string &url) {
	if (url.length() >= 7) {
		size_t first = url.find('#');
		size_t last = url.rfind(')');
		if (first != string::npos && last != string::npos && first < last)
			return url.substr(first+1, last-first-1);
	}
	return "";
}


void ClipPathReassigner::execute (XMLElement *defs, XMLElement *context) {
	vector<XMLElement*> clipPathElements;
	if (!defs || !context || !defs->getDescendants("clipPath", nullptr, clipPathElements))
		return;
	map<uint64_t, vector<XMLElement*>> clipPathMap;
	for (XMLElement *clipPathElement: clipPathElements) {
		uint64_t hash = hash_value(clipPathElement);
		clipPathMap[hash].push_back(clipPathElement); // put identical clipPath elements into a common vector
	}
	// replace clip path references
	vector<XMLElement*> descendants;
	context->getDescendants(nullptr, "clip-path", descendants);
	for (auto &mapEntry : clipPathMap) {
		vector<XMLElement*> &identicalClipPathElements = mapEntry.second;
		set<string> ids;
		for (auto elem : identicalClipPathElements)
			ids.insert(elem->getAttributeValue("id"));
		for (auto it = descendants.begin(); it != descendants.end();) {
			string id = extract_id_from_url((*it)->getAttributeValue("clip-path"));
			if (ids.find(id) == ids.end())
				++it;
			else {
				(*it)->addAttribute("clip-path", string("url(#") + (*ids.begin()) + ")");
				it = descendants.erase(it);  // no need to process this element again
			}
		}
	}
	// remove redundant clipPath elements
	for (auto mapEntry : clipPathMap) {
		auto it = mapEntry.second.begin();
		for (++it; it != mapEntry.second.end(); ++it)
			XMLElement::detach(*it);
	}
}
