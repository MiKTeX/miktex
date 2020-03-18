/*************************************************************************
** TransformSimplifier.cpp                                              **
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
#include <cstdlib>
#include "TransformSimplifier.hpp"
#include "../Matrix.hpp"
#include "../utility.hpp"
#include "../XMLNode.hpp"
#include "../XMLString.hpp"

using namespace std;

const char* TransformSimplifier::info () const {
	return "try to simplify and shorten the values of 'transform' attributes";
}


/** Tries to simplify the transform attributes of the context node and all its descendants. */
void TransformSimplifier::execute (XMLElement *context) {
	if (!context)
		return;
	if (const char *transform = context->getAttributeValue("transform")) {
		Matrix matrix = Matrix::parseSVGTransform(transform);
		if (!incorporateTransform(context, matrix)) {
			string decomp = decompose(matrix);
			if (decomp.length() > matrix.toSVG().length())
				context->addAttribute("transform", matrix.toSVG());
			else {
				if (decomp.empty())
					context->removeAttribute("transform");
				else
					context->addAttribute("transform", decomp);
			}
		}
	}
	// continue with child elements
	for (XMLNode *child : *context) {
		if (XMLElement *elem = child->toElement())
			execute(elem);
	}
}


/** Tries to incorporate the translation and scaling components of the 'transform' attribute
 *  of a given element into the positional and/or size attributes of that element. If successful,
 *  the 'transform' attribute is either removed or simplified to a scale() matrix.
 *  Currently, only 'image' and 'rect' elements are considered.
 *  @param[in] elem element to check
 *  @param[in] matrix matrix representing the 'transform' attribute of the element
 *  @return true on success */
bool TransformSimplifier::incorporateTransform (XMLElement *elem, const Matrix &matrix) {
	if ((elem->name() == "image" || elem->name() == "rect") && matrix.get(0, 1) == 0 && matrix.get(1, 0) == 0) {
		double tx = matrix.get(0, 2);
		double ty = matrix.get(1, 2);
		double sx = matrix.get(0, 0);
		double sy = matrix.get(1, 1);
		double x=0, y=0;

		if (const char *xstr = elem->getAttributeValue("x"))
			x = strtod(xstr, nullptr);
		if (const char *ystr = elem->getAttributeValue("y"))
			y = strtod(ystr, nullptr);
		// width and height attributes must not become negative. Hence, only apply the scaling
		// values if they are non-negative. Otherwise, keep a scaling matrix
		if (sx < 0 || sy < 0) {
			x += (sx == 0 ? 0 : tx/sx);
			y += (sy == 0 ? 0 : ty/sy);
			elem->addAttribute("transform", "scale("+XMLString(sx)+","+XMLString(sy)+")");
		}
		else {
			x = x*sx + tx;
			y = y*sy + ty;
			if (const char *wstr = elem->getAttributeValue("width"))
				elem->addAttribute("width", sx*strtod(wstr, nullptr));
			if (const char *hstr = elem->getAttributeValue("height"))
				elem->addAttribute("height", sy*strtod(hstr, nullptr));
			elem->removeAttribute("transform");
		}
		elem->addAttribute("x", x);  // update x attribute
		elem->addAttribute("y", y);  // update y attribute
		return true;
	}
	return false;
}


static string translate_cmd (double dx, double dy) {
	string ret;
	XMLString dxstr(dx), dystr(dy);
	if (dxstr != "0" || dystr != "0") {
		ret = "translate("+dxstr;
		if (dystr != "0")
			ret += " "+dystr;
		ret += ')';
	}
	return ret;
}


static string scale_cmd (double sx, double sy) {
	string ret;
	XMLString sxstr(sx), systr(sy);
	if (sxstr != "1" || systr != "1") {
		ret = "scale("+sxstr;
		if (systr != "1")
			ret += " "+systr;
		ret += ')';
	}
	return ret;
}


static string rotate_cmd (double rad) {
	string ret;
	XMLString degstr(math::rad2deg(fmod(rad, math::TWO_PI)));
	if (degstr != "0")
		ret = "rotate("+degstr+")";
	return ret;
}


static string skewx_cmd (double rad) {
	string ret;
	XMLString degstr(math::rad2deg(fmod(rad, math::PI)));
	if (degstr != "0")
		ret = "skewX("+degstr+")";
	return ret;
}


static string skewy_cmd (double rad) {
	string ret;
	XMLString degstr(math::rad2deg(fmod(rad, math::PI)));
	if (degstr != "0")
		ret = "skewY("+degstr+")";
	return ret;
}


static bool not_equal (double x, double y) {
	return abs(x-y) >= 1e-6;
}


/** Decomposes a transformation matrix into a sequence of basic SVG transformations, i.e.
 *  translation, rotation, scaling, and skewing. The algorithm (QR-based decomposition)
 *  is taken from http://frederic-wang.fr/decomposition-of-2d-transform-matrices.html.
 *  @param[in] matrix matrix to decompose
 *  @return string containing the SVG transformation commands */
string TransformSimplifier::decompose (const Matrix &matrix) {
	// transformation matrix [a b c d e f] according to
	// https://www.w3.org/TR/SVG11/coords.html#EstablishingANewUserSpace
	double a = matrix.get(0, 0);
	double b = matrix.get(1, 0);
	double c = matrix.get(0, 1);
	double d = matrix.get(1, 1);
	double e = matrix.get(0, 2);
	double f = matrix.get(1, 2);
	string ret = translate_cmd(e, f);
	double delta = a*d - b*c;
	if (not_equal(a, 0) || not_equal(b, 0)) {
		double r = sqrt(a*a + b*b);
		ret += rotate_cmd(b > 0 ? acos(a/r) : -acos(a/r));
		ret += scale_cmd(r, delta/r);
		ret += skewx_cmd(atan((a*c + b*d)/(r*r)));
	}
	else if (not_equal(c, 0) || not_equal(d, 0)) {
		double s = sqrt(c*c + d*d);
		ret += rotate_cmd(math::HALF_PI - (d > 0 ? acos(-c/s) : -acos(c/s)));
		ret += scale_cmd(delta/s, s);
		ret += skewy_cmd(atan((a*c + b*d)/(s*s)));
	}
	else
		ret += scale_cmd(0, 0);
	return ret;
}

