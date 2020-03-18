/*************************************************************************
** ToUnicodeMap.hpp                                                     **
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

#ifndef TOUNICODEMAP_HPP
#define TOUNICODEMAP_HPP

#include "NumericRanges.hpp"
#include "RangeMap.hpp"


/** Represents a mapping from character indexes to unicode points. */
class ToUnicodeMap : public RangeMap
{
	public:
		bool addMissingMappings (uint32_t maxIndex);

	protected:
		bool fillRange (uint32_t minIndex, uint32_t maxIndex, uint32_t ucp, NumericRanges<uint32_t> &used_ucps, bool ascending);
};

#endif
