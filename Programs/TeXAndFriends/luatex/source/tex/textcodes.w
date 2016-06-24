% textcodes.w
%
% Copyright 2006-2010 Taco Hoekwater <taco@@luatex.org>
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

@ catcodes @c

#define CATCODESTACK       8
#define CATCODEDEFAULT    12
#define CATCODE_MAX    32767

static sa_tree *catcode_heads = NULL;
static int catcode_max = 0;
static unsigned char *catcode_valid = NULL;

void set_cat_code(int h, int n, halfword v, quarterword gl)
{
    sa_tree_item sa_value = { 0 };
    sa_tree s = catcode_heads[h];
    if (h > catcode_max)
        catcode_max = h;
    if (s == NULL) {
        sa_value.int_value = CATCODEDEFAULT;
        s = new_sa_tree(CATCODESTACK, 1, sa_value);
        catcode_heads[h] = s;
    }
    sa_value.int_value = (int) v;
    set_sa_item(s, n, sa_value, gl);
}

halfword get_cat_code(int h, int n)
{
    sa_tree_item sa_value = { 0 };
    sa_tree s = catcode_heads[h];
    if (h > catcode_max)
        catcode_max = h;
    if (s == NULL) {
        sa_value.int_value = CATCODEDEFAULT;
        s = new_sa_tree(CATCODESTACK, 1, sa_value);
        catcode_heads[h] = s;
    }
    return (halfword) get_sa_item(s, n).int_value;
}

void unsave_cat_codes(int h, quarterword gl)
{
    int k;
    if (h > catcode_max)
        catcode_max = h;
    for (k = 0; k <= catcode_max; k++) {
        if (catcode_heads[k] != NULL)
            restore_sa_stack(catcode_heads[k], gl);
    }
}

static void initializecatcodes(void)
{
    sa_tree_item sa_value = { 0 };
    catcode_max = 0;
    catcode_heads = Mxmalloc_array(sa_tree, (CATCODE_MAX + 1));
    catcode_valid = Mxmalloc_array(unsigned char, (CATCODE_MAX + 1));
    memset(catcode_heads, 0, sizeof(sa_tree) * (CATCODE_MAX + 1));
    memset(catcode_valid, 0, sizeof(unsigned char) * (CATCODE_MAX + 1));
    catcode_valid[0] = 1;
    sa_value.int_value = CATCODEDEFAULT;
    catcode_heads[0] = new_sa_tree(CATCODESTACK, 1, sa_value);
}

static void dumpcatcodes(void)
{
    int total = 0;
    int k;
    for (k = 0; k <= catcode_max; k++) {
        if (catcode_valid[k]) {
            total++;
        }
    }
    dump_int(catcode_max);
    dump_int(total);
    for (k = 0; k <= catcode_max; k++) {
        if (catcode_valid[k]) {
            dump_int(k);
            dump_sa_tree(catcode_heads[k],"catcodes");
        }
    }
}

static void undumpcatcodes(void)
{
    int total, k, x;
    xfree(catcode_heads);
    xfree(catcode_valid);
    catcode_heads = Mxmalloc_array(sa_tree, (CATCODE_MAX + 1));
    catcode_valid = Mxmalloc_array(unsigned char, (CATCODE_MAX + 1));
    memset(catcode_heads, 0, sizeof(sa_tree) * (CATCODE_MAX + 1));
    memset(catcode_valid, 0, sizeof(unsigned char) * (CATCODE_MAX + 1));
    undump_int(catcode_max);
    undump_int(total);
    for (k = 0; k < total; k++) {
        undump_int(x);
        catcode_heads[x] = undump_sa_tree("catcodes");
        catcode_valid[x] = 1;
    }
}

int valid_catcode_table(int h)
{
    if (h <= CATCODE_MAX && h >= 0 && catcode_valid[h]) {
        return 1;
    }
    return 0;
}

void copy_cat_codes(int from, int to)
{
    if (from < 0 || from > CATCODE_MAX || catcode_valid[from] == 0) {
        uexit(1);
    }
    if (to > catcode_max)
        catcode_max = to;
    destroy_sa_tree(catcode_heads[to]);
    catcode_heads[to] = copy_sa_tree(catcode_heads[from]);
    catcode_valid[to] = 1;
}

void initex_cat_codes(int h)
{
    int k;
    if (h > catcode_max)
        catcode_max = h;
    destroy_sa_tree(catcode_heads[h]);
    catcode_heads[h] = NULL;
    set_cat_code(h, '\r', car_ret_cmd, 1);
    set_cat_code(h, ' ', spacer_cmd, 1);
    set_cat_code(h, '\\', escape_cmd, 1);
    set_cat_code(h, '%', comment_cmd, 1);
    set_cat_code(h, 127, invalid_char_cmd, 1);
    set_cat_code(h, 0, ignore_cmd, 1);
    set_cat_code(h, 0xFEFF, ignore_cmd, 1);
    for (k = 'A'; k <= 'Z'; k++) {
        set_cat_code(h, k, letter_cmd, 1);
        set_cat_code(h, k + 'a' - 'A', letter_cmd, 1);
    }
    catcode_valid[h] = 1;
}

static void freecatcodes(void)
{
    int k;
    for (k = 0; k <= catcode_max; k++) {
        if (catcode_valid[k]) {
            destroy_sa_tree(catcode_heads[k]);
        }
    }
    xfree(catcode_heads);
    xfree(catcode_valid);
}

@ lccodes @c

#define LCCODESTACK   8
#define LCCODEDEFAULT 0

static sa_tree lccode_head = NULL;

void set_lc_code(int n, halfword v, quarterword gl)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = (int) v;
    set_sa_item(lccode_head, n, sa_value, gl);
}

halfword get_lc_code(int n)
{
    return (halfword) get_sa_item(lccode_head, n).int_value;
}

static void unsavelccodes(quarterword gl)
{
    restore_sa_stack(lccode_head, gl);
}

static void initializelccodes(void)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = LCCODEDEFAULT;
    lccode_head = new_sa_tree(LCCODESTACK, 1, sa_value);
}

static void dumplccodes(void)
{
    dump_sa_tree(lccode_head,"lccodes");
}

static void undumplccodes(void)
{
    lccode_head = undump_sa_tree("lccodes");
}

static void freelccodes(void)
{
    destroy_sa_tree(lccode_head);
}

@ uccodes @c

#define UCCODESTACK   8
#define UCCODEDEFAULT 0

static sa_tree uccode_head = NULL;

void set_uc_code(int n, halfword v, quarterword gl)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = (int) v;
    set_sa_item(uccode_head, n, sa_value, gl);
}

halfword get_uc_code(int n)
{
    return (halfword) get_sa_item(uccode_head, n).int_value;
}

static void unsaveuccodes(quarterword gl)
{
    restore_sa_stack(uccode_head, gl);
}

static void initializeuccodes(void)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = UCCODEDEFAULT;
    uccode_head = new_sa_tree(UCCODESTACK, 1, sa_value);
}

static void dumpuccodes(void)
{
    dump_sa_tree(uccode_head,"uccodes");
}

static void undumpuccodes(void)
{
    uccode_head = undump_sa_tree("uccodes");
}

static void freeuccodes(void)
{
    destroy_sa_tree(uccode_head);
}

@ sfcodes @c

#define SFCODESTACK      8
#define SFCODEDEFAULT 1000

static sa_tree sfcode_head = NULL;

void set_sf_code(int n, halfword v, quarterword gl)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = (int) v;
    set_sa_item(sfcode_head, n, sa_value, gl);
}

halfword get_sf_code(int n)
{
    return (halfword) get_sa_item(sfcode_head, n).int_value;
}

static void unsavesfcodes(quarterword gl)
{
    restore_sa_stack(sfcode_head, gl);
}

static void initializesfcodes(void)
{
    sa_tree_item sa_value = { 0 };
    sa_value.int_value = SFCODEDEFAULT;
    sfcode_head = new_sa_tree(SFCODESTACK, 1, sa_value);
}

static void dumpsfcodes(void)
{
    dump_sa_tree(sfcode_head,"sfcodes");
}

static void undumpsfcodes(void)
{
    sfcode_head = undump_sa_tree("sfcodes");
}

static void freesfcodes(void)
{
    destroy_sa_tree(sfcode_head);
}

@ hjcodes @c

#define HJCODESTACK       8
#define HJCODEDEFAULT     0
#define HJCODE_MAX    16383 /* number of languages */

static sa_tree *hjcode_heads = NULL;
static int hjcode_max = 0;
static unsigned char *hjcode_valid = NULL;

@ Here we set codes but we don't initialize from lccodes.

@c void set_hj_code(int h, int n, halfword v, quarterword gl)
{
    sa_tree_item sa_value = { 0 };
    sa_tree s = hjcode_heads[h];
    if (h > hjcode_max)
        hjcode_max = h;
    if (s == NULL) {
        sa_value.int_value = HJCODEDEFAULT;
        s = new_sa_tree(HJCODESTACK, 1, sa_value);
        hjcode_heads[h] = s;
    }
    sa_value.int_value = (int) v;
    set_sa_item(s, n, sa_value, gl);
}

@ We just return the lccodes when nothing is set.

@c halfword get_hj_code(int h, int n)
{
    sa_tree s = hjcode_heads[h];
    if (s == NULL) {
        s = lccode_head;
    }
    return (halfword) get_sa_item(s, n).int_value;
}

@ We don't restore as we can have more languages in a paragraph
and hyphenation takes place at a later stage so we would get
weird grouping side effects .. so, one can overload settings
but management is then upto the used, so no:

@c
/*
    static void unsavehjcodes(quarterword gl) { }
*/

static void initializehjcodes(void)
{
    /*
        sa_tree_item sa_value = { 0 };
    */
    hjcode_max = 0;
    hjcode_heads = Mxmalloc_array(sa_tree, (HJCODE_MAX + 1));
    hjcode_valid = Mxmalloc_array(unsigned char, (HJCODE_MAX + 1));
    memset(hjcode_heads, 0, sizeof(sa_tree) * (HJCODE_MAX + 1));
    memset(hjcode_valid, 0, sizeof(unsigned char) * (HJCODE_MAX + 1));
    /*
        hjcode_valid[0] = 1;
        sa_value.int_value = HJCODEDEFAULT;
        hjcode_heads[0] = new_sa_tree(HJCODESTACK, 1, sa_value);
    */
}

void hj_codes_from_lc_codes(int h)
{
    sa_tree_item sa_value = { 0 };
    sa_tree s = hjcode_heads[h];
    if (s != NULL) {
        destroy_sa_tree(s);
    } else if (h > hjcode_max) {
        hjcode_max = h;
    }
    if (s == NULL) {
        sa_value.int_value = HJCODEDEFAULT;
        s = new_sa_tree(HJCODESTACK, 1, sa_value);
        hjcode_heads[h] = s;
    }
    hjcode_heads[h] = copy_sa_tree(lccode_head);
    hjcode_valid[h] = 1;
}

static void dumphjcodes(void)
{
    int total = 0;
    int k;
    for (k = 0; k <= hjcode_max; k++) {
        if (hjcode_valid[k]) {
            total++;
        }
    }
    dump_int(hjcode_max);
    dump_int(total);
    for (k = 0; k <= hjcode_max; k++) {
        if (hjcode_valid[k]) {
            dump_int(k);
            dump_sa_tree(hjcode_heads[k],"hjcodes");
        }
    }
}

static void undumphjcodes(void)
{
    int total, k, x;
    xfree(hjcode_heads);
    xfree(hjcode_valid);
    hjcode_heads = Mxmalloc_array(sa_tree, (HJCODE_MAX + 1));
    hjcode_valid = Mxmalloc_array(unsigned char, (HJCODE_MAX + 1));
    memset(hjcode_heads, 0, sizeof(sa_tree) * (HJCODE_MAX + 1));
    memset(hjcode_valid, 0, sizeof(unsigned char) * (HJCODE_MAX + 1));
    undump_int(hjcode_max);
    undump_int(total);
    for (k = 0; k < total; k++) {
        undump_int(x);
        hjcode_heads[x] = undump_sa_tree("hjcodes");
        hjcode_valid[x] = 1;
    }
}

static void freehjcodes(void)
{
    int k;
    for (k = 0; k <= hjcode_max; k++) {
        if (hjcode_valid[k]) {
            destroy_sa_tree(hjcode_heads[k]);
        }
    }
    xfree(hjcode_heads);
    xfree(hjcode_valid);
}

/* management */

void unsave_text_codes(quarterword grouplevel)
{
    unsavelccodes(grouplevel);
    unsaveuccodes(grouplevel);
    unsavesfcodes(grouplevel);
}

void initialize_text_codes(void)
{
    initializecatcodes();
    initializelccodes();
    initializeuccodes();
    initializesfcodes();
    initializehjcodes();
}

void free_text_codes(void)
{
    freecatcodes();
    freelccodes();
    freeuccodes();
    freesfcodes();
    freehjcodes();
}

void dump_text_codes(void)
{
    dumpcatcodes();
    dumplccodes();
    dumpuccodes();
    dumpsfcodes();
    dumphjcodes();
}

void undump_text_codes(void)
{
    undumpcatcodes();
    undumplccodes();
    undumpuccodes();
    undumpsfcodes();
    undumphjcodes();
}
