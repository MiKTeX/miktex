/*************************************************************************
** DvisvgmSpecialHandler.hpp                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef DVISVGMSPECIALHANDLER_HPP
#define DVISVGMSPECIALHANDLER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "SpecialHandler.hpp"

class InputReader;
class SpecialActions;

class DvisvgmSpecialHandler : public SpecialHandler {
	using StringVector = std::vector<std::string>;
	using MacroMap = std::unordered_map<std::string, StringVector>;

	public:
		DvisvgmSpecialHandler ();
		const char* name () const override {return "dvisvgm";}
		const char* info () const override {return "special set for embedding raw SVG snippets";}
		std::vector<const char*> prefixes() const override;
		void preprocess (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;

	protected:
		void preprocessRaw (InputReader &ir);
		void preprocessRawDef (InputReader &ir);
		void preprocessRawSet (InputReader &ir);
		void preprocessEndRawSet (InputReader &ir);
		void preprocessRawPut (InputReader &ir);
		void processRaw (InputReader &ir, SpecialActions &actions);
		void processRawDef (InputReader &ir, SpecialActions &actions);
		void processRawSet (InputReader &ir, SpecialActions &actions);
		void processEndRawSet (InputReader &ir, SpecialActions &actions);
		void processRawPut (InputReader &ir, SpecialActions &actions);
		void processBBox (InputReader &ir, SpecialActions &actions);
		void processImg (InputReader &ir, SpecialActions &actions);
		void dviPreprocessingFinished () override;
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;

	private:
		MacroMap _macros;
		MacroMap::iterator _currentMacro;
		int _nestingLevel;
};

#endif
