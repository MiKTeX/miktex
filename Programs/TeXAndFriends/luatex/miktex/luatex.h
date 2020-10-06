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
int miktex_allow_unrestricted_shell_escape();
char* miktex_banner(const char* name, const char* version);
void miktex_convert_to_unix(char* path);
#if defined(MIKTEX_WINDOWS)
void miktex_copy_wchar_to_utf8(char* dest, size_t destSize, const wchar_t* source);
#endif
void miktex_emulate__close_file_or_pipe(FILE* file);
char** miktex_emulate__do_split_command(const char* commandLine, char** argv0);
int miktex_emulate__exec_command(const char* fileName, char* const* argv, char* const* env);
FILE* miktex_emulate__runpopen(const char* commandLine, const char* mode);
int miktex_emulate__shell_cmd_is_allowed(const char* commandLine, char** safeCommandLineRet, char** examinedCommandRet);
int miktex_emulate__spawn_command(const char* fileName, char* const* argv, char* const* env);
void miktex_enable_installer(int onOff);
const char* miktex_get_aux_directory();
void miktex_invoke_editor(const char* filename, int lineno);
int miktex_is_fully_qualified_path(const char* path);
int miktex_is_output_file(const char* path);
int miktex_is_pipe(FILE* file);
int miktex_open_format_file(const char* fileName, FILE** ppFile, int renew);
void miktex_print_banner(FILE* file, const char* name, const char* version);
void miktex_set_aux_directory(const char* path);
void miktex_show_library_versions();
#if defined(MIKTEX_WINDOWS)
char* miktex_wchar_to_utf8(const wchar_t* w);
#endif

#if defined(__cplusplus)
}
#endif
