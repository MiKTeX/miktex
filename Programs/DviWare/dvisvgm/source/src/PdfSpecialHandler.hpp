/*************************************************************************
** PdfSpecialHandler.hpp                                                **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef PDFSPECIALHANDLER_HPP
#define PDFSPECIALHANDLER_HPP

#include "SpecialHandler.hpp"

class PdfSpecialHandler : public SpecialHandler
{
	public:
		PdfSpecialHandler ();
		const char* info () const override {return "pdfTeX font map specials";}
		const char* name () const override {return "pdf";}
		const char** prefixes () const override;
		bool process (const char *prefix, std::istream &is, SpecialActions &actions) override;

	private:
		bool _maplineProcessed;  ///< true if a mapline or mapfile special has already been processed
};

#endif
