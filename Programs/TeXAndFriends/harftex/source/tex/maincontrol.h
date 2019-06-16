/* maincontrol.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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


#ifndef MAINCONTROL_H
#  define MAINCONTROL_H

#  define fil_code 0            /* identifies \.{\\hfil} and \.{\\vfil} */
#  define fill_code 1           /* identifies \.{\\hfill} and \.{\\vfill} */
#  define ss_code 2             /* identifies \.{\\hss} and \.{\\vss} */
#  define fil_neg_code 3        /* identifies \.{\\hfilneg} and \.{\\vfilneg} */
#  define skip_code 4           /* identifies \.{\\hskip} and \.{\\vskip} */
#  define mskip_code 5          /* identifies \.{\\mskip} */

/* The |prefixed_command| does not have to adjust |a| so that |a mod 4=0|,
since the following routines test for the \.{\\global} prefix as follows.
*/

#  define is_global(a) (a>=4)

#  define define(A,B,C) do {			\
     if (is_global(a))				\
         geq_define((A),(quarterword)(B),(C)); \
     else					\
       eq_define((A),(quarterword)(B),(C));			\
   } while (0)

#  define word_define(A,B) do {			\
     if (is_global(a))				\
       geq_word_define((A),(B));		\
     else					\
       eq_word_define((A),(B));		\
   } while (0)

#  define define_lc_code(A,B) do {		\
     if (is_global(a))				\
       set_lc_code((A),(B),level_one);		\
     else					\
       set_lc_code((A),(B),cur_level);		\
   } while (0)

#  define define_uc_code(A,B) do {			\
     if (is_global(a))				\
       set_uc_code((A),(B),level_one);		\
     else					\
       set_uc_code((A),(B),cur_level);		\
} while (0)

#  define define_sf_code(A,B) do {			\
     if (is_global(a))				\
       set_sf_code((A),(B),level_one);		\
     else					\
       set_sf_code((A),(B),cur_level);		\
   } while (0)

#  define define_cat_code(A,B) do {			\
     if (is_global(a))					\
       set_cat_code(cat_code_table_par,(A),(B),level_one);	\
     else						\
       set_cat_code(cat_code_table_par,(A),(B),cur_level);	\
   } while (0)

#  define define_fam_fnt(A,B,C) do {		\
     if (is_global(a))				\
       def_fam_fnt((A),(B),(C),level_one);	\
     else					\
       def_fam_fnt((A),(B),(C),cur_level);	\
   } while (0)

#  define define_math_param(A,B,C) do {		\
     if (is_global(a))				\
       def_math_param((A),(B),(C),level_one);	\
     else					\
       def_math_param((A),(B),(C),cur_level);	\
   } while (0)

extern halfword cur_box;        /* box to be placed into its context */

/*
A \.{\\chardef} creates a control sequence whose |cmd| is |char_given|;
a \.{\\mathchardef} creates a control sequence whose |cmd| is |math_given|;
and the corresponding |chr| is the character code or math code. A \.{\\countdef}
or \.{\\dimendef} or \.{\\skipdef} or \.{\\muskipdef} creates a control
sequence whose |cmd| is |assign_int| or \dots\ or |assign_mu_glue|, and the
corresponding |chr| is the |eqtb| location of the internal register in question.
*/

#  define char_def_code 0       /* |shorthand_def| for \.{\\chardef} */
#  define math_char_def_code 1  /* |shorthand_def| for \.{\\mathchardef} */
#  define xmath_char_def_code 2 /* |shorthand_def| for \.{\\Umathchardef} */
#  define count_def_code 3      /* |shorthand_def| for \.{\\countdef} */
#  define attribute_def_code 4  /* |shorthand_def| for \.{\\attributedef} */
#  define dimen_def_code 5      /* |shorthand_def| for \.{\\dimendef} */
#  define skip_def_code 6       /* |shorthand_def| for \.{\\skipdef} */
#  define mu_skip_def_code 7    /* |shorthand_def| for \.{\\muskipdef} */
#  define toks_def_code 8       /* |shorthand_def| for \.{\\toksdef} */
#  define umath_char_def_code 9 /* |shorthand_def| for \.{\\Umathcharnumdef} */

extern void adjust_space_factor(void);
extern void main_control(void);
extern void app_space(void);
extern void insert_dollar_sign(void);
extern void insert_dollar_sign_par_end(void);
extern void you_cant(void);
extern void report_illegal_case(void);
extern boolean privileged(void);
extern boolean its_all_over(void);
extern void append_glue(void);
extern void append_kern(void);
extern void off_save(void);
extern void handle_right_brace(void);
extern void extra_right_brace(void);
extern void normal_paragraph(void);
extern void box_end(int box_context);
extern void scan_box(int box_context);
extern void new_graf(boolean indented);
extern void indent_in_hmode(void);
extern void head_for_vmode(void);
extern void end_graf(int);
extern void begin_insert_or_adjust(void);
extern void handle_mark(void);
extern void append_penalty(void);
extern void delete_last(void);
extern void unpackage(void);
extern void append_italic_correction(void);
extern void append_local_box(int kind);
extern void append_discretionary(void);
extern void build_local_box(void);
extern void build_discretionary(void);
extern void make_accent(void);
extern void align_error(void);
extern void no_align_error(void);
extern void omit_error(void);
extern void do_endv(void);
extern void cs_error(void);
extern void prefixed_command(void);
extern void fixup_directions(void);
extern void fixup_directions_only(void);


/* Assignments from Lua need helpers. */

#  define is_int_assign(cmd) (cmd==assign_int_cmd)
#  define is_attr_assign(cmd) (cmd==assign_attr_cmd)
#  define is_dim_assign(cmd) (cmd==assign_dimen_cmd)
#  define is_glue_assign(cmd) (cmd==assign_glue_cmd)
#  define is_mu_glue_assign(cmd) (cmd==assign_mu_glue_cmd)
#  define is_toks_assign(cmd) (cmd==assign_toks_cmd)

#  define show_code 0           /* \.{\\show} */
#  define show_box_code 1       /* \.{\\showbox} */
#  define show_the_code 2       /* \.{\\showthe} */
#  define show_lists 3          /* \.{\\showlists} */
#  define show_groups 4         /* \.{\\showgroups} */
#  define show_tokens 5         /* \.{\\showtokens} , must be odd! */
#  define show_ifs 6            /*  \.{\\showifs} */

#  define print_if_line(A) if ((A)!=0) {			\
    tprint(" entered on line "); print_int((A));	\
  }

/*
    extern halfword swap_hang_indent(halfword indentation, halfword shape_mode);
    extern halfword swap_parshape_indent(halfword indentation, halfword width, halfword shape_mode);
*/

#define swap_hang_indent(indentation) \
    ( ((shape_mode_par == 1 || shape_mode_par == 3 || shape_mode_par == -1 || shape_mode_par == -3)) ? negate(indentation) : indentation )

#define swap_parshape_indent(indentation,width) \
    ( ((shape_mode_par == 2 || shape_mode_par == 3 || shape_mode_par == -2 || shape_mode_par == -3)) ? (hsize_par - width - indentation) : indentation )

extern void get_r_token(void);
extern void assign_internal_value(int a, halfword p, int cur_val);
extern void do_register_command(int a);
extern void alter_aux(void);
extern void alter_prev_graf(void);
extern void alter_page_so_far(void);
extern void alter_integer(void);
extern void alter_box_dimen(void);
extern void new_interaction(void);
extern halfword after_token;
extern void do_assignments(void);
extern void open_or_close_in(void);
extern boolean long_help_seen;
extern void issue_message(void);
extern void give_err_help(void);
extern void shift_case(void);
extern void show_whatever(void);

extern void initialize(void);   /* this procedure gets things started properly */

/*extern int local_level;*/

extern void local_control(void);
extern halfword local_scan_box(void);
extern int current_local_level(void);
extern void end_local_control(void);
extern void local_control_message(const char *s);

#endif
