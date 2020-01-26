/* miktex/KPSE/Emulation.h:                             -*- C++ -*-

   Copyright 1993, 1995, 1996, 2005, 2008, 2009, 2010 Karl Berry
   Copyright (C) 2000-2019 Christian Schenk

   This file is part of the MiKTeX KPSEMU Library.

   The MiKTeX KPSEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX KPSEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX KPSEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/// @file miktex/KPSE/Emulation.h

/// @defgroup KPSE Kpathsea emulation
/// 
/// @brief Utilities for emulating the Kpathsea library.
///
/// MiKTeX has it's own file searching library. In order to support
/// programs relying on Kpathsea, we implement an emulation library.

/// @{

#pragma once

#if !defined(DF03C93E315147A6AEEE6CFC30D01E42)
/// @cond
#define DF03C93E315147A6AEEE6CFC30D01E42
/// @endcond

#include <miktex/First.h>
#include <miktex/Version.h>

#if defined(_MSC_VER)
#  include <io.h>
#  include <fcntl.h>
#  include <sys/types.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kpathsea/kpathsea.h>

#if defined(__cplusplus)
#  include <miktex/Core/PathName>
#else
#  include <miktex/Core/c/api.h>
#endif

#include <miktex/Core/Debug>
#include <miktex/Core/IntegerTypes>

/// @cond

// DLL import/export switch
#if !defined(D0A4167033297F40884B97769F47801C)
#  if defined(MIKTEX_KPSEMU_SHARED)
#    define MIKTEXKPSEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXKPSEXPORT
#  endif
#endif

// API decoration for exported functions and data
#define MIKTEXKPSCEEAPI(type) MIKTEXKPSEXPORT type MIKTEXCEECALL
#define MIKTEXKPSDATA(type) MIKTEXKPSEXPORT type

#define MIKTEX_KPATHSEA_LFS 1

#if defined(MIKTEX_KPATHSEA_LFS)
#  define off_t MIKTEX_INT64
#endif

/// @endcond

/* _________________________________________________________________________ */

MIKTEX_BEGIN_EXTERN_C_BLOCK;

MIKTEXKPSCEEAPI(char*) miktex_concatn(const char* s1, ...);

MIKTEX_END_EXTERN_C_BLOCK;

/* _________________________________________________________________________
 *
 * config.h
 *
 */

/// @name config.h
/// Stuff from `config.h`.
/// @{

#if defined(MIKTEX_WINDOWS)
#  define DEV_NULL "NUL"
#else
#  define DEV_NULL "/dev/null"
#endif

/// @}

/* _________________________________________________________________________
 *
 * confdefs.h
 *
 */

/// @name confdefs.h
/// Stuff from `confdefs.h`.
/// @{

#define KPSEVERSION MIKTEX_BANNER_STR

/// @}

/* _________________________________________________________________________
 *
 * paths.h
 *
 */

#define DEFAULT_BIBTEX_CSFILE "88591lat.csf"

#define DEFAULT_TEXMFCNF "{$SELFAUTOLOC,$SELFAUTODIR,$SELFAUTOPARENT}{,{/share,}/texmf{-local,}/web2c}"

/* _________________________________________________________________________
 *
 * c-fopen.h
 *
 */

/// @name c-fopen.h
/// Stuff from `c-fopen.h`.
/// @{

#if !defined(FOPEN_R_MODE)
#  define FOPEN_R_MODE "r"
#endif

#if !defined(FOPEN_W_MODE)
#  define FOPEN_W_MODE "w"
#endif

#if !defined(FOPEN_RBIN_MODE)
#  define FOPEN_RBIN_MODE "rb"
#endif

#if !defined(FOPEN_WBIN_MODE)
#  define FOPEN_WBIN_MODE "wb"
#endif

#if !defined(FOPEN_ABIN_MODE)
#  define FOPEN_ABIN_MODE "ab"
#endif

#if defined(MIKTEX_WINDOWS)
#  define SET_BINARY(f) _setmode(f, _O_BINARY)
#else
#  define SET_BINARY(f)
#endif

/// @}

/* _________________________________________________________________________
 *
 * c-memstr.h
 *
 */

/// @name c-memstr.h
/// Stuff from `c-memstr.h`.
/// @{

#if !defined(HAVE_INDEX) && !defined(index)
#  define index(s, c) strchr(s, c)
#endif

#if !defined(HAVE_RINDEX) && !defined(rindex)
#  define rindex(s, c) strrchr(s, c)
#endif

/// @}

/* _________________________________________________________________________
 *
 * c-pathch.h
 *
 */

/// @name c-patch.h
/// Stuff from `c-pathch.h`.
/// @{

#if defined(MIKTEX_WINDOWS)
#  define DIR_SEP '\\'
#  define DIR_SEP_STRING "\\"
#else
#  define DIR_SEP '/'
#  define DIR_SEP_STRING "/"
#endif

#if defined(MIKTEX_WINDOWS)
#  define IS_DIR_SEP(ch) ((ch) == '\\' || (ch) == '/')
#else
#  define IS_DIR_SEP(ch) ((ch) == '/')
#endif

#define IS_DIR_SEP_CH(ch) ((ch) == '/')

#if defined(MIKTEX_WINDOWS)
#  define ENV_SEP ';'
#  define ENV_SEP_STRING ";"
#else
#  define ENV_SEP ':'
#  define ENV_SEP_STRING ":"
#endif

#if defined(MIKTEX_WINDOWS)
#  define IS_ENV_SEP(ch) ((ch) == ';')
#else
#  define IS_ENV_SEP(ch) ((ch) == ':')
#endif

#if defined(MIKTEX_WINDOWS)
#  define NAME_BEGINS_WITH_DEVICE(name) \
    ((name)[0] != 0 && isascii((name)[0]) && isalpha((name)[0]) && ((name)[1] == ':'))
#else
#  define NAME_BEGINS_WITH_DEVICE(name) 0
#endif

/// @}

/* _________________________________________________________________________
 *
 * types.h
 *
 */

/// @name types.h
/// Stuff from `types.h`.
/// @{

#define KPSE_COMPAT_API 1

MIKTEX_BEGIN_EXTERN_C_BLOCK;

typedef char* string;
typedef const char* const_string;

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

#if !defined(HAVE_BOOLEAN)
#  define HAVE_BOOLEAN 1
#  if defined(_WIN32)
  // FIXME:  boolean is a MIDL base type (see <rpcndr.h>)
  typedef int miktex_kpse_boolean;
#  define boolean miktex_kpse_boolean
#  else
  typedef int boolean;
#  endif
#  if !defined(__cplusplus)
#    if !defined(true)
#      define true 1
#      define false 0
#    endif
#  endif
#endif

typedef enum
{
  kpse_afm_format,
  kpse_any_glyph_format,
  kpse_base_format,
  kpse_bib_format,
  kpse_bst_format,
  kpse_cid_format,
  kpse_clua_format,
  kpse_cmap_format,
  kpse_cnf_format,
  kpse_cweb_format,
  kpse_db_format,
  kpse_dvips_config_format,
  kpse_enc_format,
  kpse_fea_format,
  kpse_fmt_format,
  kpse_fontmap_format,
  kpse_gf_format,
  kpse_ist_format,
  kpse_lig_format,
  kpse_lua_format,
  kpse_mem_format,
  kpse_mf_format,
  kpse_mft_format,
  kpse_miscfonts_format,
  kpse_mfpool_format,
  kpse_mlbib_format,
  kpse_mlbst_format,
  kpse_mp_format,
  kpse_mppool_format,
  kpse_mpsupport_format,
  kpse_ocp_format,
  kpse_ofm_format,
  kpse_opentype_format,
  kpse_opl_format,
  kpse_otp_format,
  kpse_ovf_format,
  kpse_ovp_format,
  kpse_pdftex_config_format,
  kpse_pict_format,
  kpse_pk_format,
  kpse_program_binary_format,
  kpse_program_text_format,
  kpse_sfd_format,
  kpse_tex_format,
  kpse_tex_ps_header_format,
  kpse_texpool_format,
  kpse_texsource_format,
  kpse_texdoc_format,
  kpse_texmfscripts_format,
  kpse_tfm_format,
  kpse_troff_font_format,
  kpse_truetype_format,
  kpse_type1_format,
  kpse_type42_format,
  kpse_vf_format,
  kpse_web2c_format,
  kpse_web_format,

  kpse_last_format
} kpse_file_format_type;

typedef struct
{
  const_string type;
  string path;
  const_string* suffix;
  const_string* alt_suffix;
  boolean program_enabled_p;
} kpse_format_info_type;

typedef enum
{
  kpse_src_implicit,
  kpse_src_compile,
  kpse_src_texmf_cnf,
  kpse_src_client_cnf,
  kpse_src_env,
  kpse_src_x,
  kpse_src_cmdline
} kpse_src_type;

typedef struct kpathsea_instance * kpathsea;

typedef struct kpathsea_instance
{
  char* program_name;
  const_string fallback_resolutions_string;
  unsigned debug;
  boolean make_tex_discard_errors;
  char* invocation_name;
  kpse_format_info_type format_info[kpse_last_format];
} kpathsea_instance;

extern MIKTEXKPSDATA(kpathsea_instance) miktex_kpse_def_inst;

#define kpathsea_new() miktex_kpathsea_new()

#define kpathsea_finish(kpse) miktex_kpathsea_finish(kpse)

#if defined(KPSE_COMPAT_API)

#define kpse_def_inst miktex_kpse_def_inst

#define kpse_def miktex_kpse_def

#define kpse_bug_address miktex_kpathsea_bug_address

#define kpse_program_name kpse_def_inst.program_name

#define kpse_fallback_resolutions_string \
  kpse_def_inst.fallback_resolutions_string

#define kpse_format_info kpse_def_inst.format_info

#define kpathsea_debug kpse_def_inst.debug

#define kpse_make_tex_discard_errors kpse_def_inst.make_tex_discard_errors

#define kpse_invocation_name kpse_def_inst.invocation_name

#if 1 && ! defined(program_invocation_name)
#  define program_invocation_name kpse_def_inst.invocation_name
#endif

#endif

MIKTEX_END_EXTERN_C_BLOCK;

/// @}

/* _________________________________________________________________________
 *
 * lib.h
 *
 */

/// @name lib.h
/// Stuff from `lib.h`.
/// @{

#define START_WARNING() do { fputs("warning: ", stderr)
#define END_WARNING() fputs(".\n", stderr); fflush(stderr); } while (0)

#define WARNING(str)                                                    \
  START_WARNING(); fputs(str, stderr); END_WARNING()
#define WARNING1(str, e1)                                               \
  START_WARNING(); fprintf(stderr, str, e1); END_WARNING()
#define WARNING2(str, e1, e2)                                           \
  START_WARNING(); fprintf(stderr, str, e1, e2); END_WARNING()
#define WARNING3(str, e1, e2, e3)                                       \
  START_WARNING(); fprintf(stderr, str, e1, e2, e3); END_WARNING()
#define WARNING4(str, e1, e2, e3, e4)                                   \
  START_WARNING(); fprintf(stderr, str, e1, e2, e3, e4); END_WARNING()

#define START_FATAL()                                           \
  do {                                                          \
    fprintf(stderr, "%s: fatal: ", kpse_invocation_name);

#if defined(__cplusplus)
#define END_FATAL()                             \
    fputs(".\n", stderr);                       \
    throw 1;                                    \
  } while (0)
#else
#define END_FATAL()                             \
    fputs(".\n", stderr);                       \
    miktex_exit(1);                             \
  } while (0)
#endif

#define FATAL(str)                                                      \
  START_FATAL(); fputs(str, stderr); END_FATAL()

#define FATAL1(str, e1)                                                 \
  START_FATAL(); fprintf(stderr, str, e1); END_FATAL()

#define FATAL2(str, e1, e2)                                             \
  START_FATAL(); fprintf(stderr, str, e1, e2); END_FATAL()

#define FATAL3(str, e1, e2, e3)                                         \
  START_FATAL(); fprintf(stderr, str, e1, e2, e3); END_FATAL()

#define FATAL4(str, e1, e2, e3, e4)                                     \
  START_FATAL(); fprintf(stderr, str, e1, e2, e3, e4); END_FATAL()

#define FATAL5(str, e1, e2, e3, e4, e5)                                 \
  START_FATAL(); fprintf(stderr, str, e1, e2, e3, e4, e5); END_FATAL()

#define FATAL6(str, e1, e2, e3, e4, e5, e6)                             \
  START_FATAL(); fprintf(stderr, str, e1, e2, e3, e4, e5, e6); END_FATAL()

#if defined(__cplusplus)
MIKTEXNORETURN inline void FATAL_PERROR(const char* str)
{
  fprintf(stderr, "%s: ", kpse_invocation_name);
  perror(str);
  throw 1;
}
#else
#define FATAL_PERROR(str)                               \
  do {                                                  \
    fprintf(stderr, "%s: ", kpse_invocation_name);      \
    perror(str);                                        \
    miktex_exit(1);                                     \
  } while (0)
#endif

#if defined(__cplusplus)
inline int FILESTRCASEEQ(const char* s1, const char* s2)
{
  return MiKTeX::Core::PathName::Compare(s1, s2) == 0;
}
#else
#define FILESTRCASEEQ(s1, s2) (miktex_pathcmp(s1, s2) == 0)
#endif

#define STREQ(s1, s2) ((s1) != 0 && (s2) != 0 && (strcmp(s1, s2) == 0))

#define XRETALLOC(addr, n, t) \
  ((addr) = (t*)xrealloc(addr, (n) * sizeof(t)))

#define XTALLOC(n, t) ((t*)xmalloc((n) * sizeof(t)))

#define XTALLOC1(t) XTALLOC(1, t)

#if defined(__cplusplus)
inline char* concat(const char* s1, const char* s2)
{
  return miktex_concatn(s1, s2, nullptr);
}
#else
#define concat(s1, s2) concatn(s1, s2, 0)
#endif

#define concat3(s1, s2, s3) concatn(s1, s2, s3, 0)

#define find_suffix(name) miktex_find_suffix(name)

#define remove_suffix(name) miktex_remove_suffix(name)

#define xbasename(name) miktex_xbasename(name)

#define xdirname(name) miktex_xdirname(name)

#define xfclose(fp, filename) miktex_xfclose(fp, filename)

#define xfopen(filename, mode) miktex_xfopen(filename, mode)

#define xfseek(fp, offset, wherefrom, filename) \
      miktex_xfseek(fp, offset, wherefrom, filename)

#if MIKTEX_KPATHSEA_LFS
#define xfseeko(fp, offset, wherefrom, filename) \
      miktex_xfseeko64(fp, offset, wherefrom, filename)
#else
#define xfseeko(fp, offset, wherefrom, filename) \
      miktex_xfseeko(fp, offset, wherefrom, filename)
#endif

#define xftell(fp, filename) \
      miktex_xftello64(fp, filename)

#if MIKTEX_KPATHSEA_LFS
#define xftello(fp, filename) \
      miktex_xftello64(fp, filename)
#else
#define xftello(fp, filename) \
      miktex_xftello(fp, filename)
#endif

#define xmalloc(size) miktex_core_malloc(size, __FILE__, __LINE__)

#define xrealloc(ptr, size) \
        miktex_core_realloc(ptr, size, __FILE__, __LINE__)

#define xcalloc(n, size) \
        miktex_core_calloc(n, size, __FILE__, __LINE__)

#define xstrdup(str) miktex_core_strdup(str, __FILE__, __LINE__)

#define uppercasify(s) miktex_uppercasify(s)

#define kpathsea_xputenv(kpse, var, value)      \
  miktex_kpathsea_xputenv(kpse, var, value)

#if defined(KPSE_COMPAT_API)
#  define xputenv(var, value) kpathsea_xputenv(kpse_def, var, value)
#endif

#define xgetcwd() miktex_xgetcwd()

/// @}

/* _________________________________________________________________________
 *
 * concatn.h
 *
 */

/// @name concatn.h
/// Stuff from `concatn.h`.
/// @{

#define concatn miktex_concatn

/// @}

/* _________________________________________________________________________
 *
 * c-type.h
 *
 */

/// @name c-type.h
/// Stuff from `c-type.h`.
/// @{

#define ISALNUM(c) (isascii(c) && isalnum(c))

#define ISXDIGIT(c) (isascii(c) && isxdigit(c))

#define TOLOWER(c) ((isascii(c) && isupper(c)) ? tolower(c) : (c))

#define ISBLANK(c) (isascii(c) && ((c) == ' ' || (c) == '\t'))

/// @}

/* _________________________________________________________________________
 *
 * debug.h
 *
 */

/// @name debug.h
/// Stuff from `debug.h`.
/// @{

#define KPSE_DEBUG_SET(bit)

/// @}

/* _________________________________________________________________________
 *
 * c-proto.h
 *
 */

/// @name c-proto.h
/// Stuff from `c-proto.h`.
/// @{

#if defined(MIKTEX_WINDOWS)
#  define KPSEDLL MIKTEXDLLIMPORT
#else
#  define KPSEDLL
#endif

/// @}

/* _________________________________________________________________________
 *
 * c-auto.h
 *
 */

/// @name c-auto.h
/// Stuff from `c-auto.h`.
/// @{

#define MAKE_TEX_PK_BY_DEFAULT 1
#define MAKE_TEX_FMT_BY_DEFAULT 1

/// @}

/* _________________________________________________________________________
 *
 * str-list.h
 *
 */

/// @name str-list.h
/// Stuff from `str-list.h`.
/// @{

typedef struct
{
  unsigned length;
  char** list;
} str_list_type;

#define STR_LIST_LENGTH(l) ((l).length)
#define STR_LIST_EMPTY(l) (STR_LIST_LENGTH(l) == 0)

#define STR_LIST_ELT(l, n) ((l).list[(n)])

#define str_list_add(l, s) miktex_str_list_add(l, s)

/// @}

/* _________________________________________________________________________
 *
 * readable.h
 *
 */

/// @name readable.h
/// Stuff from `readable.h`.
/// @{

#define kpathsea_readable_file(kpse, name) \
  miktex_kpathsea_readable_file(kpse, name)

#if defined(KPSE_COMPAT_API)

#  define kpse_readable_file(name) \
  kpathsea_readable_file(kpse_def, name)

#endif

/// @}

/* _________________________________________________________________________
 *
 * pathsearch.h
 *
 */

/// @name pathsearch.h
/// Stuff from `pathsearch.h`.
/// @{

#define kpathsea_path_search(kpse, path, name, must_exist) \
  miktex_kpathsea_path_search(kpse, path, name, must_exist)

#define kpathsea_all_path_search(kpse, path, name) \
  miktex_kpathsea_all_path_search(kpse, path, name)

/// @}

/* _________________________________________________________________________
 *
 * tex-file.h
 *
 */

/// @name tex-file.h
/// Stuff from `tex-file.h`.
/// @{

#define kpathsea_find_file(kpse, name, format, must_exist) \
  miktex_kpathsea_find_file(kpse, name, format, must_exist)

#define kpathsea_find_file_generic(kpse, name, format, must_exist, all) \
  miktex_kpathsea_find_file_generic(kpse, name, format, must_exist, all)

#define kpathsea_in_name_ok(kpse, fname) miktex_kpathsea_in_name_ok(kpse, fname, 0)
#define kpathsea_in_name_ok_silent(kpse, fname) miktex_kpathsea_in_name_ok(kpse, fname, 1)

#define kpathsea_out_name_ok(kpse, fname) miktex_kpathsea_out_name_ok(kpse, fname, 0)
#define kpathsea_out_name_ok_silent(kpse, fname) miktex_kpathsea_out_name_ok(kpse, fname, 1)

#define kpathsea_init_format(kpse, format) \
  miktex_kpathsea_init_format(kpse, format)

#define kpathsea_maketex_option(kpse, fmtname, value) \
  miktex_kpathsea_maketex_option(kpse, fmtname, value)

#define kpathsea_open_file(kpse, name, format) miktex_kpathsea_open_file(kpse, name, format)

#define kpathsea_selfdir(kpse, argv0) \
  miktex_kpathsea_selfdir(kpse, argv0)

#define kpathsea_set_program_enabled(kpse, fmt, value, level) \
  miktex_kpathsea_set_program_enabled(kpse, fmt, value, level)

#if defined(KPSE_COMPAT_API)

#define kpse_selfdir(argv0) kpathsea_selfdir(kpse_def, argv0)

#define kpse_set_program_enabled(fmt, value, level) \
  kpathsea_set_program_enabled(kpse_def, fmt, value, level)

#define kpse_reset_program_name(progname)

#define kpse_find_file(name, format, must_exist) \
  kpathsea_find_file(kpse_def, name, format, must_exist)

#define kpse_find_ofm(name) kpse_find_file(name, kpse_ofm_format, 1)

#define kpse_find_pict(name) kpse_find_file(name, kpse_pict_format, 1)

#define kpse_find_tex(name)  kpse_find_file(name, kpse_tex_format, 1)

#define kpse_find_tfm(name) kpse_find_file(name, kpse_tfm_format, 1)

#define kpse_maketex_option(fmtname, value) \
  kpathsea_maketex_option(kpse_def, fmtname, value)

#define kpse_open_file(name, format) \
  kpathsea_open_file(kpse_def, name, format)

#define kpse_in_name_ok(fname) kpathsea_in_name_ok(kpse_def, fname)

#define kpse_out_name_ok(fname) kpathsea_out_name_ok(kpse_def, fname)

#define kpse_init_format(format) \
  kpathsea_init_format(kpse_def, format)

#endif

/// @}

/* _________________________________________________________________________
 *
 * absolute.h
 *
 */

/// @name absolute.h
/// Stuff from `absolute.h`.
/// @{

#define kpathsea_absolute_p(kpse, filename, relative_ok)        \
  miktex_kpathsea_absolute_p(kpse, filename, relative_ok)

#if defined(KPSE_COMPAT_API)
#  define kpse_absolute_p(filename, relative_ok)          \
  kpathsea_absolute_p(kpse_def, filename, relative_ok)
#endif

/// @}

/* _________________________________________________________________________
 *
 * variable.h
 *
 */

/// @name variable.h
/// Stuff from `variable.h`.
/// @{

#define kpathsea_var_expand(kpse, src) \
  miktex_kpathsea_var_expand(kpse, src)

#define kpathsea_var_value(kpse, var) \
  miktex_kpathsea_var_value(kpse, var)

#if defined(KPSE_COMPAT_API)

#  define kpse_var_expand(src) kpathsea_var_expand(kpse_def, src)

#  define kpse_var_value(var) kpathsea_var_value(kpse_def, var)

#endif

/// @}

/* _________________________________________________________________________
 *
 * expand.h
 *
 */

/// @name expand.h
/// Stuff from `expand.h`.
/// @{

#define kpathsea_path_expand(kpse, path) \
  miktex_kpathsea_path_expand(kpse, path)

#define kpathsea_brace_expand(kpse, path) \
  miktex_kpathsea_brace_expand(kpse, path)

#if defined(KPSE_COMPAT_API)
#  define kpse_brace_expand(path) kpathsea_brace_expand(kpse_def, path)
#  define kpse_path_expand(path) kpathsea_path_expand(kpse_def, path)
#endif

/// @}

/* _________________________________________________________________________
 *
 * version.h
 *
 */

/// @name version.h
/// Stuff from `version.h`.
/// @{

#define kpathsea_version_string miktex_kpathsea_version_string

#define kpathsea_bug_address miktex_kpathsea_bug_address

/// @}

/* _________________________________________________________________________
 *
 * tex-glyph.h
 *
 */

/// @name tex-glyph.h
/// Stuff from `tex-glyph.h`.
/// @{

MIKTEX_BEGIN_EXTERN_C_BLOCK;

typedef enum
{
  kpse_glyph_source_normal,
  kpse_glyph_source_alias,
  kpse_glyph_source_maketex,
  kpse_glyph_source_fallback
} kpse_glyph_source_type;

typedef struct
{
  char* name;
  unsigned dpi;
  kpse_file_format_type format;
  kpse_glyph_source_type source;
} kpse_glyph_file_type;         

MIKTEX_END_EXTERN_C_BLOCK;

#define kpathsea_find_glyph(kpse, font_name, dpi, format, glyph_file) \
  miktex_kpathsea_find_glyph(kpse, font_name, dpi, format, glyph_file)

#define kpathsea_bitmap_tolerance(kpse, dpi1, dpi2) \
  miktex_kpathsea_bitmap_tolerance(kpse, dpi1, dpi2)

#if defined(KPSE_COMPAT_API)

#define kpse_bitmap_tolerance(dpi1, dpi2) \
  kpathsea_bitmap_tolerance(kpse_def, dpi1, dpi2)

#define kpse_find_glyph(font_name, dpi, format, glyph_file) \
  kpathsea_find_glyph(kpse_def, font_name, dpi, format, glyph_file)

#define kpse_find_pk(font_name, dpi, glyph_file) \
  kpse_find_glyph(font_name, dpi, kpse_pk_format, glyph_file)

#define kpse_find_vf(name) kpse_find_file(name, kpse_vf_format, 0)

#endif

/// @}

/* _________________________________________________________________________
 *
 * tex-hush.h
 *
 */

/// @name tex-hush.h
/// Stuff from `tex-hush.h`.
/// @{

#if defined(KPSE_COMPAT_API)
#  define kpse_tex_hush(what) 0
#endif

/// @}

/* _________________________________________________________________________
 *
 * proginit.h
 *
 */

/// @name proginit.h
/// Stuff from `proginit.h`.
/// @{

#define kpathsea_init_prog(kpse, prefix, dpi, mode, fallback) \
  miktex_kpathsea_init_prog(kpse, prefix, dpi, mode, fallback)

#if defined(KPSE_COMPAT_API)

#define kpse_init_prog(prefix, dpi, mode, fallback) \
  kpathsea_init_prog(kpse_def, prefix, dpi, mode, fallback)

#endif

/// @}

/* _________________________________________________________________________
 *
 * magstep.h
 *
 */

/// @name magstep.h
/// Stuff from `magstep.h`.
/// @{

#define kpathsea_magstep_fix(kpse, dpi, bdpi, m_ret)    \
  miktex_kpathsea_magstep_fix(kpse, dpi, bdpi, m_ret)

#if defined(KPSE_COMPAT_API)
#  define kpse_magstep_fix(dpi, bdpi, m_ret) \
  kpathsea_magstep_fix(kpse_def, dpi, bdpi, m_ret)
#endif

/// @}

/* _________________________________________________________________________
 *
 * progname.h
 *
 */

/// @name progname.h
/// Stuff from `progname.h`.
/// @{

#define kpathsea_set_program_name(kpse, argv0, progname) \
  miktex_kpathsea_set_program_name(kpse, argv0, progname)

#define kpse_program_basename(argv0) \
  miktex_kpathsea_program_basename(argv0)

#if defined(KPSE_COMPAT_API)

#define kpse_set_program_name(argv0, progname) \
  kpathsea_set_program_name(kpse_def, argv0, progname)

#define kpse_set_progname(argv0) kpse_set_program_name(argv0, 0)

#endif

/// @}

/* _________________________________________________________________________
 *
 * line.h
 *
 */

/// @name line.h
/// Stuff from `line.h`.
/// @{

#define read_line(f) miktex_read_line(f)

/// @}

/* _________________________________________________________________________
 *
 * cnf.h
 *
 */

 /// @name cnf.h
 /// Stuff from `cnf.h`.
 /// @{

// FIXME
#define kpathsea_cnf_line_env_progname(kpse, l)

/// @}

/* _________________________________________________________________________ */

MIKTEX_BEGIN_EXTERN_C_BLOCK;

MIKTEXKPSCEEAPI(boolean) miktex_kpathsea_absolute_p(kpathsea kpseInstance, const char* fileName, boolean relativeOk);

MIKTEXKPSCEEAPI(char**) miktex_kpathsea_all_path_search(kpathsea kpseInstance, const char* path, const char* fileName);

MIKTEXKPSCEEAPI(int) miktex_kpathsea_bitmap_tolerance(kpathsea kpseInstance, double dpi1, double dpi2);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_brace_expand(kpathsea kpseInstance, const char* path);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_find_file(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format, int mustExist);

MIKTEXKPSCEEAPI(char**) miktex_kpathsea_find_file_generic(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format, boolean mustExist, boolean all);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_find_glyph(kpathsea kpseInstance, const char* fontName, unsigned dpi, kpse_file_format_type format, kpse_glyph_file_type* glyph_file);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_finish(kpathsea kpseInstance);

MIKTEXKPSCEEAPI(int) miktex_kpathsea_in_name_ok(kpathsea kpseInstance, const char* fileName, int silent);

MIKTEXKPSCEEAPI(const char*) miktex_kpathsea_init_format(kpathsea kpseInstance, kpse_file_format_type format);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_init_prog(kpathsea kpseInstance, const char* prefix, unsigned dpi, const char* modeString, const char* fallback);

MIKTEXKPSCEEAPI(unsigned) miktex_kpathsea_magstep_fix(kpathsea kpseInstance, unsigned dpi, unsigned bdpi, int* ret);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_maketex_option(kpathsea kpseInstance, const char* formatName, boolean value);

MIKTEXKPSCEEAPI(kpathsea) miktex_kpathsea_new();

MIKTEXKPSCEEAPI(FILE *) miktex_kpathsea_open_file(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format);

MIKTEXKPSCEEAPI(int) miktex_kpathsea_out_name_ok(kpathsea kpseInstance, const char* fileName, int silent);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_path_expand(kpathsea kpseInstance, const char* path);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_path_search(kpathsea kpseInstance, const char* path, const char* fileName, boolean mustExist);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_program_basename(const char* argv0);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_readable_file(kpathsea kpseInstance, const char* fileName);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_selfdir(kpathsea kpseInstance, const char* argv0);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_set_program_name(kpathsea kpseInstance, const char* argv0, const char* programName);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_set_program_enabled(kpathsea kpseInstance, kpse_file_format_type fmt, boolean value, kpse_src_type level);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_var_expand(kpathsea kpseInstance, const char* soure);

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_var_value(kpathsea kpseInstance, const char* varName);

MIKTEXKPSCEEAPI(void) miktex_kpathsea_xputenv(kpathsea kpseInstance, const char* varName, const char* value);

#if WITH_CONTEXT_SUPPORT
MIKTEXKPSCEEAPI(char*) miktex_kpsemu_create_texmf_cnf();
#endif

MIKTEXKPSCEEAPI(char*) miktex_find_suffix(const char* path);

MIKTEXKPSCEEAPI(char*) miktex_read_line(FILE* file);

MIKTEXKPSCEEAPI(char*) miktex_remove_suffix(const char* path);

MIKTEXKPSCEEAPI(void) miktex_str_list_add(str_list_type* stringList, char* s);

MIKTEXKPSCEEAPI(int) miktex_strcasecmp(const char* s1, const char* s2);

MIKTEXKPSCEEAPI(char*) miktex_uppercasify(const char* s);

MIKTEXKPSCEEAPI(const char*) miktex_xbasename(const char* fileName);

MIKTEXKPSCEEAPI(char*) miktex_xdirname(const char* fileName);

MIKTEXKPSCEEAPI(void) miktex_xfclose(FILE* file, const char* fileName);

MIKTEXKPSCEEAPI(FILE *) miktex_xfopen(const char* fileName, const char* modeString);

MIKTEXKPSCEEAPI(int) miktex_xfseek(FILE* file, long offset, int where, const char* fileName);

MIKTEXKPSCEEAPI(int) miktex_xfseeko(FILE* file, off_t offset, int where, const char* fileName);

MIKTEXKPSCEEAPI(int) miktex_xfseeko64(FILE* file, MIKTEX_INT64 offset, int where, const char* fileName);

MIKTEXKPSCEEAPI(long) miktex_xftell(FILE* file, const char* fileName);

MIKTEXKPSCEEAPI(off_t) miktex_xftello(FILE* file, const char* fileName);

MIKTEXKPSCEEAPI(MIKTEX_INT64) miktex_xftello64(FILE* file, const char* fileName);

MIKTEXKPSCEEAPI(char*) miktex_xgetcwd();

extern MIKTEXKPSDATA(const char*) miktex_kpathsea_version_string;

#if defined(KPSE_COMPAT_API)

extern MIKTEXKPSDATA(const char*) miktex_kpathsea_bug_address;

extern MIKTEXKPSDATA(kpathsea) miktex_kpse_def;

#endif

MIKTEX_END_EXTERN_C_BLOCK;

#endif

/// @}
