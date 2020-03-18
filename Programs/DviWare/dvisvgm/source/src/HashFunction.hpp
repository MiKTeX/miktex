/*************************************************************************
** HashFunction.hpp                                                     **
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

#ifndef HASHFUNCTION_HPP
#define HASHFUNCTION_HPP

#include <istream>
#include <memory>
#include <string>
#include <vector>

/** Common base class for all hash functions. */
class HashFunction {
   public:
		virtual ~HashFunction () =default;
		virtual int digestSize () const =0;
		virtual void reset () =0;
		virtual void update (const char *data, size_t length) =0;
		virtual void update (const std::string &data) =0;
		virtual void update (const std::vector<uint8_t> &data) =0;
		virtual std::vector<uint8_t> digestValue () const =0;
		void update (std::istream &is);
		std::string digestString () const;
		static std::vector<std::string> supportedAlgorithms ();
		static bool isSupportedAlgorithm (const std::string &algo);
		static std::unique_ptr<HashFunction> create (const std::string &name);
		static std::unique_ptr<HashFunction> create (const std::string &name, const char *data, size_t length);
		static std::unique_ptr<HashFunction> create (const std::string &name, const std::string &data);
		static std::unique_ptr<HashFunction> create (const std::string &name, const std::vector<uint8_t> &data);
};

#endif
