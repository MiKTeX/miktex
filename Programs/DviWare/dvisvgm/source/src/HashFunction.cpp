/*************************************************************************
** HashFunction.cpp                                                     **
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

#if defined(MIKTEX)
#  include <config.h>
#endif
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "HashFunction.hpp"
#include "utility.hpp"
#include "MD5HashFunction.hpp"
#include "XXHashFunction.hpp"

using namespace std;


/** Returns a vector containing the names of the currently supported hash algorithms. */
vector<string> HashFunction::supportedAlgorithms () {
	return vector<string> {
		"md5", "xxh32", "xxh64",
#ifdef ENABLE_XXH128
		"xxh128"
#endif
	};
}


/** Returns true if 'algo' is the name of a supported hash algorithm. */
bool HashFunction::isSupportedAlgorithm (const std::string &algo) {
	auto algos = supportedAlgorithms();
	return find(algos.begin(), algos.end(), algo) != algos.end();
}


/** Creates a hash function for a given algorithm name/identifier.
 *  The following names are currently supported: md5, xxh32, xxh64. */
unique_ptr<HashFunction> HashFunction::create (const string &name) {
	string lowerName = util::tolower(name);
	if (lowerName == "md5")
		return util::make_unique<MD5HashFunction>();
	if (lowerName == "xxh32")
		return util::make_unique<XXH32HashFunction>();
	if (lowerName == "xxh64")
		return util::make_unique<XXH64HashFunction>();
#ifdef ENABLE_XXH128
	if (lowerName == "xxh128")
		return util::make_unique<XXH128HashFunction>();
#endif
	return nullptr;
}


std::unique_ptr<HashFunction> HashFunction::create (const string &name, const char *data, size_t length) {
	auto hashfunc = create(name);
	if (hashfunc)
		hashfunc->update(data, length);
	return hashfunc;
}


std::unique_ptr<HashFunction> HashFunction::create (const string &name, const string &data) {
	return create(name, data.data(), data.length());
}


std::unique_ptr<HashFunction> HashFunction::create (const string &name, const vector<uint8_t> &data) {
	return create(name, reinterpret_cast<const char*>(data.data()), data.size());
}


void HashFunction::update (istream &is) {
	char buf[4096];
	while (is) {
		is.read(buf, 4096);
		update(buf, is.gcount());
	}
}


/** Returns the current digest as hexadecimal value. */
string HashFunction::digestString () const {
	ostringstream oss;
	oss << hex << setfill('0');
	for (int byte : digestBytes())
		oss << setw(2) << byte;
	return oss.str();
}
