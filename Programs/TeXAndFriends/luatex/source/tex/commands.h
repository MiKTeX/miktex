/* commands.h

   Copyright 2008 Taco Hoekwater <taco@luatex.org>

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


#ifndef COMMANDS_H
#  define COMMANDS_H

/*
Before we can go any further, we need to define symbolic names for the internal
code numbers that represent the various commands obeyed by \TeX. These codes
are somewhat arbitrary, but not completely so. For example, the command
codes for character types are fixed by the language, since a user says,
e.g., `\.{\\catcode \`\\\${} = 3}' to make \.{\char'44} a math delimiter,
and the command code |math_shift| is equal to~3. Some other codes have
been made adjacent so that |case| statements in the program need not consider
cases that are widely spaced, or so that |case| statements can be replaced
by |if| statements.

At any rate, here is the list, for future reference. First come the
``catcode'' commands, several of which share their numeric codes with
ordinary commands when the catcode cannot emerge from \TeX's scanning routine.

@ Next are the ordinary run-of-the-mill command codes.  Codes that are
|min_internal| or more represent internal quantities that might be
expanded by `\.{\\the}'.


@ The next codes are special; they all relate to mode-independent
assignment of values to \TeX's internal registers or tables.
Codes that are |max_internal| or less represent internal quantities
that might be expanded by `\.{\\the}'.

There is no matching primitive to go with |assign_attr|, but even if
there was no \.{\\attributedef}, a reserved number would still be
needed because there is an implied correspondence between the
|assign_xxx| commands and |xxx_val| expression values. That would
break down otherwise.

@ The remaining command codes are extra special, since they cannot get through
\TeX's scanner to the main control routine. They have been given values higher
than |max_command| so that their special nature is easily discernible.
The ``expandable'' commands come first.


*/

typedef enum {
    relax_cmd = 0,                        /* do nothing ( \.{\\relax} ) */
#  define escape_cmd  relax_cmd           /* escape delimiter (called \.\\ in {\sl The \TeX book\/}) */
    left_brace_cmd,                       /* beginning of a group ( \.\{ ) */
    right_brace_cmd,                      /* ending of a group ( \.\} ) */
    math_shift_cmd,                       /* mathematics shift character ( \.\$ ) */
    tab_mark_cmd,                         /* alignment delimiter ( \.\&, \.{\\span} ) */
    car_ret_cmd,                          /* end of line ( |carriage_return|, \.{\\cr}, \.{\\crcr} ) */
#  define out_param_cmd  car_ret_cmd      /* output a macro parameter */
    mac_param_cmd,                        /* macro parameter symbol ( \.\# ) */
    sup_mark_cmd,                         /* superscript ( \.{\char'136} ) */
    sub_mark_cmd,                         /* subscript ( \.{\char'137} ) */
    endv_cmd,                             /* end of \<v_j> list in alignment template */
#  define ignore_cmd endv_cmd             /* characters to ignore ( \.{\^\^@@} ) */
    spacer_cmd,                           /* characters equivalent to blank space ( \.{\ } ) */
    letter_cmd,                           /* characters regarded as letters ( \.{A..Z}, \.{a..z} ) */
    other_char_cmd,                       /* none of the special character types */
    par_end_cmd,                          /* end of paragraph ( \.{\\par} ) */
#  define active_char_cmd par_end_cmd     /* characters that invoke macros ( \.{\char`\~} ) */
#  define match_cmd par_end_cmd           /* match a macro parameter */
    stop_cmd,                             /* end of job ( \.{\\end}, \.{\\dump} ) */
#  define comment_cmd stop_cmd            /* characters that introduce comments ( \.\% ) */
#  define end_match_cmd stop_cmd          /* end of parameters to macro */
    delim_num_cmd,                        /* specify delimiter numerically ( \.{\\delimiter} ) */
#  define invalid_char_cmd delim_num_cmd  /* characters that shouldn't appear ( \.{\^\^?} ) */
#  define max_char_code_cmd delim_num_cmd /* largest catcode for individual characters */
    char_num_cmd,                         /* character specified numerically ( \.{\\char} ) */
    math_char_num_cmd,                    /* explicit math code ( \.{\\mathchar} ) */
    mark_cmd,                             /* mark definition ( \.{\\mark} ) */
    xray_cmd,                             /* peek inside of \TeX\ ( \.{\\show}, \.{\\showbox}, etc.~) */
    make_box_cmd,                         /* make a box ( \.{\\box}, \.{\\copy}, \.{\\hbox}, etc.~) */
    hmove_cmd,                            /* horizontal motion ( \.{\\moveleft}, \.{\\moveright} ) */
    vmove_cmd,                            /* vertical motion ( \.{\\raise}, \.{\\lower} ) */
    un_hbox_cmd,                          /* unglue a box ( \.{\\unhbox}, \.{\\unhcopy} ) */
    un_vbox_cmd,                          /* unglue a box ( \.{\\unvbox}, \.{\\unvcopy} or \.{\\pagediscards}, \.{\\splitdiscards} ) */
    remove_item_cmd,                      /* nullify last item ( \.{\\unpenalty}, \.{\\unkern}, \.{\\unskip} ) */
    hskip_cmd,                            /* horizontal glue ( \.{\\hskip}, \.{\\hfil}, etc.~) */
    vskip_cmd,                            /* vertical glue ( \.{\\vskip}, \.{\\vfil}, etc.~) */
    mskip_cmd,                            /* math glue ( \.{\\mskip} ) */
    kern_cmd,                             /* fixed space ( \.{\\kern}) */
    mkern_cmd,                            /* math kern ( \.{\\mkern} ) */
    leader_ship_cmd,                      /* use a box ( \.{\\shipout}, \.{\\leaders}, etc.~) */
    halign_cmd,                           /* horizontal table alignment ( \.{\\halign} ) */
    valign_cmd,                           /* vertical table alignment ( \.{\\valign} ) */
    no_align_cmd,                         /* temporary escape from alignment ( \.{\\noalign} ) */
    vrule_cmd,                            /* vertical rule ( \.{\\vrule} ) */
    hrule_cmd,                            /* horizontal rule ( \.{\\hrule} ) */
    no_vrule_cmd,                         /* no rule, just reserve space ( \.{\\novrule} )*/
    no_hrule_cmd,                         /* no rule, just reserve space ( \.{\\nohrule} )*/
    insert_cmd,                           /* vlist inserted in box ( \.{\\insert} ) */
    vadjust_cmd,                          /* vlist inserted in enclosing paragraph ( \.{\\vadjust} ) */
    ignore_spaces_cmd,                    /* gobble |spacer| tokens ( \.{\\ignorespaces} ) */
    after_assignment_cmd,                 /* save till assignment is done ( \.{\\afterassignment} ) */
    after_group_cmd,                      /* save till group is done ( \.{\\aftergroup} ) */
    break_penalty_cmd,                    /* additional badness ( \.{\\penalty} ) */
    start_par_cmd,                        /* begin paragraph ( \.{\\indent}, \.{\\noindent} ) */
    ital_corr_cmd,                        /* italic correction ( \.{\\/} ) */
    accent_cmd,                           /* attach accent in text ( \.{\\accent} ) */
    math_accent_cmd,                      /* attach accent in math ( \.{\\mathaccent} ) */
    discretionary_cmd,                    /* discretionary texts ( \.{\\-}, \.{\\discretionary} ) */
    eq_no_cmd,                            /* equation number ( \.{\\eqno}, \.{\\leqno} ) */
    left_right_cmd,                       /* variable delimiter ( \.{\\left}, \.{\\right} or \.{\\middle} ) */
    math_comp_cmd,                        /* component of formula ( \.{\\mathbin}, etc.~) */
    limit_switch_cmd,                     /* diddle limit conventions ( \.{\\displaylimits}, etc.~) */
    above_cmd,                            /* generalized fraction ( \.{\\above}, \.{\\atop}, etc.~) */
    math_style_cmd,                       /* style specification ( \.{\\displaystyle}, etc.~) */
    math_choice_cmd,                      /* choice specification ( \.{\\mathchoice} ) */
    non_script_cmd,                       /* conditional math glue ( \.{\\nonscript} ) */
    vcenter_cmd,                          /* vertically center a vbox ( \.{\\vcenter} ) */
    case_shift_cmd,                       /* force specific case ( \.{\\lowercase}, \.{\\uppercase}~) */
    message_cmd,                          /* send to user ( \.{\\message}, \.{\\errmessage} ) */
    normal_cmd,                           /* general extensions to \TeX\ that don't fit into a category */
    extension_cmd,                        /* extensions to \TeX\ ( \.{\\write}, \.{\\special}, etc.~) */
    option_cmd,
    in_stream_cmd,                        /* files for reading ( \.{\\openin}, \.{\\closein} ) */
    begin_group_cmd,                      /* begin local grouping ( \.{\\begingroup} ) */
    end_group_cmd,                        /* end local grouping ( \.{\\endgroup} ) */
    omit_cmd,                             /* omit alignment template ( \.{\\omit} ) */
    ex_space_cmd,                         /* explicit space ( \.{\\\ } ) */
    boundary_cmd,                         /* insert boundry node with value ( \.{\\*boundary} ) */
    radical_cmd,                          /* square root and similar signs ( \.{\\radical} ) */
    super_sub_script_cmd,                 /* explicit super- or subscript */
    no_super_sub_script_cmd,              /* explicit no super- or subscript */
    math_shift_cs_cmd,                    /* start- and endmath */
    end_cs_name_cmd,                      /* end control sequence ( \.{\\endcsname} ) */
    char_ghost_cmd,                       /* \.{\\leftghost}, \.{\\rightghost} character for kerning */
    assign_local_box_cmd,                 /* box for guillemets \.{\\localleftbox} or \.{\\localrightbox} */
    char_given_cmd,                       /* character code defined by \.{\\chardef} */
#  define min_internal_cmd char_given_cmd /* the smallest code that can follow \.{\\the} */
    math_given_cmd,                       /* math code defined by \.{\\mathchardef} */
    xmath_given_cmd,                      /* math code defined by \.{\\Umathchardef} or \.{\\Umathcharnumdef} */
    last_item_cmd,                        /* most recent item ( \.{\\lastpenalty}, \.{\\lastkern}, \.{\\lastskip} ) */
#  define max_non_prefixed_command_cmd last_item_cmd    /* largest command code that can't be \.{\\global} */
    toks_register_cmd,                    /* token list register ( \.{\\toks} ) */
    assign_toks_cmd,                      /* special token list ( \.{\\output}, \.{\\everypar}, etc.~) */
    assign_int_cmd,                       /* user-defined integer ( \.{\\tolerance}, \.{\\day}, etc.~) */
    assign_attr_cmd,                      /*  user-defined attributes  */
    assign_dimen_cmd,                     /* user-defined length ( \.{\\hsize}, etc.~) */
    assign_glue_cmd,                      /* user-defined glue ( \.{\\baselineskip}, etc.~) */
    assign_mu_glue_cmd,                   /* user-defined muglue ( \.{\\thinmuskip}, etc.~) */
    assign_font_dimen_cmd,                /* user-defined font dimension ( \.{\\fontdimen} ) */
    assign_font_int_cmd,                  /* user-defined font integer ( \.{\\hyphenchar}, \.{\\skewchar} ) */
    assign_hang_indent_cmd,
    set_aux_cmd,                          /* specify state info ( \.{\\spacefactor}, \.{\\prevdepth} ) */
    set_prev_graf_cmd,                    /* specify state info ( \.{\\prevgraf} ) */
    set_page_dimen_cmd,                   /* specify state info ( \.{\\pagegoal}, etc.~) */
    set_page_int_cmd,                     /* specify state info ( \.{\\deadcycles},  \.{\\insertpenalties} ) */
    set_box_dimen_cmd,                    /* change dimension of box ( \.{\\wd}, \.{\\ht}, \.{\\dp} ) */
    set_tex_shape_cmd,                    /* specify fancy paragraph shape ( \.{\\parshape} ) */
    set_etex_shape_cmd,                   /* specify etex extended list ( \.{\\interlinepenalties}, etc.~) */
    def_char_code_cmd,                    /* define a character code ( \.{\\catcode}, etc.~) */
    def_del_code_cmd,                     /* define a delimiter code ( \.{\\delcode}) */
    extdef_math_code_cmd,                 /* define an extended character code ( \.{\\Umathcode}, etc.~) */
    extdef_del_code_cmd,                  /* define an extended delimiter code ( \.{\\Udelcode}, etc.~) */
    def_family_cmd,                       /* declare math fonts ( \.{\\textfont}, etc.~) */
    set_math_param_cmd,                   /* set math parameters ( \.{\\mathquad}, etc.~) */
    set_font_cmd,                         /* set current font ( font identifiers ) */
    def_font_cmd,                         /* define a font file ( \.{\\font} ) */
    register_cmd,                         /* internal register ( \.{\\count}, \.{\\dimen}, etc.~) */
    assign_box_dir_cmd,                   /* (\.{\\boxdir}) */
    assign_dir_cmd,                       /* (\.{\\pagedir}, \.{\\textdir}) */
# define max_internal_cmd assign_dir_cmd  /* the largest code that can follow \.{\\the} */
    advance_cmd,                          /* advance a register or parameter ( \.{\\advance} ) */
    multiply_cmd,                         /* multiply a register or parameter ( \.{\\multiply} ) */
    divide_cmd,                           /* divide a register or parameter ( \.{\\divide} ) */
    prefix_cmd,                           /* qualify a definition ( \.{\\global}, \.{\\long}, \.{\\outer} ) */
    let_cmd,                              /* assign a command code ( \.{\\let}, \.{\\futurelet} ) */
    shorthand_def_cmd,                    /* code definition ( \.{\\chardef}, \.{\\countdef}, etc.~) */
    read_to_cs_cmd,                       /* read into a control sequence ( \.{\\read} ) */
    def_cmd,                              /* macro definition ( \.{\\def}, \.{\\gdef}, \.{\\xdef}, \.{\\edef} ) */
    set_box_cmd,                          /* set a box ( \.{\\setbox} ) */
    hyph_data_cmd,                        /* hyphenation data ( \.{\\hyphenation}, \.{\\patterns} ) */
    set_interaction_cmd,                  /* define level of interaction ( \.{\\batchmode}, etc.~) */
    letterspace_font_cmd,                 /* letterspace a font ( \.{\\letterspacefont} ) */
    expand_font_cmd,                      /* expand glyphs ( \.{\\expandglyphsinfont} ) */
    copy_font_cmd,                        /* create a new font instance ( \.{\\copyfont} ) */
    set_font_id_cmd,
    undefined_cs_cmd,                     /* initial state of most |eq_type| fields */
    expand_after_cmd,                     /* special expansion ( \.{\\expandafter} ) */
    no_expand_cmd,                        /* special nonexpansion ( \.{\\noexpand} ) */
    input_cmd,                            /* input a source file ( \.{\\input}, \.{\\endinput} or \.{\\scantokens} or \.{\\scantextokens} ) */
    if_test_cmd,                          /* conditional text ( \.{\\if}, \.{\\ifcase}, etc.~) */
    fi_or_else_cmd,                       /* delimiters for conditionals ( \.{\\else}, etc.~) */
    cs_name_cmd,                          /* make a control sequence from tokens ( \.{\\csname} ) */
    convert_cmd,                          /* convert to text ( \.{\\number}, \.{\\string}, etc.~) */
    variable_cmd,
    feedback_cmd,
    the_cmd,                              /* expand an internal quantity ( \.{\\the} or \.{\\unexpanded}, \.{\\detokenize} ) */
    combine_toks_cmd,
    top_bot_mark_cmd,                     /* inserted mark ( \.{\\topmark}, etc.~) */
    call_cmd,                             /* non-long, non-outer control sequence */
    long_call_cmd,                        /* long, non-outer control sequence */
    outer_call_cmd,                       /* non-long, outer control sequence */
    long_outer_call_cmd,                  /* long, outer control sequence */
    end_template_cmd,                     /* end of an alignment template */
    dont_expand_cmd,                      /* the following token was marked by \.{\\noexpand} */
    glue_ref_cmd,                         /* the equivalent points to a glue specification */
    shape_ref_cmd,                        /* the equivalent points to a parshape specification */
    box_ref_cmd,                          /* the equivalent points to a box node, or is |null| */
    data_cmd,                             /* the equivalent is simply a halfword number */
} tex_command_code;

#  define max_command_cmd set_font_id_cmd /* the largest command code seen at |big_switch| */
#  define last_cmd data_cmd
#  define max_non_prefixed_command last_item_cmd

typedef enum {
    above_code = 0,
    over_code = 1,
    atop_code = 2,
    skewed_code = 3,
    delimited_code = 4,
} fraction_codes;

typedef enum {
    number_code = 0,            /* command code for \.{\\number} */
    lua_function_code,          /* command code for \.{\\luafunction} */
    lua_code,                   /* command code for \.{\\directlua} */
    expanded_code,              /* command code for \.{\\expanded} */
    math_style_code,            /* command code for \.{\\mathstyle} */
    string_code,                /* command code for \.{\\string} */
    cs_string_code,             /* command code for \.{\\csstring} */
    roman_numeral_code,         /* command code for \.{\\romannumeral} */
    meaning_code,               /* command code for \.{\\meaning} */
    uchar_code,                 /* command code for \.{\\Uchar} */
    lua_escape_string_code,     /* command code for \.{\\luaescapestring} */
    font_id_code,               /* command code for \.{\\fontid} */
    font_name_code,             /* command code for \.{\\fontname} */
    left_margin_kern_code,      /* command code for \.{\\leftmarginkern} */
    right_margin_kern_code,     /* command code for \.{\\rightmarginkern} */
    uniform_deviate_code,       /* command code for \.{\\uniformdeviate} */
    normal_deviate_code,        /* command code for \.{\\normaldeviate} */
    math_char_class_code,
    math_char_fam_code,
    math_char_slot_code,
    insert_ht_code,             /* command code for \.{\\insertht} */
    job_name_code,              /* command code for \.{\\jobname} */
    format_name_code,           /* command code for \.{\\AlephVersion} */
    luatex_banner_code,         /* command code for \.{\\luatexbanner}: */
    luatex_revision_code,       /* command code for \.{\\luatexrevision} */
    etex_code,                  /* command code for \.{\\eTeXVersion} */
    eTeX_revision_code,         /* command code for \.{\\eTeXrevision} */
    font_identifier_code,       /* command code for \.{tex.fontidentifier} (virtual) */
    /* backend */
    dvi_feedback_code,
    pdf_feedback_code,
    dvi_variable_code,
    pdf_variable_code,
} convert_codes;

typedef enum {
    lastpenalty_code = 0,                 /* code for \.{\\lastpenalty} */
    lastattr_code,                        /* not used */
    lastkern_code,                        /* code for \.{\\lastkern} */
    lastskip_code,                        /* code for \.{\\lastskip} */
    last_node_type_code,                  /* code for \.{\\lastnodetype} */
    input_line_no_code,                   /* code for \.{\\inputlineno} */
    badness_code,                         /* code for \.{\\badness} */
    last_saved_box_resource_index_code,   /* code for \.{\\lastsavedboxresourceindex} */
    last_saved_image_resource_index_code, /* code for \.{\\lastsavedimageresourceindex} */
    last_saved_image_resource_pages_code, /* code for \.{\\lastsavedimageresourcepages} */
    last_x_pos_code,                      /* code for \.{\\lastxpos} */
    last_y_pos_code,                      /* code for \.{\\lastypos} */
    random_seed_code,                     /* code for \.{\\randomseed} */
    luatex_version_code,                  /* code for \.{\\luatexversion} */
    eTeX_minor_version_code,              /* code for \.{\\eTeXminorversion} */
    eTeX_version_code,                    /* code for \.{\\eTeXversion} */
#  define eTeX_int eTeX_version_code      /* first of \eTeX\ codes for integers */
    current_group_level_code,             /* code for \.{\\currentgrouplevel} */
    current_group_type_code,              /* code for \.{\\currentgrouptype} */
    current_if_level_code,                /* code for \.{\\currentiflevel} */
    current_if_type_code,                 /* code for \.{\\currentiftype} */
    current_if_branch_code,               /* code for \.{\\currentifbranch} */
    glue_stretch_order_code,              /* code for \.{\\gluestretchorder} */
    glue_shrink_order_code,               /* code for \.{\\glueshrinkorder} */
    font_char_wd_code,                    /* code for \.{\\fontcharwd} */
#  define eTeX_dim font_char_wd_code      /* first of \eTeX\ codes for dimensions */
    font_char_ht_code,                    /* code for \.{\\fontcharht} */
    font_char_dp_code,                    /* code for \.{\\fontchardp} */
    font_char_ic_code,                    /* code for \.{\\fontcharic} */
    par_shape_length_code,                /* code for \.{\\parshapelength} */
    par_shape_indent_code,                /* code for \.{\\parshapeindent} */
    par_shape_dimen_code,                 /* code for \.{\\parshapedimen} */
    glue_stretch_code,                    /* code for \.{\\gluestretch} */
    glue_shrink_code,                     /* code for \.{\\glueshrink} */
    mu_to_glue_code,                      /* code for \.{\\mutoglue} */
#  define eTeX_glue mu_to_glue_code       /* first of \eTeX\ codes for glue */
    glue_to_mu_code,                      /* code for \.{\\gluetomu} */
#  define eTeX_mu glue_to_mu_code         /* first of \eTeX\ codes for muglue */
    numexpr_code,                         /* code for \.{\\numexpr} */
#  define eTeX_expr numexpr_code          /* first of \eTeX\ codes for expressions */
    attrexpr_code,                        /* not used */
    dimexpr_code,                         /* code for \.{\\dimexpr} */
    glueexpr_code,                        /* code for \.{\\glueexpr} */
    muexpr_code,                          /* code for \.{\\muexpr} */
} last_item_codes;


typedef enum {
    save_cat_code_table_code=0,
    init_cat_code_table_code,
    set_random_seed_code,
    save_pos_code,
    late_lua_code,
    expand_font_code,
} normal_codes;

typedef enum {
    lp_code_base = 2,
    rp_code_base = 3,
    ef_code_base = 4,
    tag_code     = 5,
    no_lig_code  = 6,
    gp_code_base = 7
} font_codes ;

#  define immediate_code 4      /* command modifier for \.{\\immediate} */

extern void initialize_commands(void);
extern void initialize_etex_commands(void);

#endif
