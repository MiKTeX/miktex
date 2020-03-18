/*************************************************************************
** PDFParser.hpp                                                        **
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

#ifndef PDFPARSER_HPP
#define PDFPARSER_HPP

#include <initializer_list>
#include <istream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <mpark/variant.hpp>
#include "MessageException.hpp"

template <typename K, typename V>
class Dictionary {
	using Map = std::map<K, V>;

	public:
		typename Map::const_iterator begin () const {return _map.begin();}
		typename Map::const_iterator end () const   {return _map.end();}
		typename Map::iterator begin () {return _map.begin();}
		typename Map::iterator end ()   {return _map.end();}
		typename Map::const_iterator find (const K &key) const {return _map.find(key);}
		bool empty () const {return _map.empty();}
		bool exists (const K &key) const {return _map.find(key) != _map.end();}
		size_t size () const {return _map.size();}

		const V* get (const K &key) const {
			auto it = _map.find(key);
			if (it != _map.end())
				return &it->second;
			return nullptr;
		}

		std::pair<typename Map::iterator,bool> emplace (const K &key, V &&value) {
			return _map.emplace(key, std::forward<V>(value));
		}

	private:
		Map _map;
};

//////////////////////////////////////////////////////////////////////////
// PDF object types

struct PDFNull {};
struct PDFStream {};

struct PDFIndirectObject {
	PDFIndirectObject (int n, int gen) : objnum(n), gennum(gen) {}
	int objnum, gennum;
};

struct PDFObjectRef {
	PDFObjectRef (int n, int gen) : objnum(n), gennum(gen) {}
	int objnum, gennum;
};

struct PDFOperator {
	explicit PDFOperator (std::string name) : opname(std::move(name)) {}
	std::string opname;
};

struct PDFName {
	explicit PDFName (std::string val) : str(std::move(val)) {}
	bool operator == (const PDFName &name) const {return str == name.str;}
	std::string str;
};

class PDFObject;

using PDFArray = std::vector<PDFObject>;
using PDFDict = Dictionary<std::string, PDFObject>;

//////////////////////////////////////////////////////////////////////////

/** This class represents a single variadic PDF object. */
class PDFObject {
	using Value = mpark::variant<
		PDFNull,
		bool,
		int,
		double,
		PDFName,
		PDFStream,
		PDFIndirectObject,
		PDFObjectRef,
		PDFOperator,
		std::string,
		std::unique_ptr<PDFArray>,
		std::unique_ptr<PDFDict>
	>;

	public:
		PDFObject () : _value(0) {}

		template <typename T>
		explicit PDFObject (T &&value) : _value(std::forward<T>(value)) {}

		explicit PDFObject (const char *value) : _value(std::string(value)) {}
		explicit operator std::string () const;
		explicit operator double () const;

		template <typename T>
		const T* get () const {return mpark::get_if<T>(&_value);}

		void write (std::ostream &os) const;

	private:
		Value _value;
};


template<> inline const PDFArray* PDFObject::get() const {
	if (auto p = mpark::get_if<std::unique_ptr<PDFArray>>(&_value))
		return &(**p);
	return nullptr;
}


template<> inline const PDFDict* PDFObject::get() const {
	if (auto p = mpark::get_if<std::unique_ptr<PDFDict>>(&_value))
		return &(**p);
	return nullptr;
}


inline std::ostream& operator << (std::ostream &os, const PDFObject &obj) {
	obj.write(os);
	return os;
}

//////////////////////////////////////////////////////////////////////////

class InputReader;

class PDFParser {
	public:
		using PDFOperatorHandler = std::function<void (const std::string&, std::vector<PDFObject>&)>;

	public:
		std::vector<PDFObject> parse (std::istream &is);
		std::vector<PDFObject> parse (const std::string &str);
		std::vector<PDFObject> parse (InputReader &ir);
		void parse (InputReader &ir, std::vector<PDFObject> &objects);

		std::vector<PDFObject> parse (std::istream &is, const PDFOperatorHandler &opHandler);
		std::vector<PDFObject> parse (const std::string &str, const PDFOperatorHandler &opHandler);
		std::vector<PDFObject> parse (InputReader &ir, const PDFOperatorHandler &opHandler);
		void parse (InputReader &ir, std::vector<PDFObject> &objects, const PDFOperatorHandler &opHandler);

	protected:
		PDFArray parseArray (InputReader &ir, const PDFOperatorHandler &opHandler);
		PDFDict parseDict (InputReader &ir, const PDFOperatorHandler &opHandler);
};


/** If errors occur while parsing a sequence of PDF objects, an instance of this exception is thrown. */
struct PDFException : public MessageException {
	explicit PDFException (const std::string &msg) : MessageException(msg) {}
};

#endif
