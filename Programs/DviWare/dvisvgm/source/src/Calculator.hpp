/*************************************************************************
** Calculator.hpp                                                       **
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

#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <istream>
#include <map>
#include <string>
#include "MessageException.hpp"


struct CalculatorException : public MessageException {
	explicit CalculatorException (const std::string &msg) : MessageException(msg) {}
};

class Calculator {
	public:
		double eval (std::istream &is);
		double eval (const std::string &expr);
		void setVariable (const std::string &name, double value) {_variables[name] = value;}
		double getVariable (const std::string &name) const;

	protected:
		double expr (std::istream &is, bool skip);
		double term (std::istream &is, bool skip);
		double prim (std::istream &is, bool skip);
		char lex (std::istream &is);
		char lookAhead (std::istream &is);

	private:
		std::map<std::string,double> _variables;
		double _numValue=0;
		std::string _strValue;
};

#endif
