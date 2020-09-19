/*************************************************************************
** SVGOptimizer.hpp                                                     **
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

#pragma once

#include <memory>
#include <ostream>
#include <set>
#include <vector>
#include "OptimizerModule.hpp"
#include "../XMLNode.hpp"

class SVGTree;

class SVGOptimizer {
	struct ModuleEntry {
		ModuleEntry (std::string name, std::unique_ptr<OptimizerModule> mod)
			: modname(std::move(name)), module(std::move(mod)) {}

		std::string modname;
		std::unique_ptr<OptimizerModule> module;
	};
	public:
		explicit SVGOptimizer (SVGTree *svg=nullptr);
		explicit SVGOptimizer (SVGTree &svg) : SVGOptimizer(&svg) {}
		void execute ();
		void listModules (std::ostream &os) const;
		bool checkModuleString (std::string &namestr, std::vector<std::string> &unknownNames) const;

		static std::string MODULE_SEQUENCE;

	protected:
		OptimizerModule* getModule (const std::string &name) const;

	private:
		SVGTree *_svg;
		std::vector<ModuleEntry> _moduleEntries;
};




