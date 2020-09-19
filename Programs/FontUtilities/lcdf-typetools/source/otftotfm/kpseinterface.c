/* kpseinterface.{c,h} -- interface with the kpathsea library
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

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <kpathsea/config.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <kpathsea/progname.h>
#include <kpathsea/expand.h>
#include <kpathsea/c-pathch.h>
#include <kpathsea/tex-file.h>
#include "kpseinterface.h"

int kpsei_env_sep_char = ENV_SEP;

void
kpsei_init(const char* argv0, const char* progname)
{
    kpse_set_program_name(argv0, progname);
#ifdef SELFAUTODIR
    putenv("SELFAUTODIR=" SELFAUTODIR);
#endif
#ifdef SELFAUTOLOC
    putenv("SELFAUTOLOC=" SELFAUTOLOC);
#endif
#ifdef SELFAUTOPARENT
    putenv("SELFAUTOPARENT=" SELFAUTOPARENT);
#endif
#ifdef SELFAUTOGRANDPARENT
    putenv("SELFAUTOGRANDPARENT=" SELFAUTOGRANDPARENT);
#endif
}

char*
kpsei_path_expand(const char* path)
{
    return kpse_path_expand(path);
}

char*
kpsei_find_file(const char* name, int format)
{
    char *result;
    switch (format) {
      case KPSEI_FMT_WEB2C:
	return kpse_find_file(name, kpse_web2c_format, true);
    case KPSEI_FMT_ENCODING:
#if HAVE_DECL_KPSE_ENC_FORMAT
	if ((result = kpse_find_file(name, kpse_enc_format, true)))
	    return result;
#endif
	return kpse_find_file(name, kpse_tex_ps_header_format, true);
      case KPSEI_FMT_TYPE1:
	return kpse_find_file(name, kpse_type1_format, false);
      case KPSEI_FMT_TYPE42:
	return kpse_find_file(name, kpse_type42_format, false);
      case KPSEI_FMT_TRUETYPE:
	return kpse_find_file(name, kpse_truetype_format, false);
#if HAVE_DECL_KPSE_OPENTYPE_FORMAT
      case KPSEI_FMT_OPENTYPE:
	return kpse_find_file(name, kpse_opentype_format, false);
#endif
      case KPSEI_FMT_OTHER_TEXT:
	return kpse_find_file(name, kpse_program_text_format, true);
      case KPSEI_FMT_MAP:
	return kpse_find_file(name, kpse_fontmap_format, true);
      default:
	return 0;
    }
}

void
kpsei_set_debug_flags(unsigned flags)
{
    kpathsea_debug = flags;
}
