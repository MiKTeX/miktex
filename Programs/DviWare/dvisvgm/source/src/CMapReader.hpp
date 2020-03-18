/*************************************************************************
** CMapReader.hpp                                                       **
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

#ifndef CMAPREADER_HPP
#define CMAPREADER_HPP

#include <cstdlib>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include "MessageException.hpp"


struct CMap;
class InputReader;

class CMapReader {
	class Token	{
		public:
			enum class Type {UNKNOWN, END, DELIM, NUMBER, STRING, NAME, OPERATOR};

		public:
			explicit Token (InputReader &ir);
			void scan (InputReader &ir);
			Type type () const                   {return _type;}
			const std::string& strvalue () const {return _value;}
			double numvalue () const             {return std::atof(_value.c_str());}

		private:
			Type _type;
			std::string _value;
	};

	public:
		std::unique_ptr<CMap> read (const std::string &fname);
		std::unique_ptr<CMap> read (std::istream &is, const std::string &name);

	protected:
		Token popToken () {Token t=_tokens.back(); _tokens.pop_back(); return t;}
		void executeOperator (const std::string &op, InputReader &ir);
		void parseCIDChars (InputReader &ir, bool isRange);
		void op_beginbfchar (InputReader &ir);
		void op_beginbfrange (InputReader &ir);
		void op_begincidchar (InputReader &ir);
		void op_begincidrange (InputReader &ir);
		void op_def (InputReader &ir);
		void op_endcmap (InputReader &ir);
		void op_usecmap (InputReader &ir);

	private:
		std::unique_ptr<SegmentedCMap> _cmap; ///< pointer to CMap being read
		std::vector<Token> _tokens; ///< stack of tokens to be processed
		bool _inCMap=false;         ///< true if operator begincmap has been executed
};


struct CMapReaderException : public MessageException {
	explicit CMapReaderException (const std::string &msg) : MessageException(msg) {}
};

#endif
