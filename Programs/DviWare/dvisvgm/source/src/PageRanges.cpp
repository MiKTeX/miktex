/*************************************************************************
** PageRanges.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "PageRanges.hpp"

using namespace std;

using FilterFunc = bool (*)(int);

static bool is_even (int n) {return n % 2 == 0;}
static bool is_odd (int n) {return n % 2 == 1;}


/** Analyzes a string describing a range sequence.
 *  Syntax: ([0-9]+(-[0-9]*)?)|(-[0-9]+)(,([0-9]+(-[0-9]*)?)|(-[0-9]+))*
 *  @param[in] str string to parse
 *  @param[in] max_page greatest allowed value
 *  @return true on success; false denotes a syntax error */
bool PageRanges::parse (const string &str, int max_page) {
	StringInputBuffer ib(str);
	BufferInputReader ir(ib);
	while (ir && ir.peek() != ':') {
		int first=1;
		int last=max_page;
		ir.skipSpace();
		if (!isdigit(ir.peek()) && ir.peek() != '-')
			return false;
		if (isdigit(ir.peek()))
			first = ir.getInt();
		ir.skipSpace();
		if (ir.peek() != '-')
			last = first;
		else {
			while (ir.peek() == '-')
				ir.get();
			ir.skipSpace();
			if (isdigit(ir.peek()))
				last = ir.getInt();
		}
		ir.skipSpace();
		if (ir.peek() == ',') {
			ir.get();
			if (ir.eof())
				return false;
		}
		else if (!ir.eof() && ir.peek() != ':')
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
	// apply filter if present
	if (ir.peek() == ':') {
		ir.get();
		string filterName = ir.getWord();
		FilterFunc filterFunc;
		if (filterName == "even")
			filterFunc = &is_even;
		else if (filterName == "odd")
			filterFunc = &is_odd;
		else
			return false;
		*this = filter(filterFunc);
	}
	return true;
}


/** Returns a new PageRanges object that contains only the values
 *  for which the given filter function returns true. */
PageRanges PageRanges::filter (FilterFunc filterFunc) const {
	PageRanges newRanges;
	if (filterFunc == nullptr)
		newRanges = *this;
	else {
		for (const auto &range : *this) {
			for (int i=range.first; i <= range.second; i++)
				if (filterFunc(i))
					newRanges.addRange(i, i);
		}
	}
	return newRanges;
}


/** Returns the number of pages. */
size_t PageRanges::numberOfPages () const {
	size_t sum=0;
	for (const auto &entry : *this)
		sum += entry.second - entry.first + 1;
	return sum;
}
