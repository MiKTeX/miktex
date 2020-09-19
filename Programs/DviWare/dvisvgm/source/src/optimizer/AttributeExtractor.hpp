/*************************************************************************
** AttributeExtractor.hpp                                               **
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

#include <set>
#include <string>
#include "OptimizerModule.hpp"
#include "../XMLNode.hpp"

/** Moves common attributes of adjacent elements to enclosing groups. */
class AttributeExtractor : public OptimizerModule {
	using Attribute = XMLElement::Attribute;

	/** Represents a range of adjacent nodes where all elements have a common attribute. */
	struct AttributeRun {
		public:
			AttributeRun (const Attribute &attr, XMLElement *first);
			XMLNode* first () {return _first;}
			XMLNode* last ()  {return _last;}
			int length () const {return _length;}

		private:
			int _length;  ///< run length excluding non-element nodes
			XMLNode *_first, *_last;  ///< first and last node in run
	};

	public:
		void execute (XMLElement*, XMLElement *context) override {execute(context, true);};
		const char* info () const override;
		static bool groupable (const XMLElement &elem);
		static bool inheritable (const Attribute &attrib);
		static bool extractable (const Attribute &attr, XMLElement &element);

	protected:
		void execute (XMLElement *context, bool recurse);
		XMLNode* extractAttribute (XMLElement *elem);
		bool extracted (const Attribute &attr) const;

	private:
		std::set<std::string> _extractedAttributes;
		static constexpr int MIN_RUN_LENGTH = 3;
};

