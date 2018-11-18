/* texnodes.h

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


#include <stdarg.h>

#ifndef __NODES_H__
#  define __NODES_H__

#  define varmemcast(a) (memory_word *)(a)

extern memory_word *volatile varmem;
extern halfword var_mem_max;

extern halfword get_node(int s);
extern void free_node(halfword p, int s);
extern void init_node_mem(int s);
extern void dump_node_mem(void);
extern void undump_node_mem(void);

#  define max_halfword  0x3FFFFFFF
#  define max_dimen     0x3FFFFFFF
#  ifndef null
#    define null        0
#  endif
#  define null_flag    -0x40000000
#  define zero_glue     0
#  define normal        0

#  define vinfo(a)      varmem[(a)].hh.v.LH
#  define vlink(a)      varmem[(a)].hh.v.RH
#  define type(a)       varmem[(a)].hh.u.B0
#  define subtype(a)    varmem[(a)].hh.u.B1
#  define node_attr(a)  vinfo((a)+1)
#  define alink(a)      vlink((a)+1)

#  define node_size(a)  varmem[(a)].hh.v.LH

#  define rlink(a)      vlink((a)+1) /* aka alink() */
#  define tlink(a)      vinfo((a)+1) /* overlaps with node_attr() */

/* really special head node pointers that only need links */

#  define temp_node_size 2

/* attribute lists */

#  define UNUSED_ATTRIBUTE -0x7FFFFFFF  /* as low as it goes */

/* it is convenient to have attribute list nodes and attribute node
 * be the same size
 */

#  define attribute_node_size 2
#  define cache_disabled max_halfword

#  define attr_list_ref(a)   vinfo((a)+1) /* the reference count */
#  define attribute_id(a)    vinfo((a)+1)
#  define attribute_value(a) vlink((a)+1)

#  define assign_attribute_ref(n,p) do {     \
        node_attr(n) = p;attr_list_ref(p)++; \
} while (0)

#  define add_node_attr_ref(a) do {     \
    if (a!=null)  attr_list_ref((a))++; \
} while (0)

#  define  replace_attribute_list(a,b) do { \
    delete_attribute_ref(node_attr(a));     \
    node_attr(a)=b;                         \
} while (0)

extern void update_attribute_cache(void);
extern halfword copy_attribute_list(halfword n);
extern halfword do_set_attribute(halfword p, int i, int val);

#  define width_offset 2
#  define depth_offset 3
#  define height_offset 4
#  define list_offset 6

typedef enum {
    user_skip_glue,
    line_skip_glue,
    baseline_skip_glue,
    par_skip_glue,
    above_display_skip_glue,
    below_display_skip_glue,
    above_display_short_skip_glue,
    below_display_short_skip_glue,
    left_skip_glue,
    right_skip_glue,
    top_skip_glue,
    split_top_skip_glue,
    tab_skip_glue,
    space_skip_glue,
    xspace_skip_glue,
    par_fill_skip_glue,
    math_skip_glue,
    thin_mu_skip_glue,
    med_mu_skip_glue,
    thick_mu_skip_glue,
    /* math */
    cond_math_glue = 98,        /* special |subtype| to suppress glue in the next node */
    mu_glue,                    /* |subtype| for math glue */
    /* leaders */
    a_leaders,                  /* |subtype| for aligned leaders */
    c_leaders,                  /* |subtype| for centered leaders */
    x_leaders,                  /* |subtype| for expanded leaders */
    g_leaders                   /* |subtype| for global (page) leaders */
} glue_subtypes;

/* normal nodes */

#  define inf_bad              10000         /* infinitely bad value */
#  define inf_penalty          inf_bad       /* ``infinite'' penalty value */
#  define eject_penalty      -(inf_penalty)  /* ``negatively infinite'' penalty value */

#  define penalty_node_size    3
#  define penalty(a)           vlink((a)+2)

typedef enum {
    user_penalty,
    linebreak_penalty, /* includes widow, club, broken ect */
    line_penalty,
    word_penalty,
    final_penalty,
    noad_penalty,
    before_display_penalty,
    after_display_penalty,
    equation_number_penalty,
} penalty_subtypes ;

/*
#  define glue_node_size       4
#  define glue_ptr(a)          vinfo((a)+2)
#  define leader_ptr(a)        vlink((a)+2)
#  define synctex_tag_glue(a)  vinfo((a)+3)
#  define synctex_line_glue(a) vlink((a)+3)
*/

#  define glue_node_size       7
#  define glue_spec_size       5
/* define width(a)             vinfo((a)+2) */
/* define leader_ptr(a)        vlink((a)+2) */
#  define shrink(a)            vinfo((a)+3)
#  define stretch(a)           vlink((a)+3)
#  define stretch_order(a)     vinfo((a)+4)
#  define shrink_order(a)      vlink((a)+4)
#  define leader_ptr(a)        vlink((a)+5) /* should be in vlink((a)+2) but fails */
#  define synctex_tag_glue(a)  vinfo((a)+6)
#  define synctex_line_glue(a) vlink((a)+6)

#define glue_is_zero(p) \
    ((p == null) || (width(p) == 0 && stretch(p) == 0 && shrink(p) == 0))

#define glue_is_positive(p) \
    ((p == null) || (width(p) > 0))

#define reset_glue_to_zero(p) \
    if (p != null) { \
        width(p) = 0; \
        stretch(p) = 0; \
        shrink(p) = 0; \
        stretch_order(p) = 0; \
        shrink_order(p) = 0; \
    }

#define copy_glue_values(p,q) \
    if (q == null) { \
        width(p) = 0; \
        stretch(p) = 0; \
        shrink(p) = 0; \
        stretch_order(p) = 0; \
        shrink_order(p) = 0; \
    } else { \
        width(p) = width(q); \
        stretch(p) = stretch(q); \
        shrink(p) = shrink(q); \
        stretch_order(p) = stretch_order(q); \
        shrink_order(p) = shrink_order(q); \
    }


/*
    disc nodes could eventually be smaller, because the indirect
    pointers are not really needed (8 instead of 10).
*/

typedef enum {
    discretionary_disc = 0,
    explicit_disc,
    automatic_disc,
    syllable_disc,
    init_disc,                  /* first of a duo of syllable_discs */
    select_disc,                /* second of a duo of syllable_discs */
} discretionary_subtypes;

#  define disc_node_size      11
#  define pre_break_head(a)   ((a)+5)
#  define post_break_head(a)  ((a)+7)
#  define no_break_head(a)    ((a)+9)

#  define disc_penalty(a)     vlink((a)+2)
#  define pre_break(a)        vinfo((a)+3)
#  define post_break(a)       vlink((a)+3)
#  define no_break(a)         vlink((a)+4) /* we have vinfo((a)+4) for later usage */

#  define vlink_pre_break(a)  vlink(pre_break_head(a))
#  define vlink_post_break(a) vlink(post_break_head(a))
#  define vlink_no_break(a)   vlink(no_break_head(a))

#  define tlink_pre_break(a)  tlink(pre_break_head(a))
#  define tlink_post_break(a) tlink(post_break_head(a))
#  define tlink_no_break(a)   tlink(no_break_head(a))

typedef enum {
    font_kern = 0,
    explicit_kern,  /* |subtype| of kern nodes from \.{\\kern} and \.{\\/} */
    accent_kern,    /* |subtype| of kern nodes from accents */
    italic_kern,
} kern_subtypes;

#  define kern_node_size       5
#  define ex_kern(a)           vinfo((a)+3)  /* expansion factor (hz) */
#  define synctex_tag_kern(a)  vinfo((a)+4)
#  define synctex_line_kern(a) vlink((a)+4)

#  define box_node_size 9

/* todo, make an enum of this */

typedef enum {
    unknown_list              =  0,
    line_list                 =  1, /* paragraph lines */
    hbox_list                 =  2, /* \.{\\hbox} */
    indent_list               =  3, /* indentation box */
    align_row_list            =  4, /* row from a \.{\\halign} or \.{\\valign} */
    align_cell_list           =  5, /* cell from a \.{\\halign} or \.{\\valign} */
    equation_list             =  6, /* display equation */
    equation_number_list      =  7, /* display equation number */
    math_list_list            =  8,
    math_char_list            =  9,
    math_h_extensible_list    = 10,
    math_v_extensible_list    = 11,
    math_h_delimiter_list     = 12,
    math_v_delimiter_list     = 13,
    math_over_delimiter_list  = 14,
    math_under_delimiter_list = 15,
    math_numerator_list       = 16,
    math_denominator_list     = 17,
    math_limits_list          = 18,
    math_fraction_list        = 19,
    math_nucleus_list         = 20,
    math_sup_list             = 21,
    math_sub_list             = 22,
    math_degree_list          = 23,
    math_scripts_list         = 24,
    math_over_list            = 25,
    math_under_list           = 26,
    math_accent_list          = 27,
    math_radical_list         = 28,
} list_subtypes ;

#  define width(a)            varmem[(a)+2].cint
#  define depth(a)            varmem[(a)+3].cint
#  define height(a)           varmem[(a)+4].cint
#  define shift_amount(a)     vlink((a)+5)
#  define box_dir(a)          vinfo((a)+5)
#  define list_ptr(a)         vlink((a)+6)
#  define glue_order(a)       subtype((a)+6)
#  define glue_sign(a)        type((a)+6)
#  define glue_set(a)         varmem[(a)+7].gr
#  define synctex_tag_box(a)  vinfo((a)+8)
#  define synctex_line_box(a) vlink((a)+8)

/* unset nodes */

#  define glue_stretch(a)  varmem[(a)+7].cint
#  define glue_shrink      shift_amount
#  define span_count       subtype

typedef enum {
    normal_rule = 0,
    box_rule,
    image_rule,
    empty_rule,
    user_rule,
    math_over_rule,
    math_under_rule,
    math_fraction_rule,
    math_radical_rule,
    outline_rule,
} rule_subtypes;

#  define rule_node_size       9
#  define rule_dir(a)          vlink((a)+5)
#  define rule_index(a)        vinfo((a)+6)
#  define rule_transform(a)    vlink((a)+6)
#  define rule_left(a)         vinfo((a)+7)
#  define rule_right(a)        vlink((a)+7)
#  define synctex_tag_rule(a)  vinfo((a)+8)
#  define synctex_line_rule(a) vlink((a)+8)

#  define rule_math_size       rule_index
#  define rule_math_font       rule_transform

#  define mark_node_size   3
#  define mark_ptr(a)      vlink((a)+2)
#  define mark_class(a)    vinfo((a)+2)

#  define adjust_node_size 3
#  define adjust_pre       subtype
#  define adjust_ptr(a)    vlink(a+2)

#  define glyph_node_size       7
#  define character(a)          vinfo((a)+2)
#  define font(a)               vlink((a)+2)
#  define lang_data(a)          vinfo((a)+3)
#  define lig_ptr(a)            vlink((a)+3)
#  define x_displace(a)         vinfo((a)+4)
#  define y_displace(a)         vlink((a)+4)
#  define ex_glyph(a)           vinfo((a)+5)  /* expansion factor (hz) */
#  define glyph_node_data(a)    vlink((a)+5)
#  define synctex_tag_glyph(a)  vinfo((a)+6)
#  define synctex_line_glyph(a) vlink((a)+6)

#  define is_char_node(a)  (a!=null && type(a)==glyph_node)

#  define char_lang(a)     ((const int)(signed short)(((signed int)((unsigned)lang_data(a)&0x7FFF0000)<<1)>>17))
#  define char_lhmin(a)    ((const int)(((unsigned)lang_data(a) & 0x0000FF00)>>8))
#  define char_rhmin(a)    ((const int)(((unsigned)lang_data(a) & 0x000000FF)))
#  define char_uchyph(a)   ((const int)(((unsigned)lang_data(a) & 0x80000000)>>31))

#  define make_lang_data(a,b,c,d) (a>0 ? (1<<31): 0)+ \
  (b<<16)+ (((c>0 && c<256) ? c : 255)<<8)+(((d>0 && d<256) ? d : 255))

#  define init_lang_data(a)     lang_data(a)=256+1

#  define set_char_lang(a,b)    lang_data(a)=make_lang_data(char_uchyph(a),b,char_lhmin(a),char_rhmin(a))
#  define set_char_lhmin(a,b)   lang_data(a)=make_lang_data(char_uchyph(a),char_lang(a),b,char_rhmin(a))
#  define set_char_rhmin(a,b)   lang_data(a)=make_lang_data(char_uchyph(a),char_lang(a),char_lhmin(a),b)
#  define set_char_uchyph(a,b)  lang_data(a)=make_lang_data(b,char_lang(a),char_lhmin(a),char_rhmin(a))

#  define margin_kern_node_size 4
#  define margin_char(a)        vlink((a)+3)

/*@# {|subtype| of marginal kerns}*/

typedef enum {
    left_side = 0,
    right_side
} margin_kern_subtypes ;

typedef enum {
    before = 0,
    after
} math_subtypes ;

#  define math_node_size       7
/* define width(a)             vinfo((a)+2) */
/* overlaps width */
/* define shrink(a)            vinfo((a)+3) */
/* define stretch(a)           vlink((a)+3) */
/* define stretch_order(a)     vinfo((a)+4) */
/* define shrink_order(a)      vlink((a)+4) */
/* leader_ptr slot */
#  define surround(a)          vinfo((a)+5)
#  define synctex_tag_math(a)  vinfo((a)+6)
#  define synctex_line_math(a) vlink((a)+6)

#  define ins_node_size    6
#  define float_cost(a)    varmem[(a)+2].cint
#  define ins_ptr(a)       vinfo((a)+5)
#  define split_top_ptr(a) vlink((a)+5)

#  define page_ins_node_size 5

/* height = 4 */

typedef enum {
    hlist_node = 0,
    vlist_node,
    rule_node,
    ins_node,
    mark_node,
    adjust_node,
    boundary_node,
    disc_node,
    whatsit_node,
#define last_preceding_break_node whatsit_node
    local_par_node,
    dir_node,
#define last_non_discardable_node dir_node
    math_node,
    glue_node,
    kern_node,
    penalty_node,
    unset_node,
    style_node,
    choice_node,
    simple_noad,
    radical_noad,
    fraction_noad,
    accent_noad,
    fence_noad,
    math_char_node,             /* kernel fields */
    sub_box_node,
    sub_mlist_node,
    math_text_char_node,
    delim_node,                 /* shield fields */
    margin_kern_node,
    glyph_node,                 /* this and below have attributes */
    align_record_node,
    pseudo_file_node,
    pseudo_line_node,
    inserting_node,
    split_up_node,
    expr_node,
    nesting_node,
    span_node,
    attribute_node,
    glue_spec_node,
    attribute_list_node,
    temp_node,
    align_stack_node,
    movement_node,
    if_node,
    unhyphenated_node,
    hyphenated_node,
    delta_node,
    passive_node,
    shape_node,
} node_types;

#  define MAX_NODE_TYPE shape_node /* 60 */

/*
    TH: these two defines still need checking. The node ordering in luatex is not
    quite the same as in tex82, HH: but it's probably ok
*/

/*

    #  define precedes_break(a) \
    #      (type(a)<math_node && \
    #      (type(a)!=whatsit_node || (subtype(a)!=dir_node && subtype(a)!=local_par_node)))

*/

/*
    #  define precedes_break(a)  (type(a)<math_node)
    #  define non_discardable(a) (type(a)<math_node || type(a) == dir_node || type(a) == local_par_node)
*/

#  define precedes_break(a)  (type(a)<=last_preceding_break_node)
#  define precedes_kern(a)   ((type(a) == kern_node) && (subtype(a) == font_kern || subtype(a) == accent_kern))
#  define precedes_dir(a)    ((type(a) == dir_node) && (break_after_dir_mode_par == 1))
#  define non_discardable(a) (type(a)<=last_non_discardable_node)

#  define known_node_type(i) ( i >= 0 && i <= MAX_NODE_TYPE)

#  define last_known_node temp_node     /* used by \lastnodetype */

#  define movement_node_size    3
#  define if_node_size          2
#  define align_stack_node_size 6
#  define nesting_node_size     2

#  define expr_node_size        3
#  define expr_type(A)          type((A)+1)
#  define expr_state(A)         subtype((A)+1)  /* enum defined in scanning.w */
#  define expr_e_field(A)       vlink((A)+1)    /* saved expression so far */
#  define expr_t_field(A)       vlink((A)+2)    /* saved term so far */
#  define expr_n_field(A)       vinfo((A)+2)    /* saved numerator */

#  define span_node_size        3
#  define span_span(a)          vlink((a)+1)
#  define span_link(a)          vinfo((a)+1)

#  define pseudo_file_node_size 2
#  define pseudo_lines(a)       vlink((a)+1)

#  define nodetype_has_attributes(t) (((t)<=glyph_node) && ((t)!=unset_node))

#  define nodetype_has_subtype(t) ((t)!=attribute_list_node && (t)!=attribute_node && (t)!=glue_spec_node)
#  define nodetype_has_prev(t) nodetype_has_subtype((t))

/*
    style and choice nodes; style nodes can be smaller, the information is encoded in
    |subtype|, but choice nodes are on-the-spot converted to style nodes
*/

#  define style_node_size        4
#  define display_mlist(a)       vinfo((a)+2) /* mlist to be used in display style */
#  define text_mlist(a)          vlink((a)+2) /* mlist to be used in text style */
#  define script_mlist(a)        vinfo((a)+3) /* mlist to be used in script style */
#  define script_script_mlist(a) vlink((a)+3) /* mlist to be used in scriptscript style */

/*
    because noad types get changed when processing we need to make sure some if the node
    sizes match and that we don't share slots with different properties

    once it's sorted out we can go smaller and also adapt the flush etc code to it
*/

/* regular noads */

#  define noad_size      8
#  define new_hlist(a)   vlink((a)+2) /* the translation of an mlist */
#  define nucleus(a)     vinfo((a)+2) /* the |nucleus| field of a noad */
#  define supscr(a)      vlink((a)+3) /* the |supscr| field of a noad */
#  define subscr(a)      vinfo((a)+3) /* the |subscr| field of a noad */
#  define noaditalic(a)  vlink((a)+4)
#  define noadwidth(a)   vinfo((a)+4)
#  define noadheight(a)  vlink((a)+5)
#  define noaddepth(a)   vinfo((a)+5)
#  define noadextra1(a)  vlink((a)+6) /* we need to match delimiter (saves copy) */
#  define noadoptions(a) vinfo((a)+6)
#  define noadextra3(a)  vlink((a)+7) /* see (!) below */
#  define noadextra4(a)  vinfo((a)+7) /* used to store samesize */

#  define noad_fam(a)    vlink((a)+6) /* noadextra1 */

typedef enum {
    ord_noad_type = 0,
    op_noad_type_normal,
    op_noad_type_limits,
    op_noad_type_no_limits,
    bin_noad_type,
    rel_noad_type,
    open_noad_type,
    close_noad_type,
    punct_noad_type,
    inner_noad_type,
    under_noad_type,
    over_noad_type,
    vcenter_noad_type,
} noad_types;

/* accent noads */

#  define accent_noad_size      8
#  define top_accent_chr(a)     vinfo((a)+6) /* the |top_accent_chr| field of an accent noad */
#  define bot_accent_chr(a)     vlink((a)+6) /* the |bot_accent_chr| field of an accent noad */
#  define overlay_accent_chr(a) vinfo((a)+7) /* the |overlay_accent_chr| field of an accent noad */
#  define accentfraction(a)     vlink((a)+7)

typedef enum {
    bothflexible_accent,
    fixedtop_accent,
    fixedbottom_accent,
    fixedboth_accent,
} math_accent_subtypes ;

/* left and right noads */

#  define fence_noad_size      8            /* needs to match noad size */
#  define delimiteritalic(a)   vlink((a)+4)
/* define delimiterwidth(a)    vinfo((a)+4) */
#  define delimiterheight(a)   vlink((a)+5)
#  define delimiterdepth(a)    vinfo((a)+5)
#  define delimiter(a)         vlink((a)+6) /* |delimiter| field in left and right noads */
#  define delimiteroptions(a)  vinfo((a)+6)
#  define delimiterclass(a)    vlink((a)+7) /* (!) we could probably pack some more in 6 */
#  define delimitersamesize(a) vinfo((a)+7) /* set by engine */

/* when dimensions then axis else noaxis */

typedef enum {
    noad_option_set             =        0x08,
    noad_option_unused_1        = 0x00 + 0x08,
    noad_option_unused_2        = 0x01 + 0x08,
    noad_option_axis            = 0x02 + 0x08,
    noad_option_no_axis         = 0x04 + 0x08,
    noad_option_exact           = 0x10 + 0x08,
    noad_option_left            = 0x11 + 0x08,
    noad_option_middle          = 0x12 + 0x08,
    noad_option_right           = 0x14 + 0x08,
    noad_option_no_sub_script   = 0x21 + 0x08,
    noad_option_no_super_script = 0x22 + 0x08,
    noad_option_no_script       = 0x23 + 0x08,
} delimiter_options ;

#  define delimiteroptionset(a) ((delimiteroptions(a) & noad_option_set    ) == noad_option_set    )
#  define delimiteraxis(a)      ((delimiteroptions(a) & noad_option_axis   ) == noad_option_axis   )
#  define delimiternoaxis(a)    ((delimiteroptions(a) & noad_option_no_axis) == noad_option_no_axis)
#  define delimiterexact(a)     ((delimiteroptions(a) & noad_option_exact  ) == noad_option_exact  )

#  define noadoptionnosubscript(a) ( (type(a) == simple_noad) && ( \
                                     ((delimiteroptions(a) & noad_option_no_sub_script  ) == noad_option_no_sub_script) || \
                                     ((delimiteroptions(a) & noad_option_no_script      ) == noad_option_no_script    ) ))
#  define noadoptionnosupscript(a) ( (type(a) == simple_noad) && ( \
                                     ((delimiteroptions(a) & noad_option_no_super_script) == noad_option_no_super_script) || \
                                     ((delimiteroptions(a) & noad_option_no_script      ) == noad_option_no_script      ) ))

typedef enum {
    noad_delimiter_mode_noshift = 0x01,
    noad_delimiter_mode_italics = 0x02,
    noad_delimiter_mode_ordinal = 0x04,
    noad_delimiter_mode_samenos = 0x08,
    noad_delimiter_mode_charnos = 0x10,
} delimiter_modes ;

#  define delimitermodenoshift ((math_delimiters_mode_par & noad_delimiter_mode_noshift) == noad_delimiter_mode_noshift)
#  define delimitermodeitalics ((math_delimiters_mode_par & noad_delimiter_mode_italics) == noad_delimiter_mode_italics)
#  define delimitermodeordinal ((math_delimiters_mode_par & noad_delimiter_mode_ordinal) == noad_delimiter_mode_ordinal)
#  define delimitermodesamenos ((math_delimiters_mode_par & noad_delimiter_mode_samenos) == noad_delimiter_mode_samenos)
#  define delimitermodecharnos ((math_delimiters_mode_par & noad_delimiter_mode_charnos) == noad_delimiter_mode_charnos)

/* subtype of fence noads */

/*
#  define left_noad_side   1
#  define middle_noad_side 2
#  define right_noad_side  3
#  define no_noad_side     4
*/

typedef enum {
    unset_noad_side  = 0,
    left_noad_side   = 1,
    middle_noad_side = 2,
    right_noad_side  = 3,
    no_noad_side     = 4,
} fence_subtypes ;

/* fraction noads */

#  define fraction_noad_size  8
#  define thickness(a)        vlink((a)+2) /* |thickness| field in a fraction noad */
#  define numerator(a)        vlink((a)+3) /* |numerator| field in a fraction noad */
#  define denominator(a)      vinfo((a)+3) /* |denominator| field in a fraction noad */
#  define left_delimiter(a)   vlink((a)+5) /* first delimiter field of a noad */
#  define right_delimiter(a)  vinfo((a)+5) /* second delimiter field of a fraction noad */
#  define middle_delimiter(a) vlink((a)+6)
#  define fractionoptions(a)  vinfo((a)+6)
#  define fraction_fam(a)     vlink((a)+7)

#  define fractionoptionset(a) ((fractionoptions(a) & noad_option_set    ) == noad_option_set    )
#  define fractionexact(a)     ((fractionoptions(a) & noad_option_exact  ) == noad_option_exact  )
#  define fractionnoaxis(a)    ((fractionoptions(a) & noad_option_no_axis) == noad_option_no_axis)

/* radical noads */
/* this is like a fraction, but it only stores a |left_delimiter| */

#  define radical_noad_size 7
#  define radicalwidth(a)   vinfo((a)+4)
#  define degree(a)         vlink((a)+6)   /* the root degree in a radical noad */
#  define radicaloptions(a) vinfo((a)+6)

#  define radicaloptionset(a) ((radicaloptions(a) & noad_option_set   ) == noad_option_set)
#  define radicalexact(a)     ((radicaloptions(a) & noad_option_exact ) == noad_option_exact)
#  define radicalleft(a)      ((radicaloptions(a) & noad_option_left  ) == noad_option_left)
#  define radicalmiddle(a)    ((radicaloptions(a) & noad_option_middle) == noad_option_middle)
#  define radicalright(a)     ((radicaloptions(a) & noad_option_right ) == noad_option_right)

typedef enum {
    radical_noad_type,
    uradical_noad_type,
    uroot_noad_type,
    uunderdelimiter_noad_type,
    uoverdelimiter_noad_type,
    udelimiterunder_noad_type,
    udelimiterover_noad_type,
} radical_subtypes;

/* accessors for the |nucleus|-style node fields */

#  define math_kernel_node_size 3
#  define math_fam(a)           vinfo((a)+2)
#  define math_character(a)     vlink((a)+2)
#  define math_list(a)          vlink((a)+2)

/* accessors for the |delimiter|-style two-word subnode fields */

#  define math_shield_node_size 4            /* not used yet */

#  define small_fam(A)          vinfo((A)+2) /* |fam| for ``small'' delimiter */
#  define small_char(A)         vlink((A)+2) /* |character| for ``small'' delimiter */
#  define large_fam(A)          vinfo((A)+3) /* |fam| for ``large'' delimiter */
#  define large_char(A)         vlink((A)+3) /* |character| for ``large'' delimiter */

/* we should have the codes in a separate enum: extension_codes */

/* be careful: must be in sync with whatsit_node_data[] ! (at least) */

typedef enum {
    /* core */
    open_node = 0,
    write_node,
    close_node,
    special_node,
    use_box_resource_node,
    use_image_resource_node,
    save_pos_node,
    late_lua_node,
    user_defined_node,
    /* dvi backend */
    dvi_literal_node = 15,
    /* pdf backend */
    pdf_literal_node = 16,
    pdf_refobj_node,
    pdf_annot_node,
    pdf_start_link_node,
    pdf_end_link_node,
    pdf_dest_node,
    pdf_action_node,
    pdf_thread_node,
    pdf_start_thread_node,
    pdf_end_thread_node,
    pdf_thread_data_node,
    pdf_link_data_node,
    pdf_colorstack_node,
    pdf_setmatrix_node,
    pdf_save_node,
    pdf_restore_node,
} whatsit_types;

#  define first_common_whatsit      0
#  define last_common_whatsit       user_defined_node
#  define backend_first_dvi_whatsit 15
#  define backend_last_dvi_whatsit  15
#  define backend_first_pdf_whatsit 16
#  define backend_last_pdf_whatsit  31

#  define MAX_WHATSIT_TYPE 32

#  define known_whatsit_type(i) ( \
    (i >= first_common_whatsit      && i <= last_common_whatsit) || \
    (i >= backend_first_dvi_whatsit && i <= backend_last_dvi_whatsit) || \
    (i >= backend_first_pdf_whatsit && i <= backend_last_pdf_whatsit) \
)

#  define get_node_size(i,j) (i!=whatsit_node ? node_data[i].size : whatsit_node_data[j].size)
#  define get_node_name(i,j) (i!=whatsit_node ? node_data[i].name : whatsit_node_data[j].name)
#  define get_etex_code(i)                     (node_data[i].etex)

#  define GLYPH_CHARACTER     (1 << 0)
#  define GLYPH_LIGATURE      (1 << 1)
#  define GLYPH_GHOST         (1 << 2)
#  define GLYPH_LEFT          (1 << 3)
#  define GLYPH_RIGHT         (1 << 4)

typedef enum {
    glyph_unset     = 0,
    glyph_character = GLYPH_CHARACTER,
    glyph_ligature  = GLYPH_LIGATURE,
    glyph_ghost     = GLYPH_GHOST,
    glyph_left      = GLYPH_LEFT,
    glyph_right     = GLYPH_RIGHT,
} glyph_subtypes;

#  define is_character(p)        ((subtype(p)) & GLYPH_CHARACTER)
#  define is_ligature(p)         ((subtype(p)) & GLYPH_LIGATURE )
#  define is_ghost(p)            ((subtype(p)) & GLYPH_GHOST    )

#  define is_simple_character(p) (is_character(p) && !is_ligature(p) && !is_ghost(p))

#  define is_leftboundary(p)     (is_ligature(p) && ((subtype(p)) & GLYPH_LEFT  ))
#  define is_rightboundary(p)    (is_ligature(p) && ((subtype(p)) & GLYPH_RIGHT ))
#  define is_leftghost(p)        (is_ghost(p)    && ((subtype(p)) & GLYPH_LEFT  ))
#  define is_rightghost(p)       (is_ghost(p)    && ((subtype(p)) & GLYPH_RIGHT ))


#  define set_is_glyph(p)         subtype(p) = (quarterword) (subtype(p) & ~GLYPH_CHARACTER)
#  define set_is_character(p)     subtype(p) = (quarterword) (subtype(p) | GLYPH_CHARACTER)
#  define set_is_ligature(p)      subtype(p) = (quarterword) (subtype(p) | GLYPH_LIGATURE)
#  define set_is_ghost(p)         subtype(p) = (quarterword) (subtype(p) |GLYPH_GHOST)

#  define set_to_glyph(p)         subtype(p) = (quarterword)(subtype(p) & 0xFF00)
#  define set_to_character(p)     subtype(p) = (quarterword)((subtype(p) & 0xFF00) | GLYPH_CHARACTER)
#  define set_to_ligature(p)      subtype(p) = (quarterword)((subtype(p) & 0xFF00) | GLYPH_LIGATURE)
#  define set_to_ghost(p)         subtype(p) = (quarterword)((subtype(p) & 0xFF00) | GLYPH_GHOST)

#  define set_is_leftboundary(p)  { set_to_ligature(p); subtype(p) |= GLYPH_LEFT;  }
#  define set_is_rightboundary(p) { set_to_ligature(p); subtype(p) |= GLYPH_RIGHT; }
#  define set_is_leftghost(p)     { set_to_ghost(p);    subtype(p) |= GLYPH_LEFT;  }
#  define set_is_rightghost(p)    { set_to_ghost(p);    subtype(p) |= GLYPH_RIGHT; }

typedef enum {
    cancel_boundary = 0,
    user_boundary,
    protrusion_boundary,
    word_boundary,
} boundary_subtypes ;

#  define boundary_node_size 3
#  define boundary_value(a) vinfo((a)+2)

#  define special_node_size 3

typedef enum {
    normal_dir = 0,
    cancel_dir,
} dir_subtypes ;

#  define dir_node_size 5
#  define dir_dir(a)       vinfo((a)+2)
#  define dir_level(a)     vlink((a)+2)
#  define dir_refpos_h(a)  vinfo((a)+3)
#  define dir_refpos_v(a)  vlink((a)+3)
#  define dir_cur_h(a)     vinfo((a)+4)
#  define dir_cur_v(a)     vlink((a)+4)

#  define write_node_size 3
#  define close_node_size 3
#  define write_tokens(a)  vlink(a+2)
#  define write_stream(a)  vinfo(a+2)

#  define open_node_size 4
#  define open_name(a)   vlink((a)+2)
#  define open_area(a)   vinfo((a)+3)
#  define open_ext(a)    vlink((a)+3)

#  define late_lua_node_size 4
#  define late_lua_data(a)        vlink((a)+2)
#  define late_lua_reg(a)         vinfo((a)+2)
#  define late_lua_name(a)        vlink((a)+3)
#  define late_lua_type(a)        subtype((a)+3)

#  define save_pos_node_size 3

#  define local_par_size 6

#  define local_pen_inter(a)       vinfo((a)+2)
#  define local_pen_broken(a)      vlink((a)+2)
#  define local_box_left(a)        vlink((a)+3)
#  define local_box_left_width(a)  vinfo((a)+3)
#  define local_box_right(a)       vlink((a)+4)
#  define local_box_right_width(a) vinfo((a)+4)
#  define local_par_dir(a)         vinfo((a)+5)

/* type of literal data */

#  define lua_refid_literal 1 /* not a |normal| string */
#  define lua_refid_call    2 /* not a |normal| string */

/* begin of pdf backend nodes */

/* literal ctm types */

typedef enum {
    set_origin = 0,
    direct_page,
    direct_always,
    direct_raw,
    direct_text,
    direct_font,
    scan_special,
} ctm_transform_modes;

#  define pdf_refobj_node_size 3

#  define pdf_obj_objnum(a) vinfo((a) + 2)

#  define pdf_annot_node_size 8
#  define pdf_dest_node_size 8
#  define pdf_thread_node_size 8

/*
    when a whatsit node representing annotation is created, words |1..3| are
    width, height and depth of this annotation; after shipping out words |1..4|
    are rectangle specification of annotation. For whatsit node representing
    destination |pdf_ann_left| and |pdf_ann_top| are used for some types of destinations
*/

/*
    coordinates of destinations/threads/annotations (in whatsit node)
*/

#  define pdf_ann_left(a)           varmem[(a) + 2].cint
#  define pdf_ann_top(a)            varmem[(a) + 3].cint
#  define pdf_ann_right(a)          varmem[(a) + 4].cint
#  define pdf_ann_bottom(a)         varmem[(a) + 5].cint

#  define pdf_literal_data(a)       vlink((a)+2)
#  define pdf_literal_mode(a)       type((a)+2)
#  define pdf_literal_type(a)       subtype((a)+2)

#  define pdf_annot_data(a)         vinfo((a) + 6)
#  define pdf_link_attr(a)          vinfo((a) + 6)
#  define pdf_link_action(a)        vlink((a) + 6)
#  define pdf_annot_objnum(a)       varmem[(a) + 7].cint
#  define pdf_link_objnum(a)        varmem[(a) + 7].cint

#  define pdf_dest_type(a)          type((a) + 6)
#  define pdf_dest_named_id(a)      subtype((a) + 6)
#  define pdf_dest_id(a)            vlink((a) + 6)
#  define pdf_dest_xyz_zoom(a)      vinfo((a) + 7)
#  define pdf_dest_objnum(a)        vlink((a) + 7)

#  define pdf_thread_named_id(a)    subtype((a) + 6)
#  define pdf_thread_id(a)          vlink((a) + 6)
#  define pdf_thread_attr(a)        vinfo((a) + 7)

#  define pdf_end_link_node_size    3
#  define pdf_end_thread_node_size  3

#  define pdf_setmatrix_node_size   3
#  define pdf_save_node_size        3
#  define pdf_restore_node_size     3

#  define pdf_colorstack_node_size  4
#  define pdf_colorstack_stack(a)   vlink((a)+2)
#  define pdf_colorstack_cmd(a)     vinfo((a)+2)
#  define pdf_colorstack_data(a)    vlink((a)+3)
#  define pdf_setmatrix_data(a)     vlink((a)+2)

typedef enum {
    pdf_action_page = 0,
    pdf_action_goto,
    pdf_action_thread,
    pdf_action_user
} pdf_action_type;

typedef enum {
    pdf_window_notset,
    pdf_window_new,
    pdf_window_nonew,
} pdf_window_type;

#  define pdf_action_size           6

#  define pdf_action_type(a)        vlink((a)+2) /* enum pdf_action_type */
#  define pdf_action_named_id(a)    vinfo((a)+2) /* boolean */
#  define pdf_action_id(a)          vlink((a)+3) /* number or toks */
#  define pdf_action_file(a)        vinfo((a)+3) /* toks */
#  define pdf_action_new_window(a)  vlink((a)+4) /* enum pdf_window_type */
#  define pdf_action_tokens(a)      vinfo((a)+4) /* toks */
#  define pdf_action_refcount(a)    vlink((a)+5) /* number */

typedef enum {
    colorstack_set = 0,
    colorstack_push,
    colorstack_pop,
    colorstack_current
} colorstack_commands;

#  define colorstack_data colorstack_push     /* last value where data field is set */

/* end of pdf backend nodes */

/* user defined nodes */

#  define user_defined_node_size 4
#  define user_node_type(a)  vinfo((a)+2)
#  define user_node_id(a)    vlink((a)+2)
#  define user_node_value(a) vinfo((a)+3)

/* node sused in the parbuilder */

#  define active_node_size 4                    /*number of words in extended active nodes */
#  define fitness subtype                       /*|very_loose_fit..tight_fit| on final line for this break */
#  define break_node(a) vlink((a)+1)            /*pointer to the corresponding passive node */
#  define line_number(a) vinfo((a)+1)           /*line that begins at this breakpoint */
#  define total_demerits(a) varmem[(a)+2].cint  /* the quantity that \TeX\ minimizes */
#  define active_short(a) vinfo(a+3)            /* |shortfall| of this line */
#  define active_glue(a)  vlink(a+3)            /*corresponding glue stretch or shrink */

#  define passive_node_size 7
#  define cur_break(a)                   vlink((a)+1)   /*in passive node, points to position of this breakpoint */
#  define prev_break(a)                  vinfo((a)+1)   /*points to passive node that should precede this one */
#  define passive_pen_inter(a)           vinfo((a)+2)
#  define passive_pen_broken(a)          vlink((a)+2)
#  define passive_left_box(a)            vlink((a)+3)
#  define passive_left_box_width(a)      vinfo((a)+3)
#  define passive_last_left_box(a)       vlink((a)+4)
#  define passive_last_left_box_width(a) vinfo((a)+4)
#  define passive_right_box(a)           vlink((a)+5)
#  define passive_right_box_width(a)     vinfo((a)+5)
#  define serial(a)                      vlink((a)+6)   /* serial number for symbolic identification */

#  define delta_node_size 10 /* 8 fields, stored in a+1..9 */

/* helpers */

#  define couple_nodes(a,b) {assert(b!=null);vlink(a)=b;alink(b)=a;}
#  define try_couple_nodes(a,b) if (b==null) vlink(a)=b; else {couple_nodes(a,b);}
#  define uncouple_node(a) {assert(a!=null);vlink(a)=null;alink(a)=null;}

#  define cache_disabled max_halfword

extern void delete_attribute_ref(halfword b);
extern void reset_node_properties(halfword b);
extern void reassign_attribute(halfword n,halfword new);
extern void build_attribute_list(halfword b);
extern halfword current_attribute_list(void);

extern int unset_attribute(halfword n, int c, int w);
extern void set_attribute(halfword n, int c, int w);
extern int has_attribute(halfword n, int c, int w);

extern halfword new_span_node(halfword n, int c, scaled w);

extern void print_short_node_contents(halfword n);
extern void show_node_list(int i);
extern pointer actual_box_width(pointer r, scaled base_width);

typedef struct _subtype_info {
    int id;
    const char *name;
    int lua;
} subtype_info;

typedef struct _field_info {
    const char *name;
    int lua;
} field_info;

typedef struct _node_info {
    int id;
    int size;
    subtype_info *subtypes;
    field_info *fields;
    const char *name;
    int etex;
    int lua;
} node_info;

extern node_info node_data[];
extern node_info whatsit_node_data[];

extern subtype_info node_subtypes_dir[];
extern subtype_info node_subtypes_glue[];
extern subtype_info node_subtypes_mathglue[];
extern subtype_info node_subtypes_leader[];
extern subtype_info node_subtypes_boundary[];
extern subtype_info node_subtypes_penalty[];
extern subtype_info node_subtypes_kern[];
extern subtype_info node_subtypes_rule[];
extern subtype_info node_subtypes_glyph[];
extern subtype_info node_subtypes_disc[];
extern subtype_info node_subtypes_marginkern[];
extern subtype_info node_subtypes_list[];
extern subtype_info node_subtypes_adjust[];
extern subtype_info node_subtypes_math[];
extern subtype_info node_subtypes_noad[];
extern subtype_info node_subtypes_radical[];
extern subtype_info node_subtypes_accent[];
extern subtype_info node_subtypes_fence[];

extern subtype_info node_values_pdf_destination[];
extern subtype_info node_values_pdf_literal[];
extern subtype_info node_values_pdf_literal[];
extern subtype_info node_values_pdf_action[];
extern subtype_info node_values_pdf_window[];

extern subtype_info node_values_fill[];
extern subtype_info node_values_dir[];
extern subtype_info node_values_color_stack[];

extern subtype_info other_values_page_states[];

extern halfword new_node(int i, int j);
extern void flush_node_list(halfword);
extern void flush_node(halfword);
extern halfword do_copy_node_list(halfword, halfword);
extern halfword copy_node_list(halfword);
extern halfword copy_node(const halfword);
extern void check_node(halfword);
extern halfword fix_node_list(halfword);
extern int fix_node_lists;
extern char *sprint_node_mem_usage(void);
extern halfword raw_glyph_node(void);
extern halfword new_glyph_node(void);
extern int valid_node(halfword);

extern void flush_node_wrapup_dvi(halfword);
extern void flush_node_wrapup_pdf(halfword);
extern void copy_node_wrapup_dvi(halfword, halfword); /* original target */
extern void copy_node_wrapup_pdf(halfword, halfword); /* original target */
extern void check_node_wrapup_dvi(halfword); /* DEBUG_NODES mode */
extern void check_node_wrapup_pdf(halfword); /* DEBUG_NODES mode */
extern void show_node_wrapup_dvi(halfword);
extern void show_node_wrapup_pdf(halfword);

typedef enum {
    normal_g = 0, /* normal */
    sfi,
    fil,
    fill,
    filll
} glue_orders;

#  define zero_glue        0
#  define sfi_glue         zero_glue+glue_spec_size
#  define fil_glue         sfi_glue+glue_spec_size
#  define fill_glue        fil_glue+glue_spec_size
#  define ss_glue          fill_glue+glue_spec_size
#  define fil_neg_glue     ss_glue+glue_spec_size
#  define page_ins_head    fil_neg_glue+glue_spec_size

#  define contrib_head     page_ins_head+temp_node_size
#  define page_head        contrib_head+temp_node_size
#  define temp_head        page_head+temp_node_size
#  define hold_head        temp_head+temp_node_size
#  define adjust_head      hold_head+temp_node_size
#  define pre_adjust_head  adjust_head+temp_node_size
#  define active           pre_adjust_head+temp_node_size
#  define align_head       active+active_node_size
#  define end_span         align_head+temp_node_size
#  define begin_point      end_span+span_node_size
#  define end_point        begin_point+glyph_node_size
#  define var_mem_stat_max (end_point+glyph_node_size-1)

#  define stretching 1
#  define shrinking  2

#  define last_normal_node  shape_node
#  define last_whatsit_node pdf_restore_node

#  define is_running(A) ((A)==null_flag)        /* tests for a running dimension */

extern halfword tail_of_list(halfword p);

extern int var_used;

#  define cache_disabled max_halfword

extern int max_used_attr;
extern halfword attr_list_cache;

extern halfword new_null_box(void);
extern halfword new_rule(int s);
extern halfword new_glyph(int f, int c);
extern quarterword norm_min(int h);
extern halfword new_char(int f, int c);
extern scaled glyph_width(halfword p);
extern scaled glyph_height(halfword p);
extern scaled glyph_depth(halfword p);
extern halfword new_disc(void);
extern halfword new_math(scaled w, int s);
extern halfword new_spec(halfword p);
extern halfword new_param_glue(int n);
extern halfword new_glue(halfword q);
extern halfword new_skip_param(int n);
extern halfword new_kern(scaled w);
extern halfword new_penalty(int m, int s);

extern int lua_properties_enabled ;
extern int lua_properties_level ;
extern int lua_properties_use_metatable ;

extern halfword make_local_par_node(int mode);

extern void synctex_set_mode(int mode);
extern int synctex_get_mode(void);
extern void synctex_set_tag(int tag);
extern void synctex_set_line(int line);
extern void synctex_force_tag(int tag);
extern void synctex_force_line(int tag);
extern int synctex_get_tag(void);
extern void synctex_set_no_files(int flag);
extern int synctex_get_no_files(void);
extern int synctex_get_line(void);

extern void l_set_node_data(void) ;
extern void l_set_whatsit_data(void) ;

#endif

