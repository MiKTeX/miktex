/*************************************************************************
** InputReader.hpp                                                      **
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

#ifndef INPUTREADER_HPP
#define INPUTREADER_HPP

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "InputBuffer.hpp"

class InputReader {
	public:
		virtual ~InputReader() =default;
		virtual int get () =0;
		virtual int peek () const =0;
		virtual int peek (size_t n) const =0;
		virtual bool eof () const =0;
		virtual bool check (char c) const {return peek() == c;}
		virtual bool check (const char *s, bool consume=true);
		virtual void skip (size_t n);
		virtual bool skipUntil (const char *str);
		virtual std::string readUntil (const char *str);
		virtual int find (char c) const;
		virtual void skipSpace ();
		virtual int getInt ();
		virtual bool parseInt (int &val, bool accept_sign=true);
		virtual bool parseUInt (int base, unsigned &val);
		virtual bool parseUInt (unsigned &val);
		virtual char parseDouble (double &val);
		virtual double getDouble ();
		virtual std::string getWord ();
		virtual char getPunct ();
		virtual std::string getQuotedString (const char *quotechars);
		virtual std::string getString ();
		virtual std::string getString (size_t n);
		virtual std::string getString (const char *delim);
		virtual std::string getLine ();
		virtual int parseAttributes (std::map<std::string,std::string> &attr, bool requireValues, const char *quotechars=nullptr);
		virtual operator bool () const {return !eof();}
};


class StreamInputReader : public InputReader {
	public:
		explicit StreamInputReader (std::istream &is) : _is(is) {}
		int get () override        {return _is.get();}
		int peek () const override {return _is.peek();}
		int peek (size_t n) const override;
		bool eof () const override {return !_is || _is.eof();}
		std::istream& getStream () {return _is;}

	private:
		std::istream &_is;
};


class BufferInputReader : public InputReader {
	public:
		explicit BufferInputReader (InputBuffer &ib) : _ib(&ib) {}
		void assign (InputBuffer &ib) {_ib = &ib;}
		int get () override                {return _ib->get();}
		int peek () const override         {return _ib->peek();}
		int peek (size_t n) const override {return _ib->peek(n);}
		bool eof () const override         {return _ib->eof();}

	private:
		InputBuffer *_ib;
};


/** Implementation of the Knuth-Morris-Pratt search algorithm.
 *  http://www.inf.fh-flensburg.de/lang/algorithmen/pattern/kmpen.htm */
class StringMatcher {
	public:
		StringMatcher () : _charsRead(0) {}
		explicit StringMatcher (const std::string &pattern) : _charsRead(0) {setPattern(pattern);}
		void setPattern (const std::string &pattern);
		bool match (InputReader &ir);
		std::string read (InputReader &ir);
		size_t charsRead () const {return _charsRead;}

	private:
		std::string _pattern;
		std::vector<size_t> _borders;
		size_t _charsRead;
};

#endif
