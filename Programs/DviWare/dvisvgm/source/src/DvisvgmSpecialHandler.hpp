/*************************************************************************
** DvisvgmSpecialHandler.hpp                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include "XMLParser.hpp"

class InputReader;
class SpecialActions;
class SVGElement;
class SVGTree;
class XMLElement;
class XMLNode;

#ifdef _MSC_VER
// MSVC: Prevent aggressive optimization of pointers to member functions.
// Instantiating class DvisvgmSpecialHandler without the following pragma
// leads to memory corruption.
// https://docs.microsoft.com/en-us/cpp/preprocessor/pointers-to-members
#pragma pointers_to_members(full_generality, single_inheritance)
#endif

class SVGParser : public XMLParser {
	using Append = void (SVGTree::*)(std::unique_ptr<XMLNode> node);
	using PushContext = void (SVGTree::*)(std::unique_ptr<SVGElement> elem);
	using PopContext = void (SVGTree::*)();

	public:
		SVGParser () : XMLParser() {}
		void assign (SVGTree &svg, Append append, PushContext pushContext, PopContext popContext);

	protected:
		XMLElement* openElement (const std::string &tag) override;
		void appendNode (std::unique_ptr<XMLNode> node) override;
		XMLElement* finishPushContext (std::unique_ptr<XMLElement> elem) override;
		void finishPopContext () override;
		XMLElement* createElementPtr (std::string name) const override;

	private:
		SVGTree *_svg=nullptr;
		Append _append=nullptr;
		PushContext _pushContext=nullptr;
		PopContext _popContext=nullptr;
};


class DvisvgmSpecialHandler : public SpecialHandler {
	using StringVector = std::vector<std::string>;
	using MacroMap = std::unordered_map<std::string, StringVector>;

	public:
		DvisvgmSpecialHandler ();
		void preprocess (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		const char* info () const override {return "special set for embedding raw SVG fragments";}
		const char* name () const override {return handlerName();}
		static const char* handlerName ()  {return "dvisvgm";}
		std::vector<const char*> prefixes () const override;

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
		void processCurrentColor (InputReader &ir, SpecialActions &actions);
		void processMessage (InputReader &ir, SpecialActions &actions);
		void dviPreprocessingFinished () override;
		void dviBeginPage (unsigned pageno, SpecialActions &actions) override;
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;

	private:
		MacroMap _macros;
		MacroMap::iterator _currentMacro;
		int _nestingLevel=0;    ///< nesting depth of rawset specials
		SVGParser _defsParser;  ///< parses XML added by 'rawdef' specials
		SVGParser _pageParser;  ///< parses XML added by 'raw' specials
};

#endif
