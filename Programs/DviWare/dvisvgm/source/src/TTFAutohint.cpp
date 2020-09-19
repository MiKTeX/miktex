/*************************************************************************
** TTFAutohint.cpp                                                      **
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

#include "TTFAutohint.hpp"

using namespace std;

#ifndef HAVE_TTFAUTOHINT_H
TTFAutohint::TTFAutohint () {}

bool TTFAutohint::available () const {
	return false;
}

int TTFAutohint::autohint (const string&, const string&, bool) {
	return 0;
}

string TTFAutohint::lastErrorMessage () const {
	return "";
}

string TTFAutohint::version () const {
	return "";
}

#else  // HAVE_TTFAUTOHINT_H

#include <ttfautohint.h>

#ifndef HAVE_LIBTTFAUTOHINT
static string get_libttfautohint () {
#ifdef _WIN32
	return "ttfautohint.dll";
#else
	string dlname = "libttfautohint.so.1";
#ifdef __APPLE__
	DLLoader loader(dlname);
	if (!loader.loaded())
		dlname = "libttfautohint.1.dylib";
#endif
	return dlname;
#endif // _WIN32
}
#endif // HAVE_LIBTTFAUTOHINT


TTFAutohint::TTFAutohint () :
#ifndef HAVE_LIBTTFAUTOHINT
	DLLoader(get_libttfautohint()),
#endif
	_lastErrorMessage()
{
}


/** Returns true if the ttfautohint library is present. */
bool TTFAutohint::available () const {
#ifdef HAVE_LIBTTFAUTOHINT
	return true;
#else
	return loaded();
#endif
}


/** Runs ttfautohint on a given font file.
 *  @param[in] source name of TTF input file
 *  @param[in] target name of TTF output file
 *  @param[in] rehintIfSymbolFont if true, ttfautohint is run again with option "symbol" in case regular hinting is rejected.
 *  @return ttfautohint error code (0 = success) */
int TTFAutohint::autohint (const string &source, const string &target, bool rehintIfSymbolFont) {
#ifdef HAVE_LIBTTFAUTOHINT
	auto fn = &TTF_autohint;
#else
	auto fn = LOAD_SYMBOL(TTF_autohint);
#endif
	int ret=0;
	if (fn) {
		FILE *ttf_in = fopen(source.c_str(), "rb");
		FILE *ttf_out = fopen(target.c_str(), "wb");
		ret = fn("in-file, out-file, default-script, error-string", ttf_in, ttf_out, "latn", &_lastErrorMessage);
		if (ret == TA_Err_Missing_Glyph && rehintIfSymbolFont) {
			fseek(ttf_in, 0, SEEK_SET);
			fseek(ttf_out, 0, SEEK_SET);
			ret = fn("in-file, out-file, symbol, error-string", ttf_in, ttf_out, true, &_lastErrorMessage);
		}
		fclose(ttf_out);
		fclose(ttf_in);
	}
	return ret;
}


/** Returns the error message of the last autohint call. */
string TTFAutohint::lastErrorMessage () const {
	string message;
	if (_lastErrorMessage)
		message = reinterpret_cast<const char*>(_lastErrorMessage);
	return message;
}


/** Returns the version number of ttfautohint. */
string TTFAutohint::version () const {
#ifdef HAVE_LIBTTFAUTOHINT
	auto fn = &TTF_autohint_version;
#else
	auto fn = LOAD_SYMBOL(TTF_autohint_version);
#endif
	string ret;
	if (fn) {
		int major, minor, revision;
		fn(&major, &minor, &revision);
		ret = to_string(major)+"."+to_string(minor);
		if (revision)
			ret += "."+to_string(revision);
	}
	return ret;
}

#endif // HAVE_TTFAUTOHINT_H
