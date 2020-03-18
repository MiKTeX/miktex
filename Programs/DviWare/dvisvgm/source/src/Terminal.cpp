/*************************************************************************
** Terminal.cpp                                                         **
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

#include <config.h>
#include "Terminal.hpp"

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#ifdef GWINSZ_IN_SYS_IOCTL
#include <sys/ioctl.h>
#endif

#ifdef _WIN32
#include "windows.hpp"
#endif

#include <cstdio>


using namespace std;


const int Terminal::RED     = 1;
const int Terminal::GREEN   = 2;
const int Terminal::BLUE    = 4;

const int Terminal::CYAN    = GREEN|BLUE;
const int Terminal::YELLOW  = RED|GREEN;
const int Terminal::MAGENTA = RED|BLUE;
const int Terminal::WHITE   = RED|GREEN|BLUE;
const int Terminal::DEFAULT = -1;
const int Terminal::BLACK   = 0;

#ifdef _WIN32
int Terminal::_defaultColor;
int Terminal::_cursorHeight;
#endif

int Terminal::_fgcolor = Terminal::DEFAULT;
int Terminal::_bgcolor = Terminal::DEFAULT;


/** Initializes the terminal. This method should be called before any of the others.
 *  @param[in,out] os terminal output stream (currently unused) */
void Terminal::init (ostream &os) {
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		GetConsoleScreenBufferInfo(h, &buffer_info);
		_defaultColor = (buffer_info.wAttributes & 0xff);
		CONSOLE_CURSOR_INFO cursor_info;
		GetConsoleCursorInfo(h, &cursor_info);
		_cursorHeight = cursor_info.dwSize;
	}
#endif
}


/** Finishes the terminal output. Should be called after last terminal action.
 *  @param[in,out] os terminal output stream */
void Terminal::finish (ostream &os) {
	fgcolor(DEFAULT, os);
	bgcolor(DEFAULT, os);
	cursor(true);
}


/** Returns the number of terminal columns (number of characters per row).
 *  If it's not possible to retrieve information about the terminal size, 0 is returned. */
int Terminal::columns () {
#if defined(TIOCGWINSZ)
	struct winsize ws;
	if (ioctl(fileno(stderr), TIOCGWINSZ, &ws) < 0)
		return 0;
	return ws.ws_col;
#elif defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info))
		return 0;
	return info.dwSize.X;
#else
	return 0;
#endif
}


/** Returns the number of terminal rows.
 *  If it's not possible to retrieve information about the terminal size, 0 is returned. */
int Terminal::rows () {
#if defined(TIOCGWINSZ)
	struct winsize ws;
	if (ioctl(fileno(stderr), TIOCGWINSZ, &ws) < 0)
		return 0;
	return ws.ws_row;
#elif defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info))
		return 0;
	return info.dwSize.Y;
#else
	return 0;
#endif
}


/** Sets the foreground color.
 *  @param[in] color color code
 *  @param[in] os terminal output stream */
void Terminal::fgcolor (int color, ostream &os) {
	_fgcolor = color;

#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(h, &info);
		if (_fgcolor == DEFAULT)
			color = _defaultColor & 0x0f;
		else {
			// swap red and blue bits
			color = (color & 0x0a) | ((color & 1) << 2) | ((color & 4) >> 2);
		}
		color = (info.wAttributes & 0xf0) | (color & 0x0f);
		SetConsoleTextAttribute(h, (DWORD)color);
	}
#else
	bool light = false;
	if (color != DEFAULT && color > 7) {
		light = true;
		color %= 8;
	}
	if (color == DEFAULT) {
		os << "\x1B[0m";
		if (_bgcolor != DEFAULT)
			bgcolor(_bgcolor, os);
	}
	else
		os << "\x1B[" << (light ? '1': '0') << ';' << (30+(color & 0x07)) << 'm';
#endif
}


/** Sets the background color.
 *  @param[in] color color code
 *  @param[in] os terminal output stream */
void Terminal::bgcolor (int color, ostream &os) {
	_bgcolor = color;
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(h, &info);
		if (_bgcolor == DEFAULT)
			color = (_defaultColor & 0xf0) >> 4;
		else {
			// swap red and blue bits
			color = (color & 0x0a) | ((color & 1) << 2) | ((color & 4) >> 2);
		}
		color = (info.wAttributes & 0x0f) | ((color & 0x0f) << 4);
		SetConsoleTextAttribute(h, (DWORD)color);
	}
#else
	if (color != DEFAULT && color > 7)
		color %= 8;
	if (color == DEFAULT) {
		os << "\x1B[0m";
		if (_fgcolor != DEFAULT)
			fgcolor(_fgcolor, os);
	}
	else
		os << "\x1B[" << (40+(color & 0x07)) << 'm';
#endif
}


/** Disables or enables the console cursor
 *  @param[in] visible if false, the cursor is disabled, and enabled otherwise */
void Terminal::cursor (bool visible) {
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h != INVALID_HANDLE_VALUE) {
		CONSOLE_CURSOR_INFO cursor_info;
		cursor_info.bVisible = visible;
		cursor_info.dwSize = _cursorHeight;
		SetConsoleCursorInfo(h, &cursor_info);
	}
#endif
}
