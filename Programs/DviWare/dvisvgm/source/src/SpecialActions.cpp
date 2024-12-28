/*************************************************************************
** SpecialActions.cpp                                                   **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <cstring>
#include <regex>
#include "Calculator.hpp"
#include "SpecialActions.hpp"

using namespace std;


inline string get_color_string (SpecialActions &actions, Color (SpecialActions::*getColor)() const) {
	return SVGElement::USE_CURRENTCOLOR && SVGElement::CURRENTCOLOR == (actions.*getColor)()
		? "currentColor"
		: (actions.*getColor)().svgColorString();
}


/** Replaces constants of the form {?name} by their corresponding value.
 *  @param[in,out] str text to expand
 *  @param[in] actions interfcae to the world outside the special handler */
static void expand_constants (string &str, SpecialActions &actions) {
	bool repl_bbox = true;
	while (repl_bbox) {
		const auto pos = str.find("{?bbox ");
		if (pos == string::npos)
			repl_bbox = false;
		else {
			auto endpos = pos+7;
			while (endpos < str.length() && isalnum(str[endpos]))
				++endpos;
			if (endpos == str.length() || str[endpos] != '}')
				repl_bbox = false;
			else {
				BoundingBox &box = actions.bbox(str.substr(pos+7, endpos-pos-7));
				str.replace(pos, endpos-pos+1, box.svgViewBoxString());
			}
		}
	}
	const struct Constant {
		const char *name;
		string val;
	} constants[] = {
		{"x",           XMLString(actions.getX())},
		{"y",           XMLString(actions.getY())},
		{"color",       get_color_string(actions, &SpecialActions::getFillColor)},
		{"fillcolor",   get_color_string(actions, &SpecialActions::getFillColor)},
		{"strokecolor", get_color_string(actions, &SpecialActions::getStrokeColor)},
		{"matrix",      actions.getMatrix().toSVG()},
		{"nl",          "\n"},
		{"pageno",      to_string(actions.getCurrentPageNumber())},
		{"svgfile",     actions.getSVGFilePath(actions.getCurrentPageNumber()).relative()},
		{"svgpath",     actions.getSVGFilePath(actions.getCurrentPageNumber()).absolute()},
	};
	for (const Constant &constant : constants) {
		const string pattern = string("{?")+constant.name+"}";
		auto pos = str.find(pattern);
		while (pos != string::npos) {
			str.replace(pos, strlen(constant.name)+3, constant.val);
			pos = str.find(pattern, pos+constant.val.length());  // look for further matches
		}
	}
	// expand {?cmyk(c,m,y,k)} to #RRGGBB
	std::smatch match;
	std::regex pattern(R"(\{\?(cmyk\(([0-9.]+,){3}[0-9.]\))\})");
	while (regex_search(str, match, pattern))
		str = match.prefix().str() + Color(match[1].str()).rgbString() + match.suffix().str();
}


/** Evaluates substrings of the form {?(expr)} where 'expr' is a math expression,
 *  and replaces the substring by the computed value.
 *  @param[in,out] str string to scan for expressions */
static void evaluate_expressions (string &str, const SpecialActions &actions) {
	auto left = str.find("{?(");             // start position of expression macro
	while (left != string::npos) {
		auto right = str.find(")}", left+2);  // end position of expression macro
		if (right == string::npos)
			break;
		Calculator calc;
		calc.setVariable("x", actions.getX());
		calc.setVariable("y", actions.getY());
		string expr = str.substr(left+3, right-left-3);  // math expression to evaluate
		if (util::normalize_space(expr).empty())         // no expression given, e.g. {?( )}
			str.erase(left, right-left+2);                // => replace with empty string
		else {
			try {
				double val = calc.eval(expr);
				XMLString valstr(val);
				str.replace(left, right-left+2, valstr);
				right = left+valstr.length()-1;
			}
			catch (CalculatorException &e) {
				throw SpecialException(string(e.what())+" in '{?("+expr+")}'");
			}
		}
		left = str.find("{?(", right+1);  // find next expression macro
	}
}


/** Returns a given string with macros and arithmetic expressions expanded.
 *  @param[in] text string to be processed
 *  @return the expanded text */
string SpecialActions::expandText (const string &text) {
	string ret = text;
	evaluate_expressions(ret, *this);
	expand_constants(ret, *this);
	return ret;
}