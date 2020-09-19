/*************************************************************************
** Message.cpp                                                          **
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

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include "Message.hpp"
#include "Terminal.hpp"

using namespace std;

MessageStream::MessageStream (std::ostream &os) noexcept
	: _os(&os), _nl(true)
{
	Terminal::init(os);
}


MessageStream::~MessageStream () {
	if (_os && Message::COLORIZE)
		Terminal::finish(*_os);
}


void MessageStream::putChar (char c, ostream &os) {
	switch (c) {
		case '\r':
			os << '\r';
			_nl = true;
			_col = 1;
			return;
		case '\n':
			if (!_nl) {
				_col = 1;
				_nl = true;
				os << '\n';
			}
			return;
		default:
			if (_nl) {
				os << string(_indent, ' ');
				_col += _indent;
			}
			else {
				const int cols = Terminal::columns();
				if (cols > 0 && _col >= cols) {
#ifndef _WIN32
					// move cursor to next line explicitly (not necessary in Windows/DOS terminal)
					os << '\n';
#endif
					os << string(_indent, ' ');
					_col = _indent+1;
				}
				else
					_col++;
			}
			_nl = false;
			if (!_nl || c != '\n')
				os << c;
	}
}


MessageStream& MessageStream::operator << (const char *str) {
	if (_os && str) {
		const char *first = str;
		while (*first) {
			const char *last = strchr(first, '\n');
			if (!last)
				last = first+strlen(first)-1;
#ifndef _WIN32
			// move cursor to next line explicitly (not necessary in Windows/DOS terminal)
			const int cols = Terminal::columns();
			int len = last-first+1;
			if (cols > 0 && _col+len > cols && _indent+len <= cols)
				putChar('\n', *_os);
#endif
			while (first <= last)
				putChar(*first++, *_os);
			first = last+1;
		}
	}
	return *this;
}


MessageStream& MessageStream::operator << (const char &c) {
	if (_os)
		putChar(c, *_os);
	return *this;
}


void MessageStream::indent (bool reset) {
	if (reset)
		_indent = 0;
	_indent += 2;
}


void MessageStream::outdent (bool all) {
	if (all)
		_indent = 0;
	else if (_indent > 0)
		_indent -= 2;
}


void MessageStream::clearline () {
	if (_os) {
		int cols = Terminal::columns();
		*_os << '\r' << string(cols ? cols-1 : 79, ' ') << '\r';
		_nl = true;
		_col = 1;
	}
}

static MessageStream nullStream;
static MessageStream messageStream(cerr);


//////////////////////////////

// maximal verbosity
int Message::LEVEL = Message::MESSAGES | Message::WARNINGS | Message::ERRORS;
bool Message::COLORIZE = false;
bool Message::_initialized = false;
Message::Color Message::_classColors[9];


/** Returns the stream for usual messages. */
MessageStream& Message::mstream (bool prefix, MessageClass mclass) {
	init();
	MessageStream *ms = (LEVEL & MESSAGES) ? &messageStream : &nullStream;
	if (COLORIZE && ms && ms->os()) {
		Terminal::fgcolor(_classColors[mclass].foreground, *ms->os());
		Terminal::bgcolor(_classColors[mclass].background, *ms->os());
	}
	if (prefix)
		*ms << "\nMESSAGE: ";
	return *ms;
}


/** Returns the stream for warning messages. */
MessageStream& Message::wstream (bool prefix) {
	init();
	MessageStream *ms = (LEVEL & WARNINGS) ? &messageStream : &nullStream;
	if (COLORIZE && ms && ms->os()) {
		Terminal::fgcolor(_classColors[MC_WARNING].foreground, *ms->os());
		Terminal::bgcolor(_classColors[MC_WARNING].background, *ms->os());
	}
	if (prefix)
		*ms << "\nWARNING: ";
	return *ms;
}


/** Returns the stream for error messages. */
MessageStream& Message::estream (bool prefix) {
	init();
	MessageStream *ms = (LEVEL & ERRORS) ? &messageStream : &nullStream;
	if (COLORIZE && ms && ms->os()) {
		Terminal::fgcolor(_classColors[MC_ERROR].foreground, *ms->os());
		Terminal::bgcolor(_classColors[MC_ERROR].background, *ms->os());
	}
	if (prefix)
		*ms << "\nERROR: ";
	return *ms;
}


static bool colorchar2int (char colorchar, int *val) {
	colorchar = tolower(colorchar);
	if (colorchar >= '0' && colorchar <= '9')
		*val = int(colorchar-'0');
	else if (colorchar >= 'a' && colorchar <= 'f')
		*val = int(colorchar-'a'+10);
	else if (colorchar == '*')
		*val = -1;
	else
		return false;
	return true;
}


/** Initializes the Message class. Sets the colors for each message set.
 *  The colors can be changed via environment variable DVISVGM_COLORS. Its
 *  value must be a sequence of color entries of the form gg:BF where the
 *  two-letter ID gg specifies a message set, B the hex digit of the
 *  background, and F the hex digit of the foreground/text color.
 *  Color codes:
 *  - 1: red, 2: green, 4: blue
 *  - 0-7: dark colors
 *  - 8-F: light colors
 *  - *: default color
 *  Example: pn:01 sets page number messages to red on black background */
void Message::init () {
	if (_initialized || !Message::COLORIZE)
		return;

	// set default message colors
	_classColors[MC_ERROR]        = Color(Terminal::RED, true);
	_classColors[MC_WARNING]      = Color(Terminal::YELLOW);
	_classColors[MC_PAGE_NUMBER]  = Color(Terminal::BLUE, true);
	_classColors[MC_PAGE_SIZE]    = Color(Terminal::MAGENTA);
	_classColors[MC_PAGE_WRITTEN] = Color(Terminal::GREEN);
	_classColors[MC_STATE]        = Color(Terminal::CYAN);
	_classColors[MC_TRACING]      = Color(Terminal::BLUE);
	_classColors[MC_PROGRESS]     = Color(Terminal::MAGENTA);

	if (const char *color_str = getenv("DVISVGM_COLORS")) {
		unordered_map<string, MessageClass> classes = {
			{"er", MC_ERROR},
			{"wn", MC_WARNING},
			{"pn", MC_PAGE_NUMBER},
			{"ps", MC_PAGE_SIZE},
			{"fw", MC_PAGE_WRITTEN},
			{"sm", MC_STATE},
			{"tr", MC_TRACING},
			{"pi", MC_PROGRESS},
		};
		const char *p=color_str;

		// skip leading whitespace
		while (isspace(*p))
			++p;

		// iterate over color assignments
		while (strlen(p) >= 5) {
			auto it = classes.find(string(p, 2));
			if (it != classes.end() && p[2] == '=') {
				int bgcolor, fgcolor;
				if (colorchar2int(p[3], &bgcolor) && colorchar2int(p[4], &fgcolor)) {
					_classColors[it->second].background = bgcolor;
					_classColors[it->second].foreground = fgcolor;
				}
			}
			p += 5;

			// skip trailing characters in a malformed entry
			while (*p && !isspace(*p) && *p != ':' && *p != ';')
				++p;
			// skip separation characters
			while (isspace(*p) || *p == ':' || *p == ';')
				++p;
		}
	}
	_initialized = true;
}

