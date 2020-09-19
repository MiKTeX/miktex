/*************************************************************************
** TTFAutohint.hpp                                                      **
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

#ifndef TTFAUTOHINT_HPP
#define TTFAUTOHINT_HPP

#include <config.h>
#include <string>

#if defined(HAVE_TTFAUTOHINT_H) && !defined(HAVE_LIBTTFAUTOHINT)
#include "DLLoader.hpp"
#endif

class TTFAutohint
#if defined(HAVE_TTFAUTOHINT_H) && !defined(HAVE_LIBTTFAUTOHINT)
	: public DLLoader
#endif
{
	public:
		TTFAutohint ();
		TTFAutohint (const TTFAutohint &ta) =delete;
		bool available () const;
		int autohint (const std::string &source, const std::string &target, bool rehintIfSymbolFont);
		std::string lastErrorMessage () const;
		std::string version () const;

#ifdef HAVE_TTFAUTOHINT_H
	private:
		const unsigned char *_lastErrorMessage;
#endif
};

#endif

