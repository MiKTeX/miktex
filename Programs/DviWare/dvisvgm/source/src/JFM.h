/*************************************************************************
** JFM.h                                                                **
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

#ifndef DVISVGM_JFM_H
#define DVISVGM_JFM_H

#include <istream>
#include "TFM.h"


class JFM : public TFM
{
	public:
		JFM (std::istream &is);
		bool verticalLayout () const  {return _vertical;}
		UInt16 minChar () const       {return _minchar;}
		UInt16 maxChar () const       {return static_cast<UInt16>(_minchar+_charTypeTable.size()-1);}

	protected:
		void readTables (StreamReader &reader, int nt, int nw, int nh, int nd, int ni);
		int charIndex (int c) const;

	private:
		UInt16 _minchar;  ///< character code of first entry in character type table
		bool _vertical;   ///< true if metrics refer to vertical text layout
		std::vector<UInt16> _charTypeTable;
};

#endif
