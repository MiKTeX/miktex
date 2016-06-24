/*************************************************************************
** Message.h                                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef DVISVGM_MESSAGE_H
#define DVISVGM_MESSAGE_H

#include <algorithm>
#include <string>
#include <ostream>
#include <sstream>
#include "Terminal.h"
#include "types.h"


class Message;

class MessageStream
{
	friend class Message;

	public:
		MessageStream ();
		MessageStream (std::ostream &os);
		~MessageStream ();

		template <typename T>
		MessageStream& operator << (const T &obj) {
			std::ostringstream oss;
			oss << obj;
			(*this) << oss.str();
			return *this;
		}

		MessageStream& operator << (const char *str);
		MessageStream& operator << (const char &c);
		MessageStream& operator << (const std::string &str) {return (*this) << str.c_str();}

		void indent (int level)        {_indent = std::max(0, level*2);}
		void indent (bool reset=false);
		void outdent (bool all=false);
		void clearline ();

	protected:
		void putChar (const char c, std::ostream &os);
		std::ostream* os () {return _os;}

	private:
		std::ostream *_os;
		bool _nl;     ///< true if previous character was a newline
		int _col;     ///< current terminal column
		int _indent;  ///< indentation width (number of columns/characters)
};


class Message
{
	struct Color {
		Color () : foreground(-1), background(-1) {}
		Color (Int8 fgcolor) : foreground(fgcolor), background(-1) {}
		Color (Int8 fgcolor, bool light) : foreground(fgcolor + (light ? 8 : 0)), background(-1) {}
		Color (Int8 fgcolor, Int8 bgcolor) : foreground(fgcolor), background(bgcolor) {}
		Int8 foreground;
		Int8 background;
	};

	public:
		enum MessageClass {
			MC_ERROR,
			MC_WARNING,
			MC_MESSAGE,
			MC_PAGE_NUMBER,
			MC_PAGE_SIZE,
			MC_PAGE_WRITTEN,
			MC_STATE,
			MC_TRACING,
			MC_PROGRESS,
		};

	public:
		static MessageStream& mstream (bool prefix=false, MessageClass mclass=MC_MESSAGE);
		static MessageStream& estream (bool prefix=false);
		static MessageStream& wstream (bool prefix=false);

		enum {ERRORS=1, WARNINGS=2, MESSAGES=4};
		static int LEVEL;
		static bool COLORIZE;

	protected:
		static void init ();


	private:
		static Color _classColors[];
		static bool _initialized;
};

#endif
