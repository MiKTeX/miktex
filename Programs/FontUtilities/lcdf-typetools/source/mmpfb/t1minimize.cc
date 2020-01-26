/* t1minimize.cc -- make minimal copy of a Type 1 font
 *
 * Copyright (c) 2003-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "t1minimize.hh"
#include <efont/t1item.hh>
#include <stdio.h>

using namespace Efont;

Type1Font *
minimize(Type1Font *font)
{
    Vector<double> xuid_extension;
    xuid_extension.push_back(0x000395C1);
    Type1Font *output = Type1Font::skeleton_make_copy(font, font->font_name(), &xuid_extension);

    // Subrs
    for (int i = 0; i < font->nsubrs(); i++)
	if (Type1Subr *s = font->subr_x(i))
	    output->set_subr(s->subrno(), s->t1cs(), s->definer());

    // CharStrings
    for (int i = 0; i < font->nglyphs(); i++)
	if (Type1Subr *g = font->glyph_x(i))
	    output->add_glyph(Type1Subr::make_glyph(g->name(), g->t1cs(), g->definer()));

    return output;
}
