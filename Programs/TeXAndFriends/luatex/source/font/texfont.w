% texfont.w
%
% Copyright 2006-2013 Taco Hoekwater <taco@@luatex.org>
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

@* Main font API implementation for the original pascal parts.

Stuff to watch out for:

\item{} Knuth had a |'null_character'| that was used when a character could
not be found by the |fetch()| routine, to signal an error. This has
been deleted, but it may mean that the output of luatex is
incompatible with TeX after |fetch()| has detected an error condition.

\item{} Knuth also had a |font_glue()| optimization. I've removed that
because it was a bit of dirty programming and it also was
problematic |if 0 != null|.

@c

#include "ptexlib.h"
#include "lua/luatex-api.h"

@ @c
#define noDEBUG

#define proper_char_index(c) (c<=font_ec(f) && c>=font_bc(f))
#define do_realloc(a,b,d)    a = xrealloc(a,(unsigned)((unsigned)(b)*sizeof(d)))

texfont **font_tables = NULL;

static int font_arr_max = 0;
static int font_id_maxval = 0;

@ @c
static void grow_font_table(int id)
{
    int j;
    if (id >= font_arr_max) {
        font_bytes +=
            (int) (((id + 8 - font_arr_max) * (int) sizeof(texfont *)));
        font_tables =
            xrealloc(font_tables,
                     (unsigned) (((unsigned) id + 8) * sizeof(texfont *)));
        j = 8;
        while (j--) {
            font_tables[id + j] = NULL;
        }
        font_arr_max = id + 8;
    }
}

int new_font_id(void)
{
    int i;
    for (i = 0; i < font_arr_max; i++) {
        if (font_tables[i] == NULL) {
            break;
        }
    }
    if (i >= font_arr_max)
        grow_font_table(i);
    if (i > font_id_maxval)
        font_id_maxval = i;
    return i;
}

int max_font_id(void)
{
    return font_id_maxval;
}

void set_max_font_id(int i)
{
    font_id_maxval = i;
}

@ @c
int new_font(void)
{
    int k;
    int id;
    charinfo *ci;
    sa_tree_item sa_value = { 0 };
    id = new_font_id();
    font_bytes += (int) sizeof(texfont);
    /* most stuff is zero */
    font_tables[id] = xcalloc(1, sizeof(texfont));
    font_tables[id]->_font_name = NULL;
    font_tables[id]->_font_area = NULL;
    font_tables[id]->_font_filename = NULL;
    font_tables[id]->_font_fullname = NULL;
    font_tables[id]->_font_psname = NULL;
    font_tables[id]->_font_encodingname = NULL;
    font_tables[id]->_font_cidregistry = NULL;
    font_tables[id]->_font_cidordering = NULL;
    font_tables[id]->_left_boundary = NULL;
    font_tables[id]->_right_boundary = NULL;
    font_tables[id]->_param_base = NULL;
    font_tables[id]->_math_param_base = NULL;

    set_font_bc(id, 1);         /* ec = 0 */
    set_hyphen_char(id, '-');
    set_skew_char(id, -1);
    font_slant(id) = 0;         /* vertical */
    font_extend(id) = 1000;     /* normal width */

    /* allocate eight values including 0 */
    set_font_params(id, 7);
    for (k = 0; k <= 7; k++) {
        set_font_param(id, k, 0);
    }
    /* character info zero is reserved for notdef */
    font_tables[id]->characters = new_sa_tree(1, 1, sa_value);    /* stack size 1, default item value 0 */
    ci = xcalloc(1, sizeof(charinfo));
    set_charinfo_name(ci, xstrdup(".notdef"));
    font_tables[id]->charinfo = ci;
    font_tables[id]->charinfo_size = 1;
    font_tables[id]->charinfo_cache = NULL;

    return id;
}

@ @c
void font_malloc_charinfo(internal_font_number f, int num)
{
    int glyph = font_tables[f]->charinfo_size;
    font_bytes += (int) (num * (int) sizeof(charinfo));
    do_realloc(font_tables[f]->charinfo, (unsigned) (glyph + num), charinfo);
    memset(&(font_tables[f]->charinfo[glyph]), 0,
           (size_t) (num * (int) sizeof(charinfo)));
    font_tables[f]->charinfo_size += num;
}

@ @c
#define find_charinfo_id(f,c) (get_sa_item(font_tables[f]->characters,c).int_value)

charinfo *get_charinfo(internal_font_number f, int c)
{
    int glyph;
    charinfo *ci;
    if (proper_char_index(c)) {
        glyph = get_sa_item(font_tables[f]->characters, c).int_value;
        if (!glyph) {
            sa_tree_item sa_value = { 0 };
            int tglyph = ++font_tables[f]->charinfo_count;
            if (tglyph >= font_tables[f]->charinfo_size) {
                font_malloc_charinfo(f, 256);
            }
            font_tables[f]->charinfo[tglyph].ef = 1000; /* init */
            sa_value.int_value = tglyph;
            set_sa_item(font_tables[f]->characters, c, sa_value, 1); /* 1 = global */
            glyph = tglyph;
        }
        return &(font_tables[f]->charinfo[glyph]);
    } else if (c == left_boundarychar) {
        if (left_boundary(f) == NULL) {
            ci = xcalloc(1, sizeof(charinfo));
            font_bytes += (int) sizeof(charinfo);
            set_left_boundary(f, ci);
        }
        return left_boundary(f);
    } else if (c == right_boundarychar) {
        if (right_boundary(f) == NULL) {
            ci = xcalloc(1, sizeof(charinfo));
            font_bytes += (int) sizeof(charinfo);
            set_right_boundary(f, ci);
        }
        return right_boundary(f);
    }
    return &(font_tables[f]->charinfo[0]);
}

@ @c
static void set_charinfo(internal_font_number f, int c, charinfo * ci)
{
    int glyph;
    if (proper_char_index(c)) {
        glyph = get_sa_item(font_tables[f]->characters, c).int_value;
        if (glyph) {
            font_tables[f]->charinfo[glyph] = *ci;
        } else {
            normal_error("font","character insertion failed");
        }
    } else if (c == left_boundarychar) {
        set_left_boundary(f, ci);
    } else if (c == right_boundarychar) {
        set_right_boundary(f, ci);
    }
}

@ @c
charinfo *copy_charinfo(charinfo * ci)
{
    int x, k;
    kerninfo *kern;
    liginfo *lig;
    eight_bits *packet;
    charinfo *co = NULL;
    if (ci == NULL)
        return NULL;
    co = xmalloc(sizeof(charinfo));
    memcpy(co, ci, sizeof(charinfo));
    set_charinfo_used(co, false);
    co->name = NULL;
    co->tounicode = NULL;
    co->packets = NULL;
    co->ligatures = NULL;
    co->kerns = NULL;
    co->vert_variants = NULL;
    co->hor_variants = NULL;
    if (ci->name != NULL) {
        co->name = xstrdup(ci->name);
    }
    if (ci->tounicode != NULL) {
        co->tounicode = xstrdup(ci->tounicode);
    }
    /* kerns */
    if ((kern = get_charinfo_kerns(ci)) != NULL) {
        x = 0;
        while (!kern_end(kern[x])) {
            x++;
        }
        x++;
        co->kerns = xmalloc((unsigned) (x * (int) sizeof(kerninfo)));
        memcpy(co->kerns, ci->kerns, (size_t) (x * (int) sizeof(kerninfo)));
    }
    /* ligs */
    if ((lig = get_charinfo_ligatures(ci)) != NULL) {
        x = 0;
        while (!lig_end(lig[x])) {
            x++;
        }
        x++;
        co->ligatures = xmalloc((unsigned) (x * (int) sizeof(liginfo)));
        memcpy(co->ligatures, ci->ligatures,
               (size_t) (x * (int) sizeof(liginfo)));
    }
    /* packets */
    if ((packet = get_charinfo_packets(ci)) != NULL) {
        x = vf_packet_bytes(ci);
        co->packets = xmalloc((unsigned) x);
        memcpy(co->packets, ci->packets, (size_t) x);
    }

    /* horizontal and vertical extenders */
    if (get_charinfo_vert_variants(ci) != NULL) {
        set_charinfo_vert_variants(co, copy_variants(get_charinfo_vert_variants(ci)));
    }
    if (get_charinfo_hor_variants(ci) != NULL) {
        set_charinfo_hor_variants(co, copy_variants(get_charinfo_hor_variants(ci)));
    }
    x = ci->top_left_math_kerns;
    co->top_left_math_kerns = x;
    if (x > 0) {
        co->top_left_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
        for (k = 0; k < co->top_left_math_kerns; k++) {
            co->top_left_math_kern_array[(2 * k)] = ci->top_left_math_kern_array[(2 * k)];
            co->top_left_math_kern_array[(2 * k) + 1] = ci->top_left_math_kern_array[(2 * k) + 1];
        }
    }
    x = ci->bottom_left_math_kerns;
    co->bottom_left_math_kerns = x;
    if (x > 0) {
        co->bottom_left_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
        for (k = 0; k < co->bottom_left_math_kerns; k++) {
            co->bottom_left_math_kern_array[(2 * k)] = ci->bottom_left_math_kern_array[(2 * k)];
            co->bottom_left_math_kern_array[(2 * k) + 1] = ci->bottom_left_math_kern_array[(2 * k) + 1];
        }
    }
    x = ci->top_right_math_kerns;
    co->top_right_math_kerns = x;
    if (x > 0) {
        co->top_right_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
        for (k = 0; k < co->top_right_math_kerns; k++) {
            co->top_right_math_kern_array[(2 * k)] = ci->top_right_math_kern_array[(2 * k)];
            co->top_right_math_kern_array[(2 * k) + 1] = ci->top_right_math_kern_array[(2 * k) + 1];
        }
    }
    x = ci->bottom_right_math_kerns;
    co->bottom_right_math_kerns = x;
    if (x > 0) {
        co->bottom_right_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
        for (k = 0; k < co->bottom_right_math_kerns; k++) {
            co->bottom_right_math_kern_array[(2 * k)] = ci->bottom_right_math_kern_array[(2 * k)];
            co->bottom_right_math_kern_array[(2 * k) + 1] = ci->bottom_right_math_kern_array[(2 * k) + 1];
        }
    }
    return co;
}

charinfo *char_info(internal_font_number f, int c)
{
    if (f > font_id_maxval)
        return 0;
    if (proper_char_index(c)) {
        register int glyph = (int) find_charinfo_id(f, c);
        return &(font_tables[f]->charinfo[glyph]);
    } else if (c == left_boundarychar && left_boundary(f) != NULL) {
        return left_boundary(f);
    } else if (c == right_boundarychar && right_boundary(f) != NULL) {
        return right_boundary(f);
    }
    return &(font_tables[f]->charinfo[0]);
}

@ @c
scaled_whd get_charinfo_whd(internal_font_number f, int c)
{
    scaled_whd s;
    charinfo *i;
    i = char_info(f, c);
    s.wd = i->width;
    s.dp = i->depth;
    s.ht = i->height;
    return s;
}

@ @c
int char_exists(internal_font_number f, int c)
{
    if (f > font_id_maxval)
        return 0;
    if (proper_char_index(c)) {
        return (int) find_charinfo_id(f, c);
    } else if ((c == left_boundarychar) && has_left_boundary(f)) {
        return 1;
    } else if ((c == right_boundarychar) && has_right_boundary(f)) {
        return 1;
    }
    return 0;
}

@ @c
#if 0
static int lua_char_exists_callback(internal_font_number f, int c)
{
    int callback_id;
    int ret = 0;
    callback_id = callback_defined(char_exists_callback);
    if (callback_id != 0) {
        if (!get_callback(Luas, callback_id)) {
            lua_pop(Luas, 2);
            return 0;
        }
        lua_pushinteger(Luas, f);
        lua_pushinteger(Luas, c);
        if (lua_pcall(Luas, 2, 1, 0) != 0) {       /* two args, 1 result */
            fprintf(stdout, "error: %s\n", lua_tostring(Luas, -1));
            lua_pop(Luas, 2);
            error();
        } else {
            ret = lua_toboolean(Luas, -1);
        }
    }
    return ret;
}
#endif

@ @c
extinfo *new_variant(int glyph, int startconnect, int endconnect,
                     int advance, int repeater)
{
    extinfo *ext;
    ext = xmalloc(sizeof(extinfo));
    ext->next = NULL;
    ext->glyph = glyph;
    ext->start_overlap = startconnect;
    ext->end_overlap = endconnect;
    ext->advance = advance;
    ext->extender = repeater;
    return ext;
}


@ @c
static extinfo *copy_variant(extinfo * old)
{
    extinfo *ext;
    ext = xmalloc(sizeof(extinfo));
    ext->next = NULL;
    ext->glyph = old->glyph;
    ext->start_overlap = old->start_overlap;
    ext->end_overlap = old->end_overlap;
    ext->advance = old->advance;
    ext->extender = old->extender;
    return ext;
}

@ @c
static void dump_variant(extinfo * ext)
{
    dump_int(ext->glyph);
    dump_int(ext->start_overlap);
    dump_int(ext->end_overlap);
    dump_int(ext->advance);
    dump_int(ext->extender);
    return;
}


@ @c
static extinfo *undump_variant(void)
{
    int x;
    extinfo *ext;
    undump_int(x);
    if (x == 0)
        return NULL;
    ext = xmalloc(sizeof(extinfo));
    ext->next = NULL;
    ext->glyph = x;
    undump_int(x);
    ext->start_overlap = x;
    undump_int(x);
    ext->end_overlap = x;
    undump_int(x);
    ext->advance = x;
    undump_int(x);
    ext->extender = x;
    return ext;
}

@ @c
void add_charinfo_vert_variant(charinfo * ci, extinfo * ext)
{
    if (ci->vert_variants == NULL) {
        ci->vert_variants = ext;
    } else {
        extinfo *lst = ci->vert_variants;
        while (lst->next != NULL)
            lst = lst->next;
        lst->next = ext;
    }

}

@ @c
void add_charinfo_hor_variant(charinfo * ci, extinfo * ext)
{
    if (ci->hor_variants == NULL) {
        ci->hor_variants = ext;
    } else {
        extinfo *lst = ci->hor_variants;
        while (lst->next != NULL)
            lst = lst->next;
        lst->next = ext;
    }

}

@ @c
extinfo *copy_variants(extinfo * o)
{
    extinfo *c, *t = NULL, *h = NULL;
    while (o != NULL) {
        c = copy_variant(o);
        if (h == null)
            h = c;
        else
            t->next = c;
        t = c;
        o = o->next;
    }

    return h;
}

@ @c
static void dump_charinfo_variants(extinfo * o)
{
    while (o != NULL) {
        dump_variant(o);
        o = o->next;
    }
    dump_int(0);
    return;
}

@ @c
static extinfo *undump_charinfo_variants(void)
{
    extinfo *c, *t = NULL, *h = NULL;
    c = undump_variant();
    while (c != NULL) {
        if (h == null)
            h = c;
        else
            t->next = c;
        t = c;
        c = undump_variant();
    }
    return h;
}


@ Note that mant more small things like this are implemented
as macros in the header file.
@c
void set_charinfo_width(charinfo * ci, scaled val)
{
    ci->width = val;
}

void set_charinfo_height(charinfo * ci, scaled val)
{
    ci->height = val;
}

void set_charinfo_depth(charinfo * ci, scaled val)
{
    ci->depth = val;
}

void set_charinfo_italic(charinfo * ci, scaled val)
{
    ci->italic = val;
}

void set_charinfo_vert_italic(charinfo * ci, scaled val)
{
    ci->vert_italic = val;
}

void set_charinfo_top_accent(charinfo * ci, scaled val)
{
    ci->top_accent = val;
}

void set_charinfo_bot_accent(charinfo * ci, scaled val)
{
    ci->bot_accent = val;
}

void set_charinfo_tag(charinfo * ci, scaled val)
{
    ci->tag = (char) val;
}

void set_charinfo_remainder(charinfo * ci, scaled val)
{
    ci->remainder = val;
}

void set_charinfo_used(charinfo * ci, scaled val)
{
    ci->used = (char) val;
}

void set_charinfo_index(charinfo * ci, scaled val)
{
    ci->index = (unsigned short) val;
}

void set_charinfo_name(charinfo * ci, char *val)
{
    xfree(ci->name);
    ci->name = val;
}

void set_charinfo_tounicode(charinfo * ci, char *val)
{
    xfree(ci->tounicode);
    ci->tounicode = val;
}

void set_charinfo_ligatures(charinfo * ci, liginfo * val)
{
    dxfree(ci->ligatures, val);
}

void set_charinfo_kerns(charinfo * ci, kerninfo * val)
{
    dxfree(ci->kerns, val);
}

void set_charinfo_packets(charinfo * ci, eight_bits * val)
{
    dxfree(ci->packets, val);
}

void set_charinfo_ef(charinfo * ci, scaled val)
{
    ci->ef = val;
}

void set_charinfo_lp(charinfo * ci, scaled val)
{
    ci->lp = val;
}

void set_charinfo_rp(charinfo * ci, scaled val)
{
    ci->rp = val;
}

@ @c
void set_charinfo_vert_variants(charinfo * ci, extinfo * ext)
{
    extinfo *c, *lst;
    if (ci->vert_variants != NULL) {
        lst = ci->vert_variants;
        while (lst != NULL) {
            c = lst->next;
            free(lst);
            lst = c;
        }
    }
    ci->vert_variants = ext;
}

@ @c
void set_charinfo_hor_variants(charinfo * ci, extinfo * ext)
{
    extinfo *c, *lst;
    if (ci->hor_variants != NULL) {
        lst = ci->hor_variants;
        while (lst != NULL) {
            c = lst->next;
            free(lst);
            lst = c;
        }
    }
    ci->hor_variants = ext;

}

@ @c
int get_charinfo_math_kerns(charinfo * ci, int id)
{

    int k = 0;                  /* all callers check for |result>0| */
    if (id == top_left_kern) {
        k = ci->top_left_math_kerns;
    } else if (id == bottom_left_kern) {
        k = ci->bottom_left_math_kerns;
    } else if (id == top_right_kern) {
        k = ci->top_right_math_kerns;
    } else if (id == bottom_right_kern) {
        k = ci->bottom_right_math_kerns;
    } else {
        confusion("get_charinfo_math_kerns");
    }
    return k;
}

@ @c
void add_charinfo_math_kern(charinfo * ci, int id, scaled ht, scaled krn)
{
    int k;
    if (id == top_left_kern) {
        k = ci->top_left_math_kerns;
        do_realloc(ci->top_left_math_kern_array, ((k + 1) * 2), sizeof(scaled));
        ci->top_left_math_kern_array[(2 * (k))] = ht;
        ci->top_left_math_kern_array[((2 * (k)) + 1)] = krn;
        ci->top_left_math_kerns++;
    } else if (id == bottom_left_kern) {
        k = ci->bottom_left_math_kerns;
        do_realloc(ci->bottom_left_math_kern_array, ((k + 1) * 2), sizeof(scaled));
        ci->bottom_left_math_kern_array[(2 * (k))] = ht;
        ci->bottom_left_math_kern_array[(2 * (k)) + 1] = krn;
        ci->bottom_left_math_kerns++;
    } else if (id == top_right_kern) {
        k = ci->top_right_math_kerns;
        do_realloc(ci->top_right_math_kern_array, ((k + 1) * 2), sizeof(scaled));
        ci->top_right_math_kern_array[(2 * (k))] = ht;
        ci->top_right_math_kern_array[(2 * (k)) + 1] = krn;
        ci->top_right_math_kerns++;
    } else if (id == bottom_right_kern) {
        k = ci->bottom_right_math_kerns;
        do_realloc(ci->bottom_right_math_kern_array, ((k + 1) * 2), sizeof(scaled));
        ci->bottom_right_math_kern_array[(2 * (k))] = ht;
        ci->bottom_right_math_kern_array[(2 * (k)) + 1] = krn;
        ci->bottom_right_math_kerns++;
    } else {
        confusion("add_charinfo_math_kern");
    }
}


@ @c
static void dump_math_kerns(charinfo * ci)
{
    int k, l;
    l = ci->top_left_math_kerns;
    dump_int(l);
    for (k = 0; k < l; k++) {
        dump_int(ci->top_left_math_kern_array[(2 * k)]);
        dump_int(ci->top_left_math_kern_array[(2 * k) + 1]);
    }
    l = ci->bottom_left_math_kerns;
    dump_int(l);
    for (k = 0; k < l; k++) {
        dump_int(ci->bottom_left_math_kern_array[(2 * k)]);
        dump_int(ci->bottom_left_math_kern_array[(2 * k) + 1]);
    }
    l = ci->top_right_math_kerns;
    dump_int(l);
    for (k = 0; k < l; k++) {
        dump_int(ci->top_right_math_kern_array[(2 * k)]);
        dump_int(ci->top_right_math_kern_array[(2 * k) + 1]);
    }
    l = ci->bottom_right_math_kerns;
    dump_int(l);
    for (k = 0; k < l; k++) {
        dump_int(ci->bottom_right_math_kern_array[(2 * k)]);
        dump_int(ci->bottom_right_math_kern_array[(2 * k) + 1]);
    }
}

@ @c
static void undump_math_kerns(charinfo * ci)
{
    int k;
    int x;
    undump_int(x);
    ci->top_left_math_kerns = x;
    if (x > 0)
        ci->top_left_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
    for (k = 0; k < ci->top_left_math_kerns; k++) {
        undump_int(x);
        ci->top_left_math_kern_array[(2 * k)] = (scaled) x;
        undump_int(x);
        ci->top_left_math_kern_array[(2 * k) + 1] = (scaled) x;
    }
    undump_int(x);
    ci->bottom_left_math_kerns = x;
    if (x > 0)
        ci->bottom_left_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
    for (k = 0; k < ci->bottom_left_math_kerns; k++) {
        undump_int(x);
        ci->bottom_left_math_kern_array[(2 * k)] = (scaled) x;
        undump_int(x);
        ci->bottom_left_math_kern_array[(2 * k) + 1] = (scaled) x;
    }
    undump_int(x);
    ci->top_right_math_kerns = x;
    if (x > 0)
        ci->top_right_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
    for (k = 0; k < ci->top_right_math_kerns; k++) {
        undump_int(x);
        ci->top_right_math_kern_array[(2 * k)] = (scaled) x;
        undump_int(x);
        ci->top_right_math_kern_array[(2 * k) + 1] = (scaled) x;
    }
    undump_int(x);
    ci->bottom_right_math_kerns = x;
    if (x > 0)
        ci->bottom_right_math_kern_array = xmalloc((unsigned) (2 * (int) sizeof(scaled) * x));
    for (k = 0; k < ci->bottom_right_math_kerns; k++) {
        undump_int(x);
        ci->bottom_right_math_kern_array[(2 * k)] = (scaled) x;
        undump_int(x);
        ci->bottom_right_math_kern_array[(2 * k) + 1] = (scaled) x;
    }
}


@ In TeX, extensibles were fairly simple things.
   This function squeezes a TFM extensible into the vertical extender structures.
   |advance==0| is a special case for TFM fonts, because finding the proper
   advance width during tfm reading can be tricky


  a small complication arises if |rep| is the only non-zero: it needs to be
  doubled as a non-repeatable to avoid mayhem */

@c
void set_charinfo_extensible(charinfo * ci, int top, int bot, int mid, int rep)
{
    extinfo *ext;
    set_charinfo_vert_variants(ci, NULL);       /* clear old */
    if (bot == 0 && top == 0 && mid == 0 && rep != 0) {
        ext = new_variant(rep, 0, 0, 0, EXT_NORMAL);
        add_charinfo_vert_variant(ci, ext);
        ext = new_variant(rep, 0, 0, 0, EXT_REPEAT);
        add_charinfo_vert_variant(ci, ext);
        return;
    }
    if (bot != 0) {
        ext = new_variant(bot, 0, 0, 0, EXT_NORMAL);
        add_charinfo_vert_variant(ci, ext);
    }
    if (rep != 0) {
        ext = new_variant(rep, 0, 0, 0, EXT_REPEAT);
        add_charinfo_vert_variant(ci, ext);
    }
    if (mid != 0) {
        ext = new_variant(mid, 0, 0, 0, EXT_NORMAL);
        add_charinfo_vert_variant(ci, ext);
        if (rep != 0) {
            ext = new_variant(rep, 0, 0, 0, EXT_REPEAT);
            add_charinfo_vert_variant(ci, ext);
        }
    }
    if (top != 0) {
        ext = new_variant(top, 0, 0, 0, EXT_NORMAL);
        add_charinfo_vert_variant(ci, ext);
    }
}

@ Note that many more simple things like this are implemented as macros
in the header file.

@c
scaled get_charinfo_width(charinfo * ci)
{
    return ci->width;
}

scaled get_charinfo_height(charinfo * ci)
{
    return ci->height;
}

scaled get_charinfo_depth(charinfo * ci)
{
    return ci->depth;
}

scaled get_charinfo_italic(charinfo * ci)
{
    return ci->italic;
}

scaled get_charinfo_vert_italic(charinfo * ci)
{
    return ci->vert_italic;
}

scaled get_charinfo_top_accent(charinfo * ci)
{
    return ci->top_accent;
}

scaled get_charinfo_bot_accent(charinfo * ci)
{
    return ci->bot_accent;
}

char get_charinfo_tag(charinfo * ci)
{
    return ci->tag;
}

int get_charinfo_remainder(charinfo * ci)
{
    return ci->remainder;
}

char get_charinfo_used(charinfo * ci)
{
    return ci->used;
}

int get_charinfo_index(charinfo * ci)
{
    return ci->index;
}

char *get_charinfo_name(charinfo * ci)
{
    return ci->name;
}

char *get_charinfo_tounicode(charinfo * ci)
{
    return ci->tounicode;
}

liginfo *get_charinfo_ligatures(charinfo * ci)
{
    return ci->ligatures;
}

kerninfo *get_charinfo_kerns(charinfo * ci)
{
    return ci->kerns;
}

eight_bits *get_charinfo_packets(charinfo * ci)
{
    return ci->packets;
}

int get_charinfo_ef(charinfo * ci)
{
    return ci->ef;
}

int get_charinfo_rp(charinfo * ci)
{
    return ci->rp;
}

int get_charinfo_lp(charinfo * ci)
{
    return ci->lp;
}

extinfo *get_charinfo_vert_variants(charinfo * ci)
{
    extinfo *w = NULL;
    if (ci->vert_variants != NULL)
        w = ci->vert_variants;
    return w;
}

extinfo *get_charinfo_hor_variants(charinfo * ci)
{
    extinfo *w = NULL;
    if (ci->hor_variants != NULL)
        w = ci->hor_variants;
    return w;
}


scaled char_width(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    scaled w = get_charinfo_width(ci);
    return w;
}

scaled calc_char_width(internal_font_number f, int c, int ex)
{
    charinfo *ci = char_info(f, c);
    scaled w = get_charinfo_width(ci);
    if (ex != 0)
        w = round_xn_over_d(w, 1000 + ex, 1000);
    return w;
}

scaled char_depth(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    scaled d = get_charinfo_depth(ci);
    return d;
}

scaled char_height(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    scaled h = get_charinfo_height(ci);
    return h;
}

scaled char_italic(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    scaled i = get_charinfo_italic(ci);
    return i;
}

scaled char_vert_italic(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    scaled i = get_charinfo_vert_italic(ci);
    return i;
}

scaled char_top_accent(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_top_accent(ci);
}

scaled char_bot_accent(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_bot_accent(ci);
}


int char_remainder(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_remainder(ci);
}

char char_tag(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_tag(ci);
}

char char_used(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_used(ci);
}

char *char_name(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_name(ci);
}

int char_index(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_index(ci);
}

liginfo *char_ligatures(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_ligatures(ci);
}

kerninfo *char_kerns(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_kerns(ci);
}

eight_bits *char_packets(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_packets(ci);
}

@ @c
void set_font_params(internal_font_number f, int b)
{
    int i;
    i = font_params(f);
    if (i != b) {
        font_bytes +=
            (int) ((b - (int) font_params(f) + 1) * (int) sizeof(scaled));
        do_realloc(param_base(f), (b + 2), int);
        font_params(f) = b;
        if (b > i) {
            while (i < b) {
                i++;
                set_font_param(f, i, 0);
            }
        }
    }
}

@ @c
void set_font_math_params(internal_font_number f, int b)
{
    int i;
    i = font_math_params(f);
    if (i != b) {
        font_bytes +=
            ((b - (int) font_math_params(f) + 1) * (int) sizeof(scaled));
        do_realloc(math_param_base(f), (b + 2), int);
        font_math_params(f) = b;
        if (b > i) {
            while (i < b) {
                i++;
                set_font_math_param(f, i, undefined_math_parameter);
            }
        }
    }
}

@ @c
int copy_font(int f)
{
    int i, ci_cnt, ci_size;
    charinfo *ci;
    int k = new_font();

    {
        ci = font_tables[k]->charinfo;
        ci_cnt = font_tables[k]->charinfo_count;
        ci_size = font_tables[k]->charinfo_size;
        memcpy(font_tables[k], font_tables[f], sizeof(texfont));
        font_tables[k]->charinfo = ci;
        font_tables[k]->charinfo_count = ci_cnt;
        font_tables[k]->charinfo_size = ci_size;
    }

    font_malloc_charinfo(k, font_tables[f]->charinfo_count);
    set_font_cache_id(k, 0);
    set_font_used(k, 0);
    set_font_touched(k, 0);

    font_tables[k]->_font_name = NULL;
    font_tables[k]->_font_filename = NULL;
    font_tables[k]->_font_fullname = NULL;
    font_tables[k]->_font_psname = NULL;
    font_tables[k]->_font_encodingname = NULL;
    font_tables[k]->_font_area = NULL;
    font_tables[k]->_font_cidregistry = NULL;
    font_tables[k]->_font_cidordering = NULL;
    font_tables[k]->_left_boundary = NULL;
    font_tables[k]->_right_boundary = NULL;

    set_font_name(k, xstrdup(font_name(f)));
    if (font_filename(f) != NULL)
        set_font_filename(k, xstrdup(font_filename(f)));
    if (font_fullname(f) != NULL)
        set_font_fullname(k, xstrdup(font_fullname(f)));
    if (font_psname(f) != NULL)
        set_font_psname(k, xstrdup(font_psname(f)));
    if (font_encodingname(f) != NULL)
        set_font_encodingname(k, xstrdup(font_encodingname(f)));
    if (font_area(f) != NULL)
        set_font_area(k, xstrdup(font_area(f)));
    if (font_cidregistry(f) != NULL)
        set_font_cidregistry(k, xstrdup(font_cidregistry(f)));
    if (font_cidordering(f) != NULL)
        set_font_cidordering(k, xstrdup(font_cidordering(f)));

    i = (int) (sizeof(*param_base(f)) * (unsigned) (font_params(f)+1));
    font_bytes += i;
    param_base(k) = xmalloc((unsigned) (i+1));
    memcpy(param_base(k), param_base(f), (size_t) (i));

    if (font_math_params(f) > 0) {
        i = (int) (sizeof(*math_param_base(f)) *
                   (unsigned) font_math_params(f));
        font_bytes += i;
        math_param_base(k) = xmalloc((unsigned) i);
        memcpy(math_param_base(k), math_param_base(f), (size_t) i);
    }

    for (i = 0; i <= font_tables[f]->charinfo_count; i++) {
        ci = copy_charinfo(&font_tables[f]->charinfo[i]);
        font_tables[k]->charinfo[i] = *ci;
    }

    if (left_boundary(f) != NULL) {
        ci = copy_charinfo(left_boundary(f));
        set_charinfo(k, left_boundarychar, ci);
    }

    if (right_boundary(f) != NULL) {
        ci = copy_charinfo(right_boundary(f));
        set_charinfo(k, right_boundarychar, ci);
    }
    /* not updated yet: */
    font_tables[k]->charinfo_count = font_tables[f]->charinfo_count;
    return k;
}

@ @c
void delete_font(int f)
{
    int i;
    charinfo *co;
    assert(f > 0);
    if (font_tables[f] != NULL) {
        set_font_name(f, NULL);
        set_font_filename(f, NULL);
        set_font_fullname(f, NULL);
        set_font_psname(f, NULL);
        set_font_encodingname(f, NULL);
        set_font_area(f, NULL);
        set_font_cidregistry(f, NULL);
        set_font_cidordering(f, NULL);
        set_left_boundary(f, NULL);
        set_right_boundary(f, NULL);

        for (i = font_bc(f); i <= font_ec(f); i++) {
            if (quick_char_exists(f, i)) {
                co = char_info(f, i);
                set_charinfo_name(co, NULL);
                set_charinfo_tounicode(co, NULL);
                set_charinfo_packets(co, NULL);
                set_charinfo_ligatures(co, NULL);
                set_charinfo_kerns(co, NULL);
                set_charinfo_vert_variants(co, NULL);
                set_charinfo_hor_variants(co, NULL);
            }
        }
        /* free .notdef */
        set_charinfo_name(font_tables[f]->charinfo + 0, NULL);
        free(font_tables[f]->charinfo);
        destroy_sa_tree(font_tables[f]->characters);

        free(param_base(f));
        if (math_param_base(f) != NULL)
            free(math_param_base(f));
        free(font_tables[f]);
        font_tables[f] = NULL;

        if (font_id_maxval == f) {
            font_id_maxval--;
        }
    }
}

@ @c
void create_null_font(void)
{
    int i = new_font();
    assert(i == 0);
    set_font_name(i, xstrdup("nullfont"));
    set_font_area(i, xstrdup(""));
    set_font_touched(i, 1);
}

@ @c
boolean is_valid_font(int id)
{
    int ret = 0;
    if (id >= 0 && id <= font_id_maxval && font_tables[id] != NULL)
        ret = 1;
    return ret;
}

@ @c
boolean cmp_font_area(int id, str_number t)
{
    char *tt = NULL;
    char *tid = font_area(id);
    if (t == 0) {
        if (tid == NULL || strlen(tid) == 0)
            return 1;
        else
            return 0;
    }
    tt = makecstring(t);
    if ((tt == NULL || strlen(tt) == 0) && (tid == NULL || strlen(tid) == 0))
        return 1;
    if (tt == NULL || strcmp(tid, tt) != 0)
        return 0;
    free(tt);
    return 1;
}

@ Here come some subroutines to deal with expanded fonts for HZ-algorithm.
return 1 == identical
@c
static boolean cmp_font_name(int id, char *tt)
{
    char *tid;
    if (!is_valid_font(id))
        return 0;
    tid = font_name(id);
    if (tt == NULL && tid == NULL)
        return 1;
    if (tt == NULL || tid == NULL || strcmp(tid, tt) != 0)
        return 0;
    return 1;
}

@ @c
int test_no_ligatures(internal_font_number f)
{
    int c;
    for (c = font_bc(f); c <= font_ec(f); c++) {
        if (has_lig(f, c))      /* |char_exists(f,c)| */
            return 0;
    }
    return 1;
}

@ @c
int get_tag_code(internal_font_number f, int c)
{
    small_number i;
    if (char_exists(f, c)) {
        i = char_tag(f, c);
        if (i == lig_tag)
            return 1;
        else if (i == list_tag)
            return 2;
        else if (i == ext_tag)
            return 4;
        else
            return 0;
    }
    return -1;
}

@ @c
int get_lp_code(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_lp(ci);
}

int get_rp_code(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_rp(ci);
}

int get_ef_code(internal_font_number f, int c)
{
    charinfo *ci = char_info(f, c);
    return get_charinfo_ef(ci);
}

@ @c
void set_tag_code(internal_font_number f, int c, int i)
{
    int fixedi;
    charinfo *co;
    if (char_exists(f, c)) {
        /* |abs(fix_int(i-7,0))| */
        fixedi = -(i < -7 ? -7 : (i > 0 ? 0 : i));
        co = char_info(f, c);
        if (fixedi >= 4) {
            if (char_tag(f, c) == ext_tag)
                set_charinfo_tag(co, (char_tag(f, c) - ext_tag));
            fixedi = fixedi - 4;
        }
        if (fixedi >= 2) {
            if (char_tag(f, c) == list_tag)
                set_charinfo_tag(co, (char_tag(f, c) - list_tag));
            fixedi = fixedi - 2;
        };
        if (fixedi >= 1) {
            if (has_lig(f, c))
                set_charinfo_ligatures(co, NULL);
            if (has_kern(f, c))
                set_charinfo_kerns(co, NULL);
        }
    }
}

@ @c
void set_lp_code(internal_font_number f, int c, int i)
{
    charinfo *co;
    if (char_exists(f, c)) {
        co = char_info(f, c);
        set_charinfo_lp(co, i);
    }
}

void set_rp_code(internal_font_number f, int c, int i)
{
    charinfo *co;
    if (char_exists(f, c)) {
        co = char_info(f, c);
        set_charinfo_rp(co, i);
    }
}

void set_ef_code(internal_font_number f, int c, int i)
{
    charinfo *co;
    if (char_exists(f, c)) {
        co = char_info(f, c);
        set_charinfo_ef(co, i);
    }
}

@ @c
void set_no_ligatures(internal_font_number f)
{
    int c;
    charinfo *co;
    if (font_tables[f]->ligatures_disabled)
        return;
    co = char_info(f, left_boundarychar);
    set_charinfo_ligatures(co, NULL);
    co = char_info(f, right_boundarychar);      /* this is weird */
    set_charinfo_ligatures(co, NULL);
    for (c = 0; c < font_tables[f]->charinfo_count; c++) {
        co = font_tables[f]->charinfo + c;
        set_charinfo_ligatures(co, NULL);
    }
    font_tables[f]->ligatures_disabled = 1;
}

@ @c
liginfo get_ligature(internal_font_number f, int lc, int rc)
{
    int k = 0;
    liginfo t, u;
    charinfo *co;
    t.lig = 0;
    t.type = 0;
    t.adj = 0;
    if (lc == non_boundarychar || rc == non_boundarychar || (!has_lig(f, lc)))
        return t;
    co = char_info(f, lc);
    while (1) {
        u = charinfo_ligature(co, k);
        if (lig_end(u))
            break;
        if (lig_char(u) == rc) {
            if (lig_disabled(u)) {
                return t;
            } else {
                return u;
            }
        }
        k++;
    }
    return t;
}

@ @c
scaled raw_get_kern(internal_font_number f, int lc, int rc)
{
    int k = 0;
    kerninfo u;
    charinfo *co;
    if (lc == non_boundarychar || rc == non_boundarychar)
        return 0;
    co = char_info(f, lc);
    while (1) {
        u = charinfo_kern(co, k);
        if (kern_end(u))
            break;
        if (kern_char(u) == rc) {
            if (kern_disabled(u))
                return 0;
            else
                return kern_kern(u);
        }
        k++;
    }
    return 0;
}

@ @c
scaled get_kern(internal_font_number f, int lc, int rc)
{
    if (lc == non_boundarychar || rc == non_boundarychar || (!has_kern(f, lc)))
        return 0;
    return raw_get_kern(f, lc, rc);
}


@ dumping and undumping fonts

@c
#define dump_string(a)			\
    if (a!=NULL) {			\
      x = (int)(strlen(a)+1);		\
      dump_int(x);  dump_things(*a, x);	\
    } else {				\
	x = 0; dump_int(x);		\
    }

static void dump_charinfo(int f, int c)
{
    charinfo *co;
    int x;
    liginfo *lig;
    kerninfo *kern;
    dump_int(c);
    co = char_info(f, c);
    set_charinfo_used(co, 0);
    dump_int(get_charinfo_width(co));
    dump_int(get_charinfo_height(co));
    dump_int(get_charinfo_depth(co));
    dump_int(get_charinfo_italic(co));
    dump_int(get_charinfo_vert_italic(co));
    dump_int(get_charinfo_top_accent(co));
    dump_int(get_charinfo_bot_accent(co));
    dump_int(get_charinfo_tag(co));
    dump_int(get_charinfo_ef(co));
    dump_int(get_charinfo_rp(co));
    dump_int(get_charinfo_lp(co));
    dump_int(get_charinfo_remainder(co));
    dump_int(get_charinfo_used(co));
    dump_int(get_charinfo_index(co));
    dump_string(get_charinfo_name(co));
    dump_string(get_charinfo_tounicode(co));

    /* ligatures */
    x = 0;
    if ((lig = get_charinfo_ligatures(co)) != NULL) {
        while (!lig_end(lig[x])) {
            x++;
        }
        x++;
        dump_int(x);
        dump_things(*lig, x);
    } else {
        dump_int(x);
    }
    /* kerns */
    x = 0;
    if ((kern = get_charinfo_kerns(co)) != NULL) {
        while (!kern_end(kern[x])) {
            x++;
        }
        x++;
        dump_int(x);
        dump_things(*kern, x);
    } else {
        dump_int(x);
    }
    /* packets */
    x = vf_packet_bytes(co);
    dump_int(x);
    if (x > 0) {
        dump_things(*get_charinfo_packets(co), x);
    }

    if (get_charinfo_tag(co) == ext_tag) {
        dump_charinfo_variants(get_charinfo_vert_variants(co));
        dump_charinfo_variants(get_charinfo_hor_variants(co));
    }
    dump_math_kerns(co);
}

static void dump_font_entry(texfont * f)
{
    int x;
    dump_int(f->_font_size);
    dump_int(f->_font_dsize);
    dump_int(f->_font_cidversion);
    dump_int(f->_font_cidsupplement);
    dump_int(f->_font_ec);
    x = (int) f->_font_checksum;
    dump_int(x);
    dump_int(f->_font_used);
    dump_int(f->_font_touched);
    dump_int(f->_font_cache_id);
    dump_int(f->_font_encodingbytes);
    dump_int(f->_font_oldmath);
    dump_int(f->_font_slant);
    dump_int(f->_font_extend);
    dump_int(f->font_max_shrink);
    dump_int(f->font_max_stretch);
    dump_int(f->_font_step);
    dump_int(f->_font_auto_expand);
    dump_int(f->_font_tounicode);
    dump_int(f->_font_type);
    dump_int(f->_font_format);
    dump_int(f->_font_embedding);
    dump_int(f->_font_bc);
    dump_int(f->_hyphen_char);
    dump_int(f->_skew_char);
    dump_int(f->_font_natural_dir);
    dump_int(f->_font_params);
    dump_int(f->_font_math_params);
    dump_int(f->ligatures_disabled);
    dump_int(f->_pdf_font_num);
    dump_int(f->_pdf_font_attr);
}

void dump_font(int f)
{
    int i, x;

    set_font_used(f, 0);
    font_tables[f]->charinfo_cache = NULL;
    dump_font_entry(font_tables[f]);
    dump_string(font_name(f));
    dump_string(font_area(f));
    dump_string(font_filename(f));
    dump_string(font_fullname(f));
    dump_string(font_psname(f));
    dump_string(font_encodingname(f));
    dump_string(font_cidregistry(f));
    dump_string(font_cidordering(f));

    dump_things(*param_base(f), (font_params(f) + 1));

    if (font_math_params(f) > 0) {
        dump_things(*math_param_base(f), (font_math_params(f) + 1 ));
    }
    if (has_left_boundary(f)) {
        dump_int(1);
        dump_charinfo(f, left_boundarychar);
    } else {
        dump_int(0);
    }
    if (has_right_boundary(f)) {
        dump_int(1);
        dump_charinfo(f, right_boundarychar);
    } else {
        dump_int(0);
    }

    for (i = font_bc(f); i <= font_ec(f); i++) {
        if (quick_char_exists(f, i)) {
            dump_charinfo(f, i);
        }
    }
}

@ @c
static int undump_charinfo(int f)
{
    charinfo *co;
    int x, i;
    char *s = NULL;
    liginfo *lig = NULL;
    kerninfo *kern = NULL;
    eight_bits *packet = NULL;

    undump_int(i);
    co = get_charinfo(f, i);
    undump_int(x);
    set_charinfo_width(co, x);
    undump_int(x);
    set_charinfo_height(co, x);
    undump_int(x);
    set_charinfo_depth(co, x);
    undump_int(x);
    set_charinfo_italic(co, x);
    undump_int(x);
    set_charinfo_vert_italic(co, x);
    undump_int(x);
    set_charinfo_top_accent(co, x);
    undump_int(x);
    set_charinfo_bot_accent(co, x);
    undump_int(x);
    set_charinfo_tag(co, x);
    undump_int(x);
    set_charinfo_ef(co, x);
    undump_int(x);
    set_charinfo_rp(co, x);
    undump_int(x);
    set_charinfo_lp(co, x);
    undump_int(x);
    set_charinfo_remainder(co, x);
    undump_int(x);
    set_charinfo_used(co, x);
    undump_int(x);
    set_charinfo_index(co, x);

    /* name */
    undump_int(x);
    if (x > 0) {
        font_bytes += x;
        s = xmalloc((unsigned) x);
        undump_things(*s, x);
    }
    set_charinfo_name(co, s);
    /* tounicode */
    undump_int(x);
    if (x > 0) {
        font_bytes += x;
        s = xmalloc((unsigned) x);
        undump_things(*s, x);
    }
    set_charinfo_tounicode(co, s);
    /* ligatures */
    undump_int(x);
    if (x > 0) {
        font_bytes += (int) ((unsigned) x * sizeof(liginfo));
        lig = xmalloc((unsigned) ((unsigned) x * sizeof(liginfo)));
        undump_things(*lig, x);
    }
    set_charinfo_ligatures(co, lig);
    /* kerns */
    undump_int(x);
    if (x > 0) {
        font_bytes += (int) ((unsigned) x * sizeof(kerninfo));
        kern = xmalloc((unsigned) ((unsigned) x * sizeof(kerninfo)));
        undump_things(*kern, x);
    }
    set_charinfo_kerns(co, kern);

    /* packets */
    undump_int(x);
    if (x > 0) {
        font_bytes += x;
        packet = xmalloc((unsigned) x);
        undump_things(*packet, x);
    }
    set_charinfo_packets(co, packet);

    if (get_charinfo_tag(co) == ext_tag) {
        set_charinfo_vert_variants(co, undump_charinfo_variants());
        set_charinfo_hor_variants(co, undump_charinfo_variants());
    }
    undump_math_kerns(co);
    return i;
}

#define undump_font_string(a) \
    undump_int (x); \
    if (x>0) { \
        font_bytes += x; \
        s = xmalloc((unsigned)x); \
        undump_things(*s,x); \
        a(f,s); \
    }

static void undump_font_entry(texfont * f)
{
    int x = 0;
    /* *INDENT-OFF* */
    undump_int(x); f->_font_size = x;
    undump_int(x); f->_font_dsize = x;
    undump_int(x); f->_font_cidversion = x;
    undump_int(x); f->_font_cidsupplement = x;
    undump_int(x); f->_font_ec = x;
    undump_int(x); f->_font_checksum = (unsigned)x;
    undump_int(x); f->_font_used = (char)x;
    undump_int(x); f->_font_touched = (char)x;
    undump_int(x); f->_font_cache_id = x;
    undump_int(x); f->_font_encodingbytes = (char)x;
    undump_int(x); f->_font_oldmath = x;
    undump_int(x); f->_font_slant = x;
    undump_int(x); f->_font_extend = x;
    undump_int(x); f->font_max_shrink = x;
    undump_int(x); f->font_max_stretch = x;
    undump_int(x); f->_font_step = x;
    undump_int(x); f->_font_auto_expand = x;
    undump_int(x); f->_font_tounicode = (char)x;
    undump_int(x); f->_font_type = x;
    undump_int(x); f->_font_format = x;
    undump_int(x); f->_font_embedding = x;
    undump_int(x); f->_font_bc = x;
    undump_int(x); f->_hyphen_char = x;
    undump_int(x); f->_skew_char = x;
    undump_int(x); f->_font_natural_dir = x;
    undump_int(x); f->_font_params = x;
    undump_int(x); f->_font_math_params = x;
    undump_int(x); f->ligatures_disabled = x;
    undump_int(x); f->_pdf_font_num = x;
    undump_int(x); f->_pdf_font_attr = x;
    /* *INDENT-ON* */
}

void undump_font(int f)
{
    int x, i;
    texfont *tt;
    charinfo *ci;
    char *s;
    sa_tree_item sa_value = { 0 };
    grow_font_table(f);
    tt = xmalloc(sizeof(texfont));
    memset(tt, 0, sizeof(texfont));
    font_bytes += (int) sizeof(texfont);
    undump_font_entry(tt);
    font_tables[f] = tt;

    undump_font_string(set_font_name);
    undump_font_string(set_font_area);
    undump_font_string(set_font_filename);
    undump_font_string(set_font_fullname);
    undump_font_string(set_font_psname);
    undump_font_string(set_font_encodingname);
    undump_font_string(set_font_cidregistry);
    undump_font_string(set_font_cidordering);

    i = (int) (sizeof(*param_base(f)) * ((unsigned) font_params(f) + 1));
    font_bytes += i;
    param_base(f) = xmalloc((unsigned) i);
    undump_things(*param_base(f), (font_params(f) + 1));

    if (font_math_params(f) > 0) {
        i = (int) (sizeof(*math_param_base(f)) *
                   ((unsigned) font_math_params(f) + 1));
        font_bytes += i;
        math_param_base(f) = xmalloc((unsigned) i);
        undump_things(*math_param_base(f), (font_math_params(f) + 1));
    }

    /* stack size 1, default item value 0 */
    font_tables[f]->characters = new_sa_tree(1, 1, sa_value);
    ci = xcalloc(1, sizeof(charinfo));
    set_charinfo_name(ci, xstrdup(".notdef"));
    font_tables[f]->charinfo = ci;
    undump_int(x);
    if (x) {
        i = undump_charinfo(f); /* left boundary */
    }
    undump_int(x);
    if (x) {
        i = undump_charinfo(f); /* right boundary */
    }

    i = font_bc(f);
    while (i < font_ec(f)) {
        i = undump_charinfo(f);
    }
}

/* moved from pdffont.w */

@ @c
int pk_dpi; /* PK pixel density value from \.{texmf.cnf} */

@ @c
internal_font_number tfm_lookup(char *s, scaled fs)
{   /* looks up for a TFM with name |s| loaded at |fs| size; if found then flushes |s| */
    internal_font_number k;
    if (fs != 0) {
        for (k = 1; k <= max_font_id(); k++) {
            if (cmp_font_name(k, s) && font_size(k) == fs) {
                return k;
            }
        }
    } else {
        for (k = 1; k <= max_font_id(); k++) {
            if (cmp_font_name(k, s)) {
                return k;
            }
        }
    }
    return null_font;
}

@ @c
int fix_expand_value(internal_font_number f, int e)
{                               /* return the multiple of |font_step(f)| that is nearest to |e| */
    int step;
    int max_expand;
    boolean neg;
    if (e == 0)
        return 0;
    if (e < 0) {
        e = -e;
        neg = true;
        max_expand = font_max_shrink(f);
    } else {
        neg = false;
        max_expand = font_max_stretch(f);
    }
    if (e > max_expand) {
        e = max_expand;
    } else {
        step = font_step(f);
        if (e % step > 0)
            e = step * round_xn_over_d(e, 1, step);
    }
    if (neg)
        e = -e;
    return e;
}

@ @c
void set_expand_params(internal_font_number f, boolean auto_expand,
                       int stretch_limit, int shrink_limit, int font_step)
{                               /* expand a font with given parameters */
    set_font_step(f, font_step);
    set_font_auto_expand(f, auto_expand);
    set_font_max_shrink(f, shrink_limit);
    set_font_max_stretch(f, stretch_limit);
}

@ @c
void read_expand_font(void)
{                               /* read font expansion spec and load expanded font */
    int shrink_limit, stretch_limit, font_step;
    internal_font_number f;
    boolean auto_expand;
    /* read font expansion parameters */
    scan_font_ident();
    f = cur_val;
    if (f == null_font)
        normal_error("font expansion", "invalid font identifier");
    /*if (pdf_font_blink(f) != null_font)*/
    /*    normal_error("font expansion",*/
    /*              "\\fontexpand cannot be used this way (the base font has been expanded)");*/
    scan_optional_equals();
    scan_int();
    stretch_limit = fix_int(cur_val, 0, 1000);
    scan_int();
    shrink_limit = fix_int(cur_val, 0, 500);
    scan_int();
    font_step = fix_int(cur_val, 0, 100);
    if (font_step == 0)
        normal_error("font expansion", "invalid step");
    stretch_limit = stretch_limit - stretch_limit % font_step;
    if (stretch_limit < 0)
        stretch_limit = 0;
    shrink_limit = shrink_limit - shrink_limit % font_step;
    if (shrink_limit < 0)
        shrink_limit = 0;
    if ((stretch_limit == 0) && (shrink_limit == 0))
        normal_error("font expansion", "invalid limit(s)");
    auto_expand = false;
    if (scan_keyword("autoexpand")) {
        auto_expand = true;
        /* Scan an optional space */
        get_x_token();
        if (cur_cmd != spacer_cmd)
            back_input();
    }

    if (font_step(f) != 0) {
        /* this font has been expanded, ensure the expansion parameters are identical */
        if (font_step(f) != font_step)
            normal_error("font expansion","font has been expanded with different expansion step");

        if (((font_max_stretch(f) == 0) && (stretch_limit != 0)) ||
            ((font_max_stretch(f) > 0)
             && (font_max_stretch(f) != stretch_limit)))
            normal_error("font expansion","font has been expanded with different stretch limit");

        if (((font_max_shrink(f) == 0) && (shrink_limit != 0)) ||
            ((font_max_shrink(f) > 0)
             && (font_max_shrink(f) != shrink_limit)))
            normal_error("font expansion","font has been expanded with different shrink limit");

        if (font_auto_expand(f) != auto_expand)
            normal_error("font expansion","font has been expanded with different auto expansion value");
    } else {
        if (font_used(f))
            normal_warning("font expansion", "font should be expanded before its first use");
        set_expand_params(f, auto_expand, stretch_limit, shrink_limit,
                          font_step);
    }
}

@ @c
void new_letterspaced_font(small_number a)
{                               /* letter-space a font by creating a virtual font */
    pointer u;                  /* user's font identifier */
    str_number t;               /* name for the frozen font identifier */
    internal_font_number f, k;
    boolean nolig = false;
    get_r_token();
    u = cur_cs;
    if (u >= hash_base)
        t = cs_text(u);
    else
        t = maketexstring("FONT");
    define(u, set_font_cmd, null_font);
    scan_optional_equals();
    scan_font_ident();
    k = cur_val;
    scan_int();
    if (scan_keyword("nolig"))
       nolig=true;
    f = letter_space_font(k, fix_int(cur_val, -1000, 1000), nolig);
    equiv(u) = f;
    eqtb[font_id_base + f] = eqtb[u];
    font_id_text(f) = t;
}

@ @c
void make_font_copy(small_number a)
{                               /* make a font copy for further use with font expansion */
    pointer u;                  /* user's font identifier */
    str_number t;               /* name for the frozen font identifier */
    internal_font_number f, k;
    get_r_token();
    u = cur_cs;
    if (u >= hash_base)
        t = cs_text(u);
    else
        t = maketexstring("FONT");
    define(u, set_font_cmd, null_font);
    scan_optional_equals();
    scan_font_ident();
    k = cur_val;
    f = copy_font_info(k);
    equiv(u) = f;
    eqtb[font_id_base + f] = eqtb[u];
    font_id_text(f) = t;
}

@ @c
void glyph_to_unicode(void)
{
    str_number s1, s2;
    scan_toks(false, true);
    s1 = tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    scan_toks(false, true);
    s2 = tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    def_tounicode(s1, s2);
    flush_str(s2);
    flush_str(s1);
}
