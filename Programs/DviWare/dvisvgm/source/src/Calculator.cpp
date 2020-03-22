/*************************************************************************
** Calculator.cpp                                                       **
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

#include <cmath>
#include <sstream>
#include <stdexcept>
#include "Calculator.hpp"

using namespace std;

// token types
const char END    = 0;
const char NUMBER = 1;
const char NAME   = 2;


/** Evaluates a given arithmetic expression and returns its value.
 *  The evaluator is implemented as a recursive descent parser.
 *  @param[in] is reads expression from this stream
 *  @return expression value */
double Calculator::eval (istream &is) {
	double ret = expr(is, false);
	if (lookAhead(is) > 0)
		throw CalculatorException("expression syntax error");
	return ret;
}


/** Evaluates a given arithmetic expression and returns its value.
 *  @param[in] expr expression to evaluate
 *  @return expression value */
double Calculator::eval (const string &expr) {
	istringstream iss;
	iss.str(expr);
	return eval(iss);
}


/** Evaluates the root rule of the expression grammar. */
double Calculator::expr (istream &is, bool skip) { // expr:
	double left = term(is, skip);
	for (;;) {
		switch (lookAhead(is)) {
			case '+': left += term(is, true); break;  // term '+' term => $1 + $3
			case '-': left -= term(is, true); break;  // term '-' term => $1 - $3
			default : return left;                    // term => $1
		}
	}
}


double Calculator::term (istream &is, bool skip) { // term:
	double left = prim(is, skip);
	for (;;)
		switch (lookAhead(is)) {
			case '*': left *= prim(is, true); break;  // prim '*' prim => $1 * $3
			case '(': left *= prim(is, false); break; // prim '*' prim => $1 * $3
			case '/': {                               // prim '/' prim => $1 / $3
				double denom = prim(is, true);
				if (denom == 0)
					throw CalculatorException("division by zero");
				left /= denom;
				break;
			}
			case '%': {                               // prim '%' prim => $1 mod $3
				double denom = prim(is, true);
				if (denom == 0)
					throw CalculatorException("division by zero");
				left -= denom*floor(left/denom);
				break;
			}
			default:                                  // prim => $1
				return left;
		}
}


double Calculator::prim (istream &is, bool skip) { // prim:
	if (skip)
		lex(is);
	switch (lookAhead(is)) {
		case NUMBER: {                               // NUMBER => $1
			lex(is);
			double ret = _numValue;
			if (lookAhead(is) == NAME) {              // NUMBER NAME => $1 * $2
				lex(is);
				ret *= getVariable(_strValue);
			}
			return ret;
		}
		case NAME: {                                 // NAME => getVariable($1)
			lex(is);
			return getVariable(_strValue);
		}
		case '-':                                    // '-' prim => -$2
			return -prim(is, true);
		case '(': {                                  // '(' expr ')' => $2
			double e = expr(is, true);
			if (lookAhead(is) != ')')
				throw CalculatorException("')' expected");
			lex(is);
			return e;
		}
		default:
			throw CalculatorException("primary expression expected");
	}
}


/** Determines type of next token without swallowing it. That means
 *  the same token will be read again next time. */
char Calculator::lookAhead (istream &is) {
	is >> ws;
	int c = is.peek();
	if (is.eof())
		return END;
	if (isdigit(c) || c == '.')
		return NUMBER;
	if (isalpha(c))
		return NAME;
	return char(c);
}


/** Reads next token and returns its type. The token value is either assigned
 *  to the object members numValue or strValue depending on the type. The token
 *  type is represented by a unique integer. In contrast to method 'lookAhead'
 *  lex consumes the read token.
 *  @param[in] is next token is read from this stream
 *  @return token type */
char Calculator::lex (istream &is) {
	int tokenType = lookAhead(is);
	switch (tokenType) {
		case NUMBER: {
			string str;
			while (isdigit(is.peek()) || is.peek() == '.')
				str += char(is.get());
			try {
				_numValue = stod(str);
			}
			catch (const exception&) {
				throw CalculatorException("invalid number: "+str);
			}
			break;
		}
		case NAME: {
			_strValue.clear();
			while (isalpha(is.peek()))
				_strValue += char(is.get());
			break;
		}
		default:
			tokenType = is.get();
	}
	return char(tokenType);
}


/** Returns the value of a previously defined variable. If there
 *  is no variable of the given name, a CalculatorException is thrown.
 *  @param[in] name name of variable
 *  @return assigned value */
double Calculator::getVariable (const string &name) const {
	auto it = _variables.find(name);
	if (it == _variables.end())
		throw CalculatorException("undefined variable '" + name + "'");
	return it->second;
}

