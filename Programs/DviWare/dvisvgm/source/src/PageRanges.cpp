/*************************************************************************
** PageRanges.cpp                                                       **
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

#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "PageRanges.hpp"

using namespace std;


/** Analyzes a string describing a range sequence.
 *  Syntax: ([0-9]+(-[0-9]*)?)|(-[0-9]+)(,([0-9]+(-[0-9]*)?)|(-[0-9]+))*
 *  @param[in] str string to parse
 *  @param[in] max_page greatest allowed value
 *  @return true on success; false denotes a syntax error */
bool PageRanges::parse (const string &str, int max_page) {
	StringInputBuffer ib(str);
	BufferInputReader ir(ib);
	while (ir) {
		int first=1;
		int last=max_page;
		ir.skipSpace();
		if (!isdigit(ir.peek()) && ir.peek() != '-')
			return false;

		if (isdigit(ir.peek()))
			first = ir.getInt();
		ir.skipSpace();
		if (ir.peek() == '-') {
			while (ir.peek() == '-')
				ir.get();
			ir.skipSpace();
			if (isdigit(ir.peek()))
				last = ir.getInt();
		}
		else
			last = first;
		ir.skipSpace();
		if (ir.peek() == ',') {
			ir.get();
			if (ir.eof())
				return false;
		}
		else if (!ir.eof())
			return false;
		if (first > last)
			swap(first, last);
		first = max(1, first);
		last  = max(first, last);
		if (max_page > 0) {
			first = min(first, max_page);
			last  = min(last, max_page);
		}
		addRange(first, last);
	}
	return true;
}


/** Returns the number of pages. */
size_t PageRanges::numberOfPages () const {
	size_t sum=0;
	for (const auto &entry : *this)
		sum += entry.second - entry.first + 1;
	return sum;
}
