/*************************************************************************
** GroupCollapser.hpp                                                   **
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

#pragma once

#include "OptimizerModule.hpp"

/** Joins the attributes of nested groups and removes groups without attributes. */
class GroupCollapser : public OptimizerModule {
	public:
		void execute (XMLElement*, XMLElement *context) override {execute(context);};
		void execute (XMLElement *context) {execute(context, 0);}
		const char* info () const override;

		static bool COMBINE_TRANSFORMS;

	protected:
		void execute (XMLElement *context, int depth);
		bool moveAttributes (XMLElement &source, XMLElement &dest);
		static bool collapsible (const XMLElement &elem);
		static bool unwrappable (const XMLElement &source, const XMLElement &dest);

	private:
		bool _transformCombined=false;  ///< true if transform attributes have been combined
};
