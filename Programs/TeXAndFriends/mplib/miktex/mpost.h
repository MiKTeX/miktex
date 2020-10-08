/* mplib/miktex/mpost.h:

   Copyright (C) 2017-2020 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#pragma once

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

void miktex_print_banner(FILE* file, const char* name, const char* version);
void miktex_show_library_versions();

#if defined(__cplusplus)
}
#endif
