/*************************************************************************
** PapersizeSpecialHandler.hpp                                          **
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

#ifndef PAPERSIZESPECIALHANDLER_HPP
#define PAPERSIZESPECIALHANDLER_HPP

#include <utility>
#include <vector>
#include "Length.hpp"
#include "SpecialHandler.hpp"

class PapersizeSpecialHandler : public SpecialHandler {
	using DoublePair = std::pair<double,double>;       // (width, height)
	using PageSize = std::pair<unsigned,DoublePair>;   // page number -> (width, height)

	public:
		void preprocess (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		const char* info () const override {return "special to set the page size";}
		const char* name () const override {return "papersize";}
		std::vector<const char*> prefixes() const override;
		void storePaperSize (unsigned pageno, Length width, Length height);
		void reset () {_pageSizes.clear();}

	protected:
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;
		void applyPaperSize (unsigned pageno, SpecialActions &actions);

	private:
		std::vector<PageSize> _pageSizes;
};

#endif
