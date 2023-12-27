/**
 * @file luatex/miktex/luatex.h
 * @author Christian Schenk
 * @brief MiKTeX LuaTeX helpers
 *
 * @copyright Copyright © 2016-2023 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

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
int miktex_hack__is_luaotfload_file(const char* path);
int miktex_is_output_file(const char* path);
int miktex_is_pipe(FILE* file);
int miktex_open_format_file(const char* fileName, FILE** ppFile, int renew);
FILE* miktex_open_output_file(const char* fileName);
void miktex_print_banner(FILE* file, const char* name, const char* version);
void miktex_set_aux_directory(const char* path);
void miktex_show_library_versions();
#if defined(MIKTEX_WINDOWS)
char* miktex_wchar_to_utf8(const wchar_t* w);
#endif

#if defined(__cplusplus)
}
#endif
