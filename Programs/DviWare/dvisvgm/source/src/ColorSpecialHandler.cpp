/*************************************************************************
** ColorSpecialHandler.cpp                                              **
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
#include <sstream>
#include <vector>
#include "ColorSpecialHandler.hpp"
#include "SpecialActions.hpp"

using namespace std;


static double read_double (istream &is) {
	is.clear();
	double v;
	is >> v;
	if (is.fail())
		throw SpecialException("number expected");
	return v;
}


/** Reads multiple double values from a given stream. The number of
 *  values read is determined by the size of the result vector.
 *  @param[in]  is stream to be read from
 *  @param[out] vec the resulting values */
static void read_doubles (istream &is, vector<double> &vec) {
	for (double &val : vec)
		val = read_double(is);
}


/** Reads a color statement from an input stream and converts it to a color object.
 *  A color statement has the following syntax:
 *  _color model_ _component values_
 *  Currently, the following color models are supported: rgb, cmyk, hsb and gray.
 *  Examples: rgb 1 0.5 0, gray 0.5
 *  @param[in] model the color model
 *  @param[in] is stream to be read from
 *  @return resulting Color object */
Color ColorSpecialHandler::readColor (const string &model, istream &is) {
	Color color;
	if (model == "rgb") {
		vector<double> rgb(3);
		read_doubles(is, rgb);
		color.setRGB(rgb[0], rgb[1], rgb[2]);
	}
	else if (model == "cmyk") {
		vector<double> cmyk(4);
		read_doubles(is, cmyk);
		color.setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
	}
	else if (model == "hsb") {
		vector<double> hsb(3);
		read_doubles(is, hsb);
		color.setHSB(hsb[0], hsb[1], hsb[2]);
	}
	else if (model == "gray")
		color.setGray(read_double(is));
	else if (!color.setPSName(model, true))
		throw SpecialException("unknown color statement");
	return color;
}


/** Reads the color model (rgb, cmyk, hsb, or gray) and the corresponding color components
 *  from a given input stream.
 *  @param[in] is stream to be read from
 *  @return resulting Color object */
Color ColorSpecialHandler::readColor (istream &is) {
	string model;
	is >> model;
	return readColor(model, is);
}


bool ColorSpecialHandler::process (const string&, istream &is, SpecialActions &actions) {
	char colortype=0;
	auto pos = is.tellg();
	string cmd;
	is >> cmd;
	if (cmd == "push")               // color push [fill|stroke] <model> <params>
		colortype = processPush(is);
	else if (cmd == "pop") {
		if (!_colorStack.empty())     // color pop
			_colorStack.pop_back();
	}
	else if (cmd == "set")           // color set [fill|stroke] <model> <params>
		colortype = processSet(is);
	else {                           // color [fill|stroke] <model> <params>
		while (!_colorStack.empty())
			_colorStack.pop_back();
		is.seekg(pos);
		colortype = processPush(is);
	}
	if (_colorStack.empty()) {
		if (colortype == 0 || colortype == 'f')
			actions.setFillColor(_defaultFillColor);
		if (colortype == 0 || colortype == 's')
			actions.setStrokeColor(_defaultStrokeColor);
	}
	else {
		if (colortype == 0 || colortype == 'f')
			actions.setFillColor(_colorStack.back().fillColor);
		if (colortype == 0 || colortype == 's')
			actions.setStrokeColor(_colorStack.back().strokeColor);
	}
	return true;
}


/** Parses [fill|stroke] <model> <params>.
 *  @param[in] is stream to read from
 *  @param[out] type specified type color type ('f'=fill, 's'=stroke, 0=none specified)
 *  @return color object representing the specified color */
static Color read_color_and_type (istream &is, char &type) {
	string token;
	string model;
	is >> token;
	if (token == "fill" || token == "stroke") {
		is >> model;
		type = token[0];
	}
	else {
		model = std::move(token);
		type = '\0';
	}
	return ColorSpecialHandler::readColor(model, is);
}


/** Handles push [fill|stroke] <model> <params> which pushes a new color pair
 *  onto the stack. If 'fill' or 'stroke' is specified, only that color value is set.
 *  The other one is copied from the current corresponding value.
 *  @return color type specified in the special command ('f'=fill, 's'=stroke, 0=none specified) */
char ColorSpecialHandler::processPush (istream &is) {
	_colorStack.emplace_back(ColorPair{});
	return processSet(is);
}


/** Handles set [fill|stroke] <model> <params> which changes the current
 *  color pair without pushing new ones. If the stack is empty, the default
 *  color values (usually black) are changed.
 *  @return color type specified in the special command ('f'=fill, 's'=stroke, 0=none specified) */
char ColorSpecialHandler::processSet (istream &is) {
	char type;
	Color color = read_color_and_type(is, type);
	Color &fillColor = _colorStack.empty() ? _defaultFillColor : _colorStack.back().fillColor;
	Color &strokeColor = _colorStack.empty() ? _defaultStrokeColor : _colorStack.back().strokeColor;
	if (type == 0 || type == 'f')
		fillColor = color;
	if (type == 0 || type == 's')
		strokeColor = color;
	return type;
}


vector<const char*> ColorSpecialHandler::prefixes () const {
	vector<const char*> pfx {"color"};
	return pfx;
}

