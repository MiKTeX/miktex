/*************************************************************************
** NoPsSpecialHandler.hpp                                               **
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

#ifndef NOPSSPECIALHANDLER_HPP
#define NOPSSPECIALHANDLER_HPP

#include "SpecialHandler.hpp"

class NoPsSpecialHandler : public SpecialHandler {
	public:
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		const char* name () const override {return nullptr;}
		const char* info () const override {return nullptr;}
		std::vector<const char*> prefixes() const override;

	protected:
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;

	private:
		size_t _count=0;  // number of PS specials skipped
};

#endif
