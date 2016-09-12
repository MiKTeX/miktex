% luafont.w
%
% Copyright 2006-2011 Taco Hoekwater <taco@@luatex.org>
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

@ @c
#include "ptexlib.h"
#include "lua/luatex-api.h"

#define noVERBOSE

/* todo: also keys */

const char *font_type_strings[] = {
    "unknown", "virtual", "real", NULL
};

const char *font_format_strings[] = {
    "unknown", "type1", "type3", "truetype", "opentype", NULL
};

const char *font_embedding_strings[] = {
    "unknown", "no", "subset", "full", NULL
};

const char *ligature_type_strings[] = {
    "=:", "=:|", "|=:", "|=:|", "", "=:|>", "|=:>", "|=:|>", "", "", "", "|=:|>>", NULL
};

const char *MATH_param_names[] = {
    "nil",
    "ScriptPercentScaleDown",
    "ScriptScriptPercentScaleDown",
    "DelimitedSubFormulaMinHeight",
    "DisplayOperatorMinHeight",
    "MathLeading",
    "AxisHeight",
    "AccentBaseHeight",
    "FlattenedAccentBaseHeight",
    "SubscriptShiftDown",
    "SubscriptTopMax",
    "SubscriptBaselineDropMin",
    "SuperscriptShiftUp",
    "SuperscriptShiftUpCramped",
    "SuperscriptBottomMin",
    "SuperscriptBaselineDropMax",
    "SubSuperscriptGapMin",
    "SuperscriptBottomMaxWithSubscript",
    "SpaceAfterScript",
    "UpperLimitGapMin",
    "UpperLimitBaselineRiseMin",
    "LowerLimitGapMin",
    "LowerLimitBaselineDropMin",
    "StackTopShiftUp",
    "StackTopDisplayStyleShiftUp",
    "StackBottomShiftDown",
    "StackBottomDisplayStyleShiftDown",
    "StackGapMin",
    "StackDisplayStyleGapMin",
    "StretchStackTopShiftUp",
    "StretchStackBottomShiftDown",
    "StretchStackGapAboveMin",
    "StretchStackGapBelowMin",
    "FractionNumeratorShiftUp",
    "FractionNumeratorDisplayStyleShiftUp",
    "FractionDenominatorShiftDown",
    "FractionDenominatorDisplayStyleShiftDown",
    "FractionNumeratorGapMin",
    "FractionNumeratorDisplayStyleGapMin",
    "FractionRuleThickness",
    "FractionDenominatorGapMin",
    "FractionDenominatorDisplayStyleGapMin",
    "SkewedFractionHorizontalGap",
    "SkewedFractionVerticalGap",
    "OverbarVerticalGap",
    "OverbarRuleThickness",
    "OverbarExtraAscender",
    "UnderbarVerticalGap",
    "UnderbarRuleThickness",
    "UnderbarExtraDescender",
    "RadicalVerticalGap",
    "RadicalDisplayStyleVerticalGap",
    "RadicalRuleThickness",
    "RadicalExtraAscender",
    "RadicalKernBeforeDegree",
    "RadicalKernAfterDegree",
    "RadicalDegreeBottomRaisePercent",
    "MinConnectorOverlap",
    "SubscriptShiftDownWithSuperscript",
    "FractionDelimiterSize",
    "FractionDelimiterDisplayStyleSize",
    "NoLimitSubFactor",
    "NoLimitSupFactor",
    NULL,
};

/* here for now, may be useful elsewhere */

int ff_checkoption (lua_State *L, int narg, const char *def, const char *const lst[]);

int ff_checkoption (lua_State *L, int narg, const char *def, const char *const lst[]) {
    const char *name = (def) ? luaL_optstring(L, narg, def) : luaL_checkstring(L, narg);
    int i;
    for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
        return i;
    return -1;
}

#define dump_intfield(L,n,c) \
    lua_push_string_by_name(L,n); \
    lua_pushinteger(L, c); \
    lua_rawset(L, -3); \

#define dump_stringfield(L,n,c) \
    lua_push_string_by_name(L,n); \
    lua_pushstring(L, c); \
    lua_rawset(L, -3);

#define dump_booleanfield(L,n,c) \
    lua_push_string_by_name(L,n); \
    lua_pushboolean(L, c); \
    lua_rawset(L, -3);

static void dump_math_kerns(lua_State * L, charinfo * co, int l, int id)
{
    int i;
    for (i = 0; i < l; i++) {
        lua_newtable(L);
        if (id==top_left_kern) {
            dump_intfield(L, height, co->top_left_math_kern_array[(2*i)]);
            dump_intfield(L, kern,   co->top_left_math_kern_array[(2*i)+1]);
        } else if (id==top_right_kern) {
            dump_intfield(L, height, co->top_right_math_kern_array[(2*i)]);
            dump_intfield(L, kern,   co->top_right_math_kern_array[(2*i)+1]);
        } else if (id==bottom_right_kern) {
            dump_intfield(L, height, co->bottom_right_math_kern_array[(2*i)]);
            dump_intfield(L, kern,   co->bottom_right_math_kern_array[(2*i)+1]);
        } else if (id==bottom_left_kern) {
            dump_intfield(L, height, co->bottom_left_math_kern_array[(2*i)]);
            dump_intfield(L, kern,   co->bottom_left_math_kern_array[(2*i)+1]);
        }
        lua_rawseti(L, -2, (i + 1));
    }
}

static void font_char_to_lua(lua_State * L, internal_font_number f, charinfo * co)
{
    liginfo *l;
    kerninfo *ki;

    lua_createtable(L, 0, 10);

    dump_intfield(L,width,get_charinfo_width(co));
    dump_intfield(L,height,get_charinfo_height(co));
    dump_intfield(L,depth,get_charinfo_depth(co));

    if (get_charinfo_italic(co) != 0) {
       dump_intfield(L,italic,get_charinfo_italic(co));
    }
    if (get_charinfo_vert_italic(co) != 0) {
       dump_intfield(L,vert_italic,get_charinfo_vert_italic(co));
    }
    if (get_charinfo_top_accent(co) !=0 && get_charinfo_top_accent(co) != INT_MIN) {
       dump_intfield(L,top_accent,get_charinfo_top_accent(co));
    }
    if (get_charinfo_bot_accent(co) != 0 && get_charinfo_bot_accent(co) != INT_MIN) {
       dump_intfield(L,bot_accent,get_charinfo_bot_accent(co));
    }
    if (get_charinfo_ef(co) != 1000) {
        dump_intfield(L,expansion_factor,get_charinfo_ef(co));
    }
    if (get_charinfo_lp(co) != 0) {
        dump_intfield(L,left_protruding,get_charinfo_lp(co));
    }
    if (get_charinfo_rp(co) != 0) {
        dump_intfield(L,right_protruding,get_charinfo_rp(co));
    }
    if (font_encodingbytes(f) == 2) {
        dump_intfield(L,index,get_charinfo_index(co));
    }
    if (get_charinfo_name(co) != NULL) {
        dump_stringfield(L,name,get_charinfo_name(co));
    }
    if (get_charinfo_tounicode(co) != NULL) {
        dump_stringfield(L,tounicode,get_charinfo_tounicode(co));
    }
    if (get_charinfo_tag(co) == list_tag) {
        dump_intfield(L,next,get_charinfo_remainder(co));
    }
    if (get_charinfo_used(co)) {
        dump_booleanfield(L,used,(get_charinfo_used(co) ? true : false));
    }
    if (get_charinfo_tag(co) == ext_tag) {
        extinfo *h;
        h = get_charinfo_hor_variants(co);
        if (h != NULL) {
            int i = 1;
            lua_push_string_by_name(L,horiz_variants);
            lua_newtable(L);
            while (h != NULL) {
                lua_createtable(L, 0, 5);
                dump_intfield(L, glyph, h->glyph);
                dump_intfield(L, extender, h->extender);
                dump_intfield(L, start, h->start_overlap);
                dump_intfield(L, end, h->end_overlap);
                dump_intfield(L, advance, h->advance);
                lua_rawseti(L, -2, i);
                i++;
                h = h->next;
            }
            lua_rawset(L, -3);
        }
        h = get_charinfo_vert_variants(co);
        if (h != NULL) {
            int i = 1;
            lua_push_string_by_name(L,vert_variants);
            lua_newtable(L);
            while (h != NULL) {
                lua_createtable(L, 0, 5);
                dump_intfield(L, glyph, h->glyph);
                dump_intfield(L, extender, h->extender);
                dump_intfield(L, start, h->start_overlap);
                dump_intfield(L, end, h->end_overlap);
                dump_intfield(L, advance, h->advance);
                lua_rawseti(L, -2, i);
                i++;
                h = h->next;
            }
            lua_rawset(L, -3);
        }
    }
    ki = get_charinfo_kerns(co);
    if (ki != NULL) {
        int i;
        lua_push_string_by_name(L,kerns);
        lua_createtable(L, 10, 1);
        for (i = 0; !kern_end(ki[i]); i++) {
            if (kern_disabled(ki[i])) {
                /* skip like in lookup */
            } else {
                lua_rawgeti(L, -1, kern_char(ki[i]));
                if (lua_type(L,-1) == LUA_TNIL) {
                    lua_pop(L,1);
                    if (kern_char(ki[i]) == right_boundarychar) {
                        lua_push_string_by_name(L,right_boundary);
                    } else {
                        lua_pushinteger(L, kern_char(ki[i]));
                    }
                    lua_pushinteger(L, kern_kern(ki[i]));
                    lua_rawset(L, -3);
                } else {
                    /* first one wins */
                    lua_pop(L,1);
                }
            }
        }
        lua_rawset(L, -3);
    }
    l = get_charinfo_ligatures(co);
    if (l != NULL) {
        int i;
        lua_push_string_by_name(L,ligatures);
        lua_createtable(L, 10, 1);
        for (i = 0; !lig_end(l[i]); i++) {
            if (lig_char(l[i]) == right_boundarychar) {
                lua_push_string_by_name(L,right_boundary);
            } else {
                lua_pushinteger(L, lig_char(l[i]));
            }
            lua_createtable(L, 0, 2);
            lua_push_string_by_name(L,type);
            lua_pushinteger(L, lig_type(l[i]));
            lua_rawset(L, -3);
            lua_push_string_by_name(L,char);
            lua_pushinteger(L, lig_replacement(l[i]));
            lua_rawset(L, -3);
            lua_rawset(L, -3);
        }
        lua_rawset(L, -3);
    }

    lua_push_string_by_name(L,mathkern);
    lua_newtable(L);
    {
    int i, j;
    i = get_charinfo_math_kerns(co, top_right_kern);
    j = 0;
    if (i > 0) {
        j++;
        lua_push_string_by_name(L,top_right);
        lua_newtable(L);
        dump_math_kerns(L, co, i, top_right_kern);
        lua_rawset(L, -3);
    }
    i = get_charinfo_math_kerns(co, top_left_kern);
    if (i > 0) {
        j++;
        lua_push_string_by_name(L,top_left);
        lua_newtable(L);
        dump_math_kerns(L, co, i, top_left_kern);
        lua_rawset(L, -3);
    }
    i = get_charinfo_math_kerns(co, bottom_right_kern);
    if (i > 0) {
        j++;
        lua_push_string_by_name(L,bottom_right);
        lua_newtable(L);
        dump_math_kerns(L, co, i, bottom_right_kern);
        lua_rawset(L, -3);
    }
    i = get_charinfo_math_kerns(co, bottom_left_kern);
    if (i > 0) {
        j++;
        lua_push_string_by_name(L,bottom_left);
        lua_newtable(L);
        dump_math_kerns(L, co, i, bottom_left_kern);
        lua_rawset(L, -3);
    }
    if (j > 0)
        lua_rawset(L, -3);
    else
        lua_pop(L, 2);
    }
}

static void write_lua_parameters(lua_State * L, int f)
{
    int k;
    lua_push_string_by_name(L,parameters);
    lua_newtable(L);
    for (k = 1; k <= font_params(f); k++) {
        switch (k) {
            case slant_code:
                dump_intfield(L,slant,font_param(f, k));
                break;
            case space_code:
                dump_intfield(L,space,font_param(f, k));
                break;
            case space_stretch_code:
                dump_intfield(L,space_stretch,font_param(f, k));
                break;
            case space_shrink_code:
                dump_intfield(L,space_shrink,font_param(f, k));
                break;
            case x_height_code:
                dump_intfield(L,x_height,font_param(f, k));
                break;
            case quad_code:
                dump_intfield(L,quad,font_param(f, k));
                break;
            case extra_space_code:
                dump_intfield(L,extra_space,font_param(f, k));
                break;
            default:
                lua_pushinteger(L, font_param(f, k));
                lua_rawseti(L, -2, k);
        }
    }
    lua_rawset(L, -3);
}

@ @c
static void write_lua_math_parameters(lua_State * L, int f)
{
    int k;
    lua_push_string_by_name(L,MathConstants);
    lua_newtable(L);
    for (k = 1; k <= font_math_params(f); k++) {
        lua_pushinteger(L, font_math_param(f, k));
        if (k <= MATH_param_max) {
            lua_setfield(L, -2, MATH_param_names[k]);
        } else {
            lua_rawseti(L, -2, k);
        }
    }
    lua_rawset(L, -3);
}

int font_to_lua(lua_State * L, int f)
{
    int k;
    charinfo *co;
    if (font_cache_id(f) > 0) {
        /* fetch the table from the registry if it was
           saved there by |font_from_lua()| */
        lua_rawgeti(L, LUA_REGISTRYINDEX, font_cache_id(f));
        /* fontdimens can be changed from tex code */
        write_lua_parameters(L, f);
        return 1;
    }

    lua_newtable(L);
    lua_push_string_by_name(L,name);
    lua_pushstring(L, font_name(f));
    lua_rawset(L, -3);
    if (font_area(f) != NULL) {
        dump_stringfield(L,area,font_area(f));
    }
    if (font_filename(f) != NULL) {
        dump_stringfield(L,filename,font_filename(f));
    }
    if (font_fullname(f) != NULL) {
        dump_stringfield(L,fullname,font_fullname(f));
    }
    if (font_psname(f) != NULL) {
        dump_stringfield(L,psname,font_psname(f));
    }
    if (font_encodingname(f) != NULL) {
        dump_stringfield(L,encodingname,font_encodingname(f));
    }

    dump_booleanfield(L,used,(font_used(f) ? true : false));
    dump_stringfield(L,type,font_type_strings[font_type(f)]);
    dump_stringfield(L,format,font_format_strings[font_format(f)]);
    dump_stringfield(L,embedding,font_embedding_strings[font_embedding(f)]);

    dump_intfield(L,units_per_em,font_units_per_em(f));
    dump_intfield(L,size,font_size(f));
    dump_intfield(L,designsize,font_dsize(f));
    dump_intfield(L,checksum,font_checksum(f));
    dump_intfield(L,slant,font_slant(f));
    dump_intfield(L,extend,font_extend(f));
    dump_intfield(L,direction,font_natural_dir(f));
    dump_intfield(L,encodingbytes,font_encodingbytes(f));
    dump_booleanfield(L,oldmath,font_oldmath(f));
    dump_intfield(L,tounicode,font_tounicode(f));

    /* the next one is read only */
    if (font_max_shrink(f) != 0) {
        dump_intfield(L,shrink,font_max_shrink(f));
    }
    if (font_max_stretch(f) != 0) {
        dump_intfield(L,stretch,font_max_stretch(f));
    }
    if (font_step(f) != 0) {
        dump_intfield(L,step,font_step(f));
    }
    if (font_auto_expand(f) != 0) {
        dump_booleanfield(L,auto_expand,font_auto_expand(f));
    }
    if (pdf_font_attr(f) != 0) {
        char *s = makecstring(pdf_font_attr(f));
        dump_stringfield(L,attributes,s);
        free(s);
    }

    /* params */
    write_lua_parameters(L, f);
    write_lua_math_parameters(L, f);

    /* chars */
    lua_push_string_by_name(L,characters);
    lua_createtable(L, font_tables[f]->charinfo_size, 0);       /* all characters */
    if (has_left_boundary(f)) {
        co = get_charinfo(f, left_boundarychar);
        lua_push_string_by_name(L,left_boundary);
        font_char_to_lua(L, f, co);
        lua_rawset(L, -3);
    }
    if (has_right_boundary(f)) {
        co = get_charinfo(f, right_boundarychar);
        lua_push_string_by_name(L,right_boundary);
        font_char_to_lua(L, f, co);
        lua_rawset(L, -3);
    }
    for (k = font_bc(f); k <= font_ec(f); k++) {
        if (quick_char_exists(f, k)) {
            lua_pushinteger(L, k);
            co = get_charinfo(f, k);
            font_char_to_lua(L, f, co);
            lua_rawset(L, -3);
        }
    }
    lua_rawset(L, -3);

    if (font_cache_id(f) == 0) {        /* renew */
        int r;
        lua_pushvalue(L, -1);
        r = luaL_ref(L, LUA_REGISTRYINDEX);     /* pops the table */
        set_font_cache_id(f, r);
    }
    return 1;
}

#define count_hash_items(L,name,n) \
    n = 0; \
    lua_key_rawgeti(name); \
    if (lua_type(L, -1) == LUA_TTABLE) { \
        lua_pushnil(L); \
        while (lua_next(L, -2) != 0) { \
            n++; \
            lua_pop(L, 1); \
        } \
    } \
    if (n) { \
        /* keep table on stack */ \
    } else{ \
        lua_pop(L, 1); \
    }

@ @c
#define streq(a,b) (strcmp(a,b)==0)

#define append_packet(k) { *(cp++) = (eight_bits) (k); }

#define do_store_four(l) {                 \
    append_packet((l & 0xFF000000) >> 24); \
    append_packet((l & 0x00FF0000) >> 16); \
    append_packet((l & 0x0000FF00) >> 8);  \
    append_packet((l & 0x000000FF));       \
}

@ @c
static void append_float(eight_bits ** cpp, float a)
{
    unsigned int i;
    eight_bits *cp = *cpp;
    union U {
        float a;
        eight_bits b[sizeof(float)];
    } u;
    u.a = a;
    for (i = 0; i < sizeof(float); i++)
        append_packet(u.b[i]);
    *cpp = cp;
}

static int n_enum_field(lua_State * L, int name_index, int dflt, const char **values)
{
    int k, t;
    const char *s;
    int i = dflt;
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    t = lua_type(L,-1);
    if (t == LUA_TNUMBER) {
        i = (int) lua_tointeger(L, -1);
    } else if (t == LUA_TSTRING) {
        s = lua_tostring(L, -1);
        k = 0;
        while (values[k] != NULL) {
            if (strcmp(values[k], s) == 0) {
                i = k;
                break;
            }
            k++;
        }
    }
    lua_pop(L, 1);
    return i;
}

static int n_boolean_field(lua_State * L, int name_index, int dflt)
{
    int i = dflt;
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    if (lua_isboolean(L, -1)) {
        i = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
    return i;
}

static char *n_string_field_copy(lua_State * L, int name_index, const char *dflt)
{
    char *i;
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    if (lua_type(L,-1) == LUA_TSTRING) {
        i = xstrdup(lua_tostring(L, -1));
    } else if (dflt == NULL) {
        i = NULL;
    } else {
        i = xstrdup(dflt);
    }
    lua_pop(L, 1);
    return i;
}

static const char *n_string_field(lua_State * L, int name_index)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    return lua_tostring(L,-1);
}

static int n_some_field(lua_State * L, int name_index)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    return lua_type(L,-1);
}

/*static void init_font_string_pointers(lua_State * L){}*/

static int count_char_packet_bytes(lua_State * L)
{
    register int i;
    register int l = 0;
    int ff = 0;
    for (i = 1; i <= (int) lua_rawlen(L, -1); i++) {
        lua_rawgeti(L, -1, i);
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1);
            if (lua_type(L,-1) == LUA_TSTRING) {
                const char *s = lua_tostring(L, -1);
                if (lua_key_eq(s, font)) {
                    l += 5;
                    ff = 1;
                } else if (lua_key_eq(s, char)) {
                    if (ff == 0) {
                        l += 5;
                    }
                    l += 5;
                    ff = 1;
                } else if (lua_key_eq(s, slot)) {
                    l += 10;
                    ff = 1;
                } else if (lua_key_eq(s, comment) || lua_key_eq(s, nop)) {
                    ;
                } else if (lua_key_eq(s, push) || lua_key_eq(s, pop)) {
                    l++;
                } else if (lua_key_eq(s, rule)) {
                    l += 9;
                } else if (lua_key_eq(s, right) || lua_key_eq(s, node)
                           || lua_key_eq(s, down) || lua_key_eq(s, image)) {
                    l += 5;
                } else if (lua_key_eq(s, scale)) {
                    l += sizeof(float) + 1;
                } else if (lua_key_eq(s, special) || lua_key_eq(s, lua)) {
                    size_t len;
                    lua_rawgeti(L, -2, 2);
                    if (lua_type(L,-1) == LUA_TSTRING) {
                        (void) lua_tolstring(L, -1, &len);
                        lua_pop(L, 1);
                        if (len > 0) {
                            l = (int) (l + 5 + (int) len);
                        }
                    } else {
                        lua_pop(L, 1);
                        normal_error("vf command","invalid packet special");
                        /* fprintf(stdout, "invalid packet special!\n"); */
                    }
                } else {
                    normal_error("vf command","unknown packet command");
                    /* fprintf(stdout, "unknown packet command %s!\n", s); */
                }
            } else {
                normal_error("vf command","no packet command");
             /* fprintf(stdout, "no packet command!\n"); */
            }
            lua_pop(L, 1);      /* command name */
        }
        lua_pop(L, 1);          /* item */
    }
    return l;
}

static scaled sp_to_dvi(halfword sp, halfword atsize)
{
    double result, mult;
    mult = (double) (atsize / 65536.0);
    result = (double) (sp * 16.0);
    return floor(result / mult);
}

@ @c
static void read_char_packets(lua_State * L, int *l_fonts, charinfo * co, internal_font_number f, int atsize)
{
    int i, n, m;
    size_t l;
    int cmd;
    const char *s;
    eight_bits *cpackets, *cp;
    int ff = 0;
    int max_f = 0;
    int pc = count_char_packet_bytes(L);
    if (pc <= 0)
        return;
    while (l_fonts[(max_f + 1)] != 0)
        max_f++;
    cp = cpackets = xmalloc((unsigned) (pc + 1));
    for (i = 1; i <= (int) lua_rawlen(L, -1); i++) {
        lua_rawgeti(L, -1, i);
        if (lua_istable(L, -1)) {
            /* fetch the command code */
            lua_rawgeti(L, -1, 1);
            if (lua_type(L,-1) == LUA_TSTRING) {
                s = lua_tostring(L, -1);
                cmd = 0;
                if (lua_key_eq(s, font)) {
                    cmd = packet_font_code;
                } else if (lua_key_eq(s, char)) {
                    cmd = packet_char_code;
                    if (ff == 0) {
                        append_packet(packet_font_code);
                        ff = l_fonts[1];
                        do_store_four(ff);
                    }
                } else if (lua_key_eq(s, slot)) {
                    cmd = packet_nop_code;
                    lua_rawgeti(L, -2, 2);
                    n = (int) luaL_checkinteger(L, -1);
                    if (n ==0) {
                        ff = f;
                    } else {
                        ff = (n > max_f ? l_fonts[1] : l_fonts[n]);
                    }
                    lua_rawgeti(L, -3, 3);
                    n = (int) luaL_checkinteger(L, -1);
                    lua_pop(L, 2);
                    append_packet(packet_font_code);
                    do_store_four(ff);
                    append_packet(packet_char_code);
                    do_store_four(n);
                } else if (lua_key_eq(s, comment) || lua_key_eq(s, nop)) {
                    cmd = packet_nop_code;
                } else if (lua_key_eq(s, node)) {
                    cmd = packet_node_code;
                } else if (lua_key_eq(s, push)) {
                    cmd = packet_push_code;
                } else if (lua_key_eq(s, pop)) {
                    cmd = packet_pop_code;
                } else if (lua_key_eq(s, rule)) {
                    cmd = packet_rule_code;
                } else if (lua_key_eq(s, right)) {
                    cmd = packet_right_code;
                } else if (lua_key_eq(s, down)) {
                    cmd = packet_down_code;
                } else if (lua_key_eq(s, special)) {
                    cmd = packet_special_code;
                } else if (lua_key_eq(s, image)) {
                    cmd = packet_image_code;
                } else if (lua_key_eq(s, scale)) {
                    cmd = packet_scale_code;
                } else if (lua_key_eq(s, lua)) {
                    cmd = packet_lua_code;
                }
                switch (cmd) {
                    case packet_push_code:
                    case packet_pop_code:
                        append_packet(cmd);
                        break;
                    case packet_font_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        n = (int) luaL_checkinteger(L, -1);
                        if (n == 0) {
                            ff = n;
                        } else {
                            ff = (n > max_f ? l_fonts[1] : l_fonts[n]);
                        }
                        do_store_four(ff);
                        lua_pop(L, 1);
                        break;
                    case packet_node_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        n = copy_node_list(nodelist_from_lua(L));
                        do_store_four(n);
                        lua_pop(L, 1);
                        break;
                    case packet_char_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        n = (int) luaL_checkinteger(L, -1);
                        do_store_four(n);
                        lua_pop(L, 1);
                        break;
                    case packet_right_code:
                    case packet_down_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        n = (int) luaL_checkinteger(L, -1);
                        do_store_four(sp_to_dvi(n, atsize));
                        lua_pop(L, 1);
                        break;
                    case packet_rule_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        n = (int) luaL_checkinteger(L, -1);
                        do_store_four(sp_to_dvi(n, atsize));
                        lua_rawgeti(L, -3, 3);
                        n = (int) luaL_checkinteger(L, -1);
                        do_store_four(sp_to_dvi(n, atsize));
                        lua_pop(L, 2);
                        break;
                    case packet_special_code:
                    case packet_lua_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        s = luaL_checklstring(L, -1, &l);
                        if (l > 0) {
                            do_store_four(l);
                            m = (int) l;
                            while (m > 0) {
                                n = *s++;
                                m--;
                                append_packet(n);
                            }
                        }
                        lua_pop(L, 1);
                        break;
                    case packet_image_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);      /* img/imgtable? ... */
                        if (lua_istable(L, -1)) {   /* imgtable ... */
                            lua_getglobal(L, "img");        /* imglib imgtable ... */
                            lua_pushstring(L, "new");       /* `new' imglib imgtable ... */
                            lua_gettable(L, -2);    /* f imglib imgtable ... */
                            lua_insert(L, -3);      /* imglib imgtable f ... */
                            lua_pop(L, 1);  /* imgtable f ... */
                            lua_call(L, 1, 1);
                        }           /* img ... */
                        luaL_checkudata(L, -1, TYPE_IMG);   /* img ... --- just typecheck */
                        n = luaL_ref(L, LUA_REGISTRYINDEX);  /* ... */
                        do_store_four(n);
                        break;
                    case packet_nop_code:
                        break;
                    case packet_scale_code:
                        append_packet(cmd);
                        lua_rawgeti(L, -2, 2);
                        append_float(&cp, (float) luaL_checknumber(L, -1));
                        lua_pop(L, 1);
                        break;
                    default:
                        normal_error("vf command","invalid packet code");
                        /* fprintf(stdout, "Unknown char packet code %s\n", s); */
                }
            }
            lua_pop(L, 1);      /* command code */
        } else {
            normal_error("vf command","commands has to be a tbale");
            /* fprintf(stdout, "Found a `commands' item that is not a table\n"); */
        }
        lua_pop(L, 1);          /* command table */
    }
    append_packet(packet_end_code);
    set_charinfo_packets(co, cpackets);
    return;
}

@ @c
static void read_lua_cidinfo(lua_State * L, int f)
{
    int i;
    char *s;
    /*lua_getfield(L, -1, "cidinfo");*/
    lua_key_rawgeti(cidinfo);
    if (lua_istable(L, -1)) {
        i = lua_numeric_field_by_index(L,lua_key_index(version), 0);
        set_font_cidversion(f, i);
        i = lua_numeric_field_by_index(L,lua_key_index(supplement), 0);
        set_font_cidsupplement(f, i);
        s = n_string_field_copy(L, lua_key_index(registry), "Adobe");       /* Adobe-Identity-0 */
        set_font_cidregistry(f, s);
        s = n_string_field_copy(L, lua_key_index(ordering), "Identity");
        set_font_cidordering(f, s);
    }
    lua_pop(L, 1);
}


@ @c
static void read_lua_parameters(lua_State * L, int f)
{
    int i, n, t;
    const char *s;
    /*lua_getfield(L, -1, "parameters");*/
    lua_key_rawgeti(parameters);
    if (lua_istable(L, -1)) {
        /* the number of parameters is the max(IntegerKeys(L)),7) */
        n = 7;
        lua_pushnil(L);         /* first key */
        while (lua_next(L, -2) != 0) {
            if (lua_type(L, -2) == LUA_TNUMBER) {
                i = (int) lua_tointeger(L, -2);
                if (i > n)
                    n = i;
            }
            lua_pop(L, 1);      /* pop value */
        }
        if (n > 7)
            set_font_params(f, n);
        /* sometimes it is handy to have all integer keys */
        for (i = 1; i <= 7; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_type(L, -1) == LUA_TNUMBER) {
                n = lua_roundnumber(L, -1); /* round ? */
                set_font_param(f, i, n);
            }
            lua_pop(L, 1);
        }
        lua_pushnil(L);         /* first key */
        while (lua_next(L, -2) != 0) {
            t = lua_type(L,-2);
            if (t == LUA_TNUMBER) {
                i = (int) lua_tointeger(L, -2);
                if (i >= 8) {
                    if (lua_type(L,-1) == LUA_TNUMBER) {
                        n = lua_roundnumber(L, -1);
                    } else {
                        n = 0;
                    }
                    set_font_param(f, i, n);
                }
            } else if (t == LUA_TSTRING) {
                s = lua_tostring(L, -2);
                if (lua_type(L,-1) == LUA_TNUMBER) {
                    n = lua_roundnumber(L, -1);
                } else {
                    n = 0;
                }
                if (lua_key_eq(s, slant)) {
                    set_font_param(f, slant_code, n);
                } else if (lua_key_eq(s, space)) {
                    set_font_param(f, space_code, n);
                } else if (lua_key_eq(s, space_stretch)) {
                    set_font_param(f, space_stretch_code, n);
                } else if (lua_key_eq(s, space_shrink)) {
                    set_font_param(f, space_shrink_code, n);
                } else if (lua_key_eq(s, x_height)) {
                    set_font_param(f, x_height_code, n);
                } else if (lua_key_eq(s, quad)) {
                    set_font_param(f, quad_code, n);
                } else if (lua_key_eq(s, extra_space)) {
                    set_font_param(f, extra_space_code, n);
                }
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

}

@ @c
static void read_lua_math_parameters(lua_State * L, int f)
{
    int i = 0, n = 0, t;
    /*lua_getfield(L, -1, "MathConstants");*/
    lua_key_rawgeti(MathConstants);
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            t = lua_type(L,-2);
            if (t == LUA_TNUMBER) {
                i = (int) lua_tointeger(L, -2);
            } else if (t == LUA_TSTRING) {
                i = ff_checkoption(L, -2, NULL, MATH_param_names);
            }
            n = (int) lua_roundnumber(L, -1);
            if (i > 0) {
                set_font_math_param(f, i, n);
            }
            lua_pop(L, 1);      /* pop value */
        }
        set_font_oldmath(f,false);
    } else {
        set_font_oldmath(f,true);
    }
    lua_pop(L, 1);
}

@ @c
#define MIN_INF -0x7FFFFFFF

static void store_math_kerns(lua_State * L, int index, charinfo * co, int id)
{
    int l, k;
    scaled ht, krn;
    lua_key_direct_rawgeti(index);
    if (lua_istable(L, -1) && ((k = (int) lua_rawlen(L, -1)) > 0)) {
        for (l = 0; l < k; l++) {
            lua_rawgeti(L, -1, (l + 1));
            if (lua_istable(L, -1)) {
                ht = (scaled) lua_numeric_field_by_index(L, lua_key_index(height), MIN_INF);
                krn = (scaled) lua_numeric_field_by_index(L, lua_key_index(kern), MIN_INF);
                if (krn > MIN_INF && ht > MIN_INF)
                    add_charinfo_math_kern(co, id, ht, krn);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

@ @c
static void font_char_from_lua(lua_State * L, internal_font_number f, int i, int *l_fonts, boolean has_math)
{
    int k, r, t, lt, u, n;
    charinfo *co;
    kerninfo *ckerns;
    liginfo *cligs;
    scaled j;
    const char *s;
    int nl = 0;                 /* number of ligature table items */
    int nk = 0;                 /* number of kern table items */
    int ctr = 0;
    int atsize = font_size(f);
    if (lua_istable(L, -1)) {
        co = get_charinfo(f, i);
        set_charinfo_tag(co, 0);
        j = lua_numeric_field_by_index(L, lua_key_index(width), 0);
        set_charinfo_width(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(height), 0);
        set_charinfo_height(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(depth), 0);
        set_charinfo_depth(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(italic), 0);
        set_charinfo_italic(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(vert_italic), 0);
        set_charinfo_vert_italic(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(index), 0);
        set_charinfo_index(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(expansion_factor), 1000);
        set_charinfo_ef(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(left_protruding), 0);
        set_charinfo_lp(co, j);
        j = lua_numeric_field_by_index(L, lua_key_index(right_protruding), 0);
        set_charinfo_rp(co, j);
        k = n_boolean_field(L, lua_key_index(used), 0);
        set_charinfo_used(co, k);
        s = n_string_field(L, lua_key_index(name));
        if (s != NULL)
            set_charinfo_name(co, xstrdup(s));
        else
            set_charinfo_name(co, NULL);
        /* n_string_field leaves a value on stack*/
        lua_pop(L,1);
        u = n_some_field(L,lua_key_index(tounicode));
        if (u == LUA_TNUMBER) {
            u = lua_tointeger(L,-1);
            if (u < 0) {
                set_charinfo_tounicode(co, NULL);
            } else if (u < 0xD7FF || (u > 0xDFFF && u <= 0xFFFF)) {
                char *s = malloc(5);
                sprintf(s,"%04X",(unsigned int) u);
                set_charinfo_tounicode(co,s);
            } else {
                char *s = malloc(9);
                u = u - 0x10000;
                sprintf(s,"%04X%04X",(unsigned int) (floor(u/1024)+0xD800),(unsigned int) (u%1024+0xDC00));
                set_charinfo_tounicode(co,s);
            }
        } else if (u == LUA_TTABLE) {
            n = lua_rawlen(L,-1);
            u = 0;
            for (k = 1; k <= n; k++) {
                lua_rawgeti(L, -1, k);
                if (lua_type(L,-1) == LUA_TNUMBER) {
                    u = lua_tointeger(L,-1);
                } else {
                    lua_pop(L, 1);
                    break;
                }
                if (u < 0) {
                    u = -1;
                    lua_pop(L, 1);
                    break;
                } else if (u < 0xD7FF || (u > 0xDFFF && u <= 0xFFFF)) {
                    u = u + 4;
                } else {
                    u = u + 8;
                }
                lua_pop(L, 1);
            }
            if (u>0) {
                char *s = malloc(u+1);
                char *t = s ;
                for (k = 1; k <= n; k++) {
                    lua_rawgeti(L, -1, k);
                    u = lua_tointeger(L,-1);
                    if (u < 0xD7FF || (u > 0xDFFF && u <= 0xFFFF)) {
                        sprintf(t,"%04X",(unsigned int) u);
                        t += 4;
                    } else {
                        u = u - 0x10000;
                        sprintf(t,"%04X%04X",(unsigned int) (floor(u/1024)+0xD800),(unsigned int) (u%1024+0xDC00));
                        t += 8;
                    }
                    lua_pop(L, 1);
                }
                set_charinfo_tounicode(co,s);
            } else {
                set_charinfo_tounicode(co, NULL);
            }
        } else if (u == LUA_TSTRING) {
            s = lua_tostring(L,-1);
            set_charinfo_tounicode(co, xstrdup(s));
        } else {
            set_charinfo_tounicode(co, NULL);
        }
        /* ... leaves a value on stack*/
        lua_pop(L,1);

        if (has_math) {
            j = lua_numeric_field_by_index(L, lua_key_index(top_accent), INT_MIN);
            set_charinfo_top_accent(co, j);
            j = lua_numeric_field_by_index(L, lua_key_index(bot_accent), INT_MIN);
            set_charinfo_bot_accent(co, j);
            k = lua_numeric_field_by_index(L, lua_key_index(next), -1);
            if (k >= 0) {
                set_charinfo_tag(co, list_tag);
                set_charinfo_remainder(co, k);
            }

            lua_key_rawgeti(extensible);
            if (lua_istable(L, -1)) {
                int top, bot, mid, rep;
                top = lua_numeric_field_by_index(L, lua_key_index(top), 0);
                bot = lua_numeric_field_by_index(L, lua_key_index(bot), 0);
                mid = lua_numeric_field_by_index(L, lua_key_index(mid), 0);
                rep = lua_numeric_field_by_index(L, lua_key_index(rep), 0);
                if (top != 0 || bot != 0 || mid != 0 || rep != 0) {
                    set_charinfo_tag(co, ext_tag);
                    set_charinfo_extensible(co, top, bot, mid, rep);
                } else {
                    formatted_warning("font", "lua-loaded font %s char U+%X has an invalid extensible field",
                        font_name(f), (int) i);
                }
            }
            lua_pop(L, 1);

            lua_key_rawgeti(horiz_variants);
            if (lua_istable(L, -1)) {
                int glyph, startconnect, endconnect, advance, extender;
                extinfo *h;
                set_charinfo_tag(co, ext_tag);
                set_charinfo_hor_variants(co, NULL);
                for (k = 1;; k++) {
                    lua_rawgeti(L, -1, k);
                    if (lua_istable(L, -1)) {
                        glyph = lua_numeric_field_by_index(L, lua_key_index(glyph), 0);
                        extender = lua_numeric_field_by_index(L, lua_key_index(extender), 0);
                        startconnect = lua_numeric_field_by_index(L, lua_key_index(start), 0);
                        endconnect = lua_numeric_field_by_index(L, lua_key_index(end), 0);
                        advance = lua_numeric_field_by_index(L, lua_key_index(advance), 0);
                        h = new_variant(glyph, startconnect, endconnect, advance, extender);
                        add_charinfo_hor_variant(co, h);
                        lua_pop(L, 1);
                    } else {
                        lua_pop(L, 1);
                        break;
                    }
                }
            }
            lua_pop(L, 1);

            lua_key_rawgeti(vert_variants);
            if (lua_istable(L, -1)) {
                int glyph, startconnect, endconnect, advance, extender;
                extinfo *h;
                set_charinfo_tag(co, ext_tag);
                set_charinfo_vert_variants(co, NULL);
                for (k = 1;; k++) {
                    lua_rawgeti(L, -1, k);
                    if (lua_istable(L, -1)) {
                        glyph = lua_numeric_field_by_index(L, lua_key_index(glyph), 0);
                        extender = lua_numeric_field_by_index(L, lua_key_index(extender), 0);
                        startconnect = lua_numeric_field_by_index(L, lua_key_index(start), 0);
                        endconnect = lua_numeric_field_by_index(L, lua_key_index(end), 0);
                        advance = lua_numeric_field_by_index(L, lua_key_index(advance), 0);
                        h = new_variant(glyph, startconnect, endconnect, advance, extender);
                        add_charinfo_vert_variant(co, h);
                        lua_pop(L, 1);
                    } else {
                        lua_pop(L, 1);
                        break;
                    }
                }
            }
            lua_pop(L, 1);

            /*
                Here is a complete example:

                |mathkern = {|
                    | bottom_left  = { { height = 420, kern = 80  }, { height = 520, kern = 4   } },|
                    | bottom_right = { { height = 0,   kern = 48  } },|
                    | top_left     = { { height = 620, kern = 0   }, { height = 720, kern = -80 } },|
                    | top_right    = { { height = 676, kern = 115 }, { height = 776, kern = 45  } },|
                |}|

            */

            lua_key_rawgeti(mathkern);
            if (lua_istable(L, -1)) {
                store_math_kerns(L,lua_key_index(top_left), co, top_left_kern);
                store_math_kerns(L,lua_key_index(top_right), co, top_right_kern);
                store_math_kerns(L,lua_key_index(bottom_right), co, bottom_right_kern);
                store_math_kerns(L,lua_key_index(bottom_left), co, bottom_left_kern);
            }
            lua_pop(L, 1);
        }
        /* end of |has_math| */
        count_hash_items(L, kerns, nk);
        if (nk > 0) {
            /* kerns table still on stack */
            ckerns = xcalloc((unsigned) (nk + 1), sizeof(kerninfo));
            ctr = 0;
            lua_pushnil(L); /* traverse hash */
            while (lua_next(L, -2) != 0) {
                k = non_boundarychar;
                lt = lua_type(L,-2);
                if (lt == LUA_TNUMBER) {
                    k = (int) lua_tointeger(L, -2); /* adjacent char */
                    if (k < 0)
                        k = non_boundarychar;
                } else if (lt == LUA_TSTRING) {
                    s = lua_tostring(L, -2);
                    if (lua_key_eq(s, right_boundary)) {
                        k = right_boundarychar;
                        if (!has_right_boundary(f))
                            set_right_boundary(f, get_charinfo(f, right_boundarychar));
                    }
                }
                j = lua_roundnumber(L, -1); /* movement */
                if (k != non_boundarychar) {
                    set_kern_item(ckerns[ctr], k, j);
                    ctr++;
                } else {
                    formatted_warning("font", "lua-loaded font %s char U+%X has an invalid kern field",
                        font_name(f), (int) i);
                }
                lua_pop(L, 1);
            }
            /* guard against empty tables */
            if (ctr > 0) {
                set_kern_item(ckerns[ctr], end_kern, 0);
                set_charinfo_kerns(co, ckerns);
            } else {
                formatted_warning("font", "lua-loaded font %s char U+%X has an invalid kerns field",
                    font_name(f), (int) i);
            }
            lua_pop(L, 1);
        }

        /* packet commands */
        lua_key_rawgeti(commands);
        if (lua_istable(L, -1)) {
            lua_pushnil(L);     /* first key */
            if (lua_next(L, -2) != 0) {
                lua_pop(L, 2);
                read_char_packets(L, (int *) l_fonts, co, f, atsize);
            }
        }
        lua_pop(L, 1);

        /* ligatures */
        count_hash_items(L, ligatures, nl);
        if (nl > 0) {
            /* ligatures table still on stack */
            cligs = xcalloc((unsigned) (nl + 1), sizeof(liginfo));
            ctr = 0;
            lua_pushnil(L); /* traverse hash */
            while (lua_next(L, -2) != 0) {
                k = non_boundarychar;
                lt = lua_type(L,-2);
                if (lt == LUA_TNUMBER) {
                    k = (int) lua_tointeger(L, -2); /* adjacent char */
                    if (k < 0) {
                        k = non_boundarychar;
                    }
                } else if (lt == LUA_TSTRING) {
                    s = lua_tostring(L, -2);
                    if (lua_key_eq(s, right_boundary)) {
                        k = right_boundarychar;
                        if (!has_right_boundary(f))
                            set_right_boundary(f, get_charinfo(f, right_boundarychar));
                    }
                }
                r = -1;
                if (lua_istable(L, -1)) {
                    r = lua_numeric_field_by_index(L, lua_key_index(char), -1);    /* ligature */
                }
                if (r != -1 && k != non_boundarychar) {
                    t = n_enum_field(L, lua_key_index(type), 0, ligature_type_strings);
                    set_ligature_item(cligs[ctr], (char) ((t * 2) + 1), k, r);
                    ctr++;
                } else {
                    formatted_warning("font", "lua-loaded font %s char U+%X has an invalid ligature field",
                        font_name(f), (int) i);
                }
                lua_pop(L, 1);      /* iterator value */
            }
            /* guard against empty tables */
            if (ctr > 0) {
                set_ligature_item(cligs[ctr], 0, end_ligature, 0);
                set_charinfo_ligatures(co, cligs);
            } else {
                formatted_warning("font", "lua-loaded font %s char U+%X has an invalid ligatures field",
                    font_name(f), (int) i);
            }
            lua_pop(L, 1);      /* ligatures table */
        }
    }
}

@ The caller has to fix the state of the lua stack when there is an error!

@c
int font_from_lua(lua_State * L, int f)
{
    int i, n, r, t, lt;
    int s_top;                  /* lua stack top */
    int bc;                     /* first char index */
    int ec;                     /* last char index */
    char *s;
    const char *ss;
    int *l_fonts = NULL;
    int save_ref ;
    boolean no_math = false;

    /* will we save a cache of the luat table? */

    save_ref = 1; /* we start with  ss = "yes" */
    ss = NULL;
    ss = n_string_field(L, lua_key_index(cache));
    if (lua_key_eq(ss, no))
        save_ref = -1;
    else if (lua_key_eq(ss, renew))
        save_ref = 0;
    /* n_string_field leaves a value on stack*/
    lua_pop(L,1);

    /* the table is at stack index -1 */

    s = n_string_field_copy(L,lua_key_index(area), "");
    set_font_area(f, s);
    s = n_string_field_copy(L, lua_key_index(filename), NULL);
    set_font_filename(f, s);
    s = n_string_field_copy(L, lua_key_index(encodingname), NULL);
    set_font_encodingname(f, s);

    s = n_string_field_copy(L, lua_key_index(name), NULL);
    set_font_name(f, s);
    s = n_string_field_copy(L, lua_key_index(fullname), font_name(f));
    set_font_fullname(f, s);

    if (s == NULL) {
        formatted_error("font","lua-loaded font '%d' has no name!", f);
        return false;
    }
    s = n_string_field_copy(L, lua_key_index(psname), NULL);
    set_font_psname(f, s);

    i = lua_numeric_field_by_index(L,lua_key_index(units_per_em), 0);
    set_font_units_per_em(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(designsize), 655360);
    set_font_dsize(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(size), font_dsize(f));
    set_font_size(f, i);
    set_font_checksum(f, (unsigned)(lua_unsigned_numeric_field_by_index(L,lua_key_index(checksum), 0))) ;
    i = lua_numeric_field_by_index(L,lua_key_index(direction), 0);
    set_font_natural_dir(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(encodingbytes), 0);
    set_font_encodingbytes(f, (char) i);
    i = n_boolean_field(L,lua_key_index(oldmath), 0);
    set_font_oldmath(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(tounicode), 0);
    set_font_tounicode(f, (char) i);

    i = lua_numeric_field_by_index(L,lua_key_index(extend), 1000);
    if (i < FONT_EXTEND_MIN)
        i = FONT_EXTEND_MIN;
    if (i > FONT_EXTEND_MAX)
        i = FONT_EXTEND_MAX;
    set_font_extend(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(slant), 0);
    if (i < FONT_SLANT_MIN)
        i = FONT_SLANT_MIN;
    if (i > FONT_SLANT_MAX)
        i = FONT_SLANT_MAX;
    set_font_slant(f, i);

    i = lua_numeric_field_by_index(L,lua_key_index(hyphenchar), default_hyphen_char_par);
    set_hyphen_char(f, i);
    i = lua_numeric_field_by_index(L,lua_key_index(skewchar), default_skew_char_par);
    set_skew_char(f, i);
    i = n_boolean_field(L, lua_key_index(used), 0);
    set_font_used(f, (char) i);

    s = n_string_field_copy(L, lua_key_index(attributes), NULL);
    if (s != NULL && strlen(s) > 0) {
        i = maketexstring(s);
        set_pdf_font_attr(f, i);
    }
    free(s);

    i = n_enum_field(L, lua_key_index(type), unknown_font_type, font_type_strings);
    set_font_type(f, i);
    i = n_enum_field(L, lua_key_index(format), unknown_format, font_format_strings);
    set_font_format(f, i);
    i = n_enum_field(L, lua_key_index(embedding), unknown_embedding, font_embedding_strings);
    set_font_embedding(f, i);
    if (font_encodingbytes(f) == 0 && (font_format(f) == opentype_format || font_format(f) == truetype_format)) {
        set_font_encodingbytes(f, 2);
    }

    /* now fetch the base fonts, if needed */
    count_hash_items(L, fonts, n);
    if (n > 0) {
        /* font table still on stack */
        l_fonts = xmalloc((unsigned) ((unsigned) (n + 2) * sizeof(int)));
        memset(l_fonts, 0, (size_t) ((unsigned) (n + 2) * sizeof(int)));
        for (i = 1; i <= n; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_istable(L, -1)) {
                lua_key_rawgeti(id);
                if (lua_isnumber(L, -1)) {
                    l_fonts[i] = (int) lua_tointeger(L, -1);
                    if (l_fonts[i] == 0) {
                        l_fonts[i] = (int) f;
                    }
                    lua_pop(L, 2); /* pop id and entry */
                    continue;
                }
                lua_pop(L, 1); /* pop id */
            };
            ss = NULL;
            if (lua_istable(L, -1)) {
                ss = n_string_field(L, lua_key_index(name));
                /* string is anchored */
                lua_pop(L,1);
            }
            if (ss != NULL) {
                t = lua_numeric_field_by_index(L, lua_key_index(size), -1000);
                /* TODO: the stack is messed up, otherwise this explicit resizing would not be needed */
                s_top = lua_gettop(L);
                if (strcmp(font_name(f), ss) == 0)
                    l_fonts[i] = f;
                else
                    l_fonts[i] = find_font_id(ss, t);
                lua_settop(L, s_top);
            } else {
                formatted_error("font","invalid local font in lua-loaded font '%s'", font_name(f));
            }
            lua_pop(L, 1); /* pop list entry */
        }
        lua_pop(L, 1); /* pop font table */
    } else if (font_type(f) == virtual_font_type) {
        formatted_error("font","invalid local fonts in lua-loaded font '%s'", font_name(f));
    } else {
        l_fonts = xmalloc(3 * sizeof(int));
        l_fonts[0] = 0;
        l_fonts[1] = f;
        l_fonts[2] = 0;
    }

    /* parameters */
    no_math = n_boolean_field(L, lua_key_index(nomath), 0);
    read_lua_parameters(L, f);
    if (!no_math) {
        read_lua_math_parameters(L, f);
        if (n_boolean_field(L, lua_key_index(oldmath), 0)) {
            set_font_oldmath(f,true);
        }

    } else {
        set_font_oldmath(f,true);
    }
    read_lua_cidinfo(L, f);

    /* characters */
    lua_key_rawgeti(characters);
    if (lua_istable(L, -1)) {
        /* find the array size values */
        int num = 0;            /* number of charinfo's to add */
        ec = 0;
        bc = -1;
        lua_pushnil(L);         /* first key */
        while (lua_next(L, -2) != 0) {
            if (lua_isnumber(L, -2)) {
                i = (int) lua_tointeger(L, -2);
                if (i >= 0) {
                    if (lua_istable(L, -1)) {
                        num++;
                        if (i > ec)
                            ec = i;
                        if (bc < 0)
                            bc = i;
                        if (bc >= 0 && i < bc)
                            bc = i;
                    }
                }
            }
            lua_pop(L, 1);
        }
        if (bc != -1) {
            font_malloc_charinfo(f, num);
            set_font_bc(f, bc);
            set_font_ec(f, ec);
            lua_pushnil(L);     /* first key */
            while (lua_next(L, -2) != 0) {
                lt = lua_type(L,-2);
                if (lt == LUA_TNUMBER) {
                    i = (int) lua_tointeger(L, -2);
                    if (i >= 0) {
                        font_char_from_lua(L, f, i, l_fonts, !no_math);
                    }
                } else if (lt == LUA_TSTRING) {
                    const char *ss1 = lua_tostring(L, -2);
                    if (lua_key_eq(ss1, left_boundary)) {
                        font_char_from_lua(L, f, left_boundarychar, l_fonts,
                                           !no_math);
                    } else if (lua_key_eq(ss1, right_boundary)) {
                        font_char_from_lua(L, f, right_boundarychar, l_fonts,
                                           !no_math);
                    }
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            /*
                Handle font expansion last: the |copy_font| routine is called eventually,
                and that needs to know |bc| and |ec|.
            */

            if (font_type(f) != virtual_font_type) {
                int fstep = lua_numeric_field_by_index(L, lua_key_index(step), 0);
                if (fstep < 0)
                    fstep = 0;
                if (fstep > 100)
                    fstep = 100;
                if (fstep != 0) {
                    int fshrink = lua_numeric_field_by_index(L, lua_key_index(shrink), 0);
                    int fstretch =lua_numeric_field_by_index(L, lua_key_index(stretch), 0);
                    int fexpand = n_boolean_field(L, lua_key_index(auto_expand), 0);
                    if (fshrink < 0)
                        fshrink = 0;
                    if (fshrink > 500)
                        fshrink = 500;
                    fshrink -= (fshrink % fstep);
                    if (fshrink < 0)
                        fshrink = 0;
                    if (fstretch < 0)
                        fstretch = 0;
                    if (fstretch > 1000)
                        fstretch = 1000;
                    fstretch -= (fstretch % fstep);
                    if (fstretch < 0)
                        fstretch = 0;
                    set_expand_params(f, fexpand, fstretch, fshrink, fstep);
                }
            }

        } else {
            /* jikes, no characters */
            formatted_warning("font","lua-loaded font '%d' with name '%s' has no characters", f, font_name(f));
        }

        if (save_ref > 0) {
            r = luaL_ref(L, LUA_REGISTRYINDEX); /* pops the table */
            set_font_cache_id(f, r);
        } else {
            lua_pop(L, 1);
            set_font_cache_id(f, save_ref);
        }
    } else {
        /* jikes, no characters */
        formatted_warning("font","lua-loaded font '%d' with name '%s' has no character table", f, font_name(f));
    }
    if (l_fonts != NULL)
        free(l_fonts);
    return true;
}

@* Ligaturing.

@c
static void nesting_append(halfword nest1, halfword newn)
{
    halfword tail = tlink(nest1);
    if (tail == null) {
        couple_nodes(nest1, newn);
    } else {
        couple_nodes(tail, newn);
    }
    tlink(nest1) = newn;
}

static void nesting_prepend(halfword nest1, halfword newn)
{
    halfword head = vlink(nest1);
    couple_nodes(nest1, newn);
    if (head == null) {
        tlink(nest1) = newn;
    } else {
        couple_nodes(newn, head);
    }
}

static void nesting_prepend_list(halfword nest1, halfword newn)
{
    halfword head = vlink(nest1);
    couple_nodes(nest1, newn);
    if (head == null) {
        tlink(nest1) = tail_of_list(newn);
    } else {
        halfword tail = tail_of_list(newn);
        couple_nodes(tail, head);
    }
}

static int test_ligature(liginfo * lig, halfword left, halfword right)
{
    if (type(left) != glyph_node)
        return 0;
    if (font(left) != font(right))
        return 0;
    if (is_ghost(left) || is_ghost(right))
        return 0;
    *lig = get_ligature(font(left), character(left), character(right));
    if (is_valid_ligature(*lig)) {
        return 1;
    }
    return 0;
}

static int try_ligature(halfword * frst, halfword fwd)
{
    halfword cur = *frst;
    liginfo lig;
    if (test_ligature(&lig, cur, fwd)) {
        int move_after = (lig_type(lig) & 0x0C) >> 2;
        int keep_right = ((lig_type(lig) & 0x01) != 0);
        int keep_left = ((lig_type(lig) & 0x02) != 0);
        halfword newgl = raw_glyph_node();
        font(newgl) = font(cur);
        character(newgl) = lig_replacement(lig);
        set_is_ligature(newgl);
        /*
            below might not be correct in contrived border case.
            but we use it only for debugging, so ...
        */
        if (character(cur) < 0) {
            set_is_leftboundary(newgl);
        }
        if (character(fwd) < 0) {
            set_is_rightboundary(newgl);
        }
        if (character(cur) < 0) {
            if (character(fwd) < 0) {
                build_attribute_list(newgl);
            } else {
                add_node_attr_ref(node_attr(fwd));
                node_attr(newgl) = node_attr(fwd);
            }
        } else {
            add_node_attr_ref(node_attr(cur));
            node_attr(newgl) = node_attr(cur);
        }

        /*
            TODO/FIXME if this ligature is consists of another ligature
            we should add it's |lig_ptr| to the new glyphs |lig_ptr| (and
            cleanup the no longer needed node) LOW PRIORITY
        */
        /* left side */
        if (keep_left) {
            halfword new_first = copy_node(cur);
            lig_ptr(newgl) = new_first;
            couple_nodes(cur, newgl);
            if (move_after) {
                move_after--;
                cur = newgl;
            }
        } else {
            halfword prev = alink(cur);
            uncouple_node(cur);
            lig_ptr(newgl) = cur;
            couple_nodes(prev, newgl);
            cur = newgl;        /* as cur has disappeared */
        }
        /* right side */
        if (keep_right) {
            halfword new_second = copy_node(fwd);
            /* correct, because we {\it know\/} |lig_ptr| points to {\it one\/} node */
            couple_nodes(lig_ptr(newgl), new_second);
            couple_nodes(newgl, fwd);
            if (move_after) {
                move_after--;
                cur = fwd;
            }
        } else {
            halfword next = vlink(fwd);
            uncouple_node(fwd);
            /* correct, because we {\it know\/} |lig_ptr| points to {\it one\/} node */
            couple_nodes(lig_ptr(newgl), fwd);
            if (next != null) {
                couple_nodes(newgl, next);
            }
        }
        /* check and return */
        *frst = cur;
        return 1;
    }
    return 0;
}

@ there shouldn't be any ligatures here - we only add them at the end of
 |xxx_break| in a \.{DISC-1 - DISC-2} situation and we stop processing \.{DISC-1}
 (we continue with \.{DISC-1}'s |post_| and |no_break|.

@c
static halfword handle_lig_nest(halfword root, halfword cur)
{
    if (cur == null)
        return root;
    while (vlink(cur) != null) {
        halfword fwd = vlink(cur);
        if (type(cur) == glyph_node && type(fwd) == glyph_node &&
                font(cur) == font(fwd) && try_ligature(&cur, fwd)) {
            continue;
        }
        cur = vlink(cur);
    }
    tlink(root) = cur;
    return root;
}

static halfword handle_lig_word(halfword cur)
{
    halfword right = null;
    if (type(cur) == boundary_node) {
        halfword prev = alink(cur);
        halfword fwd = vlink(cur);
        /* no need to uncouple |cur|, it is freed */
        flush_node(cur);
        if (fwd == null) {
            vlink(prev) = fwd;
            return prev;
        }
        couple_nodes(prev, fwd);
        if (type(fwd) != glyph_node)
            return prev;
        cur = fwd;
    } else if (has_left_boundary(font(cur))) {
        halfword prev = alink(cur);
        halfword p = new_glyph(font(cur), left_boundarychar);
        couple_nodes(prev, p);
        couple_nodes(p, cur);
        cur = p;
    }
    if (has_right_boundary(font(cur))) {
        right = new_glyph(font(cur), right_boundarychar);
    }
    while (1) {
        /* A glyph followed by ... */
        if (type(cur) == glyph_node) {
            halfword fwd = vlink(cur);
            if (fwd == null) {  /* last character of paragraph */
                if (right == null)
                    break;
                /* \.{--\\par} prohibits use of |couple_nodes| here */
                try_couple_nodes(cur, right);
                right = null;
                continue;
            }
            if (type(fwd) == glyph_node) {      /* |GLYPH - GLYPH| */
                if (font(cur) != font(fwd))
                    break;
                if (try_ligature(&cur, fwd))
                    continue;
            } else if (type(fwd) == disc_node) {        /* |GLYPH - DISC| */

                /* if  \.{a{bx}{}{y}} and \.{a+b=>B} convert to \.{{Bx}{}{ay}} */
                halfword pre = vlink_pre_break(fwd);
                halfword nob = vlink_no_break(fwd);
                halfword next, tail;
                liginfo lig;
                /* Check on: a{b?}{?}{?} and a+b=>B : {B?}{?}{a?} */
                /* Check on: a{?}{?}{b?} and a+b=>B : {a?}{?}{B?} */
                if ((pre != null && type(pre) == glyph_node
                     && test_ligature(&lig, cur, pre))
                    || (nob != null && type(nob) == glyph_node
                        && test_ligature(&lig, cur, nob))) {
                    /* move cur from before disc, to skipped part */
                    halfword prev = alink(cur);
                    uncouple_node(cur);
                    couple_nodes(prev, fwd);
                    nesting_prepend(no_break(fwd), cur);
                    /* now ligature the |pre_break| */
                    nesting_prepend(pre_break(fwd), copy_node(cur));
                    /* As we have removed cur, we need to start again ... */
                    cur = prev;
                }
                /* Check on: a{?}{?}{}b and a+b=>B : {a?}{?b}{B} */
                next = vlink(fwd);
                if (nob == null && next != null && type(next) == glyph_node
                    && test_ligature(&lig, cur, next)) {
                    /* move |cur| from before |disc| to |no_break| part */
                    halfword prev = alink(cur);
                    uncouple_node(cur);
                    couple_nodes(prev, fwd);
                    couple_nodes(no_break(fwd), cur);   /* we {\it know\/} it's empty */
                    /* now copy cur the |pre_break| */
                    nesting_prepend(pre_break(fwd), copy_node(cur));
                    /* move next from after disc to |no_break| part */
                    tail = vlink(next);
                    uncouple_node(next);
                    try_couple_nodes(fwd, tail);
                    couple_nodes(cur, next);    /* we {\it know\/} this works */
                    tlink(no_break(fwd)) = next;        /* and make sure the list is correct */
                    /* now copy next to the |post_break| */
                    nesting_append(post_break(fwd), copy_node(next));
                    /* As we have removed cur, we need to start again ... */
                    cur = prev;
                }
                /* we are finished with the |pre_break| */
                handle_lig_nest(pre_break(fwd), vlink_pre_break(fwd));
            } else if (type(fwd) == boundary_node) {
                halfword next = vlink(fwd);
                try_couple_nodes(cur, next);
                flush_node(fwd);
                if (right != null) {
                    flush_node(right);  /* Shame, didn't need it */
                    /* no need to reset |right|, we're going to leave the loop anyway */
                }
                break;
            } else {
                /* fwd is something unknown */
                if (right == null)
                    break;
                couple_nodes(cur, right);
                couple_nodes(right, fwd);
                right = null;
                continue;
            }
            /* A discretionary followed by ... */
        } else if (type(cur) == disc_node) {
            /* If \.{{?}{x}{?}} or \.{{?}{?}{y}} then ... */
            if (vlink_no_break(cur) != null || vlink_post_break(cur) != null) {
                halfword prev = 0;
                halfword fwd;
                liginfo lig;
                if (subtype(cur) == select_disc) {
                    prev = alink(cur);
                    if (vlink_post_break(cur) != null)
                        handle_lig_nest(post_break(prev), vlink_post_break(prev));
                    if (vlink_no_break(cur) != null)
                        handle_lig_nest(no_break(prev), vlink_no_break(prev));
                }
                if (vlink_post_break(cur) != null)
                    handle_lig_nest(post_break(cur), vlink_post_break(cur));
                if (vlink_no_break(cur) != null)
                    handle_lig_nest(no_break(cur), vlink_no_break(cur));
                while ((fwd = vlink(cur)) != null) {
                    halfword nob, pst, next;
                    if (type(fwd) != glyph_node)
                        break;
                    if (subtype(cur) != select_disc) {
                        nob = tlink_no_break(cur);
                        pst = tlink_post_break(cur);
                        if ((nob == null || !test_ligature(&lig, nob, fwd)) &&
                            (pst == null || !test_ligature(&lig, pst, fwd)))
                            break;
                        nesting_append(no_break(cur), copy_node(fwd));
                        handle_lig_nest(no_break(cur), nob);
                    } else {
                        int dobreak = 0;
                        nob = tlink_no_break(prev);
                        pst = tlink_post_break(prev);
                        if ((nob == null || !test_ligature(&lig, nob, fwd)) &&
                            (pst == null || !test_ligature(&lig, pst, fwd)))
                            dobreak = 1;
                        if (!dobreak) {
                            nesting_append(no_break(prev), copy_node(fwd));
                            handle_lig_nest(no_break(prev), nob);
                            nesting_append(post_break(prev), copy_node(fwd));
                            handle_lig_nest(post_break(prev), pst);
                        }
                        dobreak = 0;
                        nob = tlink_no_break(cur);
                        pst = tlink_post_break(cur);
                        if ((nob == null || !test_ligature(&lig, nob, fwd)) &&
                            (pst == null || !test_ligature(&lig, pst, fwd)))
                            dobreak = 1;
                        if (!dobreak) {
                            nesting_append(no_break(cur), copy_node(fwd));
                            handle_lig_nest(no_break(cur), nob);
                        }
                        if (dobreak)
                            break;
                    }
                    next = vlink(fwd);
                    uncouple_node(fwd);
                    try_couple_nodes(cur, next);
                    nesting_append(post_break(cur), fwd);
                    handle_lig_nest(post_break(cur), pst);
                }
                if (fwd != null && type(fwd) == disc_node) {
                        halfword next = vlink(fwd);
                        if (vlink_no_break(fwd) == null
                        && vlink_post_break(fwd) == null
                        && next != null
                        && type(next) == glyph_node
                        && ((tlink_post_break(cur) != null && test_ligature(&lig, tlink_post_break(cur), next)) ||
                            (tlink_no_break  (cur) != null && test_ligature(&lig, tlink_no_break  (cur), next)))) {
                        /* Building an |init_disc| followed by a |select_disc|
                          \.{{a-}{b}{AB} {-}{}{}} 'c'
                         */
                        /* is it tail necessary ? */
                        halfword last1 = vlink(next), tail ;
                        uncouple_node(next);
                        try_couple_nodes(fwd, last1);
                        /* \.{{a-}{b}{AB} {-}{c}{}} */
                        nesting_append(post_break(fwd), copy_node(next));
                        /* \.{{a-}{b}{AB} {-}{c}{-}} */
                        if (vlink_no_break(cur) != null) {
                            nesting_prepend(no_break(fwd), copy_node(vlink_pre_break(fwd)));
                        }
                        /* \.{{a-}{b}{AB} {b-}{c}{-}} */
                        if (vlink_post_break(cur) != null)
                            nesting_prepend_list(pre_break(fwd), copy_node_list(vlink_post_break(cur)));
                        /* \.{{a-}{b}{AB} {b-}{c}{AB-}} */
                        if (vlink_no_break(cur) != null) {
                            nesting_prepend_list(no_break(fwd), copy_node_list(vlink_no_break(cur)));
                        }
                        /* \.{{a-}{b}{ABC} {b-}{c}{AB-}} */
                        tail = tlink_no_break(cur);
                        nesting_append(no_break(cur), copy_node(next));
                        handle_lig_nest(no_break(cur), tail);
                        /* \.{{a-}{BC}{ABC} {b-}{c}{AB-}} */
                        tail = tlink_post_break(cur);
                        nesting_append(post_break(cur), next);
                        handle_lig_nest(post_break(cur), tail);
                        /* and set the subtypes */
                        subtype(cur) = init_disc;
                        subtype(fwd) = select_disc;
                    }
                }
            }

        } else {
            /* NO GLYPH NOR DISC */
            return cur;
        }
        /* step-to-next-node */
        /* \.{--\\par} allows |vlink(cur)| to be null */
        cur = vlink(cur);
    }

    return cur;
}

@ Return value is the new tail, head should be a dummy

@c
halfword handle_ligaturing(halfword head, halfword tail)
{
    halfword save_tail1; /* trick to allow explicit |node==null| tests */
    halfword cur, prev;

    if (vlink(head) == null)
        return tail;
    save_tail1 = vlink(tail);
    vlink(tail) = null;

    /* |if (fix_node_lists)| */
    fix_node_list(head);

    prev = head;
    cur = vlink(prev);

    while (cur != null) {
        if (type(cur) == glyph_node || (type(cur) == boundary_node)) {
            cur = handle_lig_word(cur);
        }
        prev = cur;
        cur = vlink(cur);
    }
    if (prev == null)
        prev = tail;

    if (valid_node(save_tail1)) {
        try_couple_nodes(prev, save_tail1);
    }
    return prev;
}


@* Kerning.

@c
static void add_kern_before(halfword left, halfword right)
{
    if ((!is_rightghost(right)) &&
        font(left) == font(right) && has_kern(font(left), character(left))) {
        int k = raw_get_kern(font(left), character(left), character(right));
        if (k != 0) {
            halfword kern = new_kern(k);
            halfword prev = alink(right);
            couple_nodes(prev, kern);
            couple_nodes(kern, right);
            /* update the attribute list (inherit from left) */
            delete_attribute_ref(node_attr(kern));
            add_node_attr_ref(node_attr(left));
            node_attr(kern) = node_attr(left);
        }
    }
}

static void add_kern_after(halfword left, halfword right, halfword aft)
{
    if ((!is_rightghost(right)) &&
        font(left) == font(right) && has_kern(font(left), character(left))) {
        int k = raw_get_kern(font(left), character(left), character(right));
        if (k != 0) {
            halfword kern = new_kern(k);
            halfword next = vlink(aft);
            couple_nodes(aft, kern);
            try_couple_nodes(kern, next);
            /* update the attribute list (inherit from left == aft) */
            delete_attribute_ref(node_attr(kern));
            add_node_attr_ref(node_attr(aft));
            node_attr(kern) = node_attr(aft);
        }
    }
}

static void do_handle_kerning(halfword root, halfword init_left, halfword init_right)
{
    halfword cur = vlink(root);
    halfword left = null;
    if (cur == null) {
        if (init_left != null && init_right != null) {
            add_kern_after(init_left, init_right, root);
            tlink(root) = vlink(root);
        }
        return;
    }
    if (type(cur) == glyph_node) {
        set_is_glyph(cur);
        if (init_left != null)
            add_kern_before(init_left, cur);
        left = cur;
    }
    while ((cur = vlink(cur)) != null) {
        if (type(cur) == glyph_node) {
            set_is_glyph(cur);
            if (left != null) {
                add_kern_before(left, cur);
                if (character(left) < 0 || is_ghost(left)) {
                    halfword prev = alink(left);
                    couple_nodes(prev, cur);
                    flush_node(left);
                }
            }
            left = cur;
        } else {
            if (type(cur) == disc_node) {
                halfword right = type(vlink(cur)) == glyph_node ? vlink(cur) : null;
                do_handle_kerning(pre_break(cur), left, null);
                do_handle_kerning(post_break(cur), null, right);
                if (vlink_post_break(cur) != null)
                    tlink_post_break(cur) = tail_of_list(vlink_post_break(cur));
                do_handle_kerning(no_break(cur), left, right);
                if (vlink_no_break(cur) != null)
                    tlink_no_break(cur) = tail_of_list(vlink_no_break(cur));    /* needed? */
            }
            if (left != null) {
                if (character(left) < 0 || is_ghost(left)) {
                    halfword prev = alink(left);
                    couple_nodes(prev, cur);
                    flush_node(left);
                }
                left = null;
            }
        }
    }
    if (left != null) {
        if (init_right != null)
            add_kern_after(left, init_right, left);
        if (character(left) < 0 || is_ghost(left)) {
            halfword prev = alink(left);
            halfword next = vlink(left);
            if (next != null) {
                couple_nodes(prev, next);
                tlink(root) = next;
            } else if (prev != root) {
                vlink(prev) = null;
                tlink(root) = prev;
            } else {
                vlink(root) = null;
                tlink(root) = null;
            }
            flush_node(left);
        }
    }
}

halfword handle_kerning(halfword head, halfword tail)
{
    halfword save_link;
    save_link = vlink(tail);
    vlink(tail) = null;
    tlink(head) = tail;
    do_handle_kerning(head, null, null);
    tail = tlink(head);
    if (valid_node(save_link)) {
        try_couple_nodes(tail, save_link);
    }
    return tail;
}

@* ligaturing and kerning : lua-interface.

@c
static halfword run_lua_ligkern_callback(halfword head, halfword tail, int callback_id)
{
    int i;
    int top = lua_gettop(Luas);
    if (!get_callback(Luas, callback_id)) {
        lua_pop(Luas, 2);
        return tail;
    }
    nodelist_to_lua(Luas, head);
    nodelist_to_lua(Luas, tail);
    if ((i=lua_pcall(Luas, 2, 0, 0)) != 0) {
        luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        return tail;
    }
    fix_node_list(head);
    lua_settop(Luas, top);
    return tail;
}

halfword new_ligkern(halfword head, halfword tail)
{
    int callback_id = 0;
    if (vlink(head) == null)
        return tail;
    callback_id = callback_defined(ligaturing_callback);
    if (callback_id > 0) {
        tail = run_lua_ligkern_callback(head, tail, callback_id);
        if (tail == null)
            tail = tail_of_list(head);
    } else if (callback_id == 0) {
        tail = handle_ligaturing(head, tail);
    }
    callback_id = callback_defined(kerning_callback);
    if (callback_id > 0) {
        tail = run_lua_ligkern_callback(head, tail, callback_id);
        if (tail == null) {
            tail = tail_of_list(head);
        }
    } else if (callback_id == 0) {
        halfword nest1 = new_node(nesting_node, 1);
        halfword cur = vlink(head);
        halfword aft = vlink(tail);
        couple_nodes(nest1, cur);
        tlink(nest1) = tail;
        vlink(tail) = null;
        do_handle_kerning(nest1, null, null);
        couple_nodes(head, vlink(nest1));
        tail = tlink(nest1);
        try_couple_nodes(tail, aft);
        flush_node(nest1);
    }
    return tail;
}
