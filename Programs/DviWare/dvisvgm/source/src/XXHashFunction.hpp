/*************************************************************************
** XXHashFunction.hpp                                                   **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef XXHASHFUNCTION_HPP
#define XXHASHFUNCTION_HPP

#include <xxhash.h>
#include "HashFunction.hpp"


template <int HASH_SIZE>
struct XXHInterface {
};

template<>
struct XXHInterface<4> {
	using State = XXH32_state_t;
	static constexpr auto createState = &XXH32_createState;
	static constexpr auto freeState = &XXH32_freeState;
	static constexpr auto reset = &XXH32_reset;
	static constexpr auto update = &XXH32_update;
	static constexpr auto digest = &XXH32_digest;
};

template<>
struct XXHInterface<8> {
	using State = XXH64_state_t;
	static constexpr auto createState = &XXH64_createState;
	static constexpr auto freeState = &XXH64_freeState;
	static constexpr auto reset = &XXH64_reset;
	static constexpr auto update = &XXH64_update;
	static constexpr auto digest = &XXH64_digest;
};


/** Implements the HashFunction class for the xxHash algorithms. */
template <int HASH_BYTES>
class XXHashFunction : public HashFunction {
	using Interface = XXHInterface<HASH_BYTES>;
	public:
		XXHashFunction () : _state(Interface::createState()) {Interface::reset(_state, 0);}
		XXHashFunction(const char *data, size_t length) : XXHashFunction() {update(data, length);}
		XXHashFunction(const std::string &data) : XXHashFunction() {update(data);}
		XXHashFunction(const std::vector<uint8_t> &data) : XXHashFunction() {update(data);}
		~XXHashFunction () {Interface::freeState(_state);}
		int digestSize () const override {return HASH_BYTES;}
		void reset () override {Interface::reset(_state, 0);}
		void update (const char *data, size_t length) override {Interface::update(_state, data, length);}
		void update (const std::string &data) override {update(data.data(), data.length());}
		void update (const std::vector<uint8_t> &data) override {update(reinterpret_cast<const char*>(data.data()), data.size());}

		std::vector<uint8_t> digestValue () const override {
			std::vector<uint8_t> hash(HASH_BYTES);
			auto digest = Interface::digest(_state);
			for (int i=HASH_BYTES-1; i >= 0; i--) {
				hash[i] = digest & 0xff;
				digest >>= 8;
			}
			return hash;
		}

		static unsigned version () {return XXH_versionNumber();}

	private:
		typename Interface::State *_state;
};

using XXH32HashFunction = XXHashFunction<4>;
using XXH64HashFunction = XXHashFunction<8>;

#endif
