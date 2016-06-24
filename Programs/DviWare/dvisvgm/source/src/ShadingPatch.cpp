/*************************************************************************
** ShadingPatch.cpp                                                     **
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

#include <sstream>
#include "ShadingPatch.h"
#include "TensorProductPatch.h"
#include "TriangularPatch.h"

using namespace std;

/** Get functions to get/set the current color depending on the assigned color space. */
void ShadingPatch::colorQueryFuncs (ColorGetter &getter, ColorSetter &setter) const {
	switch (_colorspace) {
		case Color::CMYK_SPACE:
			getter = &Color::getCMYK;
			setter = &Color::setCMYK;
			break;
		case Color::LAB_SPACE:
			getter = &Color::getLab;
			setter = &Color::setLab;
			break;
		case Color::RGB_SPACE:
			getter = &Color::getRGB;
			setter = &Color::setRGB;
			break;
		case Color::GRAY_SPACE:
			getter = &Color::getGray;
			setter = &Color::setGray;
	}
}


/** Factory method: Creates a shading patch object depending on the given PostScript shading type. */
ShadingPatch* ShadingPatch::create (int psShadingType, Color::ColorSpace cspace) {
	switch (psShadingType) {
		case 4: return new TriangularPatch(cspace);
		case 5: return new LatticeTriangularPatch(cspace);
		case 6: return new CoonsPatch(cspace);
		case 7: return new TensorProductPatch(cspace);
	}
	ostringstream oss;
	if (psShadingType > 0 && psShadingType < 4)
		oss << "shading type " << psShadingType << " not supported";
	else
		oss << "invalid shading type " << psShadingType;
	throw ShadingException(oss.str());
}
