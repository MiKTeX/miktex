/*************************************************************************
** SVGOptimizer.cpp                                                     **
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

#include <algorithm>
#include <array>
#include <map>
#include "SVGOptimizer.hpp"
#include "../SVGTree.hpp"

#include "AttributeExtractor.hpp"
#include "GroupCollapser.hpp"
#include "RedundantElementRemover.hpp"
#include "TextSimplifier.hpp"
#include "TransformSimplifier.hpp"
#include "WSNodeRemover.hpp"

using namespace std;

string SVGOptimizer::MODULE_SEQUENCE;

SVGOptimizer::SVGOptimizer (SVGTree *svg) : _svg(svg) {
	// optimizer modules available to the user; must be listed in default order
//	_moduleEntries.emplace_back(ModuleEntry("remove-ws", util::make_unique<WSNodeRemover>()));
	_moduleEntries.emplace_back(ModuleEntry("simplify-text", util::make_unique<TextSimplifier>()));
	_moduleEntries.emplace_back(ModuleEntry("group-attributes", util::make_unique<AttributeExtractor>()));
	_moduleEntries.emplace_back(ModuleEntry("collapse-groups", util::make_unique<GroupCollapser>()));
	_moduleEntries.emplace_back(ModuleEntry("simplify-transform", util::make_unique<TransformSimplifier>()));
	_moduleEntries.emplace_back(ModuleEntry("remove-clippath", util::make_unique<RedundantElementRemover>()));
}


void SVGOptimizer::execute () {
	if (!_svg || MODULE_SEQUENCE == "none")
		return;
	if (MODULE_SEQUENCE.empty())
		MODULE_SEQUENCE = "remove-clippath"; // default behaviour of previous dvisvgm releases
	if (MODULE_SEQUENCE == "all") {
		for (const auto &entry : _moduleEntries)
			entry.module->execute(_svg->defsNode(), _svg->pageNode());
	}
	else {
		vector<string> names = util::split(MODULE_SEQUENCE, ",");
		for (const string &name : names) {
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
	if (namestr.empty() || namestr == "all" || namestr == "none")
		return true;
	vector<string> givenNames = util::split(namestr, ",");
	for (const string &name : givenNames) {
		if (!getModule(name))
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

