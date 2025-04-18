/*************************************************************************
** ShadingPatch.cpp                                                     **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <sstream>
#include "ShadingPatch.hpp"
#include "TensorProductPatch.hpp"
#include "TriangularPatch.hpp"
#include "utility.hpp"

using namespace std;


/** Factory method: Creates a shading patch object depending on the given PostScript shading type. */
unique_ptr<ShadingPatch> ShadingPatch::create (int psShadingType, Color::ColorSpace cspace) {
	switch (psShadingType) {
		case 4: return util::make_unique<TriangularPatch>(cspace);
		case 5: return util::make_unique<LatticeTriangularPatch>(cspace);
		case 6: return util::make_unique<CoonsPatch>(cspace);
		case 7: return util::make_unique<TensorProductPatch>(cspace);
	}
	ostringstream oss;
	if (psShadingType > 0 && psShadingType < 4)
		oss << "shading type " << psShadingType << " not supported";
	else
		oss << "invalid shading type " << psShadingType;
	throw ShadingException(oss.str());
}
