/* equivalents.h

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EQUIVALENTS_H
#  define EQUIVALENTS_H

/*

Like the preceding parameters, the following quantities can be changed at compile
time to extend or reduce \TeX's capacity. But if they are changed, it is
necessary to rerun the initialization program \.{INITEX} @.INITEX@> to generate
new tables for the production \TeX\ program. One can't simply make helter-skelter
changes to the following constants, since certain rather complex initialization
numbers are computed from them. They are defined here using \.{WEB} macros,
instead of being put into \PASCAL's |const| list, in order to emphasize this
distinction.

*/

#  define font_base                    0  /* smallest internal font number; must not be less than |min_quarterword| */
#  define biggest_reg              65535  /* the largest allowed register number; must be |< max_quarterword| */
#  define number_regs              65536  /* |biggest_reg+1| */
#  define number_attrs             65536  /* total numbeer of attributes */
#  define biggest_char           1114111  /* 0x10FFFF, the largest allowed character number; must be |< max_halfword| */
#  define too_big_char   (biggest_char+1) /* 1114112, |biggest_char+1| */
#  define special_char   (biggest_char+2) /* 1114113, |biggest_char+2| */
#  define number_chars   (biggest_char+3) /* 1114112, |biggest_char+1| */
#  define number_fonts (5535-font_base+1)
#  define biggest_lang             32767
#  define too_big_lang             32768
#  define text_size                    0  /* size code for the largest size in a family */
#  define script_size                  1  /* size code for the medium size in a family */
#  define script_script_size           2  /* size code for the smallest size in a family */

/*

Each entry in |eqtb| is a |memory_word|. Most of these words are of type
|two_halves|, and subdivided into three fields:

\yskip\hangg 1) The |eq_level| (a quarterword) is the level of grouping at which
this equivalent was defined. If the level is |level_zero|, the equivalent has
never been defined; |level_one| refers to the outer level (outside of all
groups), and this level is also used for global definitions that never go away.
Higher levels are for equivalents that will disappear at the end of their group.
@^global definitions@>

\yskip\hangg 2) The |eq_type| (another quarterword) specifies what kind of entry
this is. There are many types, since each \TeX\ primitive like \.{\\hbox},
\.{\\def}, etc., has its own special code. The list of command codes above
includes all possible settings of the |eq_type| field.

\yskip\hangg 3) The |equiv| (a halfword) is the current equivalent value. This
may be a font number, a pointer into |mem|, or a variety of other things.

Many locations in |eqtb| have symbolic names. The purpose of the next paragraphs
is to define these names, and to set up the initial values of the equivalents.

In the first region we have a single entry for the `null csname' of length zero.
In luatex, the active characters and and single-letter control sequence names are
part of the next region.

Then comes region~2, which corresponds to the hash table that we will define
later. The maximum address in this region is used for a dummy control sequence
that is perpetually undefined. There also are several locations for control
sequences that are perpetually defined (since they are used in error recovery).

@ Region 3 of |eqtb| contains the |number_regs| \.{\\skip} registers, as well as
the glue parameters defined here. It is important that the ``muskip'' parameters
have larger numbers than the others.

@ Region 4 of |eqtb| contains the local quantities defined here. The bulk of this
region is taken up by five tables that are indexed by eight-bit characters; these
tables are important to both the syntactic and semantic portions of \TeX. There
are also a bunch of special things like font and token parameters, as well as the
tables of \.{\\toks} and \.{\\box} registers.

@ Region 5 of |eqtb| contains the integer parameters and registers defined here,
as well as the |del_code| table. The latter table differs from the
|cat_code..math_code| tables that precede it, since delimiter codes are fullword
integers while the other kinds of codes occupy at most a halfword. This is what
makes region~5 different from region~4. We will store the |eq_level| information
in an auxiliary array of quarterwords that will be defined later.

@ The integer parameters should really be initialized by a macro package; the
following initialization does the minimum to keep \TeX\ from complete failure.

@^null delimiter@>

@ The final region of |eqtb| contains the dimension parameters defined here, and
the |number_regs| \.{\\dimen} registers.

*/

#  define null_cs 1                                                     /* equivalent of \.{\\csname\\endcsname} */
#  define hash_base (null_cs+1)                                         /* beginning of region 2, for the hash table */
#  define frozen_control_sequence (hash_base+hash_size)                 /* for error recovery */
#  define frozen_protection (frozen_control_sequence)                   /* inaccessible but definable */
#  define frozen_cr (frozen_control_sequence+1)                         /* permanent `\.{\\cr}' */
#  define frozen_end_group (frozen_control_sequence+2)                  /* permanent `\.{\\endgroup}' */
#  define frozen_right (frozen_control_sequence+3)                      /* permanent `\.{\\right}' */
#  define frozen_fi (frozen_control_sequence+4)                         /* permanent `\.{\\fi}' */
#  define frozen_end_template (frozen_control_sequence+5)               /* permanent `\.{\\endtemplate}' */
#  define frozen_endv (frozen_control_sequence+6)                       /* second permanent `\.{\\endtemplate}' */
#  define frozen_relax (frozen_control_sequence+7)                      /* permanent `\.{\\relax}' */
#  define end_write (frozen_control_sequence+8)                         /* permanent `\.{\\endwrite}' */
#  define frozen_dont_expand (frozen_control_sequence+9 )               /* permanent `\.{\\notexpanded:}' */
#  define frozen_primitive (frozen_control_sequence+11 )                /* permanent `\.{\\primitive}' */
#  define frozen_special (frozen_control_sequence+12 )                  /* permanent `\.{\\special}' */
#  define frozen_null_font (frozen_control_sequence+13 )                /* permanent `\.{\\nullfont}' */
#  define font_id_base (frozen_null_font-font_base )                    /* begins table of |number_fonts| permanent font identifiers */
#  define undefined_control_sequence (frozen_null_font+number_fonts)
#  define glue_base (undefined_control_sequence+1)                      /* beginning of region 3 */

#  define line_skip_code 0                                              /* interline glue if |baseline_skip| is infeasible */
#  define baseline_skip_code 1                                          /* desired glue between baselines */
#  define par_skip_code 2                                               /* extra glue just above a paragraph */
#  define above_display_skip_code 3                                     /* extra glue just above displayed math */
#  define below_display_skip_code 4                                     /* extra glue just below displayed math */
#  define above_display_short_skip_code 5                               /* glue above displayed math following short lines */
#  define below_display_short_skip_code 6                               /* glue below displayed math following short lines */
#  define left_skip_code 7                                              /* glue at left of justified lines */
#  define right_skip_code 8                                             /* glue at right of justified lines */
#  define top_skip_code 9                                               /* glue at top of main pages */
#  define split_top_skip_code 10                                        /* glue at top of split pages */
#  define tab_skip_code 11                                              /* glue between aligned entries */
#  define space_skip_code 12                                            /* glue between words (if not |zero_glue|) */
#  define xspace_skip_code 13                                           /* glue after sentences (if not |zero_glue|) */
#  define par_fill_skip_code 14                                         /* glue on last line of paragraph */
#  define math_skip_code 15
#  define thin_mu_skip_code 16                                          /* thin space in math formula */
#  define med_mu_skip_code 17                                           /* medium space in math formula */
#  define thick_mu_skip_code 18                                         /* thick space in math formula */
#  define glue_pars 19                                                  /* total number of glue parameters */

#  define skip_base (glue_base+glue_pars)                               /* table of |number_regs| ``skip'' registers */
#  define mu_skip_base (skip_base+number_regs)                          /* table of |number_regs| ``muskip'' registers */
#  define local_base (mu_skip_base+number_regs)                         /* beginning of region 4 */

#  define par_shape_loc (local_base)                                    /* specifies paragraph shape */
#  define output_routine_loc (local_base+1)                             /* points to token list for \.{\\output} */
#  define every_par_loc (local_base+2)                                  /* points to token list for \.{\\everypar} */
#  define every_math_loc (local_base+3)                                 /* points to token list for \.{\\everymath} */
#  define every_display_loc (local_base+4)                              /* points to token list for \.{\\everydisplay} */
#  define every_hbox_loc (local_base+5)                                 /* points to token list for \.{\\everyhbox} */
#  define every_vbox_loc (local_base+6)                                 /* points to token list for \.{\\everyvbox} */
#  define every_job_loc (local_base+7)                                  /* points to token list for \.{\\everyjob} */
#  define every_cr_loc (local_base+8)                                   /* points to token list for \.{\\everycr} */
#  define err_help_loc (local_base+9)                                   /* points to token list for \.{\\errhelp} */
#  define every_eof_loc (local_base+10)                                 /* points to token list for \.{\\everyeof} */

#  define backend_toks_base (local_base+11)
#  define backend_toks_last (local_base+20)

#  define toks_base (local_base+21)                                     /* table of |number_regs| token list registers */

#  define etex_pen_base (toks_base+number_regs)                         /* start of table of \eTeX's penalties */
#  define inter_line_penalties_loc (etex_pen_base)                      /* additional penalties between lines */
#  define club_penalties_loc (etex_pen_base+1)                          /* penalties for creating club lines */
#  define widow_penalties_loc (etex_pen_base+2)                         /* penalties for creating widow lines */
#  define display_widow_penalties_loc (etex_pen_base+3)                 /* ditto, just before a display */
#  define etex_pens (etex_pen_base+4)                                   /* end of table of \eTeX's penalties */
#  define local_left_box_base (etex_pens)
#  define local_right_box_base (local_left_box_base+1)
#  define box_base (local_right_box_base+1)                             /* table of |number_regs| box registers */
#  define cur_font_loc (box_base+number_regs)                           /* internal font number outside math mode */
#  define internal_math_param_base (cur_font_loc+1 )                    /* current math parameter data index  */
#  define cat_code_base (internal_math_param_base+1)                    /* current category code data index  */
#  define lc_code_base (cat_code_base+1)                                /* table of |number_chars| lowercase mappings */
#  define uc_code_base (lc_code_base+1)                                 /* table of |number_chars| uppercase mappings */
#  define sf_code_base (uc_code_base+1)                                 /* table of |number_chars| spacefactor mappings */
#  define math_code_base (sf_code_base+1)                               /* table of |number_chars| math mode mappings */
#  define int_base (math_code_base+1)                                   /* beginning of region 5 */

#  define pretolerance_code 0                                           /* badness tolerance before hyphenation */
#  define tolerance_code 1                                              /* badness tolerance after hyphenation */
#  define line_penalty_code 2                                           /* added to the badness of every line */
#  define hyphen_penalty_code 3                                         /* penalty for break after discretionary hyphen */
#  define ex_hyphen_penalty_code 4                                      /* penalty for break after explicit hyphen */
#  define club_penalty_code 5                                           /* penalty for creating a club line */
#  define widow_penalty_code 6                                          /* penalty for creating a widow line */
#  define display_widow_penalty_code 7                                  /* ditto, just before a display */
#  define broken_penalty_code 8                                         /* penalty for breaking a page at a broken line */
#  define bin_op_penalty_code 9                                         /* penalty for breaking after a binary operation */
#  define rel_penalty_code 10                                           /* penalty for breaking after a relation */
#  define pre_display_penalty_code 11                                   /* penalty for breaking just before a displayed formula */
#  define post_display_penalty_code 12                                  /* penalty for breaking just after a displayed formula */
#  define inter_line_penalty_code 13                                    /* additional penalty between lines */
#  define double_hyphen_demerits_code 14                                /* demerits for double hyphen break */
#  define final_hyphen_demerits_code 15                                 /* demerits for final hyphen break */
#  define adj_demerits_code 16                                          /* demerits for adjacent incompatible lines */
#  define mag_code 17                                                   /* magnification ratio */
#  define delimiter_factor_code 18                                      /* ratio for variable-size delimiters */
#  define looseness_code 19                                             /* change in number of lines for a paragraph */
#  define time_code 20                                                  /* current time of day */
#  define day_code 21                                                   /* current day of the month */
#  define month_code 22                                                 /* current month of the year */
#  define year_code 23                                                  /* current year of our Lord */
#  define show_box_breadth_code 24                                      /* nodes per level in |show_box| */
#  define show_box_depth_code 25                                        /* maximum level in |show_box| */
#  define hbadness_code 26                                              /* hboxes exceeding this badness will be shown by |hpack| */
#  define vbadness_code 27                                              /* vboxes exceeding this badness will be shown by |vpack| */
#  define pausing_code 28                                               /* pause after each line is read from a file */
#  define tracing_online_code 29                                        /* show diagnostic output on terminal */
#  define tracing_macros_code 30                                        /* show macros as they are being expanded */
#  define tracing_stats_code 31                                         /* show memory usage if \TeX\ knows it */
#  define tracing_paragraphs_code 32                                    /* show line-break calculations */
#  define tracing_pages_code 33                                         /* show page-break calculations */
#  define tracing_output_code 34                                        /* show boxes when they are shipped out */
#  define tracing_lost_chars_code 35                                    /* show characters that aren't in the font */
#  define tracing_commands_code 36                                      /* show command codes at |big_switch| */
#  define tracing_restores_code 37                                      /* show equivalents when they are restored */
#  define uc_hyph_code 38                                               /* hyphenate words beginning with a capital letter */
#  define output_penalty_code 39                                        /* penalty found at current page break */
#  define max_dead_cycles_code 40                                       /* bound on consecutive dead cycles of output */
#  define hang_after_code 41                                            /* hanging indentation changes after this many lines */
#  define floating_penalty_code 42                                      /* penalty for insertions heldover after a split */
#  define global_defs_code 43                                           /* override \.{\\global} specifications */
#  define cur_fam_code 44                                               /* current family */
#  define escape_char_code 45                                           /* escape character for token output */
#  define default_hyphen_char_code 46                                   /* value of \.{\\hyphenchar} when a font is loaded */
#  define default_skew_char_code 47                                     /* value of \.{\\skewchar} when a font is loaded */
#  define end_line_char_code 48                                         /* character placed at the right end of the buffer */
#  define new_line_char_code 49                                         /* character that prints as |print_ln| */
#  define language_code 50                                              /* current hyphenation table */
#  define left_hyphen_min_code 51                                       /* minimum left hyphenation fragment size */
#  define right_hyphen_min_code 52                                      /* minimum right hyphenation fragment size */
#  define holding_inserts_code 53                                       /* do not remove insertion nodes from \.{\\box255} */
#  define error_context_lines_code 54                                   /* maximum intermediate line pairs shown */
#  define local_inter_line_penalty_code 55                              /* local \.{\\interlinepenalty} */
#  define local_broken_penalty_code 56                                  /* local \.{\\brokenpenalty} */
#  define no_local_whatsits_code 57                                     /* counts local whatsits */
#  define no_local_dirs_code 58

#  define disable_lig_code 60
#  define disable_kern_code 61
#  define disable_space_code 62
#  define cat_code_table_code 63
#  define output_box_code 64
#  define cur_lang_code 65                                              /* current language id */
#  define ex_hyphen_char_code 66
#  define hyphenation_min_code 67                                       /* minimum word length */
#  define adjust_spacing_code 68                                        /* level of spacing adjusting */
#  define protrude_chars_code 69                                        /* protrude chars at left/right edge of paragraphs */
#  define output_mode_code 70                                           /* switch on PDF output if positive */
#  define draft_mode_code 71
#  define tracing_fonts_code 72
#  define tracing_assigns_code 73                                       /* show assignments */
#  define tracing_groups_code 74                                        /* show save/restore groups */
#  define tracing_ifs_code 75                                           /* show conditionals */
#  define tracing_scan_tokens_code 76                                   /* show pseudo file open and close */
#  define tracing_nesting_code 77                                       /* show incomplete groups and ifs within files */
#  define pre_display_direction_code 78                                 /* text direction preceding a display */
#  define last_line_fit_code 79                                         /* adjustment for last line of paragraph */
#  define saving_vdiscards_code 80                                      /* save items discarded from vlists */
#  define saving_hyph_codes_code 81                                     /* save hyphenation codes for languages */
#  define suppress_fontnotfound_error_code 82                           /* suppress errors for missing fonts */
#  define suppress_long_error_code 83                                   /* suppress errors for missing fonts */
#  define suppress_ifcsname_error_code 84                               /* suppress errors for failed \.{\\ifcsname} */
#  define suppress_outer_error_code 85                                  /* suppress errors for \.{\\outer} */
#  define suppress_mathpar_error_code 86                                /* suppress errors for \.{\\par}} in math */
#  define math_eqno_gap_step_code 87                                    /* factor/1000 used for distance between eq and eqno */
#  define math_display_skip_mode_code 88
#  define math_scripts_mode_code 89
#  define math_nolimits_mode_code 90
#  define math_rules_mode_code 91
#  define math_rules_fam_code 92
#  define math_italics_mode_code 93
#  define synctex_code 94                                               /* is synctex file generation enabled ?  */
#  define shape_mode_code 95
#  define first_valid_language_code 96
#  define hyphenation_bounds_code 97
#  define math_skip_mode_code 98
#  define math_pre_display_gap_factor_code 99
#  define hyphen_penalty_mode_code 100
#  define automatic_hyphen_penalty_code 101
#  define explicit_hyphen_penalty_code 102
#  define automatic_hyphen_mode_code 103
#  define compound_hyphen_mode_code 104
#  define break_after_dir_mode_code 105
#  define exception_penalty_code 106

#  define pre_bin_op_penalty_code 107
#  define pre_rel_penalty_code 108
#  define math_penalties_mode_code 109
#  define math_delimiters_mode_code 110
#  define math_script_box_mode_code 111
#  define math_script_char_mode_code 112
#  define math_rule_thickness_mode_code 113
#  define math_flatten_mode_code 114

#  define copy_lua_input_nodes_code 115
#  define suppress_primitive_error_code 116
#  define fixup_boxes_code 117

#  define math_option_code (suppress_primitive_error_code+1)

#  define mathoption_int_base_code (math_option_code+1)                 /* one reserve */
#  define mathoption_int_last_code (mathoption_int_base_code+8)

#  define backend_int_base_code (mathoption_int_last_code+1)
#  define backend_int_last_code (backend_int_base_code+32)              /* we need some 25 but take some slack */

#  define tex_int_pars (backend_int_last_code+1)                        /* total number of integer parameters */

#  define mathoption_int_base (int_base+mathoption_int_base_code)
#  define mathoption_int_last (int_base+mathoption_int_last_code)

#  define backend_int_base (int_base+backend_int_base_code)
#  define backend_int_last (int_base+backend_int_last_code)

#  define page_direction_code (tex_int_pars)
#  define body_direction_code (tex_int_pars+1)
#  define par_direction_code  (tex_int_pars+2)
#  define text_direction_code (tex_int_pars+3)
#  define math_direction_code (tex_int_pars+4)
#  define line_direction_code (tex_int_pars+5)                          /* gets remapped so is no real register */

#  define int_pars (tex_int_pars+5)                                     /* total number of integer parameters */

#  define dir_base (int_base+tex_int_pars)
#  define count_base (int_base+int_pars)                                /* |number_regs| user \.{\\count} registers */
#  define attribute_base (count_base+number_regs)                       /* |number_attrs| user \.{\\attribute} registers */
#  define del_code_base (attribute_base+number_attrs)                   /* |number_chars| delimiter code mappings */
#  define dimen_base (del_code_base+1)                                  /* beginning of region 6 */

#  define par_indent_code 0                                             /* indentation of paragraphs */
#  define math_surround_code 1                                          /* space around math in text */
#  define line_skip_limit_code 2                                        /* threshold for |line_skip| instead of |baseline_skip| */
#  define hsize_code 3                                                  /* line width in horizontal mode */
#  define vsize_code 4                                                  /* page height in vertical mode */
#  define max_depth_code 5                                              /* maximum depth of boxes on main pages */
#  define split_max_depth_code 6                                        /* maximum depth of boxes on split pages */
#  define box_max_depth_code 7                                          /* maximum depth of explicit vboxes */
#  define hfuzz_code 8                                                  /* tolerance for overfull hbox messages */
#  define vfuzz_code 9                                                  /* tolerance for overfull vbox messages */
#  define delimiter_shortfall_code 10                                   /* maximum amount uncovered by variable delimiters */
#  define null_delimiter_space_code 11                                  /* blank space in null delimiters */
#  define script_space_code 12                                          /* extra space after subscript or superscript */
#  define pre_display_size_code 13                                      /* length of text preceding a display */
#  define display_width_code 14                                         /* length of line for displayed equation */
#  define display_indent_code 15                                        /* indentation of line for displayed equation */
#  define overfull_rule_code 16                                         /* width of rule that identifies overfull hboxes */
#  define hang_indent_code 17                                           /* amount of hanging indentation */
#  define h_offset_code 18                                              /* amount of horizontal offset when shipping pages out */
#  define v_offset_code 19                                              /* amount of vertical offset when shipping pages out */
#  define emergency_stretch_code 20                                     /* reduces badnesses on final pass of line-breaking */
#  define page_left_offset_code 21
#  define page_top_offset_code 22
#  define page_right_offset_code 23
#  define page_bottom_offset_code 24
#  define px_dimen_code 25
#  define page_width_code 26                                            /* page width of the output */
#  define page_height_code 27                                           /* page height of the output */

#  define backend_dimen_base (dimen_base+28)
#  define backend_dimen_last (dimen_base+37)

#  define dimen_pars (38)                                               /* total number of dimension parameters */

#  define scaled_base (dimen_base+dimen_pars)                           /* table of |number_regs| user-defined \.{\\dimen} registers */
#  define eqtb_size (scaled_base+biggest_reg)                           /* largest subscript of |eqtb| */

extern memory_word *eqtb;

extern halfword eqtb_top;                                               /* maximum of the |eqtb| */

extern quarterword xeq_level[(eqtb_size + 1)];
extern void initialize_equivalents(void);

#  define eq_level_field(A) (A).hh.b1
#  define eq_type_field(A) (A).hh.b0
#  define equiv_field(A) (A).hh.rh

#  define eq_level(A) eq_level_field(eqtb[(A)])                         /* level of definition */
#  define eq_type(A) eq_type_field(eqtb[(A)])                           /* command code for equivalent */
#  define equiv(A) equiv_field(eqtb[(A)])                               /* equivalent value */

#  define set_eq_level(A,B) eq_level((A)) = (B)
#  define set_eq_type(A,B) eq_type((A)) = (B)
#  define set_equiv(A,B) equiv((A)) = (B)

typedef struct save_record_ {
    quarterword type_;
    quarterword level_;
    memory_word word_;
} save_record;

extern save_record *save_stack;
extern int save_ptr;                                                    /* first unused entry on |save_stack| */
extern int max_save_stack;                                              /* maximum usage of save stack */
extern quarterword cur_level;                                           /* current nesting level for groups */
extern group_code cur_group;                                            /* current group type */
extern int cur_boundary;                                                /* where the current level begins */

#  define save_type(A) save_stack[(A)].type_                            /* classifies a |save_stack| entry */
#  define save_level(A) save_stack[(A)].level_                          /* saved level for regions 5 and 6, or group code */
#  define save_value(A) save_stack[(A)].word_.cint                      /* |eqtb| location or token or |save_stack| location */
#  define save_word(A) save_stack[(A)].word_                            /* |eqtb| entry */

/*

We use the notation |saved(k)| to stand for an item that appears in location
|save_ptr+k| of the save stack.

*/

#  define saved_type(A) save_stack[save_ptr+(A)].type_
#  define saved_level(A) save_stack[save_ptr+(A)].level_
#  define saved_value(A) save_stack[save_ptr+(A)].word_.cint

#  define set_saved_record(A,B,C,D) do {	\
        saved_type(A) = (quarterword)(B);   \
        saved_level(A) = (quarterword)(C);  \
        saved_value(A) = (D);               \
    } while (0)

#  define restore_old_value 0                                           /* |save_type| when a value should be restored later */
#  define restore_zero 1                                                /* |save_type| when an undefined entry should be restored */
#  define insert_token 2                                                /* |save_type| when a token is being saved for later use */
#  define level_boundary 3                                              /* |save_type| corresponding to beginning of group */
#  define saved_line 4
#  define saved_adjust 5
#  define saved_insert 6
#  define saved_disc 7
#  define saved_boxtype 8
#  define saved_textdir 9
#  define saved_eqno 10
#  define saved_choices 11
#  define saved_math 12
#  define saved_boxcontext 13
#  define saved_boxspec 14
#  define saved_boxdir 15
#  define saved_boxattr 16
#  define saved_boxpack 17
#  define saved_attrlist 18
#  define saved_eqtb 19

extern void print_save_stack(void);

#  define int_par(A)   eqtb[int_base+(A)].cint
#  define dimen_par(A) eqtb[dimen_base+(A)].cint
#  define loc_par(A)   equiv(local_base+(A))
#  define glue_par(A)  equiv(glue_base+(A))

typedef enum {
    c_mathoption_old_code = 0,                  /* this one is stable */
    /*
    c_mathoption_umathcode_meaning_code,
    */
} math_option_codes ;

#  define mathoption_int_par(A) eqtb[mathoption_int_base+(A)].cint

/* if nonzero, this magnification should be used henceforth */

extern int mag_set;
extern void prepare_mag(void);

/*

@ Here are the group codes that are used to discriminate between different kinds
of groups. They allow \TeX\ to decide what special actions, if any, should be
performed when a group ends. \def\grp{\.{\char'173...\char'175}}

Some groups are not supposed to be ended by right braces. For example, the `\.\$'
that begins a math formula causes a |math_shift_group| to be started, and this
should be terminated by a matching `\.\$'. Similarly, a group that starts with
\.{\\left} should end with \.{\\right}, and one that starts with \.{\\begingroup}
should end with \.{\\endgroup}.

*/

typedef enum {
    bottom_level = 0,    /* group code for the outside world */
    simple_group,        /* group code for local structure only */
    hbox_group,          /* code for `\.{\\hbox}\grp' */
    adjusted_hbox_group, /* code for `\.{\\hbox}\grp' in vertical mode */
    vbox_group,          /* code for `\.{\\vbox}\grp' */
    vtop_group,          /* code for `\.{\\vtop}\grp' */
    align_group,         /* code for `\.{\\halign}\grp', `\.{\\valign}\grp' */
    no_align_group,      /* code for `\.{\\noalign}\grp' */
    output_group,        /* code for output routine */
    math_group,          /* code for, e.g., `\.{\char'136}\grp' */
    disc_group,          /* code for `\.{\\discretionary}\grp\grp\grp' */
    insert_group,        /* code for `\.{\\insert}\grp', `\.{\\vadjust}\grp' */
    vcenter_group,       /* code for `\.{\\vcenter}\grp' */
    math_choice_group,   /* code for `\.{\\mathchoice}\grp\grp\grp\grp' */
    semi_simple_group,   /* code for `\.{\\begingroup...\\endgroup}' */
    math_shift_group,    /* code for `\.{\$...\$}' */
    math_left_group,     /* code for `\.{\\left...\\right}' */
    local_box_group,     /* code for `\.{\\localleftbox...\\localrightbox}' */
    split_off_group,     /* box code for the top part of a \.{\\vsplit} */
    split_keep_group,    /* box code for the bottom part of a \.{\\vsplit} */
    preamble_group,      /* box code for the preamble processing  in an alignment */
    align_set_group,     /* box code for the final item pass in an alignment */
    fin_row_group        /* box code for a provisory line in an alignment */
} tex_group_codes;

typedef enum {
    new_graf_par_code = 0,
    local_box_par_code,
    hmode_par_par_code,
    penalty_par_code,
    math_par_code,
} tex_par_codes ;

typedef enum {
    display_style = 0,           /* |subtype| for \.{\\displaystyle} */
    cramped_display_style,       /* |subtype| for \.{\\crampeddisplaystyle} */
    text_style,                  /* |subtype| for \.{\\textstyle} */
    cramped_text_style,          /* |subtype| for \.{\\crampedtextstyle} */
    script_style,                /* |subtype| for \.{\\scriptstyle} */
    cramped_script_style,        /* |subtype| for \.{\\crampedscriptstyle} */
    script_script_style,         /* |subtype| for \.{\\scriptscriptstyle} */
    cramped_script_script_style, /* |subtype| for \.{\\crampedscriptscriptstyle} */
} math_style_subtypes;

typedef enum { /* this could move to directions.h */
    dir_TLT = 0,
    dir_TRT,
    dir_LTL,
    dir_RTT,
} dir_codes;

#  define max_group_code local_box_group                        /* which is wrong, but is what the web says */

extern int cur_cmd;                                             /* current command set by |get_next| */
extern halfword cur_chr;                                        /* operand of current command */
extern halfword cur_cs;                                         /* control sequence found here, zero if none found */
extern halfword cur_tok;                                        /* packed representative of |cur_cmd| and |cur_chr| */

extern void show_cur_cmd_chr(void);

extern void new_save_level(group_code c);                       /* begin a new level of grouping */
extern void eq_destroy(memory_word w);                          /* gets ready to forget |w| */
extern void eq_save(halfword p, quarterword l);                 /* saves |eqtb[p]| */
extern void eq_define(halfword p, quarterword t, halfword e);   /* new data for |eqtb| */
extern void eq_word_define(halfword p, int w);
extern void geq_define(halfword p, quarterword t, halfword e);  /* global |eq_define| */
extern void geq_word_define(halfword p, int w);                 /* global |eq_word_define| */
extern void save_for_after(halfword t);
extern void unsave(void);                                       /* pops the top level off the save stack */
extern void show_save_groups(void);

#  define level_zero 0                                          /* level for undefined quantities */
#  define level_one 1                                           /* outermost level for defined quantities */

extern void show_eqtb(halfword n);

/* \csname \ifcsname */

extern halfword last_cs_name;

/*
    The *_par macros expand to the variables that are (in most cases) also accessible
    at the users end. Most are registers but some are in the (stack) lists.

    More *_par will move here : there is no real need for thse macros but because there
    were already a bunch and because they were defined all over the place we moved them
    here.

*/

#define space_skip_par                     glue_par(space_skip_code)
#define xspace_skip_par                    glue_par(xspace_skip_code)
#define math_skip_par                      glue_par(math_skip_code)
#define math_skip_mode                     int_par(math_skip_mode_code)

#define pre_display_size_par               dimen_par(pre_display_size_code)
#define display_width_par                  dimen_par(display_width_code)
#define display_indent_par                 dimen_par(display_indent_code)
#define math_surround_par                  dimen_par(math_surround_code)

#define display_skip_mode_par              int_par(math_display_skip_mode_code)
#define math_eqno_gap_step_par             int_par(math_eqno_gap_step_code)

#define body_direction_par                 int_par(body_direction_code)
#define page_direction_par                 int_par(page_direction_code)
#define par_direction_par                  int_par(par_direction_code)
#define text_direction_par                 int_par(text_direction_code)
#define math_direction_par                 int_par(math_direction_code)

#define shape_mode_par                     int_par(shape_mode_code)
#define hyphenation_bounds_par             int_par(hyphenation_bounds_code)
#define first_valid_language_par           int_par(first_valid_language_code)

#define hsize_par                          dimen_par(hsize_code)
#define vsize_par                          dimen_par(vsize_code)
#define hfuzz_par                          dimen_par(hfuzz_code)
#define vfuzz_par                          dimen_par(vfuzz_code)
#define hbadness_par                       int_par(hbadness_code)
#define vbadness_par                       int_par(vbadness_code)

#define baseline_skip_par                  glue_par(baseline_skip_code)
#define par_indent_par                     dimen_par(par_indent_code)
#define hang_indent_par                    dimen_par(hang_indent_code)
#define hang_after_par                     int_par(hang_after_code)
#define left_skip_par                      glue_par(left_skip_code)
#define right_skip_par                     glue_par(right_skip_code)

#define emergency_stretch_par              dimen_par(emergency_stretch_code)
#define pretolerance_par                   int_par(pretolerance_code)
#define tolerance_par                      int_par(tolerance_code)
#define looseness_par                      int_par(looseness_code)
#define adjust_spacing_par                 int_par(adjust_spacing_code)
#define adj_demerits_par                   int_par(adj_demerits_code)
#define protrude_chars_par                 int_par(protrude_chars_code)
#define line_penalty_par                   int_par(line_penalty_code)
#define last_line_fit_par                  int_par(last_line_fit_code)
#define double_hyphen_demerits_par         int_par(double_hyphen_demerits_code)
#define final_hyphen_demerits_par          int_par(final_hyphen_demerits_code)
#define inter_line_penalty_par             int_par(inter_line_penalty_code)
#define club_penalty_par                   int_par(club_penalty_code)
#define broken_penalty_par                 int_par(broken_penalty_code)
#define display_widow_penalty_par          int_par(display_widow_penalty_code)
#define widow_penalty_par                  int_par(widow_penalty_code)
#define line_skip_limit_par                dimen_par(line_skip_limit_code)

#define delimiter_shortfall_par            dimen_par(delimiter_shortfall_code)
#define null_delimiter_space_par           dimen_par(null_delimiter_space_code)
#define script_space_par                   dimen_par(script_space_code)
#define max_depth_par                      dimen_par(max_depth_code)
#define box_max_depth_par                  dimen_par(box_max_depth_code)
#define split_max_depth_par                dimen_par(split_max_depth_code)
#define overfull_rule_par                  dimen_par(overfull_rule_code)
#define box_max_depth_par                  dimen_par(box_max_depth_code)
#define top_skip_par                       glue_par(top_skip_code)
#define split_top_skip_par                 glue_par(split_top_skip_code)

#define cur_fam_par                        int_par(cur_fam_code)
#define pre_display_direction_par          int_par(pre_display_direction_code)
#define pre_display_penalty_par            int_par(pre_display_penalty_code)
#define post_display_penalty_par           int_par(post_display_penalty_code)

#define cur_fam_par_in_range               ((cur_fam_par>=0)&&(cur_fam_par<256))

#define local_inter_line_penalty_par       int_par(local_inter_line_penalty_code)
#define local_broken_penalty_par           int_par(local_broken_penalty_code)
#define local_left_box_par                 equiv(local_left_box_base)
#define local_right_box_par                equiv(local_right_box_base)

#define end_line_char_par                  int_par(end_line_char_code)
#define new_line_char_par                  int_par(new_line_char_code)
#define escape_char_par                    int_par(escape_char_code)

#define end_line_char_inactive             ((end_line_char_par < 0) || (end_line_char_par > 127))

#define delimiter_factor_par               int_par(delimiter_factor_code)
#define bin_op_penalty_par                 int_par(bin_op_penalty_code)
#define rel_penalty_par                    int_par(rel_penalty_code)
#define pre_bin_op_penalty_par             int_par(pre_bin_op_penalty_code)
#define pre_rel_penalty_par                int_par(pre_rel_penalty_code)
#define math_penalties_mode_par            int_par(math_penalties_mode_code)
#define math_delimiters_mode_par           int_par(math_delimiters_mode_code)
#define math_script_box_mode_par           int_par(math_script_box_mode_code)
#define math_script_char_mode_par          int_par(math_script_char_mode_code)
#define math_rule_thickness_mode_par       int_par(math_rule_thickness_mode_code)
#define math_flatten_mode_par              int_par(math_flatten_mode_code)
#define null_delimiter_space_par           dimen_par(null_delimiter_space_code)
#define disable_lig_par                    int_par(disable_lig_code)
#define disable_kern_par                   int_par(disable_kern_code)
#define disable_space_par                  int_par(disable_space_code)
#define math_scripts_mode_par              int_par(math_scripts_mode_code)
#define math_nolimits_mode_par             int_par(math_nolimits_mode_code)
#define math_italics_mode_par              int_par(math_italics_mode_code)
#define math_rules_mode_par                int_par(math_rules_mode_code)
#define math_rules_fam_par                 int_par(math_rules_fam_code)

#define thin_mu_skip_par                   glue_par(thin_mu_skip_code)
#define med_mu_skip_par                    glue_par(med_mu_skip_code)
#define thick_mu_skip_par                  glue_par(thick_mu_skip_code)

#define every_math_par                     equiv(every_math_loc)
#define every_display_par                  equiv(every_display_loc)
#define every_cr_par                       equiv(every_cr_loc)
#define every_hbox_par                     equiv(every_hbox_loc)
#define every_vbox_par                     equiv(every_vbox_loc)
#define every_eof_par                      equiv(every_eof_loc)
#define every_par_par                      equiv(every_par_loc)
#define err_help_par                       equiv(err_help_loc)

#define no_local_whatsits_par              int_par(no_local_whatsits_code)
#define no_local_dirs_par                  int_par(no_local_dirs_code)

#define prev_depth_par                     cur_list.prev_depth_field
#define prev_graf_par                      cur_list.pg_field
#define tail_par                           cur_list.tail_field
#define head_par                           cur_list.head_field
#define mode_par                           cur_list.mode_field
#define dirs_par                           cur_list.dirs_field
#define space_factor_par                   cur_list.space_factor_field
#define incompleat_noad_par                cur_list.incompleat_noad_field
#define mode_line_par                      cur_list.ml_field
#define aux_par                            cur_list.eTeX_aux_field
#define delim_par                          aux_par

#define par_shape_par_ptr                  equiv(par_shape_loc)
#define inter_line_penalties_par_ptr       equiv(inter_line_penalties_loc)
#define club_penalties_par_ptr             equiv(club_penalties_loc)
#define widow_penalties_par_ptr            equiv(widow_penalties_loc)
#define display_widow_penalties_par_ptr    equiv(display_widow_penalties_loc)

#define page_width_par                     dimen_par(page_width_code)
#define page_height_par                    dimen_par(page_height_code)
#define h_offset_par                       dimen_par(h_offset_code)
#define v_offset_par                       dimen_par(v_offset_code)
#define page_left_offset_par               dimen_par(page_left_offset_code)
#define page_top_offset_par                dimen_par(page_top_offset_code)
#define page_right_offset_par              dimen_par(page_right_offset_code)
#define page_bottom_offset_par             dimen_par(page_bottom_offset_code)
#define px_dimen_par                       dimen_par(px_dimen_code)

#define max_dead_cycles_par                int_par(max_dead_cycles_code)
#define output_box_par                     int_par(output_box_code)
#define holding_inserts_par                int_par(holding_inserts_code)
#define output_routine_par                 equiv(output_routine_loc)
#define floating_penalty_par               int_par(floating_penalty_code)

#define mag_par                            int_par(mag_code)

#define global_defs_par                    int_par(global_defs_code)
#define cat_code_table_par                 int_par(cat_code_table_code)
#define saving_vdiscards_par               int_par(saving_vdiscards_code)

#define tracing_output_par                 int_par(tracing_output_code)
#define tracing_stats_par                  int_par(tracing_stats_code)
#define tracing_online_par                 int_par(tracing_online_code)
#define tracing_paragraphs_par             int_par(tracing_paragraphs_code)
#define tracing_nesting_par                int_par(tracing_nesting_code)
#define tracing_lost_chars_par             int_par(tracing_lost_chars_code)
#define tracing_scan_tokens_par            int_par(tracing_scan_tokens_code)
#define tracing_ifs_par                    int_par(tracing_ifs_code)
#define tracing_commands_par               int_par(tracing_commands_code)
#define tracing_macros_par                 int_par(tracing_macros_code)
#define tracing_assigns_par                int_par(tracing_assigns_code)
#define tracing_fonts_par                  int_par(tracing_fonts_code)
#define tracing_pages_par                  int_par(tracing_pages_code)
#define tracing_restores_par               int_par(tracing_restores_code)
#define tracing_groups_par                 int_par(tracing_groups_code)

#define show_box_depth_par                 int_par(show_box_depth_code)
#define show_box_breadth_par               int_par(show_box_breadth_code)

#define pausing_par                        int_par(pausing_code)

#define suppress_outer_error_par           int_par(suppress_outer_error_code)
#define suppress_long_error_par            int_par(suppress_long_error_code)
#define suppress_mathpar_error_par         int_par(suppress_mathpar_error_code)
#define suppress_fontnotfound_error_par    int_par(suppress_fontnotfound_error_code)
#define suppress_ifcsname_error_par        int_par(suppress_ifcsname_error_code)
#define suppress_primitive_error_par       int_par(suppress_primitive_error_code)
#define error_context_lines_par            int_par(error_context_lines_code)
#define copy_lua_input_nodes_par           int_par(copy_lua_input_nodes_code)

#define math_old_par                       mathoption_int_par(c_mathoption_old_code)

/*
#define math_umathcode_meaning_par         mathoption_int_par(c_mathoption_umathcode_meaning_code)
*/

#define math_pre_display_gap_factor_par    int_par(math_pre_display_gap_factor_code)

#define time_par                           int_par(time_code)
#define day_par                            int_par(day_code)
#define month_par                          int_par(month_code)
#define year_par                           int_par(year_code)

#define output_mode_par                    int_par(output_mode_code)
#define draft_mode_par                     int_par(draft_mode_code)
#define synctex_par                        int_par(synctex_code)

#define language_par                       int_par(language_code)
#define uc_hyph_par                        int_par(uc_hyph_code)
#define left_hyphen_min_par                int_par(left_hyphen_min_code)
#define right_hyphen_min_par               int_par(right_hyphen_min_code)
#define ex_hyphen_char_par                 int_par(ex_hyphen_char_code)
#define hyphen_penalty_par                 int_par(hyphen_penalty_code)
#define ex_hyphen_penalty_par              int_par(ex_hyphen_penalty_code)
#define default_hyphen_char_par            int_par(default_hyphen_char_code)
#define default_skew_char_par              int_par(default_skew_char_code)
#define saving_hyph_codes_par              int_par(saving_hyph_codes_code)

#define hyphen_penalty_mode_par            int_par(hyphen_penalty_mode_code)
#define automatic_hyphen_penalty_par       int_par(automatic_hyphen_penalty_code)
#define explicit_hyphen_penalty_par        int_par(explicit_hyphen_penalty_code)
#define automatic_hyphen_mode_par          int_par(automatic_hyphen_mode_code)
#define compound_hyphen_mode_par           int_par(compound_hyphen_mode_code)
#define break_after_dir_mode_par           int_par(break_after_dir_mode_code)
#define exception_penalty_par              int_par(exception_penalty_code)

#define cur_lang_par                       int_par(cur_lang_code)
#define cur_font_par                       equiv(cur_font_loc)

#define fixup_boxes_par                    int_par(fixup_boxes_code)

/* */

#define math_use_current_family_code 7

/*
    #define box(A) equiv(box_base+(A))
    #define box(A) eqtb[box_base+(A)].hh.rh
*/

#define attribute(A) equiv(attribute_base+(A))
#define box(A)       equiv(box_base+(A))
#define count(A)     equiv(count_base+(A))
#define dimen(A)     equiv(scaled_base+(A))
#define mu_skip(A)   equiv(mu_skip_base+(A))
#define skip(A)      equiv(skip_base+(A))
#define toks(A)      equiv(toks_base+(A))

#define get_tex_attribute_register(j) attribute(j)
#define get_tex_box_register(j)       box(j)
#define get_tex_count_register(j)     count(j)
#define get_tex_dimen_register(j)     dimen(j)
#define get_tex_mu_skip_register(j)   mu_skip(j)
#define get_tex_skip_register(j)      skip(j)

#define font_id_text(A) cs_text(font_id_base+(A))

#define end_template_token  (cs_token_flag+frozen_end_template)
#define end_write_token     (cs_token_flag+end_write)

/* for now here */

#define xspace_skip_subtype (xspace_skip_code + 1)
#define space_skip_subtype  (space_skip_code + 1)

/*

hyphen_penalty_mode_par   automatic_disc (-)                explicit_disc (\-)
---------------------------------------------------------------------------------
0 (default)               ex_hyphen_penalty_par             ex_hyphen_penalty_par
1                         hyphen_penalty_par                hyphen_penalty_par
2                         ex_hyphen_penalty_par             hyphen_penalty_par
3                         hyphen_penalty_par                ex_hyphen_penalty_par
4                         automatic_hyphen_penalty_par      explicit_disc_penalty_par
5                         ex_hyphen_penalty_par             explicit_disc_penalty_par
6                         hyphen_penalty_par                explicit_disc_penalty_par
7                         automatic_hyphen_penalty_par      ex_hyphen_penalty_par
8                         automatic_hyphen_penalty_par      hyphen_penalty_par
*/

#define set_automatic_disc_penalty(n) \
    switch (hyphen_penalty_mode_par) { \
        case 0: \
        case 2: \
        case 5: \
            /* we take ex_hyphen_penalty */ \
            disc_penalty(n) = ex_hyphen_penalty_par; \
            break; \
        case 1: \
        case 3: \
        case 6: \
            /* we take hyphen_penalty */ \
            disc_penalty(n) = hyphen_penalty_par; \
            break; \
        case 4: \
        case 7: \
        case 8: \
            /* we take automatic_hyphen_penalty */ \
            disc_penalty(n) = automatic_hyphen_penalty_par; \
            break; \
        default: \
            /* what we've done since the beginning */ \
            disc_penalty(n) = ex_hyphen_penalty_par; \
            break; \
    }

#define set_explicit_disc_penalty(n) \
    switch (hyphen_penalty_mode_par) { \
        case 0: \
        case 3: \
        case 7: \
            /* we take ex_hyphen_penalty */ \
            disc_penalty(n) = ex_hyphen_penalty_par; \
            break; \
        case 1: \
        case 2: \
        case 8: \
            /* we take hyphen_penalty */ \
            disc_penalty(n) = hyphen_penalty_par; \
            break; \
        case 4: \
        case 5: \
        case 6: \
            /* we take automatic_hyphen_penalty */ \
            disc_penalty(n) = explicit_hyphen_penalty_par; \
            break; \
        default: \
            /* what we've done since the beginning */ \
            disc_penalty(n) = ex_hyphen_penalty_par; \
            break; \
    }

#endif
