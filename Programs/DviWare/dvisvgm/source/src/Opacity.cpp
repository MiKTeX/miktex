/*************************************************************************
** Opacity.cpp                                                          **
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
#include "Opacity.hpp"

using namespace std;

string Opacity::cssBlendMode (BlendMode bm) {
	switch (bm) {
		case BM_NORMAL    : return "normal";
		case BM_MULTIPLY  : return "multiply";
		case BM_SCREEN    : return "screen";
		case BM_OVERLAY   : return "overlay";
		case BM_SOFTLIGHT : return "soft-light";
		case BM_HARDLIGHT : return "hard-light";
		case BM_COLORDODGE: return "color-dodge";
		case BM_COLORBURN : return "color-burn";
		case BM_DARKEN    : return "darken";
		case BM_LIGHTEN   : return "lighten";
		case BM_DIFFERENCE: return "difference";
		case BM_EXCLUSION : return "exclusion";
		case BM_HUE       : return "hue";
		case BM_SATURATION: return "saturation";
		case BM_COLOR     : return "color";
		case BM_LUMINOSITY: return "luminosity";
	}
	return "";
}


static string to_lower_drop_nonalpha (const string &str) {
	string ret;
	if (!str.empty()) {
		ret.reserve(str.length());
		for (char c : str) {
			if (isalpha(c))
				ret += char(tolower(c));
		}
	}
	return ret;
}


Opacity::BlendMode Opacity::blendMode (const std::string &name) {
	struct {
		const char *name;
		BlendMode mode;
	} modes[] = {
		{"normal",     BM_NORMAL    },
		{"multiply",   BM_MULTIPLY  },
		{"screen",     BM_SCREEN    },
		{"overlay",    BM_OVERLAY   },
		{"softlight",  BM_SOFTLIGHT },
		{"hardlight",  BM_HARDLIGHT },
		{"colordodge", BM_COLORDODGE},
		{"colorburn",  BM_COLORBURN },
		{"darken",     BM_DARKEN    },
		{"lighten",    BM_LIGHTEN   },
		{"difference", BM_DIFFERENCE},
		{"exclusion",  BM_EXCLUSION },
		{"hue",        BM_HUE       },
		{"saturation", BM_SATURATION},
		{"color",      BM_COLOR     },
		{"luminosity", BM_LUMINOSITY}
	};
	string compname = to_lower_drop_nonalpha(name);
	for (const auto &m : modes) {
		if (compname == m.name)
			return m.mode;
	}
	return BM_NORMAL;
}


bool Opacity::operator == (const Opacity &opacity) const {
	return opacity._fillalpha == _fillalpha
		&& opacity._strokealpha == _strokealpha
		&& opacity._blendMode == _blendMode;
}


bool Opacity::operator != (const Opacity &opacity) const {
	return opacity._fillalpha != _fillalpha
		|| opacity._strokealpha != _strokealpha
		|| opacity._blendMode != _blendMode;
}
