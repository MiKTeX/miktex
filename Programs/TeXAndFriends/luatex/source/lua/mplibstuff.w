% mplibstuff.w
%
% Copyright 2017 LuaTeX team <bugs@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ PNG and SVG backends are not available in \LuaTeX, because it's complex
to manage the math formulas at run time. In this respect |PostScript| and the highlevel |objects|
are better, and they are the standard way. Another problem is how to emit the warning:
the |normal\_warning| function is not available when \LuaTeX\ is called as LUA only. 



@c
#include <stdio.h>

extern void normal_warning(const char *t, const char *p);
extern int lua_only;
#define mplibstuff_message(BACKEND)  do {                          \
 if (lua_only) {                                                   \
   fprintf(stdout,"mplib: " #BACKEND " backend not available.\n"); \
 } else {                                                          \
   normal_warning("mplib",  #BACKEND " backend not available.");   \
 }                                                                 \
} while (0)

 
@ @c
void mp_png_backend_initialize (void *mp);
void mp_png_backend_free (void *mp);
int mp_png_gr_ship_out (void *hh, void *options, int standalone);
int mp_png_ship_out (void *hh, const char *options);

@ @c
void mp_svg_backend_initialize (void *mp);
void mp_svg_backend_free (void *mp);
int mp_svg_ship_out (void *hh, int prologues);
int mp_svg_gr_ship_out (void *hh, int qprologues, int standalone);

@ @c
void mp_png_backend_initialize (void *mp)                         {return; }  /*{mplibstuff_message(1png);return;}*/
void mp_png_backend_free (void *mp)                               {return; }   /*{mplibstuff_message(png);return;}*/
int mp_png_gr_ship_out (void *hh, void *options, int standalone)  {mplibstuff_message(png);return 1;}
int mp_png_ship_out (void *hh, const char *options)               {mplibstuff_message(png);return 1;}

@ @c
void mp_svg_backend_initialize (void *mp)                         {return;}   /*{mplibstuff_message(svg);return;}*/
void mp_svg_backend_free (void *mp)                               {return;}   /*{mplibstuff_message(svg);return;}*/
int mp_svg_ship_out (void *hh, int prologues)                     {mplibstuff_message(svg);return 1;}
int mp_svg_gr_ship_out (void *hh, int qprologues, int standalone) {mplibstuff_message(svg);return 1;}

@ @c
const char*
cairo_version_string (void);
const char*
pixman_version_string (void);
#define CAIRO_VERSION_STRING "CAIRO NOT AVAILABLE"
const char *COMPILED_CAIRO_VERSION_STRING = CAIRO_VERSION_STRING;
#define PIXMAN_VERSION_STRING "PIXMAN NOT AVAILABLE"
const char *COMPILED_PIXMAN_VERSION_STRING = PIXMAN_VERSION_STRING;

const char*
cairo_version_string (void)
{
 return CAIRO_VERSION_STRING;
}

const char*
pixman_version_string (void)
{
 return PIXMAN_VERSION_STRING;
}





@ @c
char png_libpng_ver[] =      "PNG NOT AVAILABLE";

