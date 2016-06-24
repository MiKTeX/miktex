/* ffdummies.c
   
   Copyright 2015 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */

#ifndef _FFDUMMIES_H
#define _FFDUMMIES_H

#include "uiinterface.h"
#include "fontforgevw.h"
#include "splinefont.h"
#include <stdio.h>
#include <basics.h>
#include <ustring.h>

#include <lua.h>
#ifdef LuajitTeX
#include <lua/lauxlib_bridge.h>
#else
#include <lauxlib.h>
#endif
#include <lua/luatex-api.h>
#include <locale.h>

/* fix for non-gcc compilation: */
#if !defined(__GNUC__) || (__GNUC__ < 2)
#  define __attribute__(x)
#endif                          /* !defined(__GNUC__) || (__GNUC__ < 2) */

extern char **gww_errors;
extern int gww_error_count;
extern void gwwv_errors_free(void);
extern struct ui_interface luaui_interface;

extern int readbinfile(FILE * f, unsigned char **b, int *s);

extern char *SaveTablesPref;
extern char *coord_sep;

#endif /* _FFDUMMIES_H */
