/*

mplibstuff.w

Copyright 2019 LuaTeX team <bugs@@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

The \PNG\ and \SVG\ backends are not available in \LUATEX, because it's complex
to manage the math formulas at run time. In this respect \POSTSCRIPT\ and the
highlevel |objects| are better, and they are the standard way. Another problem is
how to emit the warning: the |normal_warning| function is not available when
\LUATEX\ is called as \LUA\ only.

*/

#include <stdio.h>

extern void normal_warning(const char *t, const char *p);

extern int lua_only;

#define mplibstuff_message(MSG) do { \
    if (lua_only) { \
        fprintf(stdout,"mplib: " #MSG " not available.\n"); \
    } else { \
        normal_warning("mplib",  #MSG " not available."); \
    } \
} while (0)

void mp_png_backend_initialize (void *mp);
void mp_png_backend_free (void *mp);
int mp_png_gr_ship_out (void *hh, void *options, int standalone);
int mp_png_ship_out (void *hh, const char *options);

void mp_svg_backend_initialize (void *mp);
void mp_svg_backend_free (void *mp);
int mp_svg_ship_out (void *hh, int prologues);
int mp_svg_gr_ship_out (void *hh, int qprologues, int standalone);

void *mp_initialize_binary_math(void *mp);


void mp_png_backend_initialize (void *mp)                         { return; }
void mp_png_backend_free (void *mp)                               { return; }
int mp_png_gr_ship_out (void *hh, void *options, int standalone)  { mplibstuff_message(png backend); return 1; }
int mp_png_ship_out (void *hh, const char *options)               { mplibstuff_message(png backend); return 1; }

void mp_svg_backend_initialize (void *mp)                         { return; }
void mp_svg_backend_free (void *mp)                               { return; }
int mp_svg_ship_out (void *hh, int prologues)                     { mplibstuff_message(svg bakend); return 1; }
int mp_svg_gr_ship_out (void *hh, int qprologues, int standalone) { mplibstuff_message(svg backend); return 1; }


void *mp_initialize_binary_math(void *mp)                         {mplibstuff_message(math binary);return NULL; }

const char* cairo_version_string (void);
const char* mpfr_get_version(void);
const char* pixman_version_string (void);


#define CAIRO_VERSION_STRING "CAIRO NOT AVAILABLE"
const char *COMPILED_CAIRO_VERSION_STRING = CAIRO_VERSION_STRING;

#define MPFR_VERSION_STRING "MPFR NOT AVAILABLE"
const char *COMPILED_MPFR_VERSION_STRING = MPFR_VERSION_STRING;


#define __GNU_MP_VERSION -1
#define __GNU_MP_VERSION_MINOR -1
#define __GNU_MP_VERSION_PATCHLEVEL -1 
int COMPILED__GNU_MP_VERSION = __GNU_MP_VERSION ;
int COMPILED__GNU_MP_VERSION_MINOR = __GNU_MP_VERSION_MINOR ;
int COMPILED__GNU_MP_VERSION_PATCHLEVEL = __GNU_MP_VERSION_PATCHLEVEL ;
const char * const COMPILED_gmp_version="GMP NOT AVAILABLE";

#define PIXMAN_VERSION_STRING "PIXMAN NOT AVAILABLE"
const char *COMPILED_PIXMAN_VERSION_STRING = PIXMAN_VERSION_STRING;

const char* cairo_version_string (void)
{
    return CAIRO_VERSION_STRING;
}

const char* mpfr_get_version(void)
{
    return MPFR_VERSION_STRING;
}

const char* pixman_version_string (void)
{
    return PIXMAN_VERSION_STRING;
}

char png_libpng_ver[] = "PNG NOT AVAILABLE";



