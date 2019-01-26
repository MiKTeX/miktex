/*************************************************************************
** MD5HashFunction.hpp                                                  **
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

#ifndef MD5HASHFUNCTION_HPP
#define MD5HASHFUNCTION_HPP

#include <config.h>
#if defined(MIKTEX)
#  include <miktex/Core/MD5>
#else
#ifdef HAVE_OPENSSL_MD5_H
#	include <openssl/md5.h>
#else
#	include <md5.h>
#endif
#endif

#include "HashFunction.hpp"

class MD5HashFunction : public HashFunction {
	public:
#if defined(MIKTEX)
          MD5HashFunction()
          {
          }
#else
		MD5HashFunction () {MD5_Init(&_context);}
#endif
		MD5HashFunction (const char *data, size_t length) : MD5HashFunction() {update(data, length);}
		MD5HashFunction (const std::string &data) : MD5HashFunction() {update(data);}
		MD5HashFunction (const std::vector<uint8_t> &data) : MD5HashFunction() {update(data);}
		int digestSize () const override {return 16;}
#if defined(MIKTEX)
                void reset() override
                {
                  md5Builder.Init();
                }
#else
		void reset () override {MD5_Init(&_context);}
#endif
#if defined(MIKTEX)
                void update(const char* data, size_t length) override
                {
                  md5Builder.Update(data, length);
                }
#else
		void update (const char *data, size_t length) override {MD5_Update(&_context, data, length);}
#endif
		void update (const std::string &data) override {update(data.data(), data.length());}
		void update (const std::vector<uint8_t> &data) override {update(reinterpret_cast<const char*>(data.data()), data.size());}

		std::vector<uint8_t> digestValue () const override {
			std::vector<uint8_t> hash(16);
#if defined(MIKTEX)
                        auto tmpContext = md5Builder;
                        memcpy(hash.data(), tmpContext.Final().data(), 16);
#else
			MD5_CTX savedContext = _context;
			MD5_Final(hash.data(), &_context);   // also erases the context structure
			_context = savedContext;
#endif
			return hash;
		}

	private:
#if defined(MIKTEX)
          MiKTeX::Core::MD5Builder md5Builder;
#else
		mutable MD5_CTX _context;
#endif
};

#endif

