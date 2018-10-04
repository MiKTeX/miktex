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


SVGOutput::SVGOutput (const string &base, const string &pattern, int zipLevel)
	: _path(base), _pattern(pattern), _stdout(base.empty()), _zipLevel(zipLevel), _page(-1)
{
}


/** Returns an output stream for the given page.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages in the DVI file
 *  @param[in] hash hash value of the current page
 *  @return output stream for the given page */
ostream& SVGOutput::getPageStream (int page, int numPages, const HashTriple &hashes) const {
	string fname = filename(page, numPages, hashes);
	if (fname.empty()) {
		if (_zipLevel == 0) {
			_osptr.reset();
			return cout;
		}
#ifdef _WIN32
		if (_setmode(_fileno(stdout), _O_BINARY) == -1)
			throw MessageException("can't open stdout in binary mode");
#endif
		return *(_osptr = util::make_unique<ZLibOutputStream>(cout, ZLIB_GZIP, _zipLevel));
	}
	if (page == _page)
		return *_osptr;

	_page = page;
	if (_zipLevel > 0)
		_osptr = util::make_unique<ZLibOutputFileStream>(fname, ZLIB_GZIP, _zipLevel);
	else
#if defined(MIKTEX_WINDOWS)
                _osptr = util::make_unique<ofstream>(UW_(fname));
#else
		_osptr = util::make_unique<ofstream>(fname);
#endif
	if (!_osptr)
		throw MessageException("can't open file "+fname+" for writing");
	return *_osptr;
}


/** Returns the name of the SVG file containing the given page.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages
 *  @param[in] hash hash value of current page */
string SVGOutput::filename (int page, int numPages, const HashTriple &hashes) const {
	if (_stdout)
		return "";

	string expanded_pattern = util::trim(expandFormatString(_pattern, page, numPages, hashes));
	// set and expand default pattern if necessary
	if (expanded_pattern.empty()) {
		string pattern = hashes.empty() ? (numPages > 1 ? "%f-%p" : "%f") : "%f-%hd";
		expanded_pattern = expandFormatString(pattern, page, numPages, hashes);
	}
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
 *  %h: hash value of current page
 *  %[0-9]?p: current page number
 *  %[0-9]?P: number of pages in DVI file
 *  %[0-9]?(expr): arithmetic expression
 *  @param[in] str string to expand
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages
 *  @param[in] hash hash value of current page (skipped if empty) */
string SVGOutput::expandFormatString (string str, int page, int numPages, const HashTriple &hashes) const {
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
			if (!isdigit(str[pos]))
				oss << setw(util::ilog10(numPages)+1) << setfill('0');
			else {
				oss << setw(str[pos]-'0') << setfill('0');
				pos++;
			}
			switch (str[pos]) {
				case 'f':
					result += _path.basename();
					break;
				case 'h': {
					char variant = pos+1 < str.length() ? str[++pos] : '\0';
					switch (variant) {
						case 'd': result += hashes.dviHash(); break;
						case 'c': result += hashes.cmbHash(); break;
						case 'o': result += hashes.optHash(); break;
						default:
							throw MessageException("hash type 'd', 'c', or 'o' expected after '%h' in filename pattern");
					}
					break;
				}
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


/** Returns true if methods 'filename' and 'getPageStream' ignore the hash
 *  parameter because it's not requested in the filename pattern. */
bool SVGOutput::ignoresHashes () const {
	return _stdout || (!_pattern.empty() && _pattern.find("%h") == string::npos);
}
