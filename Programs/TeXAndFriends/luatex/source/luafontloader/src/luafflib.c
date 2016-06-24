/* luafflib.c

   Copyright 2007-2010 Taco Hoekwater <taco@luatex.org>

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

/**
*  @desc Support interface for fontforge 20070607
*  @version  1.0
*  @author Taco Hoekwater
*/

#include "pfaedit.h"
#include "ustring.h"
#include "lib/lib.h"            /* web2c's lib, for recorder_record_input */

#include "ffdummies.h"
#include "splinefont.h"

#if defined(MIKTEX)
#  include "w2c/config.h"
#endif

#define FONT_METATABLE "fontloader.splinefont"
#define FONT_SUBFONT_METATABLE "fontloader.splinefont.subfont"
#define FONT_GLYPHS_METATABLE "fontloader.splinefont.glyphs"
#define FONT_GLYPH_METATABLE "fontloader.splinefont.glyph"

#define LUA_OTF_VERSION "0.5"

extern void normal_error(const char *t, const char *p);
extern void formatted_error(const char *t, const char *fmt, ...);

static const char *possub_type_enum[] = {
    "null", "position", "pair", "substitution",
    "alternate", "multiple", "ligature", "lcaret",
    "kerning", "vkerning", "anchors", "contextpos",
    "contextsub", "chainpos", "chainsub", "reversesub",
    "max", "kernback", "vkernback", NULL
};

#define LAST_POSSUB_TYPE_ENUM 18

#define eight_nulls() NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL

static const char *otf_lookup_type_enum[] = {
    "gsub_start", "gsub_single", "gsub_multiple", "gsub_alternate",
    "gsub_ligature", "gsub_context", "gsub_contextchain", NULL,
    "gsub_reversecontextchain", NULL, NULL, NULL, NULL, NULL, NULL, NULL,       /*0x00F */
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    "gpos_start", "gpos_single", "gpos_pair", "gpos_cursive",
    "gpos_mark2base", "gpos_mark2ligature", "gpos_mark2mark", "gpos_context",
    "gpos_contextchain", NULL, NULL, NULL, NULL, NULL, NULL, NULL,      /* 0x10F */
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
    eight_nulls(), eight_nulls(),
};

static const char *anchor_type_enum[] = {
    "mark", "basechar", "baselig", "basemark", "centry", "cexit", "max", NULL
};

#define MAX_ANCHOR_TYPE 7

static const char *anchorclass_type_enum[] = {
    "mark", "mkmk", "curs", "mklg", NULL
};

static const char *glyph_class_enum[] = {
    "automatic", "none", "base", "ligature", "mark", "component", NULL
};

static const char *ttfnames_enum[ttf_namemax] = {
    "copyright", "family", "subfamily", "uniqueid",
    "fullname", "version", "postscriptname", "trademark",
    "manufacturer", "designer", "descriptor", "venderurl",
    "designerurl", "license", "licenseurl", "idontknow",
    "preffamilyname", "prefmodifiers", "compatfull", "sampletext",
    "cidfindfontname", "wwsfamily", "wwssubfamily"
};

static const char *fpossub_format_enum[] = {
    "glyphs", "class", "coverage", "reversecoverage", NULL
};

static const char *tex_type_enum[4] = { "unset", "text", "math", "mathext" };

/* has an offset of 1, ui_none = 0. */
static const char *uni_interp_enum[9] = {
    "unset", "none", "adobe", "greek", "japanese",
    "trad_chinese", "simp_chinese", "korean", "ams"
};

#define check_isfont(L,b) (SplineFont **)luaL_checkudata(L,b,FONT_METATABLE)
#define check_isglyph(L,b) (struct splinechar **)luaL_checkudata(L,b,FONT_GLYPH_METATABLE)

/* forward declarations */
static void handle_generic_pst(lua_State * L, struct generic_pst *pst);
static void handle_generic_fpst(lua_State * L, struct generic_fpst *fpst);
static void handle_kernclass(lua_State * L, struct kernclass *kerns, const char *name);
static void handle_splinefont(lua_State * L, struct splinefont *sf);
static void handle_kernpair(lua_State * L, struct kernpair *kp);
static void handle_liglist(lua_State * L, struct liglist *ligofme);
static void handle_anchorpoint(lua_State * L, struct anchorpoint *anchor);

static int is_userdata(lua_State *L, int b, const char *utype)
{
    if (lua_type(L,b) == LUA_TUSERDATA) {
        lua_getmetatable(L, b);
        luaL_getmetatable(L, utype);
        if (lua_compare(L, -2, -1, LUA_OPEQ)) {
            lua_pop(L,2);
            return 1;
        }
        lua_pop(L,2);
    }
    return 0;
}

static void lua_ff_pushfont(lua_State * L, SplineFont * sf)
{
    SplineFont **a;
    if (sf == NULL) {
        lua_pushnil(L);
    } else {
        a = lua_newuserdata(L, sizeof(SplineFont *));
        *a = sf;
        luaL_getmetatable(L, FONT_METATABLE);
        lua_setmetatable(L, -2);
    }
    return;
}

static void lua_ff_pushsubfont(lua_State * L, SplineFont * sf)
{
    SplineFont **a;
    if (sf == NULL) {
        lua_pushnil(L);
    } else {
        a = lua_newuserdata(L, sizeof(SplineFont *));
        *a = sf;
        luaL_getmetatable(L, FONT_SUBFONT_METATABLE);
        lua_setmetatable(L, -2);
    }
    return;
}

static void lua_ff_pushglyph(lua_State * L, struct splinechar *sc)
{
    struct splinechar **a;
    if (sc == NULL) {
        lua_pushnil(L);
    } else {
        a = lua_newuserdata(L, sizeof(struct splinechar *));
        *a = sc;
        luaL_getmetatable(L, FONT_GLYPH_METATABLE);
        lua_setmetatable(L, -2);
    }
    return;
}


static int ff_open(lua_State * L)
{
    SplineFont *sf;
    const char *fontname;
    FILE *l;
    char s[511];
    size_t len;
    int args, i;
    int openflags = 1 + 4;
    fontname = luaL_checkstring(L, 1);
    /* test fontname for existance */
    if ((l = fopen(fontname, "r"))) {
        recorder_record_input(fontname);
        fclose(l);
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "font loading failed for %s (read error)\n", fontname);
        return 2;
    }
    args = lua_gettop(L);
    if (args >= 2 && (lua_type(L, 2) == LUA_TSTRING)) {
        len = strlen(fontname);
        if (*(fontname + len) != ')') {
            /* possibly fails for embedded parens in the font name */
            if (len == 0) {
                snprintf(s, 511, "%s", fontname);
            } else {
                snprintf(s, 511, "%s(%s)", fontname, lua_tolstring(L, 2, &len));
            }
        }
    } else {
        snprintf(s, 511, "%s", fontname);
    }
    if (strlen(s) > 0) {
        gww_error_count = 0;
        sf = ReadSplineFont((char *) s, openflags);
        if (sf == NULL) {
            lua_pushnil(L);
            lua_pushfstring(L, "font loading failed for %s\n", s);
            if (gww_error_count > 0) {
                for (i = 0; i < gww_error_count; i++) {
                    lua_pushstring(L, gww_errors[i]);
                    lua_concat(L, 2);
                }
                gwwv_errors_free();
            }
        } else {
            FVAppend(_FontViewCreate(sf));
            lua_ff_pushfont(L, sf);
            if (gww_error_count > 0) {
                lua_newtable(L);
                for (i = 0; i < gww_error_count; i++) {
                    lua_pushstring(L, gww_errors[i]);
                    lua_rawseti(L, -2, (i + 1));
                }
                gwwv_errors_free();
            } else {
                lua_pushnil(L);
            }
        }
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "font loading failed: empty string given\n", fontname);
    }
    return 2;
}

static int ff_close(lua_State * L)
{
    SplineFont **sf;
    /*fputs("ff_close called",stderr); */
    sf = check_isfont(L, 1);
    if (*sf != NULL) {
      if ((*sf)->fv) {        /* condition might be improved */
            FontViewClose((*sf)->fv);
        } else {
            EncMapFree((*sf)->map);
            SplineFontFree(*sf);
        }
        *sf = NULL;
    }
    return 0;
}

/*
static int notdef_loc(SplineFont * sf)
{
    int k;
    int l = -1;
    for (k = 0; k < sf->glyphcnt; k++) {
        if (sf->glyphs[k]) {
            if (strcmp(sf->glyphs[k]->name, ".notdef") == 0) {
                l = k;
            }
        }
    }
    if (l == -1)
        l = sf->glyphcnt;
    return l;
}
*/

static int notdef_loc(SplineFont * sf)
{
    int k;
    for (k = 0; k < sf->glyphcnt; k++) {
        if (sf->glyphs[k]) {
            if (strcmp(sf->glyphs[k]->name, ".notdef") == 0) {
                return k ;
            }
        }
    }
    return -1;
}

static int ff_apply_featurefile(lua_State * L)
{
    SplineFont **sf;
    char *fname;
    sf = check_isfont(L, 1);
    fname = xstrdup(luaL_checkstring(L, 2));
    SFApplyFeatureFilename(*sf, fname);
    recorder_record_input(fname);
    free(fname);
    if (gww_error_count > 0) {
        int i;
        lua_newtable(L);
        for (i = 0; i < gww_error_count; i++) {
            lua_pushstring(L, gww_errors[i]);
            lua_rawseti(L, -2, (i + 1));
        }
        gwwv_errors_free();
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int ff_apply_afmfile(lua_State * L)
{
    SplineFont **sf;
    const char *fname;
    sf = check_isfont(L, 1);
    fname = luaL_checkstring(L, 2);
    CheckAfmOfPostscript(*sf, fname, (*sf)->map);
    if (gww_error_count > 0) {
        int i;
        lua_newtable(L);
        for (i = 0; i < gww_error_count; i++) {
            lua_pushstring(L, gww_errors[i]);
            lua_rawseti(L, -2, (i + 1));
        }
        gwwv_errors_free();
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static void dump_intfield(lua_State * L, const char *name, long int field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushnumber(L, field);
    lua_rawset(L, -3);
}

static void dump_uintfield(lua_State * L, const char *name, unsigned int field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushnumber(L, field);
    lua_rawset(L, -3);
}

static void dump_realfield(lua_State * L, const char *name, real field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushnumber(L, field);
    lua_rawset(L, -3);
}

#define dump_cond_intfield(a,b,c) if ((c)!=0) { dump_intfield ((a),(b),(c)); }

static void dump_stringfield(lua_State * L, const char *name, const char *field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushstring(L, field);
    lua_rawset(L, -3);
}

static void dump_char_ref(lua_State * L, struct splinechar *spchar)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, "char");
    lua_pushstring(L, spchar->name);
    lua_rawset(L, -3);
}

static void dump_lstringfield(lua_State * L, const char *name, char *field, int len)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushlstring(L, field, len);
    lua_rawset(L, -3);
}

static void dump_enumfield(lua_State * L, const char *name, int fid, const char **fields)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushstring(L, fields[fid]);
    lua_rawset(L, -3);
}

static void dump_floatfield(lua_State * L, const char *name, double field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushnumber(L, field);
    lua_rawset(L, -3);
}

static char tag_string[5] = { 0 };

static char *make_tag_string(unsigned int field)
{
    tag_string[0] = (field & 0xFF000000) >> 24;
    tag_string[1] = (field & 0x00FF0000) >> 16;
    tag_string[2] = (field & 0x0000FF00) >> 8;
    tag_string[3] = (field & 0x000000FF);
    return (char *) tag_string;
}

static void dump_tag(lua_State * L, const char *name, unsigned int field)
{
    lua_checkstack(L, 2);
    lua_pushstring(L, name);
    lua_pushlstring(L, make_tag_string(field), 4);
    lua_rawset(L, -3);
}

static void dump_subtable_name(lua_State * L, const char *name, struct lookup_subtable *s)
{
    /* this is likely a backref */
    if (s == NULL)
        return;
    lua_checkstack(L, 2);
    if (s->next == NULL) {
        dump_stringfield(L, name, s->subtable_name);
    } else {
        /* can this really happen ? */
        int i = 0;
        lua_newtable(L);
        while (s != NULL) {
            lua_pushstring(L, s->subtable_name);
            lua_rawseti(L, -2, ++i);
            s = s->next;
        }
        lua_setfield(L, -2, name);
    }
}

#define NESTED_TABLE(a,b,c) { \
    int k = 1;\
    next = b; \
    while (next != NULL) { \
        lua_checkstack(L,2); \
        lua_pushnumber(L,k); k++; \
        lua_createtable(L,0,c); \
        a(L, next); \
        lua_rawset(L,-3); \
        next = next->next; \
    } \
}

#define NESTED_TABLE_SF(a,b,c,d) { \
    int k = 1; \
    next = b; \
    while (next != NULL) { \
        lua_checkstack(L,2); \
        lua_pushnumber(L,k); k++; \
        lua_createtable(L,0,d); \
        if (a(L, next, c)) \
            lua_rawset(L,-3); \
        else { \
            lua_pop(L,2); \
        } \
        next = next->next; \
    } \
}

static void do_handle_scriptlanglist(lua_State * L, struct scriptlanglist *sl)
{
    int k;
    dump_tag(L, "script", sl->script);

    lua_checkstack(L, 3);
    lua_newtable(L);
    for (k = 0; k < MAX_LANG; k++) {
        if (sl->langs[k] != 0) {
            lua_pushnumber(L, (k + 1));
            lua_pushstring(L, make_tag_string(sl->langs[k]));
            lua_rawset(L, -3);
        }
    }

    if (sl->lang_cnt >= MAX_LANG) {
        for (k = MAX_LANG; k < sl->lang_cnt; k++) {
            lua_pushnumber(L, (k + 1));
            lua_pushstring(L, make_tag_string(sl->morelangs[k - MAX_LANG]));
            lua_rawset(L, -3);
        }
    }
    lua_setfield(L, -2, "langs");
}

static void handle_scriptlanglist(lua_State * L, struct scriptlanglist *sll)
{
    struct scriptlanglist *next;
    NESTED_TABLE(do_handle_scriptlanglist, sll, 4);
}

static void
do_handle_featurescriptlanglist(lua_State * L,
                                struct featurescriptlanglist *features)
{
    dump_tag(L, "tag", features->featuretag);
    lua_newtable(L);
    handle_scriptlanglist(L, features->scripts);
    lua_setfield(L, -2, "scripts");
}

static void
handle_featurescriptlanglist(lua_State * L,
                             struct featurescriptlanglist *features)
{
    struct featurescriptlanglist *next;
    NESTED_TABLE(do_handle_featurescriptlanglist, features, 3);
}

static void do_handle_lookup_subtable(lua_State * L, struct lookup_subtable *subtable)
{

    dump_stringfield(L, "name", subtable->subtable_name);
    dump_stringfield(L, "suffix", subtable->suffix);

    /* struct otlookup *lookup; *//* this is the parent */

    /* dump_intfield   (L,"unused",               subtable->unused); */
    /* The next one is true if there is no fpst, false otherwise */
    /*
       dump_intfield      (L,"per_glyph_pst_or_kern",subtable->per_glyph_pst_or_kern);
     */
    dump_cond_intfield(L, "anchor_classes", subtable->anchor_classes);
    dump_cond_intfield(L, "vertical_kerning", subtable->vertical_kerning);

    if (subtable->kc != NULL) {
        lua_newtable(L);
        handle_kernclass(L, subtable->kc, subtable->subtable_name);
        lua_setfield(L, -2, "kernclass");
    }
#if 0
    if (subtable->fpst != NULL) {
        /* lua_newtable(L); */
        handle_generic_fpst(L, subtable->fpst);
        /* lua_setfield(L,-2, "fpst"); */
    }
#endif

    /* int subtable_offset; *//* used by OTF file generation */
    /* int32 *extra_subtables; *//* used by OTF file generation */
}

static void handle_lookup_subtable(lua_State * L, struct lookup_subtable *subtable)
{
    struct lookup_subtable *next;
    NESTED_TABLE(do_handle_lookup_subtable, subtable, 2);
}

static int do_handle_lookup(lua_State * L, struct otlookup *lookup, SplineFont * sf)
{
    int mc;

    dump_enumfield(L, "type", lookup->lookup_type, otf_lookup_type_enum);

    lua_newtable(L);
    if (lookup->lookup_flags & pst_r2l) {
        lua_pushstring(L, "r2l");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }
    if (lookup->lookup_flags & pst_ignorebaseglyphs) {
        lua_pushstring(L, "ignorebaseglyphs");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }
    if (lookup->lookup_flags & pst_ignoreligatures) {
        lua_pushstring(L, "ignoreligatures");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }
    if (lookup->lookup_flags & pst_ignorecombiningmarks) {
        lua_pushstring(L, "ignorecombiningmarks");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }
    mc = (lookup->lookup_flags >> 8);
    if (mc > 0 && mc < sf->mark_class_cnt && sf->mark_class_names[mc] != NULL) {
        lua_pushstring(L, "mark_class");
        lua_pushstring(L, sf->mark_class_names[mc]);
        lua_rawset(L, -3);
    }
    lua_setfield(L, -2, "flags");

    dump_stringfield(L, "name", lookup->lookup_name);

    if (lookup->features != NULL) {
        lua_newtable(L);
        handle_featurescriptlanglist(L, lookup->features);
        lua_setfield(L, -2, "features");
    }

    if (lookup->subtables != NULL) {
        lua_newtable(L);
        handle_lookup_subtable(L, lookup->subtables);
        lua_setfield(L, -2, "subtables");
    }

    /* dump_intfield   (L,"unused",           lookup->unused);  */
    /* dump_intfield   (L,"empty",            lookup->empty); */
    /* dump_intfield   (L,"store_in_afm",     lookup->store_in_afm); */
    /* dump_intfield   (L,"needs_extension",  lookup->needs_extension); */
    /* dump_intfield   (L,"temporary_kern",   lookup->temporary_kern); */
    /* dump_intfield   (L,"def_lang_checked", lookup->def_lang_checked); */
    /* dump_intfield   (L,"def_lang_found",   lookup->def_lang_found); */
    /* dump_intfield   (L,"ticked",           lookup->ticked); */
    /* dump_intfield   (L,"subcnt",           lookup->subcnt); */
    /* dump_intfield   (L,"lookup_index",     lookup->lookup_index); *//* identical to array index */
    /* dump_intfield   (L,"lookup_offset",    lookup->lookup_offset); */
    /* dump_intfield   (L,"lookup_length",    lookup->lookup_length); */
    /* dump_stringfield(L,"tempname",         lookup->tempname); */
    return 1;
}

static void handle_lookup(lua_State * L, struct otlookup *lookup, SplineFont * sf)
{
    struct otlookup *next;
    NESTED_TABLE_SF(do_handle_lookup, lookup, sf, 18);  /* 18 is a guess */
}

static void do_handle_kernpair(lua_State * L, struct kernpair *kp)
{

    if (kp->sc != NULL)
        dump_char_ref(L, kp->sc);
    dump_intfield(L, "off", kp->off);
    /*  uint16 kcid;   *//* temporary value */
    dump_subtable_name(L, "lookup", kp->subtable);
}

static void handle_kernpair(lua_State * L, struct kernpair *kp)
{
    struct kernpair *next;
    NESTED_TABLE(do_handle_kernpair, kp, 4);
}

static void handle_splinecharlist(lua_State * L, struct splinecharlist *scl)
{

    struct splinecharlist *next = scl;
    int k = 1;
    lua_checkstack(L, 10);
    while (next != NULL) {
        if (next->sc != NULL) {
            lua_pushnumber(L, k);
            k++;
            lua_pushstring(L, next->sc->name);
            lua_rawset(L, -3);
        }
        next = next->next;
    }
}

/* vs is the "variation selector" a unicode codepoint which modifieds */
/*  the code point before it. If vs is -1 then unienc is just an */
/*  alternate encoding (greek Alpha and latin A), but if vs is one */
/*  of unicode's variation selectors then this glyph is somehow a */
/*  variant shape. The specifics depend on the selector and script */
/*  fid is currently unused, but may, someday, be used to do ttcs */
/* NOTE: GlyphInfo displays vs==-1 as vs==0, and fixes things up */

static int handle_altuni(lua_State * L, struct altuni *au)
{
    struct altuni *next = au;
    int i = 0;
    int k = 1;
    lua_checkstack(L, 3);
    while (next != NULL) {
        if (next->unienc<0x10FFFF) {
            lua_newtable(L);
            dump_intfield(L, "unicode", next->unienc);
            i++;
            if (next->vs != -1) {
                dump_intfield(L, "variant", next->vs);
                /* dump_intfield(L, "fid", next->fid); */
            }
            lua_rawseti(L, -2, k++);
        } else {
            printf("ignoring variant %i %i\n",next->unienc,next->vs);
        }
        next = next->next;
    }
    return i;
}

#define interesting_vr(a) (((a)->xoff!=0) || ((a)->yoff!=0) || ((a)->h_adv_off!=0) || ((a)->v_adv_off!=0))

static void handle_vr(lua_State * L, struct vr *pos)
{

    dump_cond_intfield(L, "x", pos->xoff);
    dump_cond_intfield(L, "y", pos->yoff);
    dump_cond_intfield(L, "h", pos->h_adv_off);
    dump_cond_intfield(L, "v", pos->v_adv_off);

}

static void do_handle_generic_pst(lua_State * L, struct generic_pst *pst)
{
    int k;
    if (pst->type > LAST_POSSUB_TYPE_ENUM) {
        dump_tag(L, "type", pst->type);
    } else {
        dump_enumfield(L, "type", pst->type, possub_type_enum);
    }
    /*  unsigned int ticked: 1; */
    /*  unsigned int temporary: 1; *//* Used in afm ligature closure */
    /*  struct lookup_subtable *subtable; *//* handled by caller */

    lua_checkstack(L, 4);
    lua_pushstring(L, "specification");
    lua_createtable(L, 0, 4);
    if (pst->type == pst_position) {
        handle_vr(L, &pst->u.pos);
    } else if (pst->type == pst_pair) {
        dump_stringfield(L, "paired", pst->u.pair.paired);
        if (pst->u.pair.vr != NULL) {
            lua_pushstring(L, "offsets");
            lua_createtable(L, 2, 0);
            if (interesting_vr(pst->u.pair.vr)) {
                lua_createtable(L, 0, 4);
                handle_vr(L, pst->u.pair.vr);
                lua_rawseti(L, -2, 1);
            }
            if (interesting_vr(pst->u.pair.vr + 1)) {
                lua_createtable(L, 0, 4);
                handle_vr(L, pst->u.pair.vr + 1);
                lua_rawseti(L, -2, 2);
            }
            lua_rawset(L, -3);
        }
    } else if (pst->type == pst_substitution) {
        dump_stringfield(L, "variant", pst->u.subs.variant);
    } else if (pst->type == pst_alternate) {
        dump_stringfield(L, "components", pst->u.mult.components);
    } else if (pst->type == pst_multiple) {
        dump_stringfield(L, "components", pst->u.alt.components);
    } else if (pst->type == pst_ligature) {
        dump_stringfield(L, "components", pst->u.lig.components);
        if (pst->u.lig.lig != NULL) {
            dump_char_ref(L, pst->u.lig.lig);
        }
    } else if (pst->type == pst_lcaret) {
        for (k = 0; k < pst->u.lcaret.cnt; k++) {
            lua_pushnumber(L, (k + 1));
            lua_pushnumber(L, pst->u.lcaret.carets[k]);
            lua_rawset(L, -3);
        }
    }
    lua_rawset(L, -3);
}

static void handle_generic_pst(lua_State * L, struct generic_pst *pst)
{
    struct generic_pst *next;
    int k;
    int l = 1;
    next = pst;
    /* most likely everything arrives in proper order. But to prevent
     * surprises, better do this is the proper way
     */
    while (next != NULL) {
        if (next->subtable != NULL && next->subtable->subtable_name != NULL) {
            lua_checkstack(L, 3);       /* just in case */
            lua_getfield(L, -1, next->subtable->subtable_name);
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_setfield(L, -2, next->subtable->subtable_name);
                lua_getfield(L, -1, next->subtable->subtable_name);
            }
            k = lua_rawlen(L, -1) + 1;
            lua_pushnumber(L, k);
            lua_createtable(L, 0, 4);
            do_handle_generic_pst(L, next);
            lua_rawset(L, -3);
            next = next->next;
            lua_pop(L, 1);      /* pop the subtable */
        } else {
            /* Found a pst without subtable, or without subtable name */
            lua_pushnumber(L, l);
            l++;
            lua_createtable(L, 0, 4);
            do_handle_generic_pst(L, next);
            lua_rawset(L, -3);
            next = next->next;
        }
    }
}

static void do_handle_liglist(lua_State * L, struct liglist *ligofme)
{
    lua_checkstack(L, 2);
    if (ligofme->lig != NULL) {
        lua_createtable(L, 0, 6);
        handle_generic_pst(L, ligofme->lig);
        lua_setfield(L, -2, "lig");
    }
    dump_char_ref(L, ligofme->first);
    if (ligofme->components != NULL) {
        lua_newtable(L);
        handle_splinecharlist(L, ligofme->components);
        lua_setfield(L, -2, "components");
    }
    dump_intfield(L, "ccnt", ligofme->ccnt);
}


static void handle_liglist(lua_State * L, struct liglist *ligofme)
{
    struct liglist *next;
    NESTED_TABLE(do_handle_liglist, ligofme, 3);
}

static void do_handle_anchorpoint(lua_State * L, struct anchorpoint *anchor)
{

    if (anchor->anchor == NULL) {
        return;
    }
    if (anchor->type >= 0 && anchor->type <= MAX_ANCHOR_TYPE) {
        lua_pushstring(L, anchor_type_enum[anchor->type]);
    } else {
        lua_pushstring(L, "Anchorpoint has an unknown type!");
        lua_error(L);
    }
    /* unsigned int selected: 1; */
    /* unsigned int ticked: 1; */

    lua_rawget(L, -2);
    if (!lua_istable(L, -1)) {
        /* create the table first */
        lua_pop(L, 1);
        lua_pushstring(L, anchor_type_enum[anchor->type]);
        lua_pushvalue(L, -1);
        lua_newtable(L);
        lua_rawset(L, -4);
        lua_rawget(L, -2);
    }
    /* now the 'type' table is top of stack */
    if (anchor->type == at_baselig) {
        lua_pushstring(L, anchor->anchor->name);
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) {
            /* create the table first */
            lua_pop(L, 1);
            lua_pushstring(L, anchor->anchor->name);
            lua_pushvalue(L, -1);
            lua_newtable(L);
            lua_rawset(L, -4);
            lua_rawget(L, -2);
        }
        lua_newtable(L);
        dump_intfield(L, "x", anchor->me.x);
        dump_intfield(L, "y", anchor->me.y);
        if (anchor->has_ttf_pt)
            dump_intfield(L, "ttf_pt_index", anchor->ttf_pt_index);
        dump_intfield(L, "lig_index", anchor->lig_index);
        lua_rawseti(L, -2, (anchor->lig_index + 1));
        lua_pop(L, 1);
    } else {
        lua_pushstring(L, anchor->anchor->name);
        lua_newtable(L);
        dump_intfield(L, "x", anchor->me.x);
        dump_intfield(L, "y", anchor->me.y);
        if (anchor->has_ttf_pt)
            dump_intfield(L, "ttf_pt_index", anchor->ttf_pt_index);
        dump_intfield(L, "lig_index", anchor->lig_index);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
}

static void handle_anchorpoint(lua_State * L, struct anchorpoint *anchor)
{
    struct anchorpoint *next;
    next = anchor;
    while (next != NULL) {
        do_handle_anchorpoint(L, next);
        next = next->next;
    }
}

static void handle_glyphvariants(lua_State * L, struct glyphvariants *vars)
{
    int i;
    dump_stringfield(L, "variants", vars->variants);
    dump_intfield(L, "italic_correction", vars->italic_correction);
    lua_newtable(L);
    for (i = 0; i < vars->part_cnt; i++) {
        lua_newtable(L);
        dump_stringfield(L, "component", vars->parts[i].component);
        dump_intfield(L, "extender", vars->parts[i].is_extender);
        dump_intfield(L, "start", vars->parts[i].startConnectorLength);
        dump_intfield(L, "end", vars->parts[i].endConnectorLength);
        dump_intfield(L, "advance", vars->parts[i].fullAdvance);
        lua_rawseti(L, -2, (i + 1));
    }
    lua_setfield(L, -2, "parts");
}

static void handle_mathkernvertex(lua_State * L, struct mathkernvertex *mkv)
{
    int i;
    for (i = 0; i < mkv->cnt; i++) {
        lua_newtable(L);
        dump_intfield(L, "height", mkv->mkd[i].height);
        dump_intfield(L, "kern", mkv->mkd[i].kern);
        lua_rawseti(L, -2, (i + 1));
    }
}

static void handle_mathkern(lua_State * L, struct mathkern *mk)
{
    lua_newtable(L);
    handle_mathkernvertex(L, &(mk->top_right));
    lua_setfield(L, -2, "top_right");
    lua_newtable(L);
    handle_mathkernvertex(L, &(mk->top_left));
    lua_setfield(L, -2, "top_left");
    lua_newtable(L);
    handle_mathkernvertex(L, &(mk->bottom_right));
    lua_setfield(L, -2, "bottom_right");
    lua_newtable(L);
    handle_mathkernvertex(L, &(mk->bottom_left));
    lua_setfield(L, -2, "bottom_left");
}

static void handle_splinechar(lua_State * L, struct splinechar *glyph, int hasvmetrics)
{
    DBounds bb;
    if (glyph->xmax == 0 && glyph->ymax == 0 && glyph->xmin == 0
        && glyph->ymin == 0) {
        SplineCharFindBounds(glyph, &bb);
        glyph->xmin = bb.minx;
        glyph->ymin = bb.miny;
        glyph->xmax = bb.maxx;
        glyph->ymax = bb.maxy;
    }
    dump_stringfield(L, "name", glyph->name);
    dump_intfield(L, "unicode", glyph->unicodeenc);
    lua_createtable(L, 4, 0);
    lua_pushnumber(L, 1);
    lua_pushnumber(L, glyph->xmin);
    lua_rawset(L, -3);
    lua_pushnumber(L, 2);
    lua_pushnumber(L, glyph->ymin);
    lua_rawset(L, -3);
    lua_pushnumber(L, 3);
    lua_pushnumber(L, glyph->xmax);
    lua_rawset(L, -3);
    lua_pushnumber(L, 4);
    lua_pushnumber(L, glyph->ymax);
    lua_rawset(L, -3);
    lua_setfield(L, -2, "boundingbox");
    if (hasvmetrics) {
        dump_intfield(L, "vwidth", glyph->vwidth);
        if (glyph->tsb != 0)
           dump_intfield(L, "tsidebearing", glyph->tsb);
    }
    dump_intfield(L, "width", glyph->width);

    if (glyph->lsidebearing != glyph->xmin) {
        dump_cond_intfield(L, "lsidebearing", glyph->lsidebearing);
    }
    /* dump_intfield(L,"ttf_glyph",   glyph->ttf_glyph);  */

    /* Layer layers[2];   *//* TH Not used */
    /*  int layer_cnt;    *//* TH Not used */
    /*  StemInfo *hstem;  *//* TH Not used */
    /*  StemInfo *vstem;  *//* TH Not used */
    /*  DStemInfo *dstem; *//* TH Not used */

    /* MinimumDistance *md; *//* TH Not used */
    /* struct charviewbase *views; *//* TH Not used */
    /* struct charinfo *charinfo;  *//* TH ? (charinfo.c) */
    /* struct splinefont *parent;  *//* TH Not used */

    if (glyph->glyph_class > 0) {
        dump_enumfield(L, "class", glyph->glyph_class, glyph_class_enum);
    }
    /* TH: internal fontforge stuff
       dump_intfield(L,"changed",                  glyph->changed);
       dump_intfield(L,"changedsincelasthinted",   glyph->changedsincelasthinted);
       dump_intfield(L,"manualhints",              glyph->manualhints);
       dump_intfield(L,"ticked",                   glyph->ticked);
       dump_intfield(L,"changed_since_autosave",   glyph->changed_since_autosave);
       dump_intfield(L,"widthset",                 glyph->widthset);
       dump_intfield(L,"vconflicts",               glyph->vconflicts);
       dump_intfield(L,"hconflicts",               glyph->hconflicts);
       dump_intfield(L,"searcherdummy",            glyph->searcherdummy);
       dump_intfield(L,"changed_since_search",     glyph->changed_since_search);
       dump_intfield(L,"wasopen",                  glyph->wasopen);
       dump_intfield(L,"namechanged",              glyph->namechanged);
       dump_intfield(L,"blended",                  glyph->blended);
       dump_intfield(L,"ticked2",                  glyph->ticked2);
       dump_intfield(L,"unused_so_far",            glyph->unused_so_far);
       dump_intfield(L,"numberpointsbackards",     glyph->numberpointsbackards);
       dump_intfield(L,"instructions_out_of_date", glyph->instructions_out_of_date);
       dump_intfield(L,"complained_about_ptnums",  glyph->complained_about_ptnums);
       unsigned int vs_open: 1;
       unsigned int unlink_rm_ovrlp_save_undo: 1;
       unsigned int inspiro: 1;
       unsigned int lig_caret_cnt_fixed: 1;


       uint8 *ttf_instrs;
       int16 ttf_instrs_len;
       int16 countermask_cnt;
       HintMask *countermasks;
     */

    if (glyph->kerns != NULL) {
        lua_newtable(L);
        handle_kernpair(L, glyph->kerns);
        lua_setfield(L, -2, "kerns");
    }
    if (glyph->vkerns != NULL) {
        lua_newtable(L);
        handle_kernpair(L, glyph->vkerns);
        lua_setfield(L, -2, "vkerns");
    }

    if (glyph->dependents != NULL) {
        lua_newtable(L);
        handle_splinecharlist(L, glyph->dependents);
        lua_setfield(L, -2, "dependents");

    }
    if (glyph->possub != NULL) {
        lua_newtable(L);
        handle_generic_pst(L, glyph->possub);
        lua_setfield(L, -2, "lookups");
    }

    if (glyph->ligofme != NULL) {
        lua_newtable(L);
        handle_liglist(L, glyph->ligofme);
        lua_setfield(L, -2, "ligatures");
    }

    if (glyph->comment != NULL)
        dump_stringfield(L, "comment", glyph->comment);

    /* Color color;  *//* dont care */

    if (glyph->anchor != NULL) {
        lua_newtable(L);
        handle_anchorpoint(L, glyph->anchor);
        lua_setfield(L, -2, "anchors");
    }

    if (glyph->altuni != NULL) {
	int i;
        lua_newtable(L);
        i = handle_altuni(L, glyph->altuni);
	if (i>0) {
	    lua_setfield(L, -2, "altuni");
	} else {
	    lua_pop(L,1);
	}
    }

    if (glyph->tex_height != TEX_UNDEF)
        dump_intfield(L, "tex_height", glyph->tex_height);
    if (glyph->tex_depth != TEX_UNDEF)
        dump_intfield(L, "tex_depth", glyph->tex_depth);

    dump_cond_intfield(L, "is_extended_shape", glyph->is_extended_shape);
    if (glyph->italic_correction != TEX_UNDEF)
        dump_intfield(L, "italic_correction", glyph->italic_correction);
    if (glyph->top_accent_horiz != TEX_UNDEF)
        dump_intfield(L, "top_accent", glyph->top_accent_horiz);

    if (glyph->vert_variants != NULL) {
        lua_newtable(L);
        handle_glyphvariants(L, glyph->vert_variants);
        lua_setfield(L, -2, "vert_variants");
    }
    if (glyph->horiz_variants != NULL) {
        lua_newtable(L);
        handle_glyphvariants(L, glyph->horiz_variants);
        lua_setfield(L, -2, "horiz_variants");
    }
    if (glyph->mathkern != NULL) {
        lua_newtable(L);
        handle_mathkern(L, glyph->mathkern);
        lua_setfield(L, -2, "mathkern");
    }
}

const char *panose_values_0[] = {
    "Any", "No Fit", "Text and Display", "Script", "Decorative", "Pictorial"
};

const char *panose_values_1[] = {
    "Any", "No Fit", "Cove", "Obtuse Cove", "Square Cove",
    "Obtuse Square Cove",
    "Square", "Thin", "Bone", "Exaggerated", "Triangle", "Normal Sans",
    "Obtuse Sans", "Perp Sans", "Flared", "Rounded"
};

const char *panose_values_2[] = {
    "Any", "No Fit", "Very Light", "Light", "Thin", "Book",
    "Medium", "Demi", "Bold", "Heavy", "Black", "Nord"
};

const char *panose_values_3[] = {
    "Any", "No Fit", "Old Style", "Modern", "Even Width",
    "Expanded", "Condensed", "Very Expanded", "Very Condensed", "Monospaced"
};

const char *panose_values_4[] = {
    "Any", "No Fit", "None", "Very Low", "Low", "Medium Low",
    "Medium", "Medium High", "High", "Very High"
};

const char *panose_values_5[] = {
    "Any", "No Fit", "Gradual/Diagonal", "Gradual/Transitional",
    "Gradual/Vertical",
    "Gradual/Horizontal", "Rapid/Vertical", "Rapid/Horizontal",
    "Instant/Vertical"
};

const char *panose_values_6[] = {
    "Any", "No Fit", "Straight Arms/Horizontal", "Straight Arms/Wedge",
    "Straight Arms/Vertical",
    "Straight Arms/Single Serif", "Straight Arms/Double Serif",
    "Non-Straight Arms/Horizontal",
    "Non-Straight Arms/Wedge", "Non-Straight Arms/Vertical",
    "Non-Straight Arms/Single Serif",
    "Non-Straight Arms/Double Serif"
};

const char *panose_values_7[] = {
    "Any", "No Fit", "Normal/Contact", "Normal/Weighted", "Normal/Boxed",
    "Normal/Flattened",
    "Normal/Rounded", "Normal/Off Center", "Normal/Square", "Oblique/Contact",
    "Oblique/Weighted",
    "Oblique/Boxed", "Oblique/Flattened", "Oblique/Rounded",
    "Oblique/Off Center", "Oblique/Square"
};

const char *panose_values_8[] = {
    "Any", "No Fit", "Standard/Trimmed", "Standard/Pointed",
    "Standard/Serifed", "High/Trimmed",
    "High/Pointed", "High/Serifed", "Constant/Trimmed", "Constant/Pointed",
    "Constant/Serifed",
    "Low/Trimmed", "Low/Pointed", "Low/Serifed"
};

const char *panose_values_9[] = {
    "Any", "No Fit", "Constant/Small", "Constant/Standard",
    "Constant/Large", "Ducking/Small", "Ducking/Standard", "Ducking/Large"
};

#define panose_values_0_max 5
#define panose_values_1_max 15
#define panose_values_2_max 11
#define panose_values_3_max 9
#define panose_values_4_max 9
#define panose_values_5_max 8
#define panose_values_6_max 11
#define panose_values_7_max 15
#define panose_values_8_max 13
#define panose_values_9_max 7

#define fix_range(a,b) (b<0 ? 0 : (b>a ? 0 : b))

static void handle_pfminfo(lua_State * L, struct pfminfo pfm)
{

    dump_intfield(L, "pfmset", pfm.pfmset);
    dump_intfield(L, "winascent_add", pfm.winascent_add);
    dump_intfield(L, "windescent_add", pfm.windescent_add);
    dump_intfield(L, "hheadascent_add", pfm.hheadascent_add);
    dump_intfield(L, "hheaddescent_add", pfm.hheaddescent_add);
    dump_intfield(L, "typoascent_add", pfm.typoascent_add);
    dump_intfield(L, "typodescent_add", pfm.typodescent_add);
    dump_intfield(L, "subsuper_set", pfm.subsuper_set);
    dump_intfield(L, "panose_set", pfm.panose_set);
    dump_intfield(L, "hheadset", pfm.hheadset);
    dump_intfield(L, "vheadset", pfm.vheadset);
    dump_intfield(L, "pfmfamily", pfm.pfmfamily);
    dump_intfield(L, "weight", pfm.weight);
    dump_intfield(L, "width", pfm.width);
    dump_intfield(L, "avgwidth", pfm.avgwidth);
    dump_intfield(L, "firstchar", pfm.firstchar);
    dump_intfield(L, "lastchar", pfm.lastchar);
    lua_createtable(L, 0, 10);

    dump_enumfield(L, "familytype",      fix_range(panose_values_0_max, pfm.panose[0]), panose_values_0);
    dump_enumfield(L, "serifstyle",      fix_range(panose_values_1_max, pfm.panose[1]), panose_values_1);
    dump_enumfield(L, "weight",          fix_range(panose_values_2_max, pfm.panose[2]), panose_values_2);
    dump_enumfield(L, "proportion",      fix_range(panose_values_3_max, pfm.panose[3]), panose_values_3);
    dump_enumfield(L, "contrast",        fix_range(panose_values_4_max, pfm.panose[4]), panose_values_4);
    dump_enumfield(L, "strokevariation", fix_range(panose_values_5_max, pfm.panose[5]), panose_values_5);
    dump_enumfield(L, "armstyle",        fix_range(panose_values_6_max, pfm.panose[6]), panose_values_6);
    dump_enumfield(L, "letterform",      fix_range(panose_values_7_max, pfm.panose[7]), panose_values_7);
    dump_enumfield(L, "midline",         fix_range(panose_values_8_max, pfm.panose[8]), panose_values_8);
    dump_enumfield(L, "xheight",         fix_range(panose_values_9_max, pfm.panose[9]), panose_values_9);
    lua_setfield(L, -2, "panose");

    dump_intfield(L, "fstype", pfm.fstype);
    dump_intfield(L, "linegap", pfm.linegap);
    dump_intfield(L, "vlinegap", pfm.vlinegap);
    dump_intfield(L, "hhead_ascent", pfm.hhead_ascent);
    dump_intfield(L, "hhead_descent", pfm.hhead_descent);
    dump_intfield(L, "hhead_descent", pfm.hhead_descent);
    dump_intfield(L, "os2_typoascent", pfm.os2_typoascent);
    dump_intfield(L, "os2_typodescent", pfm.os2_typodescent);
    dump_intfield(L, "os2_typolinegap", pfm.os2_typolinegap);
    dump_intfield(L, "os2_winascent", pfm.os2_winascent);
    dump_intfield(L, "os2_windescent", pfm.os2_windescent);
    dump_intfield(L, "os2_subxsize", pfm.os2_subxsize);
    dump_intfield(L, "os2_subysize", pfm.os2_subysize);
    dump_intfield(L, "os2_subxoff", pfm.os2_subxoff);
    dump_intfield(L, "os2_subyoff", pfm.os2_subyoff);
    dump_intfield(L, "os2_supxsize", pfm.os2_supxsize);
    dump_intfield(L, "os2_supysize", pfm.os2_supysize);
    dump_intfield(L, "os2_supxoff", pfm.os2_supxoff);
    dump_intfield(L, "os2_supyoff", pfm.os2_supyoff);
    dump_intfield(L, "os2_strikeysize", pfm.os2_strikeysize);
    dump_intfield(L, "os2_strikeypos", pfm.os2_strikeypos);
    dump_lstringfield(L, "os2_vendor", pfm.os2_vendor, 4);
    dump_intfield(L, "os2_family_class", pfm.os2_family_class);
    dump_intfield(L, "os2_xheight", pfm.os2_xheight);
    dump_intfield(L, "os2_capheight", pfm.os2_capheight);
    dump_intfield(L, "os2_defaultchar", pfm.os2_defaultchar);
    dump_intfield(L, "os2_breakchar", pfm.os2_breakchar);
    if (pfm.hascodepages) {
        lua_newtable(L);
        lua_pushnumber(L, pfm.codepages[0]);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, pfm.codepages[1]);
        lua_rawseti(L, -2, 2);
        lua_setfield(L, -2, "codepages");
    }
    if (pfm.hasunicoderanges) {
        lua_newtable(L);
        lua_pushnumber(L, pfm.unicoderanges[0]);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, pfm.unicoderanges[1]);
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, pfm.unicoderanges[2]);
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, pfm.unicoderanges[3]);
        lua_rawseti(L, -2, 4);
        lua_setfield(L, -2, "unicoderanges");
    }
}


static char *do_handle_enc(lua_State * L, struct enc *enc)
{
    int i;
    char *ret = enc->enc_name;
    dump_stringfield(L, "enc_name", enc->enc_name);
    dump_intfield(L, "char_cnt", enc->char_cnt);

    lua_checkstack(L, 4);
    if (enc->char_cnt && enc->unicode != NULL) {
        lua_createtable(L, enc->char_cnt, 1);
        for (i = 0; i < enc->char_cnt; i++) {
            lua_pushnumber(L, i);
            lua_pushnumber(L, enc->unicode[i]);
            lua_rawset(L, -3);
        }
        lua_setfield(L, -2, "unicode");
    }

    if (enc->char_cnt && enc->psnames != NULL) {
        lua_createtable(L, enc->char_cnt, 1);
        for (i = 0; i < enc->char_cnt; i++) {
            lua_pushnumber(L, i);
            lua_pushstring(L, enc->psnames[i]);
            lua_rawset(L, -3);
        }
        lua_setfield(L, -2, "psnames");
    }
    dump_intfield(L, "builtin", enc->builtin);
    dump_intfield(L, "hidden", enc->hidden);
    dump_intfield(L, "only_1byte", enc->only_1byte);
    dump_intfield(L, "has_1byte", enc->has_1byte);
    dump_intfield(L, "has_2byte", enc->has_2byte);
    dump_cond_intfield(L, "is_unicodebmp", enc->is_unicodebmp);
    dump_cond_intfield(L, "is_unicodefull", enc->is_unicodefull);
    dump_cond_intfield(L, "is_custom", enc->is_custom);
    dump_cond_intfield(L, "is_original", enc->is_original);
    dump_cond_intfield(L, "is_compact", enc->is_compact);
    dump_cond_intfield(L, "is_japanese", enc->is_japanese);
    dump_cond_intfield(L, "is_korean", enc->is_korean);
    dump_cond_intfield(L, "is_tradchinese", enc->is_tradchinese);
    dump_cond_intfield(L, "is_simplechinese", enc->is_simplechinese);

    if (enc->iso_2022_escape_len > 0) {
        dump_lstringfield(L, "iso_2022_escape", enc->iso_2022_escape, enc->iso_2022_escape_len);
    }
    dump_intfield(L, "low_page", enc->low_page);
    dump_intfield(L, "high_page", enc->high_page);

    dump_stringfield(L, "iconv_name", enc->iconv_name);

    dump_intfield(L, "char_max", enc->char_max);
    return ret;
}

#if 0 /* unused */
static void handle_enc(lua_State * L, struct enc *enc)
{
    struct enc *next;
    NESTED_TABLE(do_handle_enc, enc, 24);
}
#endif

static void handle_encmap(lua_State * L, struct encmap *map, int notdef_loc)
{
    int i;
    dump_intfield(L, "enccount", map->enccount);
    dump_intfield(L, "encmax", map->encmax);
    dump_intfield(L, "backmax", map->backmax);
    /*dump_intfield(L,"ticked",   map->ticked) ; */
    if (map->remap != NULL) {
        lua_newtable(L);
        dump_intfield(L, "firstenc", map->remap->firstenc);
        dump_intfield(L, "lastenc", map->remap->lastenc);
        dump_intfield(L, "infont", map->remap->infont);
        lua_setfield(L, -2, "remap");
    }
    lua_checkstack(L, 4);
    if (map->encmax > 0 && map->map != NULL) {
        lua_createtable(L, map->encmax, 1);
        for (i = 0; i < map->encmax; i++) {
            if (map->map[i] != -1) {
                int l = map->map[i];
                lua_pushnumber(L, i);
                /*
                if (l < notdef_loc)
                    lua_pushnumber(L, (l + 1));
                else
                */
                    lua_pushnumber(L, l);
                lua_rawset(L, -3);
            }
        }
        lua_setfield(L, -2, "map");
    }

    if (map->backmax > 0 && map->backmap != NULL) {
        lua_newtable(L);
        for (i = 0; i < map->backmax; i++) {
            if (map->backmap[i] != -1) {
                /*
                if (i < notdef_loc)
                    lua_pushnumber(L, (i + 1));
                else
                */
                    lua_pushnumber(L, i);
                lua_pushnumber(L, map->backmap[i]);
                lua_rawset(L, -3);
            }
        }
        lua_setfield(L, -2, "backmap");
    }

    if (map->enc != NULL) {
	char *encname;
        lua_newtable(L);
        encname = do_handle_enc(L, map->enc);
        lua_setfield(L, -2, "enc");
        lua_pushstring(L, encname);
        lua_setfield(L, -2, "enc_name");
    }
}

static void handle_psdict(lua_State * L, struct psdict *private)
{
    int k;
    if (private->keys != NULL && private->values != NULL) {
        for (k = 0; k < private->next; k++) {
            lua_pushstring(L, private->keys[k]);
            lua_pushstring(L, private->values[k]);
            lua_rawset(L, -3);
        }
    }
}

static void do_handle_ttflangname(lua_State * L, struct ttflangname *names)
{
    int k;
    dump_stringfield(L, "lang", MSLangString(names->lang));
    lua_checkstack(L, 4);
    lua_createtable(L, 0, ttf_namemax);
    for (k = 0; k < ttf_namemax; k++) {
        lua_pushstring(L, ttfnames_enum[k]);
        lua_pushstring(L, names->names[k]);
        lua_rawset(L, -3);
    }
    lua_setfield(L, -2, "names");
}

static void handle_ttflangname(lua_State * L, struct ttflangname *names)
{
    struct ttflangname *next;
    NESTED_TABLE(do_handle_ttflangname, names, 2);
}

static void do_handle_anchorclass(lua_State * L, struct anchorclass *anchor)
{
    dump_stringfield(L, "name", anchor->name);
    dump_subtable_name(L, "lookup", anchor->subtable);
    dump_enumfield(L, "type", anchor->type, anchorclass_type_enum);
    /* uint8 has_base; */
    /* uint8 processed, has_mark, matches, ac_num; */
    /* uint8 ticked; */
}

static void handle_anchorclass(lua_State * L, struct anchorclass *anchor)
{
    struct anchorclass *next;
    NESTED_TABLE(do_handle_anchorclass, anchor, 10);
}

static void do_handle_ttf_table(lua_State * L, struct ttf_table *ttf_tab)
{

    dump_tag(L, "tag", ttf_tab->tag);
    dump_intfield(L, "len", ttf_tab->len);
    dump_intfield(L, "maxlen", ttf_tab->maxlen);
    dump_lstringfield(L, "data", (char *) ttf_tab->data, ttf_tab->len);
}

static void handle_ttf_table(lua_State * L, struct ttf_table *ttf_tab)
{
    struct ttf_table *next;
    NESTED_TABLE(do_handle_ttf_table, ttf_tab, 4);
}

static int do_handle_kernclass(lua_State * L, struct kernclass *kerns, const char *name)
{
    int k;
    int match = 0;
    if (name) {
        struct lookup_subtable *s = kerns->subtable;
        while (s != NULL) {
            if (strcmp(s->subtable_name,name)==0) {
                match = 1;
                break;
            }
            s = s->next;
        }
    } else {
        match = 1;
    }
    if (!match) {
        return 0;
    }
    lua_checkstack(L, 4);
    lua_createtable(L, kerns->first_cnt, 1);
    for (k = 0; k < kerns->first_cnt; k++) {
        lua_pushnumber(L, (k + 1));
        lua_pushstring(L, kerns->firsts[k]);
        lua_rawset(L, -3);
    }
    lua_setfield(L, -2, "firsts");

    lua_createtable(L, kerns->second_cnt, 1);
    for (k = 0; k < kerns->second_cnt; k++) {
        lua_pushnumber(L, (k + 1));
        lua_pushstring(L, kerns->seconds[k]);
        lua_rawset(L, -3);
    }
    lua_setfield(L, -2, "seconds");

    if (!name) {
	dump_subtable_name(L, "lookup", kerns->subtable);
    }
    lua_createtable(L, kerns->second_cnt * kerns->first_cnt, 1);
    for (k = 0; k < (kerns->second_cnt * kerns->first_cnt); k++) {
        if (kerns->offsets[k] != 0) {
            lua_pushnumber(L, (k + 1));
            lua_pushnumber(L, kerns->offsets[k]);
            lua_rawset(L, -3);
        }
    }
    lua_setfield(L, -2, "offsets");
    return 1;
}

static void handle_kernclass(lua_State * L, struct kernclass *kerns, const char *name)
{
    struct kernclass *next;
    NESTED_TABLE_SF(do_handle_kernclass, kerns, name, 8);
}


#define DUMP_NUMBER_ARRAY(s,cnt,item) {	\
    if (cnt>0 && item != NULL) {		\
      int kk;							\
      lua_newtable(L);					\
      for (kk=0;kk<cnt;kk++) {			\
		lua_pushnumber(L,(kk+1));		\
		lua_pushnumber(L,item[kk]);		\
		lua_rawset(L,-3); }				\
      lua_setfield(L,-2,s); } }


#define DUMP_STRING_ARRAY(s,cnt,item) {	\
    if (cnt>0 && item!=NULL) {			\
      int kk;							\
      lua_newtable(L);					\
      for (kk=0;kk<cnt;kk++) {			\
		lua_pushnumber(L,(kk+1));		\
		lua_pushstring(L,item[kk]);		\
		lua_rawset(L,-3); }				\
      lua_setfield(L,-2,s); } }

#define DUMP_EXACT_STRING_ARRAY(s,cnt,item) { \
    if (cnt>0 && item!=NULL) {				  \
      int kk;								  \
      lua_newtable(L);						  \
      for (kk=0;kk<cnt;kk++) {				  \
		lua_pushnumber(L,(kk));				  \
		lua_pushstring(L,item[kk]);			  \
		lua_rawset(L,-3); }					  \
      lua_setfield(L,-2,s); } }

static void handle_fpst_rule(lua_State * L, struct fpst_rule *rule, int format)
{
    int k;

    if (format == pst_glyphs) {

        lua_newtable(L);
        dump_stringfield(L, "names", rule->u.glyph.names);
        dump_stringfield(L, "back", rule->u.glyph.back);
        dump_stringfield(L, "fore", rule->u.glyph.fore);
        lua_setfield(L, -2, fpossub_format_enum[format]);

    } else if (format == pst_class) {

        lua_newtable(L);
        DUMP_NUMBER_ARRAY("current", rule->u.class.ncnt,
                          rule->u.class.nclasses);
        DUMP_NUMBER_ARRAY("before", rule->u.class.bcnt, rule->u.class.bclasses);
        DUMP_NUMBER_ARRAY("after", rule->u.class.fcnt, rule->u.class.fclasses);
#if 0
        DUMP_NUMBER_ARRAY("allclasses", 0, rule->u.class.allclasses);
#endif
        lua_setfield(L, -2, fpossub_format_enum[format]);

    } else if (format == pst_coverage) {

        lua_newtable(L);
        DUMP_STRING_ARRAY("current", rule->u.coverage.ncnt,
                          rule->u.coverage.ncovers);
        DUMP_STRING_ARRAY("before", rule->u.coverage.bcnt,
                          rule->u.coverage.bcovers);
        DUMP_STRING_ARRAY("after", rule->u.coverage.fcnt,
                          rule->u.coverage.fcovers);
        lua_setfield(L, -2, fpossub_format_enum[format]);

    } else if (format == pst_reversecoverage) {

        lua_newtable(L);
        DUMP_STRING_ARRAY("current", rule->u.rcoverage.always1,
                          rule->u.rcoverage.ncovers);
        DUMP_STRING_ARRAY("before", rule->u.rcoverage.bcnt,
                          rule->u.rcoverage.bcovers);
        DUMP_STRING_ARRAY("after", rule->u.rcoverage.fcnt,
                          rule->u.rcoverage.fcovers);
        dump_stringfield(L, "replacements", rule->u.rcoverage.replacements);
        lua_setfield(L, -2, fpossub_format_enum[format]);
    } else {
        fprintf(stderr, "handle_fpst_rule(): Unknown rule format: %d\n",
                format);
    }

    if (rule->lookup_cnt > 0) {
        lua_newtable(L);
        for (k = 0; k < rule->lookup_cnt; k++) {
            lua_pushnumber(L, (rule->lookups[k].seq + 1));
            if (rule->lookups[k].lookup != NULL) {
                lua_pushstring(L, rule->lookups[k].lookup->lookup_name);
            } else {
                lua_pushnil(L);
            }
            lua_rawset(L, -3);
        }
        lua_setfield(L, -2, "lookups");
    } else {
        /*
            fprintf(stderr,"handle_fpst_rule(): No lookups?\n");
        */
    }
}

static void do_handle_generic_fpst(lua_State * L, struct generic_fpst *fpst)
{
    int k;

    if (fpst->type > LAST_POSSUB_TYPE_ENUM) {
        dump_intfield(L, "type", fpst->type);
    } else {
        dump_enumfield(L, "type", fpst->type, possub_type_enum);
    }
    dump_enumfield(L, "format", fpst->format, fpossub_format_enum);

    if (fpst->format == pst_class) {
        DUMP_EXACT_STRING_ARRAY("current_class", fpst->nccnt, fpst->nclass);
        DUMP_EXACT_STRING_ARRAY("before_class", fpst->bccnt, fpst->bclass);
        DUMP_EXACT_STRING_ARRAY("after_class", fpst->fccnt, fpst->fclass);
    } else {
        DUMP_STRING_ARRAY("current_class", fpst->nccnt, fpst->nclass);
        DUMP_STRING_ARRAY("before_class", fpst->bccnt, fpst->bclass);
        DUMP_STRING_ARRAY("after_class", fpst->fccnt, fpst->fclass);
    }

    lua_checkstack(L, 4);
    if (fpst->rule_cnt > 0) {
        lua_createtable(L, fpst->rule_cnt, 1);
        for (k = 0; k < fpst->rule_cnt; k++) {
            lua_pushnumber(L, (k + 1));
            lua_newtable(L);
            handle_fpst_rule(L, &(fpst->rules[k]), fpst->format);
            lua_rawset(L, -3);
        }
        lua_setfield(L, -2, "rules");
    }
    /*dump_intfield (L,"ticked", fpst->ticked); */
}

static void handle_generic_fpst(lua_State * L, struct generic_fpst *fpst)
{
    struct generic_fpst *next;
    int k = 1;
    lua_checkstack(L, 3);
    if (fpst->subtable != NULL && fpst->subtable->subtable_name != NULL) {
        lua_pushstring(L, fpst->subtable->subtable_name);
    } else {
        lua_pushnumber(L, k);
        k++;
    }
    lua_createtable(L, 0, 10);
    do_handle_generic_fpst(L, fpst);
    lua_rawset(L, -3);
    next = fpst->next;
    while (next != NULL) {
        lua_checkstack(L, 3);
        if (next->subtable != NULL && next->subtable->subtable_name != NULL) {
            lua_pushstring(L, next->subtable->subtable_name);
        } else {
            lua_pushnumber(L, k);
            k++;
        }
        lua_createtable(L, 0, 10);
        do_handle_generic_fpst(L, next);
        lua_rawset(L, -3);
        next = next->next;
    }
}

static void do_handle_otfname(lua_State * L, struct otfname *oname)
{
    dump_intfield(L, "lang", oname->lang);
    dump_stringfield(L, "name", oname->name);
}

static void handle_otfname(lua_State * L, struct otfname *oname)
{
    struct otfname *next;
    NESTED_TABLE(do_handle_otfname, oname, 2);
}

static void handle_MATH(lua_State * L, struct MATH *MATH)
{
    dump_intfield(L, "ScriptPercentScaleDown", MATH->ScriptPercentScaleDown);
    dump_intfield(L, "ScriptScriptPercentScaleDown", MATH->ScriptScriptPercentScaleDown);
    dump_intfield(L, "DelimitedSubFormulaMinHeight", MATH->DelimitedSubFormulaMinHeight);
    dump_intfield(L, "DisplayOperatorMinHeight", MATH->DisplayOperatorMinHeight);
    dump_intfield(L, "MathLeading", MATH->MathLeading);
    dump_intfield(L, "AxisHeight", MATH->AxisHeight);
    dump_intfield(L, "AccentBaseHeight", MATH->AccentBaseHeight);
    dump_intfield(L, "FlattenedAccentBaseHeight", MATH->FlattenedAccentBaseHeight);
    dump_intfield(L, "SubscriptShiftDown", MATH->SubscriptShiftDown);
    dump_intfield(L, "SubscriptTopMax", MATH->SubscriptTopMax);
    dump_intfield(L, "SubscriptBaselineDropMin", MATH->SubscriptBaselineDropMin);
    dump_intfield(L, "SuperscriptShiftUp", MATH->SuperscriptShiftUp);
    dump_intfield(L, "SuperscriptShiftUpCramped", MATH->SuperscriptShiftUpCramped);
    dump_intfield(L, "SuperscriptBottomMin", MATH->SuperscriptBottomMin);
    dump_intfield(L, "SuperscriptBaselineDropMax", MATH->SuperscriptBaselineDropMax);
    dump_intfield(L, "SubSuperscriptGapMin", MATH->SubSuperscriptGapMin);
    dump_intfield(L, "SuperscriptBottomMaxWithSubscript", MATH->SuperscriptBottomMaxWithSubscript);
    dump_intfield(L, "SpaceAfterScript", MATH->SpaceAfterScript);
    dump_intfield(L, "UpperLimitGapMin", MATH->UpperLimitGapMin);
    dump_intfield(L, "UpperLimitBaselineRiseMin", MATH->UpperLimitBaselineRiseMin);
    dump_intfield(L, "LowerLimitGapMin", MATH->LowerLimitGapMin);
    dump_intfield(L, "LowerLimitBaselineDropMin", MATH->LowerLimitBaselineDropMin);
    dump_intfield(L, "StackTopShiftUp", MATH->StackTopShiftUp);
    dump_intfield(L, "StackTopDisplayStyleShiftUp", MATH->StackTopDisplayStyleShiftUp);
    dump_intfield(L, "StackBottomShiftDown", MATH->StackBottomShiftDown);
    dump_intfield(L, "StackBottomDisplayStyleShiftDown", MATH->StackBottomDisplayStyleShiftDown);
    dump_intfield(L, "StackGapMin", MATH->StackGapMin);
    dump_intfield(L, "StackDisplayStyleGapMin", MATH->StackDisplayStyleGapMin);
    dump_intfield(L, "StretchStackTopShiftUp", MATH->StretchStackTopShiftUp);
    dump_intfield(L, "StretchStackBottomShiftDown", MATH->StretchStackBottomShiftDown);
    dump_intfield(L, "StretchStackGapAboveMin", MATH->StretchStackGapAboveMin);
    dump_intfield(L, "StretchStackGapBelowMin", MATH->StretchStackGapBelowMin);
    dump_intfield(L, "FractionNumeratorShiftUp", MATH->FractionNumeratorShiftUp);
    dump_intfield(L, "FractionNumeratorDisplayStyleShiftUp", MATH->FractionNumeratorDisplayStyleShiftUp);
    dump_intfield(L, "FractionDenominatorShiftDown", MATH->FractionDenominatorShiftDown);
    dump_intfield(L, "FractionDenominatorDisplayStyleShiftDown", MATH->FractionDenominatorDisplayStyleShiftDown);
    dump_intfield(L, "FractionNumeratorGapMin", MATH->FractionNumeratorGapMin);
    dump_intfield(L, "FractionNumeratorDisplayStyleGapMin", MATH->FractionNumeratorDisplayStyleGapMin);
    dump_intfield(L, "FractionRuleThickness", MATH->FractionRuleThickness);
    dump_intfield(L, "FractionDenominatorGapMin", MATH->FractionDenominatorGapMin);
    dump_intfield(L, "FractionDenominatorDisplayStyleGapMin", MATH->FractionDenominatorDisplayStyleGapMin);
    dump_intfield(L, "SkewedFractionHorizontalGap", MATH->SkewedFractionHorizontalGap);
    dump_intfield(L, "SkewedFractionVerticalGap", MATH->SkewedFractionVerticalGap);
    dump_intfield(L, "OverbarVerticalGap", MATH->OverbarVerticalGap);
    dump_intfield(L, "OverbarRuleThickness", MATH->OverbarRuleThickness);
    dump_intfield(L, "OverbarExtraAscender", MATH->OverbarExtraAscender);
    dump_intfield(L, "UnderbarVerticalGap", MATH->UnderbarVerticalGap);
    dump_intfield(L, "UnderbarRuleThickness", MATH->UnderbarRuleThickness);
    dump_intfield(L, "UnderbarExtraDescender", MATH->UnderbarExtraDescender);
    dump_intfield(L, "RadicalVerticalGap", MATH->RadicalVerticalGap);
    dump_intfield(L, "RadicalDisplayStyleVerticalGap", MATH->RadicalDisplayStyleVerticalGap);
    dump_intfield(L, "RadicalRuleThickness", MATH->RadicalRuleThickness);
    dump_intfield(L, "RadicalExtraAscender", MATH->RadicalExtraAscender);
    dump_intfield(L, "RadicalKernBeforeDegree", MATH->RadicalKernBeforeDegree);
    dump_intfield(L, "RadicalKernAfterDegree", MATH->RadicalKernAfterDegree);
    dump_intfield(L, "RadicalDegreeBottomRaisePercent", MATH->RadicalDegreeBottomRaisePercent);
    dump_intfield(L, "MinConnectorOverlap", MATH->MinConnectorOverlap);
}

/* the handling of BASE is untested, no font */
static void handle_baselangextent(lua_State * L, struct baselangextent *ble);

static void do_handle_baselangextent(lua_State * L, struct baselangextent *ble)
{
    dump_tag(L, "tag", ble->lang);
    dump_intfield(L, "ascent", ble->ascent);
    dump_intfield(L, "descent", ble->descent);
    lua_newtable(L);
    handle_baselangextent(L, ble->features);
    lua_setfield(L, -2, "features");
}

static void handle_baselangextent(lua_State * L, struct baselangextent *ble)
{
    struct baselangextent *next;
    NESTED_TABLE(do_handle_baselangextent, ble, 4);
}

static void handle_base(lua_State * L, struct Base *Base)
{
    int i;
    struct basescript *next = Base->scripts;
    lua_newtable(L);
    for (i = 0; i < Base->baseline_cnt; i++) {
        lua_pushstring(L, make_tag_string(Base->baseline_tags[i]));
        lua_rawseti(L, -2, (i + 1));
    }
    lua_setfield(L, -2, "tags");
    if (next != NULL) {
        lua_newtable(L);
        while (next != NULL) {
            lua_pushstring(L, make_tag_string(next->script));
            lua_newtable(L);
            dump_intfield(L, "default_baseline", (next->def_baseline + 1));
            lua_newtable(L);
            for (i = 0; i < Base->baseline_cnt; i++) {
                if (next->baseline_pos != NULL) /* default omitted */
                    lua_pushnumber(L, next->baseline_pos[i]);
                else
                    lua_pushnumber(L, 0);
                lua_rawseti(L, -2, (i + 1));
            }
            lua_setfield(L, -2, "baseline");
            lua_newtable(L);
            handle_baselangextent(L, next->langs);
            lua_setfield(L, -2, "lang");
            lua_rawset(L, -3);
            next = next->next;
        }
        lua_setfield(L, -2, "scripts");
    }
}

static void handle_axismap(lua_State * L, struct axismap *am)
{
    int i;
    lua_checkstack(L, 3);
    lua_newtable(L);
    for (i = 0; i < am->points; i++) {
        lua_pushnumber(L, am->blends[i]);
        lua_rawseti(L, -2, (i + 1));
    }
    lua_setfield(L, -2, "blends");
    lua_newtable(L);
    for (i = 0; i < am->points; i++) {
        lua_pushnumber(L, am->designs[i]);
        lua_rawseti(L, -2, (i + 1));
    }
    lua_setfield(L, -2, "designs");
    dump_realfield(L, "min", am->min);
    dump_realfield(L, "def", am->def);
    dump_realfield(L, "max", am->max);
}

static void handle_mmset(lua_State * L, struct mmset *mm)
{
    int i;
    lua_newtable(L);
    for (i = 0; i < mm->axis_count; i++) {
        lua_pushstring(L, mm->axes[i]);
        lua_rawseti(L, -2, (i + 1));
    }
    lua_setfield(L, -2, "axes");

    dump_intfield(L, "instance_count", mm->instance_count);
    /* SplineFont *normal; *//* this is the parent */
    if (mm->instance_count > 0) {
        lua_newtable(L);
        for (i = 0; i < mm->instance_count * mm->axis_count; i++) {
            lua_pushnumber(L, mm->positions[i]);
            lua_rawseti(L, -2, (i + 1));
        }
        lua_setfield(L, -2, "positions");

        /* better not to do this */
#if 0
        {
            struct mmset *mmsave;
            lua_newtable(L);
            for (i = 0; i < mm->instance_count; i++) {
                lua_checkstack(L, 20);
                lua_createtable(L, 0, 60);
                mmsave = mm->instances[i]->mm;
                mm->instances[i]->mm = NULL;
                handle_splinefont(L, mm->instances[i]);
                mm->instances[i]->mm = mmsave;
                lua_rawseti(L, -2, (i + 1));
            }
            lua_setfield(L, -2, "instances");
        }
#endif

        lua_newtable(L);
        for (i = 0; i < mm->instance_count; i++) {
            lua_pushnumber(L, mm->defweights[i]);
            lua_rawseti(L, -2, (i + 1));
        }
        lua_setfield(L, -2, "defweights");
    }

    if (mm->axismaps != NULL) {
        lua_newtable(L);
        for (i = 0; i < mm->axis_count; i++) {
            lua_newtable(L);
            handle_axismap(L, &(mm->axismaps[i]));
            lua_rawseti(L, -2, (i + 1));
        }
        lua_setfield(L, -2, "axismaps");
    }
    dump_stringfield(L, "cdv", mm->cdv);
    dump_stringfield(L, "ndv", mm->ndv);
}

static void handle_splinefont(lua_State * L, struct splinefont *sf)
{
    int k;
    int fix_notdef = 0; /* obsolete */
    int l = -1;

    dump_stringfield(L, "table_version", LUA_OTF_VERSION);
    dump_stringfield(L, "fontname", sf->fontname);
    dump_stringfield(L, "fullname", sf->fullname);
    dump_stringfield(L, "familyname", sf->familyname);
    dump_stringfield(L, "weight", sf->weight);
    dump_stringfield(L, "copyright", sf->copyright);
    dump_stringfield(L, "filename", sf->filename);
    /* dump_stringfield(L,"defbasefilename", sf->defbasefilename); */
    dump_stringfield(L, "version", sf->version);
    dump_floatfield(L, "italicangle", sf->italicangle);
    dump_floatfield(L, "upos", sf->upos);
    dump_floatfield(L, "uwidth", sf->uwidth);
    dump_intfield(L, "ascent", sf->ascent);
    dump_intfield(L, "descent", sf->descent);
    dump_intfield(L, "notdef_loc",notdef_loc(sf));
    if (sf->uniqueid!=0) {
        dump_intfield(L, "uniqueid", sf->uniqueid);
    }

    if (sf->glyphcnt > 0) {
        dump_intfield(L, "glyphcnt", sf->glyphmax - sf->glyphmin + 1);
    } else {
        dump_intfield(L, "glyphcnt", 0);
    }

    if (sf->names != NULL) {
        /*
            this is not the best way to determine it but for now it will do; otherwise
            we need to mess with the ff library
        */
        lua_pushstring(L, "truetype");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }

    dump_intfield(L, "glyphmax", sf->glyphmax - 1);
    dump_intfield(L, "glyphmin", sf->glyphmin);
    dump_intfield(L, "units_per_em", sf->units_per_em);

    if (sf->possub != NULL) {
        lua_newtable(L);
        handle_generic_fpst(L, sf->possub);
        lua_setfield(L, -2, "lookups");
    }

    lua_checkstack(L, 4);
    lua_createtable(L, sf->glyphcnt, 0);

    /* This after-the-fact type discovery is not brilliant,
       I should really add a 'format' key in the structure */

    /*
    if ((sf->origname != NULL) &&
        (strmatch(sf->origname + strlen(sf->origname) - 4, ".pfa") == 0 ||
         strmatch(sf->origname + strlen(sf->origname) - 4, ".pfb") == 0)) {
        fix_notdef = 1;
    }
    */

    if (fix_notdef) {

        /* this is obsolete */

        /* some code to ensure that the .notdef ends up in slot 0
           (this will actually be enforced by the CFF writer) */
        for (k = 0; k < sf->glyphcnt; k++) {
            if (sf->glyphs[k]) {
                if (strcmp(sf->glyphs[k]->name, ".notdef") == 0) {
                    l = k;
                }
            }
        }
        if (l == -1) {          /* fake a .notdef at the end */
            l = sf->glyphcnt;
        }
        for (k = 0; k < l; k++) {
            if (sf->glyphs[k]) {
                lua_pushnumber(L, (k + 1));
                lua_createtable(L, 0, 12);
                handle_splinechar(L, sf->glyphs[k], sf->hasvmetrics);
                lua_rawset(L, -3);
            }
        }
        if (sf->glyphs != NULL && l < sf->glyphcnt) {
            lua_pushnumber(L, 0);
            if (sf->glyphs[l]) {
                lua_createtable(L, 0, 12);
                handle_splinechar(L, sf->glyphs[l], sf->hasvmetrics);
            } else {
                lua_createtable(L, 0, 0);
            }
            lua_rawset(L, -3);
        }
    }
    if ((l + 1) < sf->glyphcnt) {
        for (k = (l + 1); k < sf->glyphcnt; k++) {
            if (sf->glyphs[k]) {
                lua_pushnumber(L, k);
                lua_createtable(L, 0, 12);
                handle_splinechar(L, sf->glyphs[k], sf->hasvmetrics);
                lua_rawset(L, -3);
            }
        }
    }
    lua_setfield(L, -2, "glyphs");

    /* dump_intfield(L,"changed",                   sf->changed); */
    dump_intfield(L, "hasvmetrics", sf->hasvmetrics);
    dump_intfield(L, "onlybitmaps", sf->onlybitmaps);
    dump_intfield(L, "serifcheck", sf->serifcheck);
    dump_intfield(L, "isserif", sf->isserif);
    dump_intfield(L, "issans", sf->issans);
    dump_intfield(L, "encodingchanged", sf->encodingchanged);
    dump_intfield(L, "strokedfont", sf->strokedfont);
    dump_intfield(L, "use_typo_metrics", sf->use_typo_metrics);
    dump_intfield(L, "weight_width_slope_only", sf->weight_width_slope_only);
    dump_intfield(L, "head_optimized_for_cleartype",
                  sf->head_optimized_for_cleartype);

    dump_enumfield(L, "uni_interp", (sf->uni_interp + 1), uni_interp_enum);

    if (sf->map != NULL) {
        lua_newtable(L);
        handle_encmap(L, sf->map, l);
        lua_setfield(L, -2, "map");
    }

    dump_stringfield(L, "origname", sf->origname);      /* new */

    if (sf->private != NULL) {
        lua_newtable(L);
        handle_psdict(L, sf->private);
        lua_setfield(L, -2, "private");
    }

    dump_stringfield(L, "xuid", sf->xuid);

    lua_createtable(L, 0, 40);
    handle_pfminfo(L, sf->pfminfo);
    lua_setfield(L, -2, "pfminfo");

    if (sf->names != NULL) {
        lua_newtable(L);
        handle_ttflangname(L, sf->names);
        lua_setfield(L, -2, "names");
    }

    lua_createtable(L, 0, 4);
    dump_stringfield(L, "registry", sf->cidregistry);
    dump_stringfield(L, "ordering", sf->ordering);
    dump_intfield(L, "version", sf->cidversion);
    dump_intfield(L, "supplement", sf->supplement);
    lua_setfield(L, -2, "cidinfo");

    /* SplineFont *cidmaster *//* parent in a subfont */
    if (sf->subfontcnt > 0) {
        lua_createtable(L, sf->subfontcnt, 0);
        for (k = 0; k < sf->subfontcnt; k++) {
            lua_checkstack(L, 10);
            lua_newtable(L);
            handle_splinefont(L, sf->subfonts[k]);
            lua_rawseti(L, -2, (k + 1));
        }
        lua_setfield(L, -2, "subfonts");
    }

    dump_stringfield(L, "comments", sf->comments);
    dump_stringfield(L, "fontlog", sf->fontlog);

    if (sf->cvt_names != NULL) {
        lua_newtable(L);
        for (k = 0; sf->cvt_names[k] != END_CVT_NAMES; ++k) {
            lua_pushstring(L, sf->cvt_names[k]);
            lua_rawseti(L, -2, (k + 1));
        }
        lua_setfield(L, -2, "cvt_names");
    }

    if (sf->ttf_tables != NULL) {
        lua_newtable(L);
        handle_ttf_table(L, sf->ttf_tables);
        lua_setfield(L, -2, "ttf_tables");
    }

    if (sf->ttf_tab_saved != NULL) {
        lua_newtable(L);
        handle_ttf_table(L, sf->ttf_tab_saved);
        lua_setfield(L, -2, "ttf_tab_saved");
    }

    if (sf->texdata.type != tex_unset) {
        lua_newtable(L);
        dump_enumfield(L, "type", sf->texdata.type, tex_type_enum);
        lua_newtable(L);
        for (k = 0; k < 22; k++) {
            lua_pushnumber(L, k);
            lua_pushnumber(L, sf->texdata.params[k]);
            lua_rawset(L, -3);
        }
        lua_setfield(L, -2, "params");
        lua_setfield(L, -2, "texdata");
    }
    if (sf->anchor != NULL) {
        lua_newtable(L);
        handle_anchorclass(L, sf->anchor);
        lua_setfield(L, -2, "anchor_classes");
    }
    if (sf->kerns != NULL) {
        lua_newtable(L);
        handle_kernclass(L, sf->kerns, NULL);
        lua_setfield(L, -2, "kerns");
    }
    if (sf->vkerns != NULL) {
        lua_newtable(L);
        handle_kernclass(L, sf->vkerns, NULL);
        lua_setfield(L, -2, "vkerns");
    }
    if (sf->gsub_lookups != NULL) {
        lua_newtable(L);
        handle_lookup(L, sf->gsub_lookups, sf);
        lua_setfield(L, -2, "gsub");
    }
    if (sf->gpos_lookups != NULL) {
        lua_newtable(L);
        handle_lookup(L, sf->gpos_lookups, sf);
        lua_setfield(L, -2, "gpos");
    }

    if (sf->mm != NULL) {
        lua_newtable(L);
        handle_mmset(L, sf->mm);
        lua_setfield(L, -2, "mm");
    }
    dump_stringfield(L, "chosenname", sf->chosenname);

    if (sf->macstyle!=-1) {
	dump_intfield(L, "macstyle", sf->macstyle);
    }
    dump_stringfield(L, "fondname", sf->fondname);

    dump_intfield(L, "design_size", sf->design_size);
    dump_intfield(L, "fontstyle_id", sf->fontstyle_id);

    if (sf->fontstyle_name != NULL) {
        lua_newtable(L);
        handle_otfname(L, sf->fontstyle_name);
        lua_setfield(L, -2, "fontstyle_name");
    }

    dump_intfield(L, "design_range_bottom", sf->design_range_bottom);
    dump_intfield(L, "design_range_top", sf->design_range_top);
    dump_floatfield(L, "strokewidth", sf->strokewidth);

    if (sf->mark_class_cnt > 0) {
        lua_newtable(L);
        for (k = 0; k < sf->mark_class_cnt; k++) {
            if (sf->mark_class_names[k] != NULL) {
                lua_pushstring(L, sf->mark_class_names[k]);
                lua_pushstring(L, sf->mark_classes[k]);
                lua_rawset(L, -3);
            }
        }
        lua_setfield(L, -2, "mark_classes");
    }

    dump_uintfield(L, "creationtime", sf->creationtime);
    dump_uintfield(L, "modificationtime", sf->modificationtime);

    dump_intfield(L, "os2_version", sf->os2_version);
    dump_intfield(L, "sfd_version", sf->sfd_version);

    if (sf->MATH != NULL) {
        lua_newtable(L);
        handle_MATH(L, sf->MATH);
        lua_setfield(L, -2, "math");
    }

    if (sf->loadvalidation_state != 0) {
        int val, st;
        lua_newtable(L);
        val = 1;
        st = sf->loadvalidation_state;
        if (st & lvs_bad_ps_fontname) {
            lua_pushliteral(L, "bad_ps_fontname");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_glyph_table) {
            lua_pushliteral(L, "bad_glyph_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_cff_table) {
            lua_pushliteral(L, "bad_cff_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_metrics_table) {
            lua_pushliteral(L, "bad_metrics_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_cmap_table) {
            lua_pushliteral(L, "bad_cmap_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_bitmaps_table) {
            lua_pushliteral(L, "bad_bitmaps_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_gx_table) {
            lua_pushliteral(L, "bad_gx_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_ot_table) {
            lua_pushliteral(L, "bad_ot_table");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_os2_version) {
            lua_pushliteral(L, "bad_os2_version");
            lua_rawseti(L, -2, val++);
        }
        if (st & lvs_bad_sfnt_header) {
            lua_pushliteral(L, "bad_sfnt_header");
            lua_rawseti(L, -2, val++);
        }
        lua_setfield(L, -2, "validation_state");
    }

    if (sf->horiz_base != NULL) {
        lua_newtable(L);
        handle_base(L, sf->horiz_base);
        lua_setfield(L, -2, "horiz_base");
    }
    if (sf->vert_base != NULL) {
        lua_newtable(L);
        handle_base(L, sf->vert_base);
        lua_setfield(L, -2, "vert_base");
    }
    dump_intfield(L, "extrema_bound", sf->extrema_bound);
}

static int ff_make_table(lua_State * L)
{
    SplineFont *sf;
    sf = *(check_isfont(L, 1));
    if (sf == NULL) {
        lua_pushboolean(L, 0);
    } else {
        lua_createtable(L, 0, 60);
        handle_splinefont(L, sf);
    }
    return 1;
}

static void do_ff_info(lua_State * L, SplineFont * sf)
{
    lua_newtable(L);
    dump_stringfield(L, "familyname", sf->familyname);
    dump_stringfield(L, "fontname", sf->fontname);
    dump_stringfield(L, "fullname", sf->fullname);
    dump_floatfield(L, "italicangle", sf->italicangle);
    dump_stringfield(L, "version", sf->version);
    dump_stringfield(L, "weight", sf->weight);

    dump_intfield(L, "units_per_em", sf->units_per_em);

    /*dump_intfield(L, "design_range_bottom", sf->design_range_bottom);*/
    /*dump_intfield(L, "design_range_top", sf->design_range_top);*/
    /*dump_intfield(L, "design_size", sf->design_size);*/

    lua_createtable(L, 0, 40);
    handle_pfminfo(L, sf->pfminfo);
    lua_setfield(L, -2, "pfminfo");

    /* Do we need this ? */

    if (sf->names != NULL) {
         lua_newtable(L);
         handle_ttflangname(L, sf->names);
         lua_setfield(L, -2, "names");
    }
}

typedef enum {
    FK_table_version = 0,
    FK_fontname,
    FK_fullname,
    FK_familyname,
    FK_weight,
    FK_copyright,
    FK_filename,
    FK_version,
    FK_italicangle,
    FK_upos,
    FK_uwidth,
    FK_ascent,
    FK_descent,
    FK_uniqueid,
    FK_glyphcnt,
    FK_glyphmax,
    FK_glyphmin,
    FK_units_per_em,
    FK_lookups,
    FK_glyphs,
    FK_hasvmetrics,
    FK_onlybitmaps,
    FK_serifcheck,
    FK_isserif,
    FK_issans,
    FK_encodingchanged,
    FK_strokedfont,
    FK_use_typo_metrics,
    FK_weight_width_slope_only,
    FK_head_optimized_for_cleartype,
    FK_uni_interp,
    FK_map,
    FK_origname,
    FK_private,
    FK_xuid,
    FK_pfminfo,
    FK_names,
    FK_cidinfo,
    FK_subfonts,
    FK_comments,
    FK_fontlog,
    FK_cvt_names,
    FK_ttf_tables,
    FK_ttf_tab_saved,
    FK_texdata,
    FK_anchor_classes,
    FK_kerns,
    FK_vkerns,
    FK_gsub,
    FK_gpos,
    /* FK_sm, */ /* AAT is not supported anymore */
    FK_features,
    FK_mm,
    FK_chosenname,
    FK_macstyle,
    FK_fondname,
    FK_design_size,
    FK_fontstyle_id,
    FK_fontstyle_name,
    FK_design_range_bottom,
    FK_design_range_top,
    FK_strokewidth,
    FK_mark_classes,
    FK_creationtime,
    FK_modificationtime,
    FK_os2_version,
    FK_sfd_version,
    FK_math,
    FK_validation_state,
    FK_horiz_base,
    FK_vert_base,
    FK_extrema_bound,
    FK_notdef_loc,
} font_key_values;

const char *font_keys[] = {
    "table_version",
    "fontname",
    "fullname",
    "familyname",
    "weight",
    "copyright",
    "filename",
    "version",
    "italicangle",
    "upos",
    "uwidth",
    "ascent",
    "descent",
    "uniqueid",
    "glyphcnt",
    "glyphmax",
    "glyphmin",
    "units_per_em",
    "lookups",
    "glyphs",
    "hasvmetrics",
    "onlybitmaps",
    "serifcheck",
    "isserif",
    "issans",
    "encodingchanged",
    "strokedfont",
    "use_typo_metrics",
    "weight_width_slope_only",
    "head_optimized_for_cleartype",
    "uni_interp",
    "map",
    "origname",
    "private",
    "xuid",
    "pfminfo",
    "names",
    "cidinfo",
    "subfonts",
    "comments",
    "fontlog",
    "cvt_names",
    "ttf_tables",
    "ttf_tab_saved",
    "texdata",
    "anchor_classes",
    "kerns",
    "vkerns",
    "gsub",
    "gpos",
    "features",
    "mm",
    "chosenname",
    "macstyle",
    "fondname",
    "design_size",
    "fontstyle_id",
    "fontstyle_name",
    "design_range_bottom",
    "design_range_top",
    "strokewidth",
    "mark_classes",
    "creationtime",
    "modificationtime",
    "os2_version",
    "sfd_version",
    "math",
    "validation_state",
    "horiz_base",
    "vert_base",
    "extrema_bound",
    "notdef_loc",
    NULL
};

typedef enum {
    GK_name = 0,
    GK_unicode,
    GK_boundingbox,
    GK_vwidth,
    GK_width,
    GK_lsidebearing,
    GK_class,
    GK_kerns,
    GK_vkerns,
    GK_dependents,
    GK_lookups,
    GK_ligatures,
    GK_comment,
    GK_anchors,
    GK_altuni,
    GK_tex_height,
    GK_tex_depth,
    GK_is_extended_shape,
    GK_italic_correction,
    GK_top_accent,
    GK_vert_variants,
    GK_horiz_variants,
    GK_mathkern,
} font_glyph_key_values;

const char *font_glyph_keys[] = {
    "name",
    "unicode",
    "boundingbox",
    "vwidth",
    "width",
    "lsidebearing",
    "class",
    "kerns",
    "vkerns",
    "dependents",
    "lookups",
    "ligatures",
    "comment",
    "anchors",
    "altuni",
    "tex_height",
    "tex_depth",
    "is_extended_shape",
    "italic_correction",
    "top_accent",
    "vert_variants",
    "horiz_variants",
    "mathkern",
    NULL
};

static int ff_fields(lua_State * L)
{
    int i;
    const char **fields = NULL;
    if (is_userdata(L, 1, FONT_METATABLE) ||
	is_userdata(L, 1, FONT_SUBFONT_METATABLE)) {
        fields = font_keys;
    } else if (is_userdata(L, 1, FONT_GLYPH_METATABLE)) {
        fields = font_glyph_keys;
    }
    if (fields != NULL) {
        lua_newtable(L);
        for (i = 0; fields[i] != NULL; i++) {
            lua_pushstring(L, fields[i]);
            lua_rawseti(L, -2, (i + 1));
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int ff_glyphs_index(lua_State * L)
{

    SplineFont *sf;
    int gid = 0;
    int l = -1;
    int fix_notdef = 0; /* obsolete */
    lua_pushstring(L, "__sf");
    lua_rawget(L, 1);
    /* sf = *check_isfont(L, -1); */
    if (!(is_userdata(L, -1, FONT_METATABLE) ||
	  is_userdata(L, -1, FONT_SUBFONT_METATABLE))) {
        return luaL_error(L, "fontloader.__index: expected a (sub)font userdata object\n");
    }
    sf = *((SplineFont **)lua_touserdata(L, -1));

    lua_pop(L, 1);
    gid = luaL_checkinteger(L, 2);
    /* if (gid < sf->glyphmin || gid >= sf->glyphmax) {*/
    if (gid < sf->glyphmin || gid > sf->glyphmax) {
        return luaL_error(L, "fontloader.glyphs.__index: index is invalid\n");
    }
    /* This after-the-fact type discovery is not brilliant,
       I should really add a 'format' key in the structure */
    /*
    if ((sf->origname != NULL) &&
        (strmatch(sf->origname + strlen(sf->origname) - 4, ".pfa") == 0 ||
         strmatch(sf->origname + strlen(sf->origname) - 4, ".pfb") == 0)) {
        fix_notdef = 1;
    }
    */
    /* some code to ensure that the .notdef ends up in slot 0
       (this will actually be enforced by the CFF writer) */
    if (fix_notdef) {
        /* this is obsolete */
        l = notdef_loc(sf);
        /* now l is the .notdef location, adjust gid if needed */
        if (l == sf->glyphcnt) {
            /* no .notdef at all, will be created at zero */
            if (gid == 0) {
                /* we ask for .notdef but .notdef was added at end */
                gid = l;
            } else {
                /* f.glyphs[gid] == sf->glyphs[gid-1] */
                gid--;
            }
        } else if (l != 0) {
            if (gid == 0) {
                gid = l;
            } else if (gid < l) {
                gid--;
            }
        }
    }

    if (sf->glyphs[gid] && sf->glyphs[gid] != (struct splinechar *)-1) {
        lua_ff_pushglyph(L, sf->glyphs[gid]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int ff_glyph_index(lua_State * L)
{
    struct splinechar *glyph;
    int key;
    glyph = *check_isglyph(L, 1);
    if (glyph == NULL) {
        return luaL_error(L, "fontloader.glyph.__index: glyph is nonexistent\n");
    }
    if (lua_type(L, 2) != LUA_TSTRING) {  /* 1 == 'font' */
        return luaL_error(L, "fontloader.glyph.__index: can only be indexed by string\n");
    }
    key = luaL_checkoption(L, 2, NULL, font_glyph_keys);
    switch (key) {
        case GK_name:
            lua_pushstring(L, glyph->name);
            break;
        case GK_unicode:
            lua_pushnumber(L, glyph->unicodeenc);
            break;
        case GK_boundingbox:
            if (glyph->xmax == 0 && glyph->ymax == 0 && glyph->xmin == 0 && glyph->ymin == 0) {
                DBounds bb;
                SplineCharFindBounds(glyph, &bb);
                glyph->xmin = bb.minx;
                glyph->ymin = bb.miny;
                glyph->xmax = bb.maxx;
                glyph->ymax = bb.maxy;
            }
            lua_createtable(L, 4, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, glyph->xmin);
            lua_rawset(L, -3);
            lua_pushnumber(L, 2);
            lua_pushnumber(L, glyph->ymin);
            lua_rawset(L, -3);
            lua_pushnumber(L, 3);
            lua_pushnumber(L, glyph->xmax);
            lua_rawset(L, -3);
            lua_pushnumber(L, 4);
            lua_pushnumber(L, glyph->ymax);
            lua_rawset(L, -3);
            break;
        case GK_vwidth:
            lua_pushnumber(L, glyph->vwidth);
            break;
        case GK_width:
            lua_pushnumber(L, glyph->width);
            break;
        case GK_lsidebearing:
            lua_pushnumber(L, glyph->lsidebearing);
            break;
        case GK_class:
            if (glyph->glyph_class > 0) {
                lua_pushstring(L, glyph_class_enum[glyph->glyph_class]);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_kerns:
            if (glyph->kerns != NULL) {
                lua_newtable(L);
                handle_kernpair(L, glyph->kerns);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_vkerns:
            if (glyph->vkerns != NULL) {
                lua_newtable(L);
                handle_kernpair(L, glyph->vkerns);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_dependents:
            if (glyph->dependents != NULL) {
                lua_newtable(L);
                handle_splinecharlist(L, glyph->dependents);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_lookups:
            if (glyph->possub != NULL) {
                lua_newtable(L);
                handle_generic_pst(L, glyph->possub);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_ligatures:
            if (glyph->ligofme != NULL) {
                lua_newtable(L);
                handle_liglist(L, glyph->ligofme);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_comment:
            lua_pushstring(L, glyph->comment);
            break;
        case GK_anchors:
            if (glyph->anchor != NULL) {
                lua_newtable(L);
                handle_anchorpoint(L, glyph->anchor);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_altuni:
            if (glyph->altuni != NULL) {
                lua_newtable(L);
                handle_altuni(L, glyph->altuni);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_tex_height:
            if (glyph->tex_height != TEX_UNDEF) {
                lua_pushnumber(L, glyph->tex_height);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_tex_depth:
            if (glyph->tex_height != TEX_UNDEF) {
                lua_pushnumber(L, glyph->tex_depth);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_is_extended_shape:
            lua_pushnumber(L, glyph->is_extended_shape);
            break;
        case GK_italic_correction:
            if (glyph->italic_correction != TEX_UNDEF) {
                lua_pushnumber(L, glyph->italic_correction);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_top_accent:
            if (glyph->top_accent_horiz != TEX_UNDEF) {
                lua_pushnumber(L, glyph->top_accent_horiz);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_vert_variants:
            if (glyph->vert_variants != NULL) {
                lua_newtable(L);
                handle_glyphvariants(L, glyph->vert_variants);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_horiz_variants:
            if (glyph->horiz_variants != NULL) {
                lua_newtable(L);
                handle_glyphvariants(L, glyph->horiz_variants);
            } else {
                lua_pushnil(L);
            }
            break;
        case GK_mathkern:
            if (glyph->mathkern != NULL) {
                lua_newtable(L);
                handle_mathkern(L, glyph->mathkern);
            } else {
                lua_pushnil(L);
            }
            break;
        default:
            lua_pushnil(L);
    }
    return 1;
}

static int ff_index(lua_State * L)
{
    SplineFont *sf;
    int k, key;
    /* sf = *check_isfont(L, 1); */
    if (!(is_userdata(L, 1, FONT_METATABLE) || is_userdata(L, 1, FONT_SUBFONT_METATABLE))) {
        return luaL_error(L, "fontloader.__index: expected a (sub)font userdata object\n");
    }
    sf = *((SplineFont **)lua_touserdata(L, 1));
    if (sf == NULL) {
        lua_pushnil(L);
        return 1;
    }
    if (lua_type(L, 2) != LUA_TSTRING) {  /* 1 == 'font' */
        return luaL_error(L, "fontloader.__index: can only be indexed by string\n");
    }
    key = luaL_checkoption(L, 2, NULL, font_keys);
    switch (key) {
        case FK_table_version:
            lua_pushstring(L, LUA_OTF_VERSION);
            break;
        case FK_fontname:
            lua_pushstring(L, sf->fontname);
            break;
        case FK_fullname:
            lua_pushstring(L, sf->fullname);
            break;
        case FK_familyname:
            lua_pushstring(L, sf->familyname);
            break;
        case FK_weight:
            lua_pushstring(L, sf->weight);
            break;
        case FK_copyright:
            lua_pushstring(L, sf->copyright);
            break;
        case FK_filename:
            lua_pushstring(L, sf->filename);
            break;
        case FK_version:
            lua_pushstring(L, sf->version);
            break;
        case FK_italicangle:
            lua_pushnumber(L, sf->italicangle);
            break;
        case FK_upos:
            lua_pushnumber(L, sf->upos);
            break;
        case FK_uwidth:
            lua_pushnumber(L, sf->uwidth);
            break;
        case FK_ascent:
            lua_pushnumber(L, sf->ascent);
            break;
        case FK_descent:
            lua_pushnumber(L, sf->descent);
            break;
        case FK_uniqueid:
            lua_pushnumber(L, sf->uniqueid);
            break;
        case FK_glyphcnt:
            if (sf->glyphcnt > 0) {
                lua_pushnumber(L, sf->glyphmax - sf->glyphmin + 1);
            } else {
                lua_pushnumber(L, 0);
            }
            break;
        case FK_glyphmax:
            lua_pushnumber(L, sf->glyphmax - 1);
            break;
        case FK_glyphmin:
            lua_pushnumber(L, sf->glyphmin);
            break;
        case FK_units_per_em:
            lua_pushnumber(L, sf->units_per_em);
            break;
        case FK_lookups:
            if (sf->possub != NULL) {
                lua_newtable(L);
                handle_generic_fpst(L, sf->possub);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_glyphs:
            lua_newtable(L);        /* the virtual glyph table */
            lua_pushstring(L, "__sf");
            lua_pushvalue(L, 1);    /* that is our font */
            lua_rawset(L, -3);
            luaL_getmetatable(L, FONT_GLYPHS_METATABLE);
            lua_setmetatable(L, -2);        /* assign the metatable */
            break;
        case FK_hasvmetrics:
            lua_pushnumber(L, sf->hasvmetrics);
            break;
        case FK_onlybitmaps:
            lua_pushnumber(L, sf->onlybitmaps);
            break;
        case FK_serifcheck:
            lua_pushnumber(L, sf->serifcheck);
            break;
        case FK_isserif:
            lua_pushnumber(L, sf->isserif);
            break;
        case FK_issans:
            lua_pushnumber(L, sf->issans);
            break;
        case FK_encodingchanged:
            lua_pushnumber(L, sf->encodingchanged);
            break;
        case FK_strokedfont:
            lua_pushnumber(L, sf->strokedfont);
            break;
        case FK_use_typo_metrics:
            lua_pushnumber(L, sf->use_typo_metrics);
            break;
        case FK_weight_width_slope_only:
            lua_pushnumber(L, sf->weight_width_slope_only);
            break;
        case FK_head_optimized_for_cleartype:
            lua_pushnumber(L, sf->head_optimized_for_cleartype);
            break;
        case FK_uni_interp:
            lua_pushstring(L, uni_interp_enum[(sf->uni_interp + 1)]);
            break;
        case FK_map:
            if (sf->map != NULL) {
                lua_newtable(L);
                handle_encmap(L, sf->map, 0); /* notdef_loc(sf)); */
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_origname:
            lua_pushstring(L, sf->origname);
            break;
        case FK_private:
            if (sf->private != NULL) {
                lua_newtable(L);
                handle_psdict(L, sf->private);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_xuid:
            lua_pushstring(L, sf->xuid);
            break;
        case FK_pfminfo:
            lua_createtable(L, 0, 40);
            handle_pfminfo(L, sf->pfminfo);
            break;
        case FK_names:
            if (sf->names != NULL) {
                lua_newtable(L);
                handle_ttflangname(L, sf->names);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_cidinfo:
            lua_createtable(L, 0, 4);
            dump_stringfield(L, "registry", sf->cidregistry);
            dump_stringfield(L, "ordering", sf->ordering);
            dump_intfield(L, "version", sf->cidversion);
            dump_intfield(L, "supplement", sf->supplement);
            break;
        case FK_subfonts:
            if (sf->subfontcnt > 0) {
                lua_createtable(L, sf->subfontcnt, 0);
                for (k = 0; k < sf->subfontcnt; k++) {
                    lua_ff_pushsubfont(L, sf->subfonts[k]);
                    lua_rawseti(L, -2, (k + 1));
                }
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_comments:
            lua_pushstring(L, sf->comments);
            break;
        case FK_fontlog:
            lua_pushstring(L, sf->fontlog);
            break;
        case FK_cvt_names:
            if (sf->cvt_names != NULL) {
                lua_newtable(L);
                for (k = 0; sf->cvt_names[k] != END_CVT_NAMES; ++k) {
                    lua_pushstring(L, sf->cvt_names[k]);
                    lua_rawseti(L, -2, (k + 1));
                }
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_ttf_tables:
            if (sf->ttf_tables != NULL) {
                lua_newtable(L);
                handle_ttf_table(L, sf->ttf_tables);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_ttf_tab_saved:
            if (sf->ttf_tab_saved != NULL) {
                lua_newtable(L);
                handle_ttf_table(L, sf->ttf_tab_saved);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_texdata:
            if (sf->texdata.type != tex_unset) {
                lua_newtable(L);
                dump_enumfield(L, "type", sf->texdata.type, tex_type_enum);
                lua_newtable(L);
                for (k = 0; k < 22; k++) {
                    lua_pushnumber(L, k);
                    lua_pushnumber(L, sf->texdata.params[k]);
                    lua_rawset(L, -3);
                }
                lua_setfield(L, -2, "params");
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_anchor_classes:
            if (sf->anchor != NULL) {
                lua_newtable(L);
                handle_anchorclass(L, sf->anchor);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_kerns:
            if (sf->kerns != NULL) {
                lua_newtable(L);
                handle_kernclass(L, sf->kerns, NULL);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_vkerns:
            if (sf->vkerns != NULL) {
                lua_newtable(L);
                handle_kernclass(L, sf->vkerns, NULL);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_gsub:
            if (sf->gsub_lookups != NULL) {
                lua_newtable(L);
                handle_lookup(L, sf->gsub_lookups, sf);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_gpos:
            if (sf->gpos_lookups != NULL) {
                lua_newtable(L);
                handle_lookup(L, sf->gpos_lookups, sf);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_mm:
            if (sf->mm != NULL) {
                lua_newtable(L);
                handle_mmset(L, sf->mm);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_chosenname:
            lua_pushstring(L, sf->chosenname);
            break;
        case FK_macstyle:
            lua_pushnumber(L, sf->macstyle);
            break;
        case FK_fondname:
            lua_pushstring(L, sf->fondname);
            break;
        case FK_design_size:
            lua_pushnumber(L, sf->design_size);
            break;
        case FK_fontstyle_id:
            lua_pushnumber(L, sf->fontstyle_id);
            break;
        case FK_fontstyle_name:
            if (sf->fontstyle_name != NULL) {
                lua_newtable(L);
                handle_otfname(L, sf->fontstyle_name);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_design_range_bottom:
            lua_pushnumber(L, sf->design_range_bottom);
            break;
        case FK_design_range_top:
            lua_pushnumber(L, sf->design_range_top);
            break;
        case FK_strokewidth:
            lua_pushnumber(L, sf->strokewidth);
            break;
        case FK_mark_classes:
            if (sf->mark_class_cnt > 0) {
                lua_newtable(L);
                for (k = 0; k < sf->mark_class_cnt; k++) {
                    if (sf->mark_class_names[k] != NULL) {
                        lua_pushstring(L, sf->mark_class_names[k]);
                        lua_pushstring(L, sf->mark_classes[k]);
                        lua_rawset(L, -3);
                    }
                }
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_creationtime:
            lua_pushnumber(L, sf->creationtime);
            break;
        case FK_modificationtime:
            lua_pushnumber(L, sf->modificationtime);
            break;
        case FK_os2_version:
            lua_pushnumber(L, sf->os2_version);
            break;
        case FK_sfd_version:
            lua_pushnumber(L, sf->sfd_version);
            break;
        case FK_math:
            if (sf->MATH != NULL) {
                lua_newtable(L);
                handle_MATH(L, sf->MATH);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_validation_state:
            if (sf->loadvalidation_state != 0) {
                int val, st;
                lua_newtable(L);
                val = 1;
                st = sf->loadvalidation_state;
                if (st & lvs_bad_ps_fontname) {
                    lua_pushliteral(L, "bad_ps_fontname");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_glyph_table) {
                    lua_pushliteral(L, "bad_glyph_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_cff_table) {
                    lua_pushliteral(L, "bad_cff_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_metrics_table) {
                    lua_pushliteral(L, "bad_metrics_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_cmap_table) {
                    lua_pushliteral(L, "bad_cmap_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_bitmaps_table) {
                    lua_pushliteral(L, "bad_bitmaps_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_gx_table) {
                    lua_pushliteral(L, "bad_gx_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_ot_table) {
                    lua_pushliteral(L, "bad_ot_table");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_os2_version) {
                    lua_pushliteral(L, "bad_os2_version");
                    lua_rawseti(L, -2, val++);
                }
                if (st & lvs_bad_sfnt_header) {
                    lua_pushliteral(L, "bad_sfnt_header");
                    lua_rawseti(L, -2, val++);
                }
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_horiz_base:
            if (sf->horiz_base != NULL) {
                lua_newtable(L);
                handle_base(L, sf->horiz_base);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_vert_base:
            if (sf->vert_base != NULL) {
                lua_newtable(L);
                handle_base(L, sf->vert_base);
            } else {
                lua_pushnil(L);
            }
            break;
        case FK_extrema_bound:
            lua_pushnumber(L, sf->extrema_bound);
            break;
        case FK_notdef_loc:
            lua_pushinteger(L, notdef_loc(sf));
            break;
        default:
            /* can't actually happen, |luaL_checkoption| raises an error instead */
            lua_pushnil(L);
            break;
    }
    return 1;
}

static int ff_info(lua_State * L)
{
    SplineFont *sf;
    FILE *l;
    int i;
    const char *fontname;
    char *fontnamecopy;
    int openflags = 1;
    fontname = luaL_checkstring(L, 1);
    if (!strlen(fontname)) {
        lua_pushnil(L);
        lua_pushfstring(L, "font loading failed: empty string given\n", fontname);
        return 2;
    }
    /* test fontname for existance */
    if ((l = fopen(fontname, "r"))) {
        recorder_record_input(fontname);
        fclose(l);
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "font loading failed for %s (read error)\n", fontname);
        return 2;
    }
    gww_error_count = 0;
    fontnamecopy = xstrdup(fontname);
    sf = ReadSplineFontInfo(fontnamecopy, openflags);
    free(fontnamecopy);
    if (gww_error_count > 0)
        gwwv_errors_free();

    if (sf == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "font loading failed for %s\n", fontname);
        return 2;
    } else {
        if (sf->next != NULL) {
            SplineFont *sf_next;
            i = 1;
            lua_newtable(L);
            while (sf) {
                do_ff_info(L, sf);
                lua_rawseti(L, -2, i);
                i++;
                sf_next = sf->next;
                EncMapFree(sf->map);
                SplineFontFree(sf);
                sf = sf_next;
            }
        } else {
            do_ff_info(L, sf);
            EncMapFree(sf->map);
            SplineFontFree(sf);
        }
    }
    return 1;
}

static void ff_do_cff(SplineFont * sf, char *filename, unsigned char **buf,
                      int *bufsiz)
{
    FILE *f;
    int32 *bsizes = NULL;
    int flags = ps_flag_nocffsugar + ps_flag_nohints;
    EncMap *map;

    map = EncMap1to1(sf->glyphcnt);

    if (WriteTTFFont
        (filename, sf, ff_cff, bsizes, bf_none, flags, map, ly_fore)) {
        /* success */
        f = fopen(filename, "rb");
        recorder_record_input(filename);
        readbinfile(f, buf, bufsiz);
        /*fprintf(stdout,"\n%s => CFF, size: %d\n", sf->filename, *bufsiz); */
        fclose(f);
        return;
    }
    formatted_error("fontloader","%s to CFF conversion failed", sf->filename);
}

/* exported for writecff.c */

int ff_createcff(char *file, unsigned char **buf, int *bufsiz)
{
    SplineFont *sf;
    int k;
    char s[] = "tempfile.cff";
    int openflags = 1;
    int notdefpos = 0;
    sf = ReadSplineFont(file, openflags);
    if (sf) {
        /* this is not the best way. nicer to have no temp file at all */
        ff_do_cff(sf, s, buf, bufsiz);
        for (k = 0; k < sf->glyphcnt; k++) {
            if (sf->glyphs[k] && strcmp(sf->glyphs[k]->name, ".notdef") == 0) {
                notdefpos = k;
                break;
            }
        }
        remove(s);
        EncMapFree(sf->map);
        SplineFontFree(sf);
    }
    return notdefpos;
}

int ff_get_ttc_index(char *ffname, char *psname)
{
    SplineFont *sf;
    int i = 0;
    int openflags = 1;
    int index = -1;

    sf = ReadSplineFontInfo((char *) ffname, openflags);
    if (sf == NULL) {
        normal_error("fontloader","font loading failed unexpectedly");
    }
    while (sf != NULL) {
        if (strcmp(sf->fontname, psname) == 0) {
            index = i;
        }
        i++;
        sf = sf->next;
    }
    if (index>=0)
        return (i-index-1);
    return -1;
}

static struct luaL_Reg fllib[] = {
    {"open", ff_open},
    {"info", ff_info},
    {"close", ff_close},
    {"fields", ff_fields},
    {"apply_afmfile", ff_apply_afmfile},
    {"apply_featurefile", ff_apply_featurefile},
    {"to_table", ff_make_table},
    {NULL, NULL}
};

static const struct luaL_Reg fflib_m[] = {
    {"__gc", ff_close},         /* doesnt work yet! */
    {"__index", ff_index},
    {NULL, NULL}                /* sentinel */
};

int luaopen_ff(lua_State * L)
{
    static char coord_sep_string[] = ",";
    static char SaveTablesPref_string[] = "VORG,JSTF,acnt,bsln,fdsc,fmtx,hsty,just,trak,Zapf,LINO";
    InitSimpleStuff();
    setlocale(LC_ALL, "C");     /* undo whatever InitSimpleStuff has caused */
    coord_sep = coord_sep_string;
    FF_SetUiInterface(&luaui_interface);
    default_encoding = FindOrMakeEncoding("ISO8859-1");
    SaveTablesPref = SaveTablesPref_string;
    luaL_newmetatable(L, FONT_METATABLE);
    luaL_register(L, NULL, fflib_m);

    /* virtual subfont table */
    luaL_newmetatable(L, FONT_SUBFONT_METATABLE);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ff_index);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    /* virtual glyphs table */
    luaL_newmetatable(L, FONT_GLYPHS_METATABLE);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ff_glyphs_index);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    /* virtual glyph table */
    luaL_newmetatable(L, FONT_GLYPH_METATABLE);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ff_glyph_index);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    luaL_openlib(L, "fontloader", fllib, 0);

    return 1;
}
