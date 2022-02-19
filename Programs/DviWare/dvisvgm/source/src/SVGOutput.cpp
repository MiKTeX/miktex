/*************************************************************************
** SVGOutput.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;


SVGOutput::SVGOutput (const string &base, string pattern, int zipLevel)
	: _path(base), _pattern(std::move(pattern)), _stdout(base.empty()), _zipLevel(zipLevel)
{
}


/** Returns an output stream for the given page.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages in the DVI file
 *  @param[in] hash hash value of the current page
 *  @return output stream for the given page */
ostream& SVGOutput::getPageStream (int page, int numPages, const HashTriple &hashes) const {
	FilePath path = filepath(page, numPages, hashes);
	if (path.empty()) {
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
		_osptr = util::make_unique<ZLibOutputFileStream>(path.absolute(), ZLIB_GZIP, _zipLevel);
	else
#if defined(MIKTEX_WINDOWS)
                _osptr = util::make_unique<ofstream>(EXPATH_(path.absolute()));
#else
		_osptr = util::make_unique<ofstream>(path.absolute());
#endif
	if (!_osptr)
		throw MessageException("can't open file "+path.shorterAbsoluteOrRelative()+" for writing");
	return *_osptr;
}


/** Returns the path of the SVG file containing the given page number.
 *  @param[in] page number of current page
 *  @param[in] numPages total number of pages
 *  @param[in] hash hash value of current page */
FilePath SVGOutput::filepath (int page, int numPages, const HashTriple &hashes) const {
	FilePath outpath;
	if (!_stdout) {
		string expanded_pattern = util::trim(expandFormatString(_pattern, page, numPages, hashes));
		// set and expand default pattern if necessary
		if (expanded_pattern.empty()) {
			string pattern = hashes.empty() ? (numPages > 1 ? "%f-%p" : "%f") : "%f-%hd";
			expanded_pattern = expandFormatString(pattern, page, numPages, hashes);
		}
		// append suffix if necessary
		outpath.set(expanded_pattern, true);
		if (outpath.suffix().empty())
			outpath.suffix(_zipLevel > 0 ? "svgz" : "svg");
	}
	return outpath;
}


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
				case 'n':
				case 'N':
					oss << (str[pos] == 'n' ? _fileNumber : _fileCount);
					result += oss.str();
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
							throw MessageException("error in filename pattern (" + string(e.what()) + ")");
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
