/* luatex/miktex/luatex.h:

   Copyright (C) 2016-2020 Christian Schenk

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

#if defined(__cplusplus)
extern "C" {
#endif

void miktex_add_include_directory(const char* path);
void miktex_enable_installer(int onOff);
const char* miktex_get_aux_directory();
void miktex_invoke_editor(const char* filename, int lineno);
int miktex_is_absolute_path(const char* path);
int miktex_is_output_file(const char* path);
int miktex_open_format_file(const char* fileName, FILE** ppFile, int renew);
char* miktex_program_basename(const char* argv0);
void miktex_set_aux_directory(const char* path);
void miktex_show_library_versions();

#if defined(__cplusplus)
}
#endif
