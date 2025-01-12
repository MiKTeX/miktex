/*************************************************************************
** GraphicsPathParser.hpp                                               **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#pragma once

#include <cctype>
#include <cstring>
#include <istream>
#include <sstream>
#include "GraphicsPath.hpp"
#include "MessageException.hpp"

namespace internal {
	template <typename NumType>
	bool parse_number (std::istream &is, NumType &value) {
		is >> value;
		return !is.fail();
	}

	template <>
	bool parse_number (std::istream &is, double &value) {
		// don't use operator >> for parsing floating point values because it's implemented
		// differently in libstdc++ and libc++. Instead, use our own function to read the
		// value from the input stream.
		return util::read_double(is, value);
	}
}


struct GraphicsPathParserException : public MessageException {
	explicit GraphicsPathParserException (const std::string &msg) : MessageException(msg) {}
};


template <typename T>
class GraphicsPathParser {
	public:
		GraphicsPath<T> parse (std::istream &is);

		GraphicsPath<T> parse (const std::string &str) {
			std::istringstream iss(str);
			return parse(iss);
		}

	protected:
		template <typename NumType>
		NumType parseNumberOfType (std::istream &is) const {
			is >> std::ws;
			NumType number;
			if (!internal::parse_number(is, number))
				error("number expected", is);
			is >> std::ws;
			return number;
		}

		T parseNumber (std::istream &is) const {
			return parseNumberOfType<T>(is);
		}

		Pair<T> parsePair (std::istream &is) {
			T x = parseNumber(is);
			skipCommaAndWhitespace(is);
			T y = parseNumber(is);
			return Pair<T>(x, y);
		}

		void skipCommaAndWhitespace (std::istream &is) {
			is >> std::ws;
			if (is.peek() == ',') {
				is.get();
				is >> std::ws;
			}
		}

		void parseMoveTo (std::istream &is, bool relative);
		void parseClosePath ();
		void parseLineTo (std::istream &is, bool relative);
		void parseHorizontalLineTo (std::istream &is, bool relative);
		void parseVerticalLineTo (std::istream &is, bool relative);
		void parseCubicTo (std::istream &is, bool relative);
		void parseShortCubicTo (std::istream &is, bool relative);
		void parseQuadraticTo (std::istream &is, bool relative);
		void parseShortQuadraticTo (std::istream &is, bool relative);
		void parseArcTo (std::istream &is, bool relative);

		void error (const std::string &msg, std::istream &is) const {
			std::string postext;
			if (_startpos >= 0) { // valid start position?
				if (is)
					postext = " at position "+ std::to_string(is.tellg()-_startpos);
				else
					postext = " (premature end of data)";
			}
			throw GraphicsPathParserException(msg + postext);
		}

	private:
		std::istream::pos_type _startpos=0; ///< stream position where the parsing started
		GraphicsPath<T> *_path=nullptr;     ///< path being parsed
		Pair<T> _startPoint;                ///< start point of current subpath
		Pair<T> _currentPoint;              ///< current point reached by last path command
		Pair<T> _prevCtrlPoint;             ///< last control point of preceding curve command
};


/** Creates a GraphicsPath object from a SVG path data string read from a given input stream.
 *  @param[in] is stream to read from
 *  @return GraphicsPath object created from the SVG path description
 *  @throw GraphicsPathParserException if the path data contains syntax error */
template <typename T>
GraphicsPath<T> GraphicsPathParser<T>::parse (std::istream &is) {
	GraphicsPath<T> path;
	_path = &path;
	_startpos = is.tellg();
	_currentPoint = _startPoint = _prevCtrlPoint = Pair<T>(0, 0);
	int cmd=0;
	while (!is.eof()) {
		is >> std::ws;
		if (is.peek() < 0)
			break;
		if (isalpha(is.peek()))
			cmd = is.get();
		else {
			// further set of parameters appended to preceding command (command letter omitted)
			skipCommaAndWhitespace(is);
			// subsequent coordinate pairs of a "moveto" command lead to implicit "lineto" commands
			// https://www.w3.org/TR/SVG/paths.html#PathDataMovetoCommands
			if (cmd == 'M')
				cmd = 'L';
			else if (cmd == 'm')
				cmd = 'l';
		}
		int lower_cmd = std::tolower(cmd);
		bool relative = (cmd == lower_cmd);
		switch (lower_cmd) {
			case 'm': parseMoveTo(is, relative); break;
			case 'z': parseClosePath(); break;
			case 'l': parseLineTo(is, relative); break;
			case 'h': parseHorizontalLineTo(is, relative); break;
			case 'v': parseVerticalLineTo(is, relative); break;
			case 'c': parseCubicTo(is, relative); break;
			case 's': parseShortCubicTo(is, relative); break;
			case 'q': parseQuadraticTo(is, relative); break;
			case 't': parseShortQuadraticTo(is, relative); break;
			case 'a': parseArcTo(is, relative); break;
			case  0 : error("missing command at beginning of SVG path", is);
			default : error("invalid SVG path command '"+std::string(1, char(cmd))+"'", is);
		}
		if (strchr("csqt", lower_cmd) == nullptr)  // not a Bézier curve command?
			_prevCtrlPoint = _currentPoint;            // => no control point, use current point
	}
	_path = nullptr;
	return path;
}


/** Parses a single parameter pair of a "moveto" command. */
template <typename T>
void GraphicsPathParser<T>::parseMoveTo (std::istream &is, bool relative) {
	Pair<T> p = parsePair(is);
	if (!relative || _path->empty())
		_currentPoint = p;
	else
		_currentPoint += p;
	_path->moveto(_currentPoint);
	_startPoint = _currentPoint;
}


/** Handles a "closepath" command. */
template <typename T>
void GraphicsPathParser<T>::parseClosePath () {
	_path->closepath();
	_currentPoint = _startPoint;
}


/** Parses a single parameter pair of a "lineto" command. */
template <typename T>
void GraphicsPathParser<T>::parseLineTo (std::istream &is, bool relative) {
	Pair<T> p = parsePair(is);
	if (relative)
		_currentPoint += p;
	else
		_currentPoint = p;
	_path->lineto(_currentPoint);
}


/** Parses a single parameter of a horizontal "lineto" command. */
template <typename T>
void GraphicsPathParser<T>::parseHorizontalLineTo (std::istream &is, bool relative) {
	T x = parseNumber(is);
	if (relative)
		_currentPoint += Pair<T>(x, 0);
	else
		_currentPoint = Pair<T>(x, _currentPoint.y());
	_path->lineto(_currentPoint);
}


/** Parses a single parameter of a vertical "lineto" command. */
template <typename T>
void GraphicsPathParser<T>::parseVerticalLineTo (std::istream &is, bool relative) {
	T y = parseNumber(is);
	if (relative)
		_currentPoint += Pair<T>(0, y);
	else
		_currentPoint = Pair<T>(_currentPoint.x(), y);
	_path->lineto(_currentPoint);
}


/** Parses a single parameter set a "cubicto" (cubic Bézier curve) command. */
template <typename T>
void GraphicsPathParser<T>::parseCubicTo (std::istream &is, bool relative) {
	Pair<T> p1 = parsePair(is);
	Pair<T> p2 = parsePair(is);
	Pair<T> pe = parsePair(is);
	if (!relative)
		_currentPoint = pe;
	else {
		p1 += _currentPoint;
		p2 += _currentPoint;
		_currentPoint += pe;
	}
	_path->cubicto(p1, p2, _currentPoint);
	_prevCtrlPoint = p2;
}


/** Parses a single parameter set a shorthand "cubicto" (cubic Bézier curve) command. */
template <typename T>
void GraphicsPathParser<T>::parseShortCubicTo (std::istream &is, bool relative) {
	Pair<T> p1 = _prevCtrlPoint + (_currentPoint-_prevCtrlPoint)*T(2);
	Pair<T> p2 = parsePair(is);
	Pair<T> pe = parsePair(is);
	if (!relative)
		_currentPoint = pe;
	else {
		p2 += _currentPoint;
		_currentPoint += pe;
	}
	_path->cubicto(p1, p2, _currentPoint);
	_prevCtrlPoint = p2;
}


/** Parses a single parameter set a "quadto" (quadratic Bézier curve) command. */
template <typename T>
void GraphicsPathParser<T>::parseQuadraticTo (std::istream &is, bool relative) {
	Pair<T> p1 = parsePair(is);
	Pair<T> pe = parsePair(is);
	if (!relative)
		_currentPoint = pe;
	else {
		p1 += _currentPoint;
		_currentPoint += pe;
	}
	_path->quadto(p1, _currentPoint);
	_prevCtrlPoint = p1;
}


/** Parses a single parameter set a shorthand "quadto" (quadratic Bézier curve) command. */
template <typename T>
void GraphicsPathParser<T>::parseShortQuadraticTo (std::istream &is, bool relative) {
	Pair<T> p1 = _prevCtrlPoint + (_currentPoint-_prevCtrlPoint)*T(2);
	Pair<T> pe = parsePair(is);
	if (relative)
		_currentPoint += pe;
	else
		_currentPoint = pe;
	_path->quadto(p1, _currentPoint);
	_prevCtrlPoint = p1;
}


/** Parses a single parameter set an "arcto" command. */
template <typename T>
void GraphicsPathParser<T>::parseArcTo (std::istream &is, bool relative) {
	Pair<T> r = parsePair(is);
	double xrot = parseNumberOfType<double>(is);
	int largeArgFlag = parseNumberOfType<int>(is);
	if (largeArgFlag != 0 && largeArgFlag != 1)
		error("large-arc-flag must be 0 or 1", is);
	int sweepFlag = parseNumberOfType<int>(is);
	if (sweepFlag != 0 && sweepFlag != 1)
		error("sweep-flag must be 0 or 1", is);
	T x = parseNumber(is);
	T y = parseNumber(is);
	Pair<T> p(x, y);
	if (relative)
		p += _currentPoint;
	_currentPoint = p;
	_path->arcto(r.x(), r.y(), xrot, bool(largeArgFlag), bool(sweepFlag), _currentPoint);
}
