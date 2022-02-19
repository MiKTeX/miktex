/*************************************************************************
** Calculator.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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

/** Evaluates a given arithmetic expression and returns its value.
 *  The evaluator is implemented as a recursive descent parser.
 *  @param[in] is reads expression from this stream
 *  @return expression value */
double Calculator::eval (istream &is) const {
	double ret = expr(is, false);
	try {
		// check if expression has been fully evaluated (next token is of type bool)
		mpark::get<bool>(lookAhead(is));
	}
	catch (mpark::bad_variant_access &e) {
		throw CalculatorException("expression syntax error");
	}
	return ret;
}


/** Evaluates a given arithmetic expression and returns its value.
 *  @param[in] expr expression to evaluate
 *  @return expression value */
double Calculator::eval (const string &expr) const {
	istringstream iss;
	iss.str(expr);
	return eval(iss);
}


/** Evaluates the root rule of the expression grammar. */
double Calculator::expr (istream &is, bool skip) const {    // expr:
	double left = term(is, skip);
	bool ready=false;
	while (!ready) {
		Token token = lookAhead(is);
		char *op = mpark::get_if<char>(&token);
		if (!op)
			ready = true;
		else {
			switch (*op) {
				case '+': left += term(is, true); break;  // term '+' term => $1 + $3
				case '-': left -= term(is, true); break;  // term '-' term => $1 - $3
				default : ready = true;
			}
		}
	}
	return left;  // term => $1
}


double Calculator::term (istream &is, bool skip) const {    // term:
	double left = prim(is, skip);
	bool ready=false;
	while (!ready) {
		Token token = lookAhead(is);
		char *op = mpark::get_if<char>(&token);
		if (!op)
			ready = true;
		else {
			switch (*op) {
				case '*': left *= prim(is, true); break;  // prim '*' prim => $1 * $3
				case '(': left *= prim(is, false); break; // prim '(' prim => $1 * $3
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
					left -= denom * floor(left / denom);
					break;
				}
				default:
					ready = true;
			}
		}
	}
	return left;
}


/** Evaluates a primary expression of the grammar which doesn't contain any binary operators. */
double Calculator::prim (istream &is, bool skip) const { // prim:
	if (skip)
		lex(is);
	Token token = lookAhead(is);
	if (mpark::get_if<double>(&token)) {               //  NUMBER => $1
		double ret = mpark::get<double>(lex(is));
		token = lookAhead(is);
		if (mpark::get_if<string>(&token))              // NUMBER STRING => $1 * $2
			ret *= getVariable(mpark::get<string>(lex(is)));
		return ret;
	}
	if (mpark::get_if<string>(&token))                 // STRING => getVariable($1)
		return getVariable(mpark::get<string>(lex(is)));
	if (char *op = mpark::get_if<char>(&token)) {
		switch (*op) {
			case '-':                                    // '-' prim => -$2
				return -prim(is, true);
			case '(': {                                  // '(' expr ')' => $2
				double e = expr(is, true);
				try {
					if (mpark::get<char>(lookAhead(is)) != ')')
						throw CalculatorException("')' expected");
				}
				catch (mpark::bad_variant_access &e) {
					throw CalculatorException("')' expected");
				}
				is.get();   // skip processed char
				return e;
			}
		}
	}
	throw CalculatorException("primary expression expected");
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


/** Determines the type of the next token without swallowing it. That means
 *  the same token will be read again next time. The value of the returned token
 *  object is only set for character tokens and should be considered undefined
 *  for the other types. */
Calculator::Token Calculator::lookAhead (istream &is) {
	is >> ws;
	int c = is.peek();
	if (is.eof())
		return false;
	if (isdigit(c) || c == '.')
		return double(0);
	if (isalpha(c))
		return string();
	return char(c);
}


/** Reads and swallows the next token. The returned Token object is represented by
 *  a variant and thus contains both its type and value.
 *  @param[in] is next token is read from this stream
 *  @return the read token */
Calculator::Token Calculator::lex (istream &is) {
	Token token = lookAhead(is);
	if (mpark::get_if<char>(&token))
		is.get();  // token already contains the value from look ahead
	else if (mpark::get_if<double>(&token)) {
		string str;
		while (isdigit(is.peek()) || is.peek() == '.')
			str += char(is.get());
		try {
			token.emplace<double>(stod(str));
		}
		catch (const exception&) {
			throw CalculatorException("invalid number: "+str);
		}
	}
	else if (mpark::get_if<string>(&token)) {
		string name;
		while (isalpha(is.peek()))
			name += char(is.get());
		token.emplace<string>(name);
	}
	return token;
}
