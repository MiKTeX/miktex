/*************************************************************************
** utility.cpp                                                          **
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
#include <cctype>
#include <fstream>
#include <functional>
#include <iterator>
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;


/** Returns a given string with leading and trailing whitespace removed.
 *  @param[in] str the string to process
 *  @param[in] ws characters treated as whitespace
 *  @return the trimmed string */
string util::trim (const std::string &str, const char *ws) {
	size_t first = str.find_first_not_of(ws);
	if (first == string::npos)
		return "";
	size_t last = str.find_last_not_of(ws);
	return str.substr(first, last-first+1);
}


/** Removes leading and trailing whitespace from a given string, and replaces
 *  all other whitespace sequences by single spaces.
 *  @param[in] str the string to process
 *  @param[in] ws characters treated as whitespace
 *  @return the normalized string */
string util::normalize_space (string str, const char *ws) {
	str = trim(str);
	size_t first = str.find_first_of(ws);
	while (first != string::npos) {
		size_t last = str.find_first_not_of(ws, first);
		str.replace(first, last-first, " ");
		first = str.find_first_of(ws, first+1);
	}
	return str;
}


/** Replaces all occurences of a substring with another string.
 *  @param[in] str string to search through
 *  @param[in] find string to look for
 *  @param[in] repl replacement for "find"
 *  @return the resulting string */
string util::replace (string str, const string &find, const string &repl) {
	if (!find.empty() && !repl.empty()) {
		size_t first = str.find(find);
		while (first != string::npos) {
			str.replace(first, find.length(), repl);
			first = str.find(find, first+repl.length());
		}
	}
	return str;
}


/** Splits a string at all occurences of a given separator string and
 *  returns the substrings.
 *  @param[in] str string to split
 *  @param[in] sep separator to look for
 *  @return the substrings between the separators */
vector<string> util::split (const string &str, const string &sep) {
	vector<string> parts;
	if (str.empty() || sep.empty())
		parts.emplace_back(str);
	else {
		size_t left=0;
		while (left <= str.length()) {
			size_t right = str.find(sep, left);
			if (right == string::npos) {
				parts.emplace_back(str.substr(left));
				left = string::npos;
			}
			else {
				parts.emplace_back(str.substr(left, right-left));
				left = right+sep.length();
			}
		}
	}
	return parts;
}


string util::tolower (const string &str) {
	string ret=str;
	transform(str.begin(), str.end(), ret.begin(), ::tolower);
	return ret;
}


/** Returns the integer part of log10 of a given integer \f$n>0\f$.
 *  If \f$n<0\f$, the result is 0. */
int util::ilog10 (int n) {
	int result = 0;
	while (n >= 10) {
		result++;
		n /= 10;
	}
	return result;
}


/** Returns the contents of a file.
 *  @param[in] fname name/path of the file */
string util::read_file_contents (const string &fname) {
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(UW_(fname), ios::binary);
#else
	ifstream ifs(fname, ios::binary);
#endif
	return string(istreambuf_iterator<char>(ifs.rdbuf()), istreambuf_iterator<char>());
}


/** Writes a sequence of bytes given as a string to a file.
 *  @param[in] name/path of the file to write
 *  @param[in] start iterator pointing to the begin of the byte sequence
 *  @param[in] end iterator pointing to the first byte after the byte sequence to write */
void util::write_file_contents (const string &fname, string::iterator start, string::iterator end) {
#if defined(MIKTEX_WINDOWS)
        ofstream ofs(UW_(fname), ios::binary);
#else
	ofstream ofs(fname, ios::binary);
#endif
	copy(start, end, ostream_iterator<char>(ofs));
}
