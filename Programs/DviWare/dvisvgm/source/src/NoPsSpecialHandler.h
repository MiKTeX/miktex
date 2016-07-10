/*************************************************************************
** NoPsSpecialHandler.h                                                 **
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

#ifndef DVISVGM_NOPSSPECIALHANDLER_H
#define DVISVGM_NOPSSPECIALHANDLER_H

#include "SpecialHandler.h"

class NoPsSpecialHandler : public SpecialHandler, public DVIEndPageListener
{
	public:
		NoPsSpecialHandler () : _count(0) {}
		bool process (const char *prefix, std::istream &is, SpecialActions &actions);
		const char* name () const   {return 0;}
		const char* info () const   {return 0;}
		const char** prefixes () const;

	protected:
		void dviEndPage (unsigned pageno, SpecialActions &actions);

	private:
		unsigned _count;  // number of PS specials skipped
};

#endif
