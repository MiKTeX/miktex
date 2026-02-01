/*************************************************************************
** PageRanges.cpp                                                       **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <numeric>
#include "algorithm.hpp"
#include "Calculator.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "MessageException.hpp"
#include "PageRanges.hpp"

using namespace std;

using FilterFunc = bool (*)(int);

static bool is_even (int n) {return n % 2 == 0;}
static bool is_odd (int n) {return n % 2 == 1;}


/** Replaces expressions in a given string by the corresponding values and returns the result.
 *  Supported expressions:
 *  %P: maximum page number
 *  %(expr): arithmetic expression
 *  @param[in] str string to expand
 *  @param[in] max_page maximum page number
 *  @return the expanded string */
static string expand_variables (string str, int max_page) {
	string result;
	while (!str.empty()) {
		auto pos = str.find('%');
		if (pos == string::npos) {
			result += str;
			str.clear();
		}
		else {
			result += str.substr(0, pos);
			str = str.substr(pos);
			pos = 1;
			switch (str[pos]) {
				case 'P': result += to_string(max_page); break;
				case '(': {
					auto endpos = str.find(')', pos);
					if (endpos == string::npos)
						throw MessageException("missing ')' in page argument");
					if (endpos-pos > 1) {
						try {
							Calculator calculator;
							calculator.setVariable("P", max_page);
							result += to_string(static_cast<int>(calculator.eval(str.substr(pos, endpos-pos+1))));
						}
						catch (CalculatorException &e) {
							throw MessageException("error in page argument (" + string(e.what()) + ")");
						}
						pos = endpos;
					}
					break;
				}
				default:
					throw MessageException("invalid expression '%" + str.substr(pos,1) + "' in page argument");
			}
			str = str.substr(pos+1);
		}
	}
	return result;
}


/** Analyzes a string describing a range sequence.
 *  Syntax: ([0-9]+(-[0-9]*)?)|(-[0-9]+)(,([0-9]+(-[0-9]*)?)|(-[0-9]+))*
 *  @param[in] str string to parse
 *  @param[in] max_page greatest allowed value
 *  @return true on success; false denotes a syntax error */
bool PageRanges::parse (string str, int max_page) {
	if (max_page > 0)
		str = expand_variables(str, max_page);
	StringInputBuffer ib(str);
	BufferInputReader ir(ib);
	while (ir && ir.peek() != ':') {
		int first=1;
		int last=max_page;
		ir.skipSpace();
		if (!isdigit(ir.peek()) && ir.peek() != '-')
			return false;
		if (isdigit(ir.peek()))
			first = last = ir.getInt();
		ir.skipSpace();
		if (ir.peek() == '-') {
			while (ir.peek() == '-')
				ir.get();
			ir.skipSpace();
			last = isdigit(ir.peek()) ? ir.getInt() : max_page;
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
		if (first <= max_page || max_page == 0) {
			if (max_page > 0) {
				first = min(first, max_page);
				last  = min(last, max_page);
			}
			addRange(first, last);
		}
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
	return algo::accumulate(*this, 0, [](int sum, const Range &range) {
		return sum + range.second - range.first + 1;
	});
}
