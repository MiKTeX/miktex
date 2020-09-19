/*************************************************************************
** DvisvgmSpecialHandler.hpp                                            **
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

#ifndef DVISVGMSPECIALHANDLER_HPP
#define DVISVGMSPECIALHANDLER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "SpecialHandler.hpp"

class InputReader;
class SpecialActions;
class SVGTree;
class XMLElement;
class XMLNode;

#ifdef _MSC_VER
// MSVC: Prevent aggressive optimization of pointers to member functions.
// Instatiating class DvisvgmSpecialHandler without the following pragma
// leads to memory corruption.
// https://docs.microsoft.com/en-us/cpp/preprocessor/pointers-to-members
#pragma pointers_to_members(full_generality, single_inheritance)
#endif

class DvisvgmSpecialHandler : public SpecialHandler {
	class XMLParser {
		using AppendFunc = void (SVGTree::*)(std::unique_ptr<XMLNode>);
		using PushFunc = void (SVGTree::*)(std::unique_ptr<XMLElement>);
		using PopFunc = void (SVGTree::*)();
		using NameStack = std::vector<std::string>;

		public:
			XMLParser (AppendFunc append, PushFunc push, PopFunc pop)
				: _append(append), _pushContext(push), _popContext(pop) {}

			void parse (const std::string &xml, SpecialActions &actions, bool finish=false);
			void flush (SpecialActions &actions);

		protected:
			void openElement (const std::string &tag, SpecialActions &actions);
			void closeElement (const std::string &tag, SpecialActions &actions);

		private:
			AppendFunc _append;
			PushFunc _pushContext;
			PopFunc _popContext;
			std::string _xmlbuf;
			NameStack _nameStack;  ///< names of nested elements still missing a closing tag
	};

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
		int _nestingLevel=0;    ///< nesting depth of rawset specials
		XMLParser _defsParser;  ///< parses XML added by 'rawdef' specials
		XMLParser _pageParser;  ///< parses XML added by 'raw' specials
};

#endif
