/*************************************************************************
** ffwrapper.c                                                          **
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

#include <fontforge.h>
#include "ffwrapper.h"

#ifdef _NDEBUG
static void no_post_error (const char *title, const char *error, ...) {}
static void no_warning (const char *format, ...) {}
#endif


/** Initializes the FontForge library. */
void ff_init () {
	InitSimpleStuff();
#ifdef _NDEBUG
	ui_interface.post_error = &no_post_error;
	ui_interface.logwarning = &no_warning;
#endif
}


int ff_version () {
	return FONTFORGE_VERSIONDATE_RAW;
}


/** Runs the autohinter for all glyphs in a given font.
 *  @param[in,out] sf the font to be autohinted
 *  @param[in] order2 if != 0, target font requires quadratic rather than cubic splines */
static void ff_autohint (SplineFont *sf, int order2) {
	BlueData bd;
	GlobalInstrCt gic;
	if (order2)
		SFConvertToOrder2(sf);
	QuickBlues(sf, ly_fore, &bd);
	if (order2)
		InitGlobalInstrCt(&gic, sf, ly_fore, &bd);
	for (int i=0; i < sf->glyphcnt; i++) {
		SplineChar *sc = sf->glyphs[i];
		if (sc != NULL) {
			SplineChar *sc = sf->glyphs[i];
			SplineCharAutoHint(sc, ly_fore, &bd); // generate hints
			if (order2)
				NowakowskiSCAutoInstr(&gic, sc);  // generate TTF instructions
		}
	}
	if (order2)
		FreeGlobalInstrCt(&gic);
}


/** Creates a TrueType font from a FontForge SFD file.
 *  @param[in] sfdname name of SFD file
 *  @param[in] ttfname name of TrueType file
 *  @param[in] autohint run the autohinter if != 0 */
int ff_sfd_to_ttf (const char *sfdname, const char *ttfname, int autohint) {
	int ret=0;
	SplineFont *sf = SFDRead((char*)sfdname);
	if (sf) {
		if (autohint)
			ff_autohint(sf, true);
		ret = WriteTTFFont((char*)ttfname, sf, ff_ttf, 0, 0, 0, sf->map, ly_fore);
		SplineFontFree(sf);
	}
	return ret;
}
