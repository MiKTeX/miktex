/*************************************************************************
** SVGOutput.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Calculator.hpp"
#include "FileSystem.hpp"
#include "Message.hpp"
#include "SVGOutput.hpp"
#include "utility.hpp"
#include "ZLibOutputStream.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;

SVGOutput::SVGOutput (const char *base, const string &pattern, int zipLevel)
	: _path(base ? base : ""), _pattern(pattern), _stdout(base == 0), _zipLevel(zipLevel), _page(-1)
{
}


/** Returns an output stream for the given page.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages in the DVI file
 *  @return output stream for the given page */
ostream& SVGOutput::getPageStream (int page, int numPages) const {
	string fname = filename(page, numPages);
	if (fname.empty()) {
		_osptr.reset();
		return cout;
	}
	if (page == _page)
		return *_osptr;

	_page = page;
	if (_zipLevel > 0)
		_osptr = util::make_unique<ZLibOutputStream>(fname, _zipLevel);
	else
#if defined(MIKTEX_WINDOWS)
                _osptr = util::make_unique<ofstream>(UW_(fname.c_str()));
#else
		_osptr = util::make_unique<ofstream>(fname.c_str());
#endif
	if (!_osptr)
		throw MessageException("can't open file "+fname+" for writing");
	return *_osptr;
}


/** Returns the name of the SVG file containing the given page.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages */
string SVGOutput::filename (int page, int numPages) const {
	if (_stdout)
		return "";
	string expanded_pattern = util::trim(expandFormatString(_pattern, page, numPages));
	// set and expand default pattern if necessary
	if (expanded_pattern.empty())
		expanded_pattern = expandFormatString(numPages > 1 ? "%f-%p" : "%f", page, numPages);
	// append suffix if necessary
	FilePath outpath(expanded_pattern, true);
	if (outpath.suffix().empty())
		outpath.suffix(_zipLevel > 0 ? "svgz" : "svg");
	string abspath = outpath.absolute();
	string relpath = outpath.relative();
	return abspath.length() < relpath.length() ? abspath : relpath;
}


#if 0
string SVGOutput::outpath (int page, int numPages) const {
	string path = filename(page, numPages);
	if (path.empty())
		return "";
	size_t pos = path.rfind('/');
	if (pos == string::npos)
		return ".";
	if (pos == 0)
		return "/";
	return path.substr(0, pos);
}
#endif


/** Replaces expressions in a given string by the corresponding values and returns the result.
 *  Supported constructs:
 *  %f: basename of the current file (filename without suffix)
 *  %[0-9]?p: current page number
 *  %[0-9]?P: number of pages in DVI file
 *  %[0-9]?(expr): arithmetic expression */
string SVGOutput::expandFormatString (string str, int page, int numPages) const {
	string result;
	while (!str.empty()) {
		size_t pos = str.find('%');
		if (pos == string::npos) {
			result += str;
			str.clear();
		}
		else {
			result += str.substr(0, pos);
			str = str.substr(pos);
			pos = 1;
			ostringstream oss;
			if (isdigit(str[pos])) {
				oss << setw(str[pos]-'0') << setfill('0');
				pos++;
			}
			else {
				oss << setw(util::ilog10(numPages)+1) << setfill('0');
			}
			switch (str[pos]) {
				case 'f':
					result += _path.basename();
					break;
				case 'p':
				case 'P':
					oss << (str[pos] == 'p' ? page : numPages);
					result += oss.str();
					break;
				case '(': {
					size_t endpos = str.find(')', pos);
					if (endpos == string::npos)
						throw MessageException("missing ')' in filename pattern");
					else if (endpos-pos-1 > 1) {
						try {
							Calculator calculator;
							calculator.setVariable("p", page);
							calculator.setVariable("P", numPages);
							oss << floor(calculator.eval(str.substr(pos, endpos-pos+1)));
							result += oss.str();
						}
						catch (CalculatorException &e) {
							oss.str("");
							oss << "error in filename pattern (" << e.what() << ")";
							throw MessageException(oss.str());
						}
						pos = endpos;
					}
					break;
				}
			}
			str = str.substr(pos+1);
		}
	}
	return result;
}
