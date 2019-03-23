/* lkpselib.c

   Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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

#ifdef MF_LUA
#define EXTERN extern
#if defined(JIT)
#include "mfluajitd.h"
#else
#include "mfluad.h"
#endif
#include <kpathsea/version.h>
#define xfree(p) do { if (p != NULL) free(p); p = NULL; } while (0)
#else
#include "ptexlib.h"
#include "lua/luatex-api.h"
#endif
#include <kpathsea/expand.h>
#include <kpathsea/variable.h>
#include <kpathsea/tex-glyph.h>
#include <kpathsea/readable.h>
#include <kpathsea/pathsearch.h>
#include <kpathsea/str-list.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/paths.h>


static const unsigned filetypes[] = {
    kpse_gf_format,
    kpse_pk_format,
    kpse_any_glyph_format,
    kpse_tfm_format,
    kpse_afm_format,
    kpse_base_format,
    kpse_bib_format,
    kpse_bst_format,
    kpse_cnf_format,
    kpse_db_format,
    kpse_fmt_format,
    kpse_fontmap_format,
    kpse_mem_format,
    kpse_mf_format,
    kpse_mfpool_format,
    kpse_mft_format,
    kpse_mp_format,
    kpse_mppool_format,
    kpse_mpsupport_format,
    kpse_ocp_format,
    kpse_ofm_format,
    kpse_opl_format,
    kpse_otp_format,
    kpse_ovf_format,
    kpse_ovp_format,
    kpse_pict_format,
    kpse_tex_format,
    kpse_texdoc_format,
    kpse_texpool_format,
    kpse_texsource_format,
    kpse_tex_ps_header_format,
    kpse_troff_font_format,
    kpse_type1_format,
    kpse_vf_format,
    kpse_dvips_config_format,
    kpse_ist_format,
    kpse_truetype_format,
    kpse_type42_format,
    kpse_web2c_format,
    kpse_program_text_format,
    kpse_program_binary_format,
    kpse_miscfonts_format,
    kpse_web_format,
    kpse_cweb_format,
    kpse_enc_format,
    kpse_cmap_format,
    kpse_sfd_format,
    kpse_opentype_format,
    kpse_pdftex_config_format,
    kpse_lig_format,
    kpse_texmfscripts_format,
    kpse_lua_format,
    kpse_fea_format,
    kpse_cid_format,
    kpse_mlbib_format,
    kpse_mlbst_format,
    kpse_clua_format
};

static const char *const filetypenames[] = {
    "gf",
    "pk",
    "bitmap font",
    "tfm",
    "afm",
    "base",
    "bib",
    "bst",
    "cnf",
    "ls-R",
    "fmt",
    "map",
    "mem",
    "mf",
    "mfpool",
    "mft",
    "mp",
    "mppool",
    "MetaPost support",
    "ocp",
    "ofm",
    "opl",
    "otp",
    "ovf",
    "ovp",
    "graphic/figure",
    "tex",
    "TeX system documentation",
    "texpool",
    "TeX system sources",
    "PostScript header",
    "Troff fonts",
    "type1 fonts",
    "vf",
    "dvips config",
    "ist",
    "truetype fonts",
    "type42 fonts",
    "web2c files",
    "other text files",
    "other binary files",
    "misc fonts",
    "web",
    "cweb",
    "enc files",
    "cmap files",
    "subfont definition files",
    "opentype fonts",
    "pdftex config",
    "lig files",
    "texmfscripts",
    "lua",
    "font feature files",
    "cid maps",
    "mlbib",
    "mlbst",
    "clua",
    NULL
};


#ifdef MF
#define KPATHSEA_METATABLE  "mflua.kpathsea"
#else
#define KPATHSEA_METATABLE  "luatex.kpathsea"
#endif

/* set to 1 by the |program_name| function */

#ifdef MF
int program_name_set = 1;
#else
int program_name_set = 0;
#endif

#define TEST_PROGRAM_NAME_SET do {                                      \
    if (! program_name_set) {                                           \
      return luaL_error(L, "Please call kpse.set_program_name() before using the library"); \
    }                                                                   \
  } while (0)

static int find_file(lua_State * L)
{
    int i, t;
    const char *st;
    unsigned ftype = kpse_tex_format;
    int mexist = 0;
    TEST_PROGRAM_NAME_SET;
    if (lua_type(L, 1) != LUA_TSTRING) {
        luaL_error(L, "not a file name");
    }
    st = lua_tostring(L, 1);
    i = lua_gettop(L);
    while (i > 1) {
        t = lua_type(L, i) ;
        if (t == LUA_TBOOLEAN) {
            mexist = lua_toboolean(L, i);
        } else if (t == LUA_TNUMBER) {
            mexist = (int) lua_tointeger(L, i);
        } else if (t == LUA_TSTRING) {
            int op = luaL_checkoption(L, i, NULL, filetypenames);
            ftype = filetypes[op];
        } else {
            /* ignore */
        }
        i--;
    }
    if (ftype == kpse_pk_format ||
        ftype == kpse_gf_format || ftype == kpse_any_glyph_format) {
        /* ret.format, ret.name, ret.dpi */
        kpse_glyph_file_type ret;
        lua_pushstring(L, kpse_find_glyph(st, (unsigned) mexist, ftype, &ret));
    } else {
        if (mexist > 0)
            mexist = 1;
        if (mexist < 0)
            mexist = 0;
        lua_pushstring(L, kpse_find_file(st, ftype, mexist));
    }
    return 1;
}


static int lua_kpathsea_find_file(lua_State * L)
{
    int i, t;
    unsigned ftype = kpse_tex_format;
    int mexist = 0;
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *st = luaL_checkstring(L, 2);
    i = lua_gettop(L);
    while (i > 2) {
        t = lua_type(L, i) ;
        if (t == LUA_TBOOLEAN) {
            mexist = (boolean) lua_toboolean(L, i);
        } else if (t == LUA_TNUMBER) {
            mexist = (int) lua_tointeger(L, i);
        } else if (t == LUA_TSTRING) {
            int op = luaL_checkoption(L, i, NULL, filetypenames);
            ftype = filetypes[op];
        } else {
            /* ignore */
        }
        i--;
    }
    if (ftype == kpse_pk_format || ftype == kpse_gf_format || ftype == kpse_any_glyph_format) {
        /* ret.format, ret.name, ret.dpi */
        kpse_glyph_file_type ret;
        lua_pushstring(L, kpathsea_find_glyph(*kp, st, (unsigned) mexist, ftype, &ret));
    } else {
        if (mexist > 0)
            mexist = 1;
        if (mexist < 0)
            mexist = 0;
        lua_pushstring(L, kpathsea_find_file(*kp, st, ftype, mexist));
    }
    return 1;

}

static int show_texmfcnf(lua_State * L)
{
    lua_pushstring(L, DEFAULT_TEXMFCNF);
    return 1;
}

static int show_path(lua_State * L)
{
    int op = luaL_checkoption(L, -1, "tex", filetypenames);
    unsigned user_format = filetypes[op];
    TEST_PROGRAM_NAME_SET;
    if (!kpse_format_info[user_format].type)    /* needed if arg was numeric */
        kpse_init_format(user_format);
    lua_pushstring(L, kpse_format_info[user_format].path);
    return 1;
}

static int lua_kpathsea_show_path(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    int op = luaL_checkoption(L, -1, "tex", filetypenames);
    unsigned user_format = filetypes[op];
    if (!(*kp)->format_info[user_format].type)  /* needed if arg was numeric */
        kpathsea_init_format(*kp, user_format);
    lua_pushstring(L, (*kp)->format_info[user_format].path);
    return 1;
}

static int expand_path(lua_State * L)
{
    const char *st = luaL_checkstring(L, 1);
    TEST_PROGRAM_NAME_SET;
    lua_pushstring(L, kpse_path_expand(st));
    return 1;
}

static int lua_kpathsea_expand_path(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *st = luaL_checkstring(L, 2);
    lua_pushstring(L, kpathsea_path_expand(*kp, st));
    return 1;
}

static int expand_braces(lua_State * L)
{
    const char *st = luaL_checkstring(L, 1);
    TEST_PROGRAM_NAME_SET;
    lua_pushstring(L, kpse_brace_expand(st));
    return 1;
}

static int lua_kpathsea_expand_braces(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *st = luaL_checkstring(L, 2);
    lua_pushstring(L, kpathsea_brace_expand(*kp, st));
    return 1;
}


static int expand_var(lua_State * L)
{
    const char *st = luaL_checkstring(L, 1);
    TEST_PROGRAM_NAME_SET;
    lua_pushstring(L, kpse_var_expand(st));
    return 1;
}

static int lua_kpathsea_expand_var(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *st = luaL_checkstring(L, 2);
    lua_pushstring(L, kpathsea_var_expand(*kp, st));
    return 1;
}


static int var_value(lua_State * L)
{
    const char *st = luaL_checkstring(L, 1);
    TEST_PROGRAM_NAME_SET;
    lua_pushstring(L, kpse_var_value(st));
    return 1;
}

static int lua_kpathsea_var_value(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *st = luaL_checkstring(L, 2);
    lua_pushstring(L, kpathsea_var_value(*kp, st));
    return 1;
}

static unsigned find_dpi(const_string s)
{
    unsigned dpi_number = 0;
    const_string extension = find_suffix(s);

    if (extension != NULL)
        sscanf(extension, "%u", &dpi_number);

    return dpi_number;
}

/*
    Return newly-allocated NULL-terminated list of strings from MATCHES
    that are prefixed with any of the subdirectories in SUBDIRS.  That
    is, for a string S in MATCHES, its dirname must end with one of the
    elements in SUBDIRS.  For instance, if subdir=foo/bar, that will
    match a string foo/bar/baz or /some/texmf/foo/bar/baz.

    We don't reallocate the actual strings, just the list elements.
    Perhaps later we will implement wildcards or "//" or something.
*/

static string *subdir_match(str_list_type subdirs, string * matches)
{
    string *ret = XTALLOC1(string);
    unsigned len = 1;
    unsigned m;

    for (m = 0; matches[m]; m++) {
        size_t loc;
        unsigned e;
        string s = xstrdup(matches[m]);
        for (loc = strlen(s); loc > 0 && !IS_DIR_SEP(s[loc - 1]); loc--);
        while (loc > 0 && IS_DIR_SEP(s[loc - 1])) {
            loc--;
        }
        s[loc] = 0;             /* wipe out basename */

        for (e = 0; e < STR_LIST_LENGTH(subdirs); e++) {
            string subdir = STR_LIST_ELT(subdirs, e);
            size_t subdir_len = strlen(subdir);
            while (subdir_len > 0 && IS_DIR_SEP(subdir[subdir_len - 1])) {
                subdir_len--;
                subdir[subdir_len] = 0; /* remove trailing slashes from subdir spec */
            }
            if (FILESTRCASEEQ(subdir, s + loc - subdir_len)) {
                /* matched, save this one.  */
                XRETALLOC(ret, len + 1, string);
                ret[len - 1] = matches[m];
                len++;
            }
        }
        free(s);
    }
    ret[len - 1] = NULL;
    return ret;
}

/* Use the file type from -format if that was specified (i.e., the
   user_format global variable), else guess dynamically from NAME.
   Return kpse_last_format if undeterminable.  This function is also
   used to parse the -format string, a case which we distinguish by
   setting is_filename to false.

   A few filenames have been hard-coded for format types that
   differ from what would be inferred from their extensions. */

static kpse_file_format_type
find_format(kpathsea kpse, const_string name, boolean is_filename)
{
    kpse_file_format_type ret;

    if (FILESTRCASEEQ(name, "config.ps")) {
        ret = kpse_dvips_config_format;
    } else if (FILESTRCASEEQ(name, "dvipdfmx.cfg")) {
        ret = kpse_program_text_format;
    } else if (FILESTRCASEEQ(name, "fmtutil.cnf")) {
        ret = kpse_web2c_format;
    } else if (FILESTRCASEEQ(name, "glyphlist.txt")) {
        ret = kpse_fontmap_format;
    } else if (FILESTRCASEEQ(name, "mktex.cnf")) {
        ret = kpse_web2c_format;
    } else if (FILESTRCASEEQ(name, "pdfglyphlist.txt")) {
        ret = kpse_fontmap_format;
    } else if (FILESTRCASEEQ(name, "pdftex.cfg")) {
        ret = kpse_pdftex_config_format;
    } else if (FILESTRCASEEQ(name, "texmf.cnf")) {
        ret = kpse_cnf_format;
    } else if (FILESTRCASEEQ(name, "updmap.cfg")) {
        ret = kpse_web2c_format;
    } else if (FILESTRCASEEQ(name, "XDvi")) {
        ret = kpse_program_text_format;
    } else {
        int f = 0;              /* kpse_file_format_type */
        size_t name_len = strlen(name);

/* Have to rely on `try_len' being declared here, since we can't assume
   GNU C and statement expressions.  */
#define TRY_SUFFIX(ftry) (\
  try_len = (ftry) ? strlen (ftry) : 0, \
  (ftry) && try_len <= name_len \
     && FILESTRCASEEQ (ftry, name + name_len - try_len))

        while (f != kpse_last_format) {
            size_t try_len;
            const_string *ext;
            const_string ftry;
            boolean found = false;

            if (!kpse->format_info[f].type)
                kpathsea_init_format(kpse, (kpse_file_format_type) f);

            if (!is_filename) {
                /* Allow the long name, but only in the -format option.  We don't
                   want a filename confused with a format name.  */
                ftry = kpse->format_info[f].type;
                found = TRY_SUFFIX(ftry);
            }
            for (ext = kpse->format_info[f].suffix; !found && ext && *ext;
                 ext++) {
                found = TRY_SUFFIX(*ext);
            }
            for (ext = kpse->format_info[f].alt_suffix; !found && ext && *ext;
                 ext++) {
                found = TRY_SUFFIX(*ext);
            }

            if (found)
                break;

            /* Some trickery here: the extensions for kpse_fmt_format can
             * clash with other extensions in use, and we prefer for those
             * others to be preferred.  And we don't want to change the
             * integer value of kpse_fmt_format.  So skip it when first
             * enountered, then use it when we've done everything else,
             * and use it as the end-guard.
             */
            if (f == kpse_fmt_format) {
                f = kpse_last_format;
            } else if (++f == kpse_fmt_format) {
                f++;
            } else if (f == kpse_last_format) {
                f = kpse_fmt_format;
            }
        }

        /* If there was a match, f will be one past the correct value.  */
        ret = f;
    }

    return ret;
}

/* kpse:lookup("plain.tex", {}) */
static int do_lua_kpathsea_lookup(lua_State * L, kpathsea kpse, int idx)
{
    int i;
    string ret = NULL;
    string *ret_list = NULL;
    const_string name = NULL;
    string user_path = NULL;
    boolean show_all = false;
    boolean must_exist = false;
    kpse_file_format_type user_format = kpse_last_format;
    int dpi = 600;
    str_list_type subdir_paths = { 0, NULL };
    unsigned saved_debug = kpse->debug;
    int saved_mktexpk = kpse->format_info[kpse_pk_format].program_enabled_p;
    int saved_mktexmf = kpse->format_info[kpse_mf_format].program_enabled_p;
    int saved_mktextex = kpse->format_info[kpse_tex_format].program_enabled_p;
    int saved_mktextfm = kpse->format_info[kpse_tfm_format].program_enabled_p;
    name = luaL_checkstring(L, idx);
    /* todo: fetch parameter values */

    if (lua_type(L, idx + 1) == LUA_TTABLE) {
        lua_pushstring(L, "format");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TSTRING) {
            int op = luaL_checkoption(L, -1, NULL, filetypenames);
            user_format = filetypes[op];
        }
        lua_pop(L, 1);
        lua_pushstring(L, "dpi");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TNUMBER) {
            dpi = (int) lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
        lua_pushstring(L, "debug");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TNUMBER) {
            int d = 0;
            d = (int) lua_tointeger(L, -1);
            kpse->debug |= d;
        }
        lua_pop(L, 1);
        lua_pushstring(L, "path");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TSTRING) {
            user_path = xstrdup(lua_tostring(L, -1));
        }
        lua_pop(L, 1);
        lua_pushstring(L, "all");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            show_all = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);

        lua_pushstring(L, "mktexpk");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            kpathsea_maketex_option(kpse, "pk", lua_toboolean(L, -1));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "mktextex");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            kpathsea_maketex_option(kpse, "tex", lua_toboolean(L, -1));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "mktexmf");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            kpathsea_maketex_option(kpse, "mf", lua_toboolean(L, -1));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "mktextfm");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            kpathsea_maketex_option(kpse, "tfm", lua_toboolean(L, -1));
        }
        lua_pop(L, 1);


        lua_pushstring(L, "mustexist");
        lua_gettable(L, idx + 1);
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            must_exist = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        lua_pushstring(L, "subdir");
        lua_gettable(L, idx + 1);
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {      /* numeric value */
                if (lua_type(L, -1) == LUA_TSTRING) {
                    char *s = xstrdup(lua_tostring(L, -1));
                    str_list_add(&subdir_paths, s);
                    xfree(s);
                }
                lua_pop(L, 1);
            }
        } else if (lua_type(L, -1) == LUA_TSTRING) {
            char *s = xstrdup(lua_tostring(L, -1));
            str_list_add(&subdir_paths, s);
            xfree(s);
        }
        lua_pop(L, 1);
        if (STR_LIST_LENGTH(subdir_paths) > 0) {
            show_all = 1;
        }
    }
    if (user_path) {
        /* Translate ; to : if that's our ENV_SEP.  See cnf.c.  */
        if (IS_ENV_SEP(':')) {
            string loc;
            for (loc = user_path; *loc; loc++) {
                if (*loc == ';')
                    *loc = ':';
            }
        }
        user_path = kpathsea_path_expand(kpse, user_path);
        if (show_all) {
            ret_list = kpathsea_all_path_search(kpse, user_path, name);
        } else {
            ret = kpathsea_path_search(kpse, user_path, name, must_exist);
        }
        free(user_path);
    } else {
        /* No user-specified search path, check user format or guess from NAME.  */
        kpse_file_format_type fmt;
        if (user_format != kpse_last_format)
            fmt = user_format;
        else
            fmt = find_format(kpse, name, true);

        switch (fmt) {
        case kpse_pk_format:
        case kpse_gf_format:
        case kpse_any_glyph_format:
            {
                kpse_glyph_file_type glyph_ret;
                string temp = remove_suffix (name);
                /* Try to extract the resolution from the name.  */
                unsigned local_dpi = find_dpi(name);
                if (!local_dpi)
                    local_dpi = (unsigned) dpi;
                ret =
                    kpathsea_find_glyph(kpse, temp, local_dpi,
                                        fmt, &glyph_ret);
                if (temp != name)
                    free (temp);
            }
            break;

        case kpse_last_format:
            /* If the suffix isn't recognized, assume it's a tex file. */
            fmt = kpse_tex_format;
            /* fall through */

        default:
            if (show_all) {
                ret_list =
                    kpathsea_find_file_generic(kpse, name, fmt, must_exist,
                                               true);
            } else {
                ret = kpathsea_find_file(kpse, name, fmt, must_exist);
            }
        }
    }

    /* Turn single return into a null-terminated list for uniform treatment.  */
    if (ret) {
        ret_list = XTALLOC(2, string);
        ret_list[0] = ret;
        ret_list[1] = NULL;
    }

    /* Filter by subdirectories, if specified.  */
    if (STR_LIST_LENGTH(subdir_paths) > 0) {
        string *new_list = subdir_match(subdir_paths, ret_list);
        free(ret_list);
        ret_list = new_list;
    }
    kpse->debug = saved_debug;
    kpse->format_info[kpse_pk_format].program_enabled_p = saved_mktexpk;
    kpse->format_info[kpse_mf_format].program_enabled_p = saved_mktexmf;
    kpse->format_info[kpse_tex_format].program_enabled_p = saved_mktextex;
    kpse->format_info[kpse_tfm_format].program_enabled_p = saved_mktextfm;

    /* Print output.  */
    i = 0;
    if (ret_list) {
        for (; ret_list[i]; i++) {
            lua_pushstring(L, ret_list[i]);
        }
        free(ret_list);
    }
    if (i == 0) {
        i++;
        lua_pushnil(L);
    }
    return i;
}


static int lua_kpathsea_lookup(lua_State * L)
{
    kpathsea *kpsep = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    kpathsea kpse = *kpsep;
    return do_lua_kpathsea_lookup(L, kpse, 2);
}

static int lua_kpse_lookup(lua_State * L)
{
    TEST_PROGRAM_NAME_SET;
    return do_lua_kpathsea_lookup(L, kpse_def, 1);
}


/* Engine support is a bit of a problem, because we do not want
 * to interfere with the normal format discovery of |luatex|.
 * Current approach: run |os.setenv()| if you have to.
 */

static int set_program_name(lua_State * L)
{
    const char *exe_name = luaL_checkstring(L, 1);
    const char *prog_name = luaL_optstring(L, 2, exe_name);
    if (!program_name_set) {
        kpse_set_program_name(exe_name, prog_name);
        program_name_set = 1;
    } else {
        kpse_reset_program_name(prog_name);
    }
    /* fix up the texconfig entry */
    lua_checkstack(L, 3);
    lua_getglobal(L, "texconfig");
    if (lua_istable(L, -1)) {
        lua_pushstring(L, "kpse_init");
        lua_pushboolean(L, 0);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
    return 0;
}

static int init_prog(lua_State * L)
{
    const char *prefix = luaL_checkstring(L, 1);
    unsigned dpi = (unsigned) luaL_checkinteger(L, 2);
    const char *mode = luaL_checkstring(L, 3);
    const char *fallback = luaL_optstring(L, 4, NULL);
    TEST_PROGRAM_NAME_SET;
    kpse_init_prog(prefix, dpi, mode, fallback);
    return 0;
}

static int lua_kpathsea_init_prog(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    const char *prefix = luaL_checkstring(L, 2);
    unsigned dpi = (unsigned) luaL_checkinteger(L, 3);
    const char *mode = luaL_checkstring(L, 4);
    const char *fallback = luaL_optstring(L, 5, NULL);
    kpathsea_init_prog(*kp, prefix, dpi, mode, fallback);
    return 0;
}

static int lua_kpse_version(lua_State * L)
{
    lua_pushstring(L, kpathsea_version_string);
    return 1;
}

static int readable_file(lua_State * L)
{
    char *name = xstrdup(luaL_checkstring(L, 1));
    TEST_PROGRAM_NAME_SET;
    lua_pushstring(L, kpse_readable_file(name));
    free(name);
    return 1;
}

static int lua_kpathsea_readable_file(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    char *name = xstrdup(luaL_checkstring(L, 2));
    lua_pushstring(L, kpathsea_readable_file(*kp, name));
    free(name);
    return 1;
}


static int lua_kpathsea_finish(lua_State * L)
{
    kpathsea *kp = (kpathsea *) luaL_checkudata(L, 1, KPATHSEA_METATABLE);
    kpathsea_finish(*kp);
    return 0;
}

static int lua_kpathsea_new(lua_State * L)
{
    kpathsea kpse = NULL;
    kpathsea *kp = NULL;
    const char *av = luaL_checkstring(L, 1);
    const char *liar = luaL_optstring(L, 2, av);
    kpse = kpathsea_new();
    kpathsea_set_program_name(kpse, av, liar);
    kp = (kpathsea *) lua_newuserdata(L, sizeof(kpathsea *));
    *kp = kpse;
    luaL_getmetatable(L, KPATHSEA_METATABLE);
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_record_input_file(lua_State * L)
{
    const char *name = lua_tostring(L, 1);
    if (name != NULL) {
        recorder_record_input(name);
    }
    return 0;
}

static int lua_record_output_file(lua_State * L)
{
    const char *name = lua_tostring(L, 1);
    if (name != NULL) {
        recorder_record_output(name);
    }
    return 0;
}

static const struct luaL_Reg kpselib_m[] = {
    {"__gc", lua_kpathsea_finish},
    {"init_prog", lua_kpathsea_init_prog},
    {"readable_file", lua_kpathsea_readable_file},
    {"find_file", lua_kpathsea_find_file},
    {"expand_path", lua_kpathsea_expand_path},
    {"expand_var", lua_kpathsea_expand_var},
    {"expand_braces", lua_kpathsea_expand_braces},
    {"var_value", lua_kpathsea_var_value},
    {"show_path", lua_kpathsea_show_path},
    {"lookup", lua_kpathsea_lookup},
    {"version", lua_kpse_version},
    {"default_texmfcnf", show_texmfcnf},
    {"record_input_file", lua_record_input_file},
    {"record_output_file", lua_record_output_file},
    {NULL, NULL}                /* sentinel */
};

static const struct luaL_Reg kpselib_l[] = {
    {"new", lua_kpathsea_new},
    {"set_program_name", set_program_name},
    {"init_prog", init_prog},
    {"readable_file", readable_file},
    {"find_file", find_file},
    {"expand_path", expand_path},
    {"expand_var", expand_var},
    {"expand_braces", expand_braces},
    {"var_value", var_value},
    {"show_path", show_path},
    {"lookup", lua_kpse_lookup},
    {"version", lua_kpse_version},
    {"default_texmfcnf", show_texmfcnf},
    {NULL, NULL}                /* sentinel */
};

int luaopen_kpse(lua_State * L)
{
    luaL_newmetatable(L, KPATHSEA_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_openlib(L, NULL, kpselib_m, 0);
    luaL_openlib(L, "kpse", kpselib_l, 0);
    return 1;
}
