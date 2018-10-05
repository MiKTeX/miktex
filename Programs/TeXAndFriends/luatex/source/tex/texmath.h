/* texmath.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2009 Taco Hoekwater <taco@luatex.org>

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


#ifndef TEXMATH_H
#  define TEXMATH_H 1

#  define empty 0

extern pointer new_noad(void);

extern void show_math_node(halfword);
extern void flush_math(void);
extern void math_left_brace(void);
extern void finish_display_alignment(halfword, halfword, halfword);
extern halfword new_sub_box(halfword);

#  define math_reset(p) do { if (p!=null) flush_node(p); p = null; } while (0)

#  define scripts_allowed(A) ((type((A))>=simple_noad)&&(type((A))<fence_noad))

#  define default_code 010000000000     /* denotes |default_rule_thickness| */

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

extern void initialize_math(void);
extern void initialize_math_spacing(void);
extern halfword math_vcenter_group(halfword);
extern void build_choices(void);
extern void close_math_group(halfword);
extern void init_math(void);
extern void start_eq_no(void);
extern void set_math_char(mathcodeval);
extern void math_char_in_text(mathcodeval);
extern void math_math_comp(void);
extern void math_limit_switch(void);
extern void math_radical(void);
extern void math_ac(void);
extern pointer new_style(small_number);
extern void append_choices(void);
extern void sub_sup(void);
extern void no_sub_sup(void);
extern void math_fraction(void);
extern void math_left_right(void);
extern void after_math(void);

extern void scan_extdef_del_code(int level, int extcode);
extern void scan_extdef_math_code(int level, int extcode);

extern int fam_fnt(int fam_id, int size_id);
extern void def_fam_fnt(int fam_id, int size_id, int f, int lvl);
extern void dump_math_data(void);
extern void undump_math_data(void);
void unsave_math_data(int lvl);

/*
  @ We also need to compute the change in style between mlists and their
  subsidiaries. The following macros define the subsidiary style for
  an overlined nucleus (|cramped_style|), for a subscript or a superscript
  (|sub_style| or |sup_style|), or for a numerator or denominator (|num_style|
  or |denom_style|).
*/

extern const char *math_style_names[];

#  define cramped 1             /* add this to an uncramped style if you want to cramp it */

#  define cramped_style(A) 2*((A)/2)+cramped    /* cramp the style */
#  define sub_style(A) 2*((A)/4)+script_style+cramped   /* smaller and cramped */
#  define sup_style(A) 2*((A)/4)+script_style+((A)%2)   /* smaller */
#  define num_style(A) (A)+2-2*((A)/6)  /* smaller unless already script-script */
#  define denom_style(A) 2*((A)/2)+cramped+2-2*((A)/6)  /* smaller, cramped */
#  define sup_sup_style(A) sup_style(sup_style((A)))    /* smaller */

void setup_math_style(void);
void print_math_style(void);

#  define text_size 0
#  define script_size 1
#  define script_script_size 2

#  define dir_math_save cur_list.math_field
#  define m_style cur_list.math_style_field
#  define init_math_fields() do {               \
        dir_math_save=false;                    \
        m_style=-1;                             \
    } while (0)


#  define null_font 0
#  define min_quarterword 0

#  define undefined_math_parameter max_dimen

typedef enum {
    math_param_quad,
    math_param_axis,
    math_param_operator_size,
    math_param_overbar_kern,
    math_param_overbar_rule,
    math_param_overbar_vgap,
    math_param_underbar_kern,
    math_param_underbar_rule,
    math_param_underbar_vgap,
    math_param_radical_kern,
    math_param_radical_rule,
    math_param_radical_vgap,
    math_param_radical_degree_before,
    math_param_radical_degree_after,
    math_param_radical_degree_raise,
    math_param_stack_vgap,
    math_param_stack_num_up,
    math_param_stack_denom_down,
    math_param_fraction_rule,
    math_param_fraction_num_vgap,
    math_param_fraction_num_up,
    math_param_fraction_denom_vgap,
    math_param_fraction_denom_down,
    math_param_fraction_del_size,
    math_param_skewed_fraction_hgap,
    math_param_skewed_fraction_vgap,
    math_param_limit_above_vgap,
    math_param_limit_above_bgap,
    math_param_limit_above_kern,
    math_param_limit_below_vgap,
    math_param_limit_below_bgap,
    math_param_limit_below_kern,
    math_param_nolimit_sub_factor, /* bonus */
    math_param_nolimit_sup_factor, /* bonus */
    math_param_under_delimiter_vgap,
    math_param_under_delimiter_bgap,
    math_param_over_delimiter_vgap,
    math_param_over_delimiter_bgap,
    math_param_sub_shift_drop,
    math_param_sup_shift_drop,
    math_param_sub_shift_down,
    math_param_sub_sup_shift_down,
    math_param_sub_top_max,
    math_param_sup_shift_up,
    math_param_sup_bottom_min,
    math_param_sup_sub_bottom_max,
    math_param_subsup_vgap,
    math_param_space_after_script,
    math_param_connector_overlap_min,
    math_param_ord_ord_spacing,
    math_param_ord_op_spacing,
    math_param_ord_bin_spacing,
    math_param_ord_rel_spacing,
    math_param_ord_open_spacing,
    math_param_ord_close_spacing,
    math_param_ord_punct_spacing,
    math_param_ord_inner_spacing,
    math_param_op_ord_spacing,
    math_param_op_op_spacing,
    math_param_op_bin_spacing,
    math_param_op_rel_spacing,
    math_param_op_open_spacing,
    math_param_op_close_spacing,
    math_param_op_punct_spacing,
    math_param_op_inner_spacing,
    math_param_bin_ord_spacing,
    math_param_bin_op_spacing,
    math_param_bin_bin_spacing,
    math_param_bin_rel_spacing,
    math_param_bin_open_spacing,
    math_param_bin_close_spacing,
    math_param_bin_punct_spacing,
    math_param_bin_inner_spacing,
    math_param_rel_ord_spacing,
    math_param_rel_op_spacing,
    math_param_rel_bin_spacing,
    math_param_rel_rel_spacing,
    math_param_rel_open_spacing,
    math_param_rel_close_spacing,
    math_param_rel_punct_spacing,
    math_param_rel_inner_spacing,
    math_param_open_ord_spacing,
    math_param_open_op_spacing,
    math_param_open_bin_spacing,
    math_param_open_rel_spacing,
    math_param_open_open_spacing,
    math_param_open_close_spacing,
    math_param_open_punct_spacing,
    math_param_open_inner_spacing,
    math_param_close_ord_spacing,
    math_param_close_op_spacing,
    math_param_close_bin_spacing,
    math_param_close_rel_spacing,
    math_param_close_open_spacing,
    math_param_close_close_spacing,
    math_param_close_punct_spacing,
    math_param_close_inner_spacing,
    math_param_punct_ord_spacing,
    math_param_punct_op_spacing,
    math_param_punct_bin_spacing,
    math_param_punct_rel_spacing,
    math_param_punct_open_spacing,
    math_param_punct_close_spacing,
    math_param_punct_punct_spacing,
    math_param_punct_inner_spacing,
    math_param_inner_ord_spacing,
    math_param_inner_op_spacing,
    math_param_inner_bin_spacing,
    math_param_inner_rel_spacing,
    math_param_inner_open_spacing,
    math_param_inner_close_spacing,
    math_param_inner_punct_spacing,
    math_param_inner_inner_spacing,
    math_param_last
} math_parameters;

extern const char *math_param_names[];

#  define math_param_first_mu_glue math_param_ord_ord_spacing

extern void def_math_param(int param_code, int style_code, scaled value,
                           int lvl);
extern scaled get_math_param(int param_code, int style_code);


typedef enum {
    ScriptPercentScaleDown = 1,
    ScriptScriptPercentScaleDown,
    DelimitedSubFormulaMinHeight,
    DisplayOperatorMinHeight,
    MathLeading,
    AxisHeight,
    AccentBaseHeight,
    FlattenedAccentBaseHeight,
    SubscriptShiftDown,
    SubscriptTopMax,
    SubscriptBaselineDropMin,
    SuperscriptShiftUp,
    SuperscriptShiftUpCramped,
    SuperscriptBottomMin,
    SuperscriptBaselineDropMax,
    SubSuperscriptGapMin,
    SuperscriptBottomMaxWithSubscript,
    SpaceAfterScript,
    UpperLimitGapMin,
    UpperLimitBaselineRiseMin,
    LowerLimitGapMin,
    LowerLimitBaselineDropMin,
    StackTopShiftUp,
    StackTopDisplayStyleShiftUp,
    StackBottomShiftDown,
    StackBottomDisplayStyleShiftDown,
    StackGapMin,
    StackDisplayStyleGapMin,
    StretchStackTopShiftUp,
    StretchStackBottomShiftDown,
    StretchStackGapAboveMin,
    StretchStackGapBelowMin,
    FractionNumeratorShiftUp,
    FractionNumeratorDisplayStyleShiftUp,
    FractionDenominatorShiftDown,
    FractionDenominatorDisplayStyleShiftDown,
    FractionNumeratorGapMin,
    FractionNumeratorDisplayStyleGapMin,
    FractionRuleThickness,
    FractionDenominatorGapMin,
    FractionDenominatorDisplayStyleGapMin,
    SkewedFractionHorizontalGap,
    SkewedFractionVerticalGap,
    OverbarVerticalGap,
    OverbarRuleThickness,
    OverbarExtraAscender,
    UnderbarVerticalGap,
    UnderbarRuleThickness,
    UnderbarExtraDescender,
    RadicalVerticalGap,
    RadicalDisplayStyleVerticalGap,
    RadicalRuleThickness,
    RadicalExtraAscender,
    RadicalKernBeforeDegree,
    RadicalKernAfterDegree,
    RadicalDegreeBottomRaisePercent,
    MinConnectorOverlap,
    SubscriptShiftDownWithSuperscript,
    FractionDelimiterSize,
    FractionDelimiterDisplayStyleSize,
    NoLimitSubFactor,
    NoLimitSupFactor,
    MATH_param_last,
} MATH_param_codes;

#  define MATH_param_max MATH_param_last

extern const char *MATH_param_names[];

#endif
