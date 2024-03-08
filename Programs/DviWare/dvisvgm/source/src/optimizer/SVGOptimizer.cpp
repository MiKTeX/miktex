/*************************************************************************
** SVGOptimizer.cpp                                                     **
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
#include <algorithm>
#include <array>
#include <map>
#include <set>
#include "SVGOptimizer.hpp"
#include "../SVGTree.hpp"

#include "AttributeExtractor.hpp"
#include "ClipPathReassigner.hpp"
#include "GroupCollapser.hpp"
#include "RedundantElementRemover.hpp"
#include "TextSimplifier.hpp"
#include "TransformSimplifier.hpp"

using namespace std;

string SVGOptimizer::MODULE_SEQUENCE;

SVGOptimizer::SVGOptimizer (SVGTree *svg) : _svg(svg) {
	// optimizer modules available to the user; must be listed in default order
//	_moduleEntries.emplace_back(ModuleEntry("remove-ws", util::make_unique<WSNodeRemover>()));
	_moduleEntries.emplace_back("simplify-text", util::make_unique<TextSimplifier>());
	_moduleEntries.emplace_back("simplify-transform", util::make_unique<TransformSimplifier>());
	_moduleEntries.emplace_back("group-attributes", util::make_unique<AttributeExtractor>());
	_moduleEntries.emplace_back("collapse-groups", util::make_unique<GroupCollapser>());
	_moduleEntries.emplace_back("remove-clippaths", util::make_unique<RedundantElementRemover>());
	_moduleEntries.emplace_back("reassign-clippaths", util::make_unique<ClipPathReassigner>());
}


void SVGOptimizer::execute () {
	if (!_svg || MODULE_SEQUENCE == "none")
		return;

	vector<string> names = util::split(MODULE_SEQUENCE, ",", true);
	set<string> removedNames;
	if (names.empty())
		names.emplace_back("remove-clippaths"); // default behaviour of previous dvisvgm releases
	else {
		auto it = names.begin();
		if (*it == "all" || (!it->empty() && it->at(0) == '-')) {
			if (*it == "all")
				it = names.erase(it);
			// add names of all optimizer modules
			for (const auto &moduleEntry : _moduleEntries) {
				it = names.insert(it, moduleEntry.modname);
				++it;
			}
		}
	}
	// create sequence of module names to be considered
	for (auto it=names.begin(); it != names.end();) {
		if ((*it)[0] == '-') {
			removedNames.insert(it->substr(1));
			it = names.erase(it);
		}
		else {
			if ((*it)[0] == '+')
				*it = it->substr(1);
			++it;
		}
	}
	GroupCollapser::COMBINE_TRANSFORMS = (find(names.begin(), names.end(), "simplify-transform") != names.end());
	// execute optimizer modules
	for (const string &name: names) {
		if (removedNames.find(name) == removedNames.end()) {
			if (OptimizerModule *module = getModule(name))
				module->execute(_svg->defsNode(), _svg->pageNode());
		}
	}
}


void SVGOptimizer::listModules (ostream &os) const {
	size_t maxlen=0;
	map<string, const char*> infos;
	for (const auto &entry : _moduleEntries) {
		maxlen = max(maxlen, entry.modname.length());
		infos.emplace(entry.modname, entry.module->info());
	}
	for (const auto &infopair : infos) {
		os << setw(maxlen) << left << infopair.first;
		os << " | " << infopair.second << '\n';
	}
}


/** Checks if all module names given in a comma-separated list are known.
 *  @param[in] namestr comma-separated list of module names
 *  @param[out] unknownNames names not recognized
 *  @return true if all names are known */
bool SVGOptimizer::checkModuleString (string &namestr, vector<string> &unknownNames) const {
	unknownNames.clear();
	if (namestr.empty() || namestr == "none" || namestr == "all" || namestr.substr(0,4) == "all,")
		return true;
	vector<string> givenNames = util::split(namestr, ",");
	for (string name : givenNames) {
		if (name[0] == '-' || name[0] == '+')
			name = name.substr(1);
		if (!name.empty() && !getModule(name))
			unknownNames.emplace_back(name);
	}
	return unknownNames.empty();
}


OptimizerModule* SVGOptimizer::getModule (const string &name) const {
	auto it = find_if(_moduleEntries.begin(), _moduleEntries.end(), [&](const ModuleEntry &entry) {
		return entry.modname == name;
	});
	if (it != _moduleEntries.end())
		return (*it).module.get();
	return nullptr;
}

