/*************************************************************************
** PDFParser.cpp                                                        **
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

#include <cctype>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include "InputReader.hpp"
#include "PDFParser.hpp"
#include "utility.hpp"

using namespace std;


/** Parses PDF from an input stream and returns the corresponding object representation.
 *  @param[in] is input stream the PDF data is read from
 *  @param[in] opHandler handler used to treat PDF operators
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (std::istream &is, const PDFOperatorHandler &opHandler) {
	StreamInputReader ir(is);
	return parse(ir, opHandler);
}


/** Parses PDF from a string and returns the corresponding object representation.
 *  @param[in] str string that contains the PDF data
 *  @param[in] opHandler handler used to treat PDF operators
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (const std::string &str, const PDFOperatorHandler &opHandler) {
	istringstream iss(str);
	return parse(iss, opHandler);
}


/** Parses PDF from an InputReader object and returns the corresponding object representation.
 *  @param[in] ir InputReader the PDF data is read from
 *  @param[in] opHandler handler used to treat PDF operators
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (InputReader &ir, const PDFOperatorHandler &opHandler) {
	vector<PDFObject> objects;
	while (!ir.eof()) {
		ir.skipSpace();
		if (ir.peek() == '%')  // comment?
			while (ir.get() != '\n' && !ir.eof());
		else if (!ir.eof())
			parse(ir, objects, opHandler);
	}
	return objects;
}


/** Default handler for PDF operators. Just adds the operators to the
 *  object vector without evaluating them.
 *  @param[in] opname name of the operator
 *  @param[in,out] objects vector holding the parsed objects */
static void append_operator (const string &opname, vector<PDFObject> &objects) {
	objects.emplace_back(PDFOperator(opname));
}


/** Parses PDF from an input stream and returns the corresponding object representation.
 *  @param[in] is input stream the PDF data is read from
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (std::istream &is) {
	return parse(is, append_operator);
}


/** Parses PDF from a string and returns the corresponding object representation.
 *  @param[in] str string that contains the PDF data
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (const std::string &str) {
	return parse(str, append_operator);
}

/** Parses PDF from an InputReader object and returns the corresponding object representation.
 *  @param[in] ir InputReader the PDF data is read from
 *  @return the parsed objects */
vector<PDFObject> PDFParser::parse (InputReader &ir) {
	return parse(ir, append_operator);
}


/** Parses PDF from an InputReader object and appends the recognized objects to a vector.
 *  @param[in] ir InputReader the PDF data is read from
 *  @param[in,out] objects the parsed PDF objects are appended to this vector
 *  @return the parsed objects */
void PDFParser::parse (InputReader &ir, vector<PDFObject> &objects) {
	parse(ir, objects, append_operator);
}


inline bool isoctaldigit (int c) {return c >= '0' && c <= '7';}

/** Parses a PDF escape sequence of the form \FOO, where FOO is a single
 *  character or a sequence of 1-3 octal digits
 *  @return pair (s,c), s=true if c contains a parsed character */
static pair<bool,int> parse_escape_seq (InputReader &ir) {
	// leading backslash has already been read
	if (isoctaldigit(ir.peek())) {
		string str;
		for (int i=0; i < 3 && isoctaldigit(ir.peek()); i++)
			str += static_cast<char>(ir.get());
		return pair<bool,int>{true, stoi(str, nullptr, 8)};
	}
	char c = static_cast<char>(ir.get());
	switch (c) {
		case 'n': c = '\n'; break;
		case 'r': c = '\r'; break;
		case 't': c = '\t'; break;
		case 'b': c = '\b'; break;
		case 'f': c = '\f'; break;
		case '\n':
		case '\r':
			if ((c == '\n' && ir.peek() == '\r') || (c == '\r' && ir.peek() == '\n'))
				ir.get();
			return pair<bool,int>{false, 0};
	}
	return pair<bool,int>{true, c};
}


/** Parses a literal PDF string of the form (FOO). */
static string parse_literal_string (InputReader &ir) {
	string str;
	ir.get();  // skip initial '('
	int open_parens=1;
	while (ir.peek() >= 0 && open_parens > 0) {
		if (ir.peek() == '\n' || ir.peek() == '\r')
			break;
		int c = ir.get();
		switch (c) {
			case '(': open_parens++; break;
			case ')': open_parens--; break;
			case '\\':
				pair<bool,int> state = parse_escape_seq(ir);
				c = state.first ? state.second : -1;
				break;
		}
		if (open_parens > 0 && c >= 0)
			str += static_cast<char>(c);
	}
	if (open_parens > 0)
		throw PDFException("missing ')' at end of literal string");
	return str;
}


/** Gets a single hex digit from the InputReader. */
static char get_hex_digit (InputReader &ir) {
	int c = ir.get();
	if (isxdigit(c))
		return char(c);
	throw PDFException("invalid hexadecimal digit '" + string(1, char(c)) + "'");
}


/** Parses a PDF hex string of the form <FOO>, where FOO is a sequence of
 *  hex digits optionally separated by whitespace. */
static string parse_hex_string (InputReader &ir) {
	// initial '<' has already been read
	string str;
	ir.skipSpace();
	while (ir.peek() > 0 && ir.peek() != '>') {
		string hexpair;
		hexpair += get_hex_digit(ir);
		ir.skipSpace();
		if (ir.peek() > 0 && ir.peek() != '>')
			hexpair += get_hex_digit(ir);
		else if (ir.peek() == '>')
			hexpair += '0';
		ir.skipSpace();
		str += static_cast<char>(stoi(hexpair, nullptr, 16));
	}
	if (ir.peek() != '>')
		throw PDFException("missing '>' at end of hexadecimal string");
	ir.get();  // skip closing '>'
	return str;
}


using NumberVariant = mpark::variant<int, double>;

/** Parses a PDF number from a string. The number is either integer or real.
 *  @param[in] str string to parse
 *  @param[out] nv variant holding the numeric value
 *  @return true if entire string has been parsed succesfully */
static bool parse_number (const string &str, NumberVariant &nv) {
	if (str.empty())
		return false;
	try {
		size_t dotpos = str.find('.');
		if (dotpos == string::npos) {      // not a real number?
			size_t count;
			nv = NumberVariant(stoi(str, &count, 10));  // then try to convert str to int
			return count == str.length();  // successful only if all characters have been processed
		}
		string postdot = str.substr(dotpos+1);
		// ensure signless integer after dot to exclude exponental notation
		// which is not allowed in PDF real number constants
		if (!postdot.empty() && isdigit(postdot[0])) {
			size_t count;
			stoi(postdot, &count, 10);
			if (count != postdot.length())
				return false;
		}
		size_t count;
		nv = NumberVariant(stod(str, &count));
		return count == str.length();
	}
	catch (invalid_argument &e) {
		return false;
	}
}


/** Parses a PDF array from the input stream and returns a corresponding object. */
PDFArray PDFParser::parseArray (InputReader &ir, const PDFOperatorHandler &opHandler) {
	ir.get();  // skip '['
	vector<PDFObject> localObjects;
	while (!ir.eof() && ir.peek() != ']')
		parse(ir, localObjects, opHandler);
	ir.skipSpace();
	if (ir.peek() != ']')
		throw PDFException("missing ']' at end of array");
	ir.get();
	PDFArray arr;
	std::move(localObjects.begin(), localObjects.end(), back_inserter(arr));
	return arr;
}


/** Parses a PDF dictionary from the input stream and returns a corresponding object.
 *  The function expects that the first opening angle bracket denoting the start of an
 *  dictionary has already been swallowed from the stream. */
PDFDict PDFParser::parseDict (InputReader &ir, const PDFOperatorHandler &opHandler) {
	ir.get();  // skip second "<"
	vector<PDFObject> localObjects;
	while (!ir.eof() && ir.peek() != '>')
		parse(ir, localObjects, opHandler);
	if (ir.getString(2) != ">>")
		throw PDFException("missing '>>' at end of dictionary");
	PDFDict dict;
	for (auto it=localObjects.begin(); it != localObjects.end(); ++it) {
		if (!it->get<PDFName>())
			throw PDFException("name key expected in dictionary");
		const PDFName &key = *it->get<PDFName>();
		if (++it == localObjects.end())
			throw PDFException(string("missing dictionary value for key '")+key.str+"'");
		dict.emplace(key.str, std::move(*it));
	}
	return dict;
}


static PDFStream parse_stream (InputReader &ir, const char *delim) {
	do
		ir.skipUntil("endstream");
	while (ir.peek() >= 0 && !strchr(delim, ir.peek()));  // ensure delimiter after "endstream"
	return PDFStream();
}


static PDFIndirectObject parse_indirect_object (InputReader &ir, const char *delim, vector<PDFObject> &objects) {
	do
		ir.skipUntil("endobj");
	while (ir.peek() >= 0 && !strchr(delim, ir.peek()));  // ensure delimiter after "endobj"
	if (objects.size() >= 2) {
		const int *genno = objects.back().get<int>();
		objects.pop_back();
		const int *objno = objects.back().get<int>();
		objects.pop_back();
		if (objno && genno)
			return PDFIndirectObject(*objno, *genno);
	}
	throw PDFException("object and generation number expected before 'obj'");
}


static PDFObjectRef parse_object_ref (vector<PDFObject> &objects) {
	if (objects.size() >= 2) {
		const int *genno = objects.back().get<int>();
		objects.pop_back();
		const int *objno = objects.back().get<int>();
		objects.pop_back();
		if (objno && genno)
			return PDFObjectRef(*objno, *genno);
	}
	throw PDFException("object and generation number expected before 'R'");
}


/** Replaces all occurences of "#XX" (XX are two hex digits) with the corresponding character. */
static string& subst_numeric_chars (string &str) {
	for (size_t pos=str.find('#'); pos != string::npos; pos=str.find('#', pos+1)) {
		if (pos > str.length()-3)
			throw PDFException("sign character # must be followed by two hexadecimal digits");
		if (isxdigit(str[pos+1]) && isxdigit(str[pos+2])) {
			int c = stoi(str.substr(pos+1, 2), nullptr, 16);
			if (c == 0)
				throw PDFException("null character not permitted in name");
			str.replace(pos, 3, 1, static_cast<char>(c));
		}
		else
			throw PDFException("sign character # must be followed by two hexadecimal digits");
	}
	return str;
}


/** Parses a single PDF object from an InputReader object.
 *  @param[in,out] ir reader object to read the PDF data from
 *  @param[out] objects the parsed object is appended to this vector
 *  @param[in] opHandler handler used to treat PDF operators
 *  @throws PDFException on failure */
void PDFParser::parse (InputReader &ir, vector<PDFObject> &objects, const PDFOperatorHandler &opHandler) {
	static const char *delim = "()<>[]{}/% \t\n\r\f";
	ir.skipSpace();
	if (ir.peek() < 0)
		return;
	switch (ir.peek()) {
		case '(':
			objects.emplace_back(parse_literal_string(ir));	break;
		case '[':
			objects.emplace_back(util::make_unique<PDFArray>(parseArray(ir, opHandler))); break;
		case '<':
			ir.get();
			if (ir.peek() != '<')
				objects.emplace_back(parse_hex_string(ir));
			else
				objects.emplace_back(util::make_unique<PDFDict>(parseDict(ir, opHandler)));
			break;
		case '/': {
			ir.get();
			string name = ir.getString(delim);
			objects.emplace_back(PDFName(subst_numeric_chars(name)));
			break;
		}
		default: {
			string str = ir.getString(delim);
			if (str.empty())
				break;
			if (str == "null")
				objects.emplace_back(PDFNull());
			else if (str == "true")
				objects.emplace_back(true);
			else if (str == "false")
				objects.emplace_back(false);
			else if (str == "stream")
				objects.emplace_back(parse_stream(ir, delim));
			else if (str == "obj")
				objects.emplace_back(parse_indirect_object(ir, delim, objects));
			else if (str == "R")
				objects.emplace_back(parse_object_ref(objects));
			else {
				NumberVariant number;
				if (!parse_number(str, number))
					opHandler(str, objects);
				else {
					if (mpark::get_if<int>(&number))
						objects.emplace_back(mpark::get<int>(number));
					else
						objects.emplace_back(mpark::get<double>(number));
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

struct ToDoubleVisitor {
	template <typename V>
	double operator () (const V &val) {return 0;}
};

template<> double ToDoubleVisitor::operator () (const int &val)    {return static_cast<double>(val);}
template<> double ToDoubleVisitor::operator () (const double &val) {return val;}
template<> double ToDoubleVisitor::operator () (const string &val) {
	try {
		return stod(val);
	}
	catch (exception &e) {
		return 0;
	}
}


PDFObject::operator double () const {
	return mpark::visit(ToDoubleVisitor(), _value);
}


PDFObject::operator std::string () const {
	ostringstream oss;
	oss << *this;
	return oss.str();
}


static std::ostream& operator << (std::ostream &os, const PDFName &name) {return os << name.str;}
static ostream& operator << (ostream &os, const PDFNull&) {return os << "null";}
static ostream& operator << (ostream &os, const PDFStream&) {return os << "stream";}
static ostream& operator << (ostream &os, const PDFOperator &op) {return os << op.opname;}

static ostream& operator << (ostream &os, const PDFIndirectObject &obj) {
	return os << "obj(" << obj.objnum << ", " << obj.gennum << ')';
}


static ostream& operator << (ostream &os, const PDFObjectRef &ref) {
	return os << "obj(" << ref.objnum << ", " << ref.gennum << ')';
}


static ostream& operator << (ostream &os, const unique_ptr<vector<PDFObject>> &val) {
	os << '[';
	for (auto it=val->begin(); it != val->end(); ++it) {
		if (it != val->begin())
			os << ", ";
		it->write(os);
	}
	os << ']';
	return os;
}


static ostream& operator << (ostream &os, const unique_ptr<Dictionary<string,PDFObject>> &val) {
	os << "<<";
	for (auto it=val->begin(); it != val->end(); ++it) {
		if (it != val->begin())
			os << ", ";
		os << it->first << ':' << it->second;
	}
	os << ">>";
	return os;
}


struct WriteVisitor {
	explicit WriteVisitor (ostream &os) : _os(os) {}
	template <typename T> void operator () (const T &val) {_os << val;}
	ostream &_os;
};


void PDFObject::write (ostream &os) const {
	mpark::visit(WriteVisitor(os), _value);
}
