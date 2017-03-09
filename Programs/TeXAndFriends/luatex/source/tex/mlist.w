% mlist.w
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

% (HH / 0.82+):

@ In traditional \TeX\ the italic correction is added to the width of the glyph. This
is part of the engine design and related font design. In opentype math this is
different. There the italic correction had more explicit usage. The 1.7 spec
says:

italic correction:

  When a run of slanted characters is followed by a straight character (such as
  an operator or a delimiter), the italics correction of the last glyph is added
  to its advance width.

  When positioning limits on an N-ary operator (e.g., integral sign), the horizontal
  position of the upper limit is moved to the right by ½ of the italics correction,
  while the position of the lower limit is moved to the left by the same distance.

  When positioning superscripts and subscripts, their default horizontal positions are
  also different by the amount of the italics correction of the preceding glyph.

math kerning:

  Set the default horizontal position for the superscript as shifted relative to the
  position of the subscript by the italics correction of the base glyph.

Before this was specified we had to gamble a bit and assume that cambria was the font
benchmark and trust our eyes (and msword) for the logic. I must admit that I have been
fighting these italics in fonts (and the heuristics that Lua\TeX\ provided) right from
the start (e.g. using Lua based postprocessing) but by now we know more and have more
fonts to test with. More fonts are handy because not all fonts are alike when it comes
to italics. Axis are another area of concern, as it looks like opentype math fonts often
already apply that shift.

@ @c
#define is_new_mathfont(A)   ((font_math_params(A) >0) && (math_old_par == 0))
#define is_old_mathfont(A,B) ((font_math_params(A)==0) && (font_params(A)>=(B)))
#define do_new_math(A)       ((font_math_params(A) >0) && (font_oldmath(A) == 0) && (math_old_par == 0))

@
\def\LuaTeX{Lua\TeX}

@ @c

#include "ptexlib.h"
#include "lua/luatex-api.h"

@ @c
#define nDEBUG

#define reset_attributes(p,newatt) do { \
    delete_attribute_ref(node_attr(p)); \
    node_attr(p) = newatt;              \
    if (newatt!=null) {                 \
      add_node_attr_ref(node_attr(p));  \
    }                                   \
  } while (0)

#define DEFINE_MATH_PARAMETERS(A,B,C,D) do {                 \
    if (B==text_size) {                                      \
      def_math_param(A, text_style, (C),D);                  \
      def_math_param(A, cramped_text_style, (C),D);          \
    } else if (B==script_size) {                             \
      def_math_param(A, script_style, (C),D);                \
      def_math_param(A, cramped_script_style, (C),D);        \
    } else if (B==script_script_size) {                      \
      def_math_param(A, script_script_style, (C),D);         \
      def_math_param(A, cramped_script_script_style, (C),D); \
    }                                                        \
  } while (0)

#define DEFINE_DMATH_PARAMETERS(A,B,C,D) do {         \
    if (B==text_size) {                               \
      def_math_param(A, display_style,(C),D);         \
      def_math_param(A, cramped_display_style,(C),D); \
    }                                                 \
  } while (0)

#define font_MATH_par(a,b) \
  (font_math_params(a)>=b ? font_math_param(a,b) : undefined_math_parameter)

@ here are the math parameters that are font-dependant

@ Before an mlist is converted to an hlist, \TeX\ makes sure that
the fonts in family~2 have enough parameters to be math-symbol
fonts, and that the fonts in family~3 have enough parameters to be
math-extension fonts. The math-symbol parameters are referred to by using the
following macros, which take a size code as their parameter; for example,
|num1(cur_size)| gives the value of the |num1| parameter for the current size.
@^parameters for symbols@>
@^font parameters@>

@c
#define total_mathsy_params 22
#define total_mathex_params 13

#define mathsy(A,B) font_param(fam_fnt(2,A),B)

#define math_x_height(A) mathsy(A,5) /* height of `\.x' */
#define math_quad(A) mathsy(A,6)     /* \.{18mu} */
#define num1(A) mathsy(A,8)          /* numerator shift-up in display styles */
#define num2(A) mathsy(A,9)          /* numerator shift-up in non-display, non-\.{\\atop} */
#define num3(A) mathsy(A,10)         /* numerator shift-up in non-display \.{\\atop} */
#define denom1(A) mathsy(A,11)       /* denominator shift-down in display styles */
#define denom2(A) mathsy(A,12)       /* denominator shift-down in non-display styles */
#define sup1(A) mathsy(A,13)         /* superscript shift-up in uncramped display style */
#define sup2(A) mathsy(A,14)         /* superscript shift-up in uncramped non-display */
#define sup3(A) mathsy(A,15)         /* superscript shift-up in cramped styles */
#define sub1(A) mathsy(A,16)         /* subscript shift-down if superscript is absent */
#define sub2(A) mathsy(A,17)         /* subscript shift-down if superscript is present */
#define sup_drop(A) mathsy(A,18)     /* superscript baseline below top of large box */
#define sub_drop(A) mathsy(A,19)     /* subscript baseline below bottom of large box */
#define delim1(A) mathsy(A,20)       /* size of \.{\\atopwithdelims} delimiters in display styles */
#define delim2(A) mathsy(A,21)       /* size of \.{\\atopwithdelims} delimiters in non-displays */
#define axis_height(A) mathsy(A,22)  /* height of fraction lines above the baseline */

@ The math-extension parameters have similar macros, but the size code is
omitted (since it is always |cur_size| when we refer to such parameters).
@^parameters for symbols@>
@^font parameters@>

@c
#define mathex(A,B) font_param(fam_fnt(3,A),B)
#define default_rule_thickness(A) mathex(A,8)   /* thickness of \.{\\over} bars */
#define big_op_spacing1(A) mathex(A,9)  /* minimum clearance above a displayed op */
#define big_op_spacing2(A) mathex(A,10) /* minimum clearance below a displayed op */
#define big_op_spacing3(A) mathex(A,11) /* minimum baselineskip above displayed op */
#define big_op_spacing4(A) mathex(A,12) /* minimum baselineskip below displayed op */
#define big_op_spacing5(A) mathex(A,13) /* padding above and below displayed limits */

@ I (TH) made a bunch of extensions cf. the MATH table in OpenType, but some of
the MathConstants values have no matching usage in \LuaTeX\ right now.

ScriptPercentScaleDown,
ScriptScriptPercentScaleDown:
  These should be handled by the macro package, on the engine
  side there are three separate fonts

DelimitedSubFormulaMinHeight:
  This is perhaps related to word's natural math input? I have
  no idea what to do about it

MathLeading:
  LuaTeX does not currently handle multi-line displays, and
  the parameter does not seem to make much sense elsewhere

FlattenedAccentBaseHeight:
  This is based on the 'flac' GSUB feature. It would not be hard
  to support that, but proper math accent placements cf. MATH
  needs support for MathTopAccentAttachment table to be
  implemented first

Also still TODO for OpenType Math:
  * prescripts

@ this is not really a math parameter at all

@c
static void math_param_error(const char *param, int style)
{
    char s[256];
    const char *hlp[] = {
        "Sorry, but I can't typeset math unless various parameters have",
        "been set. This is normally done by loading special math fonts",
        "into the math family slots. Your font set is lacking at least",
        "the parameter mentioned earlier.",
        NULL
    };
    snprintf(s, 256, "Math error: parameter \\Umath%s\\%sstyle is not set",
             param, math_style_names[style]);
    tex_error(s, hlp);
#if 0
    flush_math();
#endif
    return;
}

@ @c
static scaled accent_base_height(int f)
{
    scaled a;
    if (do_new_math(f)) {
        a = font_MATH_par(f, AccentBaseHeight);
        if (a == undefined_math_parameter)
            a = x_height(f);
    } else {
        a = x_height(f);
    }
    return a;
}

@ The non-staticness of this function is for the benefit of |texmath.w|. Watch out,
this one uses the style! The style and size numbers don't match because we have
cramped styles.

@c
scaled get_math_quad_style(int var)
{
    scaled a = get_math_param(math_param_quad, var);
    if (a == undefined_math_parameter) {
        math_param_error("quad", var);
        return 0;
    } else {
        return a;
    }
}

@ For this reason the next one is different because it is called with a size
specifier instead of a style specifier.

@c
static scaled math_axis_size(int b)
{
    scaled a;
    int var;
    if (b == script_size)
        var = script_style;
    else if (b == script_script_size)
        var = script_script_style;
    else
        var = text_style;
    a = get_math_param(math_param_axis, var);
    if (a == undefined_math_parameter) {
        math_param_error("axis", var);
        return 0;
    } else {
        return a;
    }
}

@ @c
scaled get_math_quad_size(int b)
{
    int var;
    if (b == script_size)
        var = script_style;
    else if (b == script_script_size)
        var = script_script_style;
    else
        var = text_style;
    return get_math_param(math_param_quad, var);
}

@ @c
static scaled minimum_operator_size(int var)
{
    scaled a = get_math_param(math_param_operator_size, var);
    return a;
}

@ Old-style fonts do not define the |radical_rule|. This allows |make_radical| to select
the backward compatibility code, and it means that we can't raise an error here.

@c
static scaled radical_rule_par(int var)
{
    scaled a = get_math_param(math_param_radical_rule, var);
    return a;
}

@ now follow all the trivial math parameters

@c
#define get_math_param_or_error(a,b) do_get_math_param_or_error(a, math_param_##b, #b)
#define get_math_param_or_zero(a,b) do_get_math_param_or_zero(a, math_param_##b, #b)

static scaled do_get_math_param_or_error(int var, int param, const char *name)
{
    scaled a = get_math_param(param, var);
    if (a == undefined_math_parameter) {
        math_param_error(name, var);
        a = 0;
    }
    return a;
}

static scaled do_get_math_param_or_zero(int var, int param, const char *name)
{
    scaled a = get_math_param(param, var);
    if (a == undefined_math_parameter) {
        a = 0;
    }
    return a;
}

@ A variant on a suggestion on the list based on analysis by UV.

@c
static scaled get_delimiter_height(scaled max_d, scaled max_h, boolean axis) {
    scaled delta, delta1, delta2;
    if (axis) {
        delta2 = max_d + math_axis_size(cur_size);
    } else {
        delta2 = max_d;
    }
    delta1 = max_h + max_d - delta2;
    if (delta2 > delta1) {
        /* |delta1| is max distance from axis */
        delta1 = delta2;
    }
    delta = (delta1 / 500) * delimiter_factor_par;
    delta2 = delta1 + delta1 - delimiter_shortfall_par;
    if (delta < delta2) {
        return delta2;
    } else {
        return delta;
    }
}

@ @c
#define radical_degree_before(a) get_math_param_or_error(a, radical_degree_before)
#define radical_degree_after(a)  get_math_param_or_error(a, radical_degree_after)
#define radical_degree_raise(a)  get_math_param_or_error(a, radical_degree_raise)

#define connector_overlap_min(a) get_math_param_or_error(a, connector_overlap_min)

#define overbar_rule(a)          get_math_param_or_error(a, overbar_rule)
#define overbar_kern(a)          get_math_param_or_error(a, overbar_kern)
#define overbar_vgap(a)          get_math_param_or_error(a, overbar_vgap)

#define underbar_rule(a)         get_math_param_or_error(a, underbar_rule)
#define underbar_kern(a)         get_math_param_or_error(a, underbar_kern)
#define underbar_vgap(a)         get_math_param_or_error(a, underbar_vgap)

#define under_delimiter_vgap(a)  get_math_param_or_error(a, under_delimiter_vgap)
#define under_delimiter_bgap(a)  get_math_param_or_error(a, under_delimiter_bgap)

#define over_delimiter_vgap(a)   get_math_param_or_error(a, over_delimiter_vgap)
#define over_delimiter_bgap(a)   get_math_param_or_error(a, over_delimiter_bgap)

#define radical_vgap(a)          get_math_param_or_error(a, radical_vgap)
#define radical_kern(a)          get_math_param_or_error(a, radical_kern)

#define stack_vgap(a)            get_math_param_or_error(a, stack_vgap)
#define stack_num_up(a)          get_math_param_or_error(a, stack_num_up)
#define stack_denom_down(a)      get_math_param_or_error(a, stack_denom_down)

#define fraction_rule(a)         get_math_param_or_error(a, fraction_rule)
#define fraction_num_vgap(a)     get_math_param_or_error(a, fraction_num_vgap)
#define fraction_denom_vgap(a)   get_math_param_or_error(a, fraction_denom_vgap)
#define fraction_num_up(a)       get_math_param_or_error(a, fraction_num_up)
#define fraction_denom_down(a)   get_math_param_or_error(a, fraction_denom_down)
#define fraction_del_size_new(a) get_math_param_or_error(a, fraction_del_size)
/*
#define fraction_del_size_old(a) get_math_param(a, math_param_fraction_del_size)
*/
#define fraction_del_size_old(a) get_math_param_or_error(a, fraction_del_size)

#define skewed_fraction_hgap(a)  get_math_param_or_error(a, skewed_fraction_hgap)
#define skewed_fraction_vgap(a)  get_math_param_or_error(a, skewed_fraction_vgap)

#define limit_above_vgap(a)      get_math_param_or_error(a, limit_above_vgap)
#define limit_above_bgap(a)      get_math_param_or_error(a, limit_above_bgap)
#define limit_above_kern(a)      get_math_param_or_error(a, limit_above_kern)

#define limit_below_vgap(a)      get_math_param_or_error(a, limit_below_vgap)
#define limit_below_bgap(a)      get_math_param_or_error(a, limit_below_bgap)
#define limit_below_kern(a)      get_math_param_or_error(a, limit_below_kern)

#define nolimit_sub_factor(a)    get_math_param_or_zero(a, nolimit_sub_factor)
#define nolimit_sup_factor(a)    get_math_param_or_zero(a, nolimit_sup_factor)

#define sub_shift_drop(a)        get_math_param_or_error(a, sub_shift_drop)
#define sup_shift_drop(a)        get_math_param_or_error(a, sup_shift_drop)
#define sub_shift_down(a)        get_math_param_or_error(a, sub_shift_down)
#define sub_sup_shift_down(a)    get_math_param_or_error(a, sub_sup_shift_down)
#define sup_shift_up(a)          get_math_param_or_error(a, sup_shift_up)
#define sub_top_max(a)           get_math_param_or_error(a, sub_top_max)
#define sup_bottom_min(a)        get_math_param_or_error(a, sup_bottom_min)
#define sup_sub_bottom_max(a)    get_math_param_or_error(a, sup_sub_bottom_max)
#define subsup_vgap(a)           get_math_param_or_error(a, subsup_vgap)

#define space_after_script(a)    get_math_param_or_error(a, space_after_script)

@ @c
void fixup_math_parameters(int fam_id, int size_id, int f, int lvl)
{

    if (is_new_mathfont(f)) {   /* fix all known parameters */

        DEFINE_MATH_PARAMETERS(math_param_quad, size_id,
            font_size(f), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_quad, size_id,
            font_size(f), lvl);
        DEFINE_MATH_PARAMETERS(math_param_axis, size_id,
            font_MATH_par(f, AxisHeight), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_axis, size_id,
            font_MATH_par(f, AxisHeight), lvl);
        DEFINE_MATH_PARAMETERS(math_param_overbar_kern, size_id,
            font_MATH_par(f, OverbarExtraAscender), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_kern, size_id,
            font_MATH_par(f, OverbarExtraAscender), lvl);
        DEFINE_MATH_PARAMETERS(math_param_overbar_rule, size_id,
            font_MATH_par(f, OverbarRuleThickness), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_rule, size_id,
            font_MATH_par(f, OverbarRuleThickness), lvl);
        DEFINE_MATH_PARAMETERS(math_param_overbar_vgap, size_id,
            font_MATH_par(f, OverbarVerticalGap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_vgap, size_id,
            font_MATH_par(f, OverbarVerticalGap), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_kern, size_id,
            font_MATH_par(f, UnderbarExtraDescender), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_kern, size_id,
            font_MATH_par(f, UnderbarExtraDescender), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_rule, size_id,
            font_MATH_par(f, UnderbarRuleThickness), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_rule, size_id,
            font_MATH_par(f, UnderbarRuleThickness), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_vgap, size_id,
           font_MATH_par(f, UnderbarVerticalGap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_vgap, size_id,
            font_MATH_par(f, UnderbarVerticalGap), lvl);

        DEFINE_MATH_PARAMETERS(math_param_under_delimiter_vgap, size_id,
            font_MATH_par(f, StretchStackGapAboveMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_under_delimiter_vgap, size_id,
            font_MATH_par(f, StretchStackGapAboveMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_under_delimiter_bgap, size_id,
            font_MATH_par(f, StretchStackBottomShiftDown), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_under_delimiter_bgap, size_id,
            font_MATH_par(f, StretchStackBottomShiftDown), lvl);

        DEFINE_MATH_PARAMETERS(math_param_over_delimiter_vgap, size_id,
            font_MATH_par(f, StretchStackGapBelowMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_over_delimiter_vgap, size_id,
            font_MATH_par(f, StretchStackGapBelowMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_over_delimiter_bgap, size_id,
            font_MATH_par(f, StretchStackTopShiftUp), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_over_delimiter_bgap, size_id,
            font_MATH_par(f, StretchStackTopShiftUp), lvl);

        DEFINE_MATH_PARAMETERS(math_param_stack_num_up, size_id,
           font_MATH_par(f, StackTopShiftUp), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_num_up, size_id,
            font_MATH_par(f, StackTopDisplayStyleShiftUp), lvl);
        DEFINE_MATH_PARAMETERS(math_param_stack_denom_down, size_id,
            font_MATH_par(f, StackBottomShiftDown), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_denom_down, size_id,
            font_MATH_par(f, StackBottomDisplayStyleShiftDown), lvl);
        DEFINE_MATH_PARAMETERS(math_param_stack_vgap, size_id,
            font_MATH_par(f, StackGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_vgap, size_id,
            font_MATH_par(f, StackDisplayStyleGapMin), lvl);

        DEFINE_MATH_PARAMETERS(math_param_radical_kern, size_id,
            font_MATH_par(f, RadicalExtraAscender), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_kern, size_id,
            font_MATH_par(f, RadicalExtraAscender), lvl);

        DEFINE_DMATH_PARAMETERS(math_param_operator_size, size_id,
            font_MATH_par(f, DisplayOperatorMinHeight), lvl);

        DEFINE_MATH_PARAMETERS(math_param_radical_rule, size_id,
            font_MATH_par(f, RadicalRuleThickness), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_rule, size_id,
            font_MATH_par(f, RadicalRuleThickness), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_vgap, size_id,
            font_MATH_par(f, RadicalVerticalGap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_vgap, size_id,
            font_MATH_par(f, RadicalDisplayStyleVerticalGap), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_before, size_id,
            font_MATH_par(f, RadicalKernBeforeDegree), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_before, size_id,
            font_MATH_par(f, RadicalKernBeforeDegree), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_after, size_id,
            font_MATH_par(f, RadicalKernAfterDegree), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_after, size_id,
            font_MATH_par(f, RadicalKernAfterDegree), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_raise, size_id,
            font_MATH_par(f, RadicalDegreeBottomRaisePercent), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_raise, size_id,
            font_MATH_par(f, RadicalDegreeBottomRaisePercent), lvl);

        if (size_id == text_size) {
            def_math_param(math_param_sup_shift_up, display_style,
                font_MATH_par(f, SuperscriptShiftUp), lvl);
            def_math_param(math_param_sup_shift_up, cramped_display_style,
                font_MATH_par(f, SuperscriptShiftUpCramped), lvl);
            def_math_param(math_param_sup_shift_up, text_style,
                font_MATH_par(f, SuperscriptShiftUp), lvl);
            def_math_param(math_param_sup_shift_up, cramped_text_style,
                font_MATH_par(f, SuperscriptShiftUpCramped), lvl);
        } else if (size_id == script_size) {
            def_math_param(math_param_sup_shift_up, script_style,
                font_MATH_par(f, SuperscriptShiftUp), lvl);
            def_math_param(math_param_sup_shift_up, cramped_script_style,
                font_MATH_par(f, SuperscriptShiftUpCramped), lvl);
        } else if (size_id == script_script_size) {
            def_math_param(math_param_sup_shift_up, script_script_style,
                font_MATH_par(f, SuperscriptShiftUp), lvl);
            def_math_param(math_param_sup_shift_up, cramped_script_script_style,
                font_MATH_par(f, SuperscriptShiftUpCramped), lvl);
        }

        DEFINE_MATH_PARAMETERS(math_param_sub_shift_drop, size_id,
            font_MATH_par(f, SubscriptBaselineDropMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_shift_drop, size_id,
            font_MATH_par(f, SubscriptBaselineDropMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sup_shift_drop, size_id,
            font_MATH_par(f, SuperscriptBaselineDropMax), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sup_shift_drop, size_id,
            font_MATH_par(f, SuperscriptBaselineDropMax), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sub_shift_down, size_id,
            font_MATH_par(f, SubscriptShiftDown), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_shift_down, size_id,
            font_MATH_par(f, SubscriptShiftDown), lvl);

        if (font_MATH_par(f, SubscriptShiftDownWithSuperscript) != undefined_math_parameter) {
            DEFINE_MATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
                font_MATH_par(f, SubscriptShiftDownWithSuperscript), lvl);
            DEFINE_DMATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
                font_MATH_par(f, SubscriptShiftDownWithSuperscript), lvl);
        } else {
            DEFINE_MATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
                font_MATH_par(f, SubscriptShiftDown), lvl);
            DEFINE_DMATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
                font_MATH_par(f, SubscriptShiftDown), lvl);
        }

        DEFINE_MATH_PARAMETERS(math_param_sub_top_max, size_id,
            font_MATH_par(f, SubscriptTopMax), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_top_max, size_id,
            font_MATH_par(f, SubscriptTopMax), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sup_bottom_min, size_id,
            font_MATH_par(f, SuperscriptBottomMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sup_bottom_min, size_id,
            font_MATH_par(f, SuperscriptBottomMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sup_sub_bottom_max, size_id,
            font_MATH_par(f, SuperscriptBottomMaxWithSubscript), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sup_sub_bottom_max, size_id,
            font_MATH_par(f, SuperscriptBottomMaxWithSubscript), lvl);
        DEFINE_MATH_PARAMETERS(math_param_subsup_vgap, size_id,
            font_MATH_par(f, SubSuperscriptGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_subsup_vgap, size_id,
            font_MATH_par(f, SubSuperscriptGapMin), lvl);

        DEFINE_MATH_PARAMETERS(math_param_limit_above_vgap, size_id,
            font_MATH_par(f, UpperLimitGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_vgap, size_id,
            font_MATH_par(f, UpperLimitGapMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_above_bgap, size_id,
            font_MATH_par(f, UpperLimitBaselineRiseMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_bgap, size_id,
            font_MATH_par(f, UpperLimitBaselineRiseMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_above_kern, size_id,
            0, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_kern, size_id,
            0, lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_vgap, size_id,
            font_MATH_par(f, LowerLimitGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_vgap, size_id,
            font_MATH_par(f, LowerLimitGapMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_bgap, size_id,
            font_MATH_par(f, LowerLimitBaselineDropMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_bgap, size_id,
            font_MATH_par(f, LowerLimitBaselineDropMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_kern, size_id,
            0, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_kern, size_id,
            0, lvl);
        DEFINE_MATH_PARAMETERS(math_param_nolimit_sub_factor, size_id,
            font_MATH_par(f, NoLimitSubFactor), lvl); /* bonus */
        DEFINE_DMATH_PARAMETERS(math_param_nolimit_sub_factor, size_id,
            font_MATH_par(f, NoLimitSubFactor), lvl); /* bonus */
        DEFINE_MATH_PARAMETERS(math_param_nolimit_sup_factor, size_id,
            font_MATH_par(f, NoLimitSupFactor), lvl); /* bonus */
        DEFINE_DMATH_PARAMETERS(math_param_nolimit_sup_factor, size_id,
            font_MATH_par(f, NoLimitSupFactor), lvl); /* bonus */

        DEFINE_MATH_PARAMETERS(math_param_fraction_rule, size_id,
            font_MATH_par(f, FractionRuleThickness), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_rule, size_id,
            font_MATH_par(f, FractionRuleThickness), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_num_vgap, size_id,
            font_MATH_par(f, FractionNumeratorGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_num_vgap, size_id,
            font_MATH_par(f, FractionNumeratorDisplayStyleGapMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_num_up, size_id,
            font_MATH_par(f, FractionNumeratorShiftUp), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_num_up, size_id,
            font_MATH_par(f, FractionNumeratorDisplayStyleShiftUp), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_denom_vgap, size_id,
            font_MATH_par(f, FractionDenominatorGapMin), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_denom_vgap, size_id,
            font_MATH_par(f,FractionDenominatorDisplayStyleGapMin), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_denom_down, size_id,
            font_MATH_par(f, FractionDenominatorShiftDown), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_denom_down, size_id,
            font_MATH_par(f, FractionDenominatorDisplayStyleShiftDown), lvl);

        DEFINE_MATH_PARAMETERS(math_param_fraction_del_size, size_id,
            font_MATH_par(f, FractionDelimiterSize), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_del_size, size_id,
            font_MATH_par(f, FractionDelimiterDisplayStyleSize), lvl);

        DEFINE_MATH_PARAMETERS(math_param_skewed_fraction_hgap, size_id,
            font_MATH_par(f, SkewedFractionHorizontalGap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_skewed_fraction_hgap, size_id,
            font_MATH_par(f, SkewedFractionHorizontalGap), lvl);
        DEFINE_MATH_PARAMETERS(math_param_skewed_fraction_vgap, size_id,
            font_MATH_par(f, SkewedFractionVerticalGap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_skewed_fraction_vgap, size_id,
            font_MATH_par(f, SkewedFractionVerticalGap), lvl);

        DEFINE_MATH_PARAMETERS(math_param_space_after_script, size_id,
            font_MATH_par(f, SpaceAfterScript), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_space_after_script, size_id,
            font_MATH_par(f, SpaceAfterScript), lvl);

        DEFINE_MATH_PARAMETERS(math_param_connector_overlap_min, size_id,
            font_MATH_par(f, MinConnectorOverlap), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_connector_overlap_min, size_id,
            font_MATH_par(f, MinConnectorOverlap), lvl);

    } else if (fam_id == 2 && is_old_mathfont(f, total_mathsy_params)) {

        /* fix old-style |sy| parameters */

        DEFINE_MATH_PARAMETERS(math_param_quad, size_id,
            math_quad(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_quad, size_id,
            math_quad(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_axis, size_id,
            axis_height(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_axis, size_id,
            axis_height(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_stack_num_up, size_id,
            num3(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_num_up, size_id,
            num1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_stack_denom_down, size_id,
            denom2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_denom_down, size_id,
            denom1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_num_up, size_id,
            num2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_num_up, size_id,
            num1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_denom_down, size_id,
            denom2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_denom_down, size_id,
            denom1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_del_size, size_id,
            delim2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_del_size, size_id,
            delim1(size_id), lvl);

        DEFINE_MATH_PARAMETERS(math_param_skewed_fraction_hgap, size_id,
            0, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_skewed_fraction_hgap, size_id,
            0, lvl);
        DEFINE_MATH_PARAMETERS(math_param_skewed_fraction_vgap, size_id,
            0, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_skewed_fraction_vgap, size_id,
            0, lvl);

        if (size_id == text_size) {
            def_math_param(math_param_sup_shift_up, display_style,
                sup1(size_id), lvl);
            def_math_param(math_param_sup_shift_up, cramped_display_style,
                sup3(size_id), lvl);
            def_math_param(math_param_sup_shift_up, text_style,
                sup2(size_id), lvl);
            def_math_param(math_param_sup_shift_up, cramped_text_style,
               sup3(size_id), lvl);
        } else if (size_id == script_size) {
            def_math_param(math_param_sub_shift_drop, display_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop, cramped_display_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop, text_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop, cramped_text_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, display_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, cramped_display_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, text_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, cramped_text_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_up, script_style,
                sup2(size_id), lvl);
            def_math_param(math_param_sup_shift_up, cramped_script_style,
                sup3(size_id), lvl);
        } else if (size_id == script_script_size) {
            def_math_param(math_param_sub_shift_drop, script_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop, cramped_script_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop, script_script_style,
                sub_drop(size_id), lvl);
            def_math_param(math_param_sub_shift_drop,
                cramped_script_script_style, sub_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, script_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, cramped_script_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop, script_script_style,
                sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_drop,
                cramped_script_script_style, sup_drop(size_id), lvl);
            def_math_param(math_param_sup_shift_up, script_script_style,
                sup2(size_id), lvl);
            def_math_param(math_param_sup_shift_up, cramped_script_script_style,
                sup3(size_id), lvl);
        }

        DEFINE_MATH_PARAMETERS(math_param_sub_shift_down, size_id,
            sub1(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_shift_down, size_id,
            sub1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
            sub2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_sup_shift_down, size_id,
            sub2(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sub_top_max, size_id,
            (abs(math_x_height(size_id) * 4) / 5), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sub_top_max, size_id,
            (abs(math_x_height(size_id) * 4) / 5), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sup_bottom_min, size_id,
            (abs(math_x_height(size_id)) / 4), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sup_bottom_min, size_id,
            (abs(math_x_height(size_id)) / 4), lvl);
        DEFINE_MATH_PARAMETERS(math_param_sup_sub_bottom_max, size_id,
            (abs(math_x_height(size_id) * 4) / 5), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_sup_sub_bottom_max, size_id,
            (abs(math_x_height(size_id) * 4) / 5), lvl);

        /*
            The display-size |radical_vgap| is done twice because it needs
            values from both the sy and the ex font.
        */

        DEFINE_DMATH_PARAMETERS(math_param_radical_vgap, size_id,
            (default_rule_thickness(size_id) + (abs(math_x_height(size_id)) / 4)), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_raise, size_id,
            60, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_raise, size_id,
            60, lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_before, size_id,
            xn_over_d(get_math_quad_size(size_id), 5, 18), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_before, size_id,
            xn_over_d(get_math_quad_size(size_id), 5, 18), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_degree_after, size_id,
            (-xn_over_d (get_math_quad_size(size_id), 10, 18)), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_degree_after, size_id,
            (-xn_over_d (get_math_quad_size(size_id), 10, 18)), lvl);

    } else if (fam_id == 3 && is_old_mathfont(f, total_mathex_params)) {

        /* fix old-style |ex| parameters */

        DEFINE_MATH_PARAMETERS(math_param_overbar_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_overbar_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_overbar_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_overbar_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_underbar_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_underbar_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_radical_kern, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_radical_vgap, size_id,
           (default_rule_thickness(size_id) + (abs(default_rule_thickness(size_id)) / 4)), lvl);
        DEFINE_MATH_PARAMETERS(math_param_stack_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_stack_vgap, size_id,
            7 * default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_rule, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_num_vgap, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_num_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_fraction_denom_vgap, size_id,
            default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_fraction_denom_vgap, size_id,
            3 * default_rule_thickness(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_above_vgap, size_id,
            big_op_spacing1(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_vgap, size_id,
            big_op_spacing1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_above_bgap, size_id,
            big_op_spacing3(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_bgap, size_id,
            big_op_spacing3(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_above_kern, size_id,
            big_op_spacing5(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_above_kern, size_id,
            big_op_spacing5(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_vgap, size_id,
            big_op_spacing2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_vgap, size_id,
            big_op_spacing2(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_bgap, size_id,
            big_op_spacing4(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_bgap, size_id,
            big_op_spacing4(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_limit_below_kern, size_id,
           big_op_spacing5(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_limit_below_kern, size_id,
            big_op_spacing5(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_nolimit_sub_factor, size_id,
            font_MATH_par(f, NoLimitSubFactor), lvl); /* bonus */
        DEFINE_DMATH_PARAMETERS(math_param_nolimit_sub_factor, size_id,
            font_MATH_par(f, NoLimitSubFactor), lvl); /* bonus */
        DEFINE_MATH_PARAMETERS(math_param_nolimit_sup_factor, size_id,
            font_MATH_par(f, NoLimitSupFactor), lvl); /* bonus */
        DEFINE_DMATH_PARAMETERS(math_param_nolimit_sup_factor, size_id,
            font_MATH_par(f, NoLimitSupFactor), lvl); /* bonus */
        DEFINE_MATH_PARAMETERS(math_param_subsup_vgap, size_id,
            4 * default_rule_thickness(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_subsup_vgap, size_id,
            4 * default_rule_thickness(size_id), lvl);

        /*
            All of the |space_after_script|s are done in |finalize_math_parameters|
            because the \.{\\scriptspace} may have been altered by the user
        */

        DEFINE_MATH_PARAMETERS(math_param_connector_overlap_min, size_id,
            0, lvl);
        DEFINE_DMATH_PARAMETERS(math_param_connector_overlap_min, size_id,
            0, lvl);

        DEFINE_MATH_PARAMETERS(math_param_under_delimiter_vgap, size_id,
            big_op_spacing2(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_under_delimiter_vgap, size_id,
            big_op_spacing2(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_under_delimiter_bgap, size_id,
            big_op_spacing4(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_under_delimiter_bgap, size_id,
            big_op_spacing4(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_over_delimiter_vgap, size_id,
            big_op_spacing1(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_over_delimiter_vgap, size_id,
            big_op_spacing1(size_id), lvl);
        DEFINE_MATH_PARAMETERS(math_param_over_delimiter_bgap, size_id,
            big_op_spacing3(size_id), lvl);
        DEFINE_DMATH_PARAMETERS(math_param_over_delimiter_bgap, size_id,
            big_op_spacing3(size_id), lvl);

        /*
            The display-size |radical_vgap| is done twice because it needs
            values from both the sy and the ex font.
        */

        DEFINE_DMATH_PARAMETERS(math_param_radical_vgap, size_id,
            (default_rule_thickness(size_id) + (abs(math_x_height(size_id)) / 4)), lvl);

    }
}

@ This needs to be called just at the start of |mlist_to_hlist|, for
backward compatibility with \.{\\scriptspace}.

@c
static void finalize_math_parameters(void)
{
    int saved_trace = tracing_assigns_par;
    tracing_assigns_par = 0;
    if (get_math_param(math_param_space_after_script, display_style) == undefined_math_parameter) {
        def_math_param(math_param_space_after_script, display_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, text_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, script_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, script_script_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, cramped_display_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, cramped_text_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, cramped_script_style,
            script_space_par, level_one);
        def_math_param(math_param_space_after_script, cramped_script_script_style,
            script_space_par, level_one);
    }
    tracing_assigns_par = saved_trace;
}

@ In order to convert mlists to hlists, i.e., noads to nodes, we need several
subroutines that are conveniently dealt with now.

Let us first introduce the macros that make it easy to get at the parameters and
other font information. A size code, which is a multiple of 256, is added to a
family number to get an index into the table of internal font numbers
for each combination of family and size.  (Be alert: Size codes get
larger as the type gets smaller.)

@c
static const char *math_size_string(int s)
{
    if (s == text_size)
        return "textfont";
    else if (s == script_size)
        return "scriptfont";
    else
        return "scriptscriptfont";
}

@ When the style changes, the following piece of program computes associated
information:

@c
#define setup_cur_size(a) do { \
    if (a==script_style || a==cramped_script_style) \
        cur_size = script_size; \
    else if (a==script_script_style || a==cramped_script_script_style) \
        cur_size = script_script_size; \
    else \
        cur_size = text_size; \
} while (0)


@ a simple routine that creates a flat copy of a nucleus
@c
static pointer math_clone(pointer q)
{
    pointer x;
    if (q == null)
        return null;
    x = new_node(type(q), 0);
    reset_attributes(x, node_attr(q));
    if (type(q) == math_char_node) {
        math_fam(x) = math_fam(q);
        math_character(x) = math_character(q);
    } else {
        math_list(x) = math_list(q);
    }
    return x;
}

@ Here is a function that returns a pointer to a rule node having a given
  thickness |t|. The rule will extend horizontally to the boundary of the vlist
  that eventually contains it.

@c
static pointer do_fraction_rule(scaled t, pointer att, halfword some_rule, halfword cur_size, halfword cur_fam)
{
    pointer p;                  /* the new node */
    if (math_rules_mode_par) {
        p = new_rule(some_rule);
        rule_math_size(p) = cur_size;
        rule_math_font(p) = fam_fnt(cur_fam, cur_size);
    } else {
        p = new_rule(normal_rule);
    }
    rule_dir(p) = math_direction_par;
    height(p) = t;
    depth(p) = 0;
    reset_attributes(p, att);
    return p;
}

@ The |overbar| function returns a pointer to a vlist box that consists of
  a given box |b|, above which has been placed a kern of height |k| under a
  fraction rule of thickness |t| under additional space of height |ht|.

@c
static pointer overbar(pointer b, scaled k, scaled t, scaled ht, pointer att, halfword index, halfword cur_size, halfword cur_fam)
{
    pointer p, q;               /* nodes being constructed */
    p = new_kern(k);
    reset_attributes(p, att);
    couple_nodes(p,b);
    q = do_fraction_rule(t, att, index, cur_size, cur_fam);
    couple_nodes(q,p);
    p = new_kern(ht);
    reset_attributes(p, att);
    couple_nodes(p,q);
    q = vpackage(p, 0, additional, max_dimen, math_direction_par);
    reset_attributes(q, att);
    return q;
}

@ Here is a subroutine that creates a new box, whose list contains a
single character, and whose width includes the italic correction for
that character. The height or depth of the box will be negative, if
the height or depth of the character is negative; thus, this routine
may deliver a slightly different result than |hpack| would produce.

@c
static pointer char_box(internal_font_number f, int c, pointer bb)
{
    pointer b, p; /* the new box and its character node */
    b = new_null_box();
    if (do_new_math(f))
        width(b) = char_width(f, c);
    else
        width(b) = char_width(f, c) + char_italic(f, c);
    height(b) = char_height(f, c);
    depth(b) = char_depth(f, c);
    reset_attributes(b, bb);
    p = new_char(f, c);
    reset_attributes(p, bb);
    list_ptr(b) = p;
    return b;
}

@ Another handy subroutine computes the height plus depth of
  a given character:

@c
static scaled height_plus_depth(internal_font_number f, int c)
{
    return (char_height(f, c) + char_depth(f, c));
}

@ When we build an extensible character, it's handy to have the
  following subroutine, which puts a given character on top
  of the characters already in box |b|:

@c
static scaled stack_into_box(pointer b, internal_font_number f, int c)
{
    pointer p, q; /* new node placed into |b| */
    p = char_box(f, c, node_attr(b)); /* italic gets added to width */
    if (type(b) == vlist_node) {
        try_couple_nodes(p,list_ptr(b));
        list_ptr(b) = p;
        height(b) = height(p);
        if (width(b) < width(p))
            width(b) = width(p);
        return height_plus_depth(f, c);
    } else {
        q = list_ptr(b);
        if (q == null) {
            list_ptr(b) = p;
        } else {
            while (vlink(q) != null)
                q = vlink(q);
            couple_nodes(q,p);
        }
        if (height(b) < height(p))
            height(b) = height(p);
        if (depth(b) < depth(p))
            depth(b) = depth(p);
        return char_width(f, c);
    }
}

static void stack_glue_into_box(pointer b, scaled min, scaled max) {
    halfword p = new_glue(zero_glue);
    width(p) = min;
    stretch(p) = max - min;
    reset_attributes(p, node_attr(b));
    if (type(b) == vlist_node) {
        try_couple_nodes(p,list_ptr(b));
        list_ptr(b) = p;
    } else {
        halfword q = list_ptr(b);
        if (q == null) {
            list_ptr(b) = p;
        } else {
            while (vlink(q) != null)
                q = vlink(q);
            couple_nodes(q,p);
        }
    }
}

@ \TeX's most important routine for dealing with formulas is called
 |mlist_to_hlist|.  After a formula has been scanned and represented
 as an mlist, this routine converts it to an hlist that can be placed
 into a box or incorporated into the text of a paragraph.  The
 explicit parameter |cur_mlist| points to the first node or noad in
 the given mlist (and it might be |null|); the parameter |penalties|
 is |true| if penalty nodes for potential line breaks are to be
 inserted into the resulting hlist, the parameter |cur_style| is a
 style code.  After |mlist_to_hlist| has acted, |vlink(temp_head)|
 points to the translated hlist.

 Since mlists can be inside mlists, the procedure is recursive. And since this
 is not part of \TeX's inner loop, the program has been written in a manner
 that stresses compactness over efficiency.
@^recursion@>

@c
int cur_size; /* size code corresponding to |cur_style|  */

@ @c
static pointer get_delim_box(pointer q, extinfo * ext, internal_font_number f, scaled v,
                             pointer att, int cur_style, int boxtype)
{
    pointer b;                  /* new box */
    scaled b_max;               /* natural (maximum) size of the stack */
    scaled s_max;               /* amount of possible shrink in the stack */
    extinfo *cur;
    scaled min_overlap, prev_overlap;
    int i;                      /* a temporary counter number of extensible pieces */
    int with_extenders;         /* number of times to repeat each repeatable item in |ext| */
    int num_extenders, num_normal;
    scaled a, c, d;

    assert(ext != NULL);
    b = new_null_box();
    type(b) = (quarterword) boxtype;
    reset_attributes(b, att);
    min_overlap = connector_overlap_min(cur_style);
    assert(min_overlap >= 0);
    with_extenders = -1;
    num_extenders = 0;
    num_normal = 0;

    cur = ext;
    while (cur != NULL) {
        if (!char_exists(f, cur->glyph)) {
            const char *hlp[] = {
                "Each glyph part in an extensible item should exist in the font.",
                "I will give up trying to find a suitable size for now. Fix your font!",
                NULL
            };
            tex_error("Variant part doesn't exist.", hlp);
            width(b) = null_delimiter_space_par;
            return b;
        }
        if (cur->extender > 0)
            num_extenders++;
        else
            num_normal++;
        /* no negative overlaps or advances are allowed */
        if (cur->start_overlap < 0 || cur->end_overlap < 0 || cur->advance < 0) {
            const char *hlp[] = {
                "All measurements in extensible items should be positive.",
                "To get around this problem, I have changed the font metrics.",
                "Fix your font!",
                NULL
            };
            tex_error("Extensible recipe has negative fields.", hlp);
            if (cur->start_overlap < 0)
                cur->start_overlap = 0;
            if (cur->end_overlap < 0)
                cur->end_overlap = 0;
            if (cur->advance < 0)
                cur->advance = 0;
        }
        cur = cur->next;
    }
    if (num_normal == 0) {
        const char *hlp[] = {
            "Each extensible recipe should have at least one non-repeatable part.",
            "To get around this problem, I have changed the first part to be",
            "non-repeatable. Fix your font!",
            NULL
        };
        tex_error("Extensible recipe has no fixed parts.", hlp);
        ext->extender = 0;
        num_normal = 1;
        num_extenders--;
    }
    /*
        |ext| holds a linked list of numerous items that may or may not be
        repeatable. For the total height, we have to figure out how many items
        are needed to create a stack of at least |v|.

        The next |while| loop does  that. It has two goals: it finds out
        the natural height |b_max|  of the all the parts needed to reach
        at least |v|,  and it sets |with_extenders| to the number of times
        each of the repeatable items in |ext| has to be repeated to reach
        that height.

    */
    cur = ext;
    b_max = 0;
    while (b_max < v && num_extenders > 0) {
        b_max = 0;
        prev_overlap = 0;
        with_extenders++;
        for (cur = ext; cur != NULL; cur = cur->next) {
            if (cur->extender == 0) {
                c = cur->start_overlap;
                if (min_overlap < c)
                    c = min_overlap;
                if (prev_overlap < c)
                    c = prev_overlap;
                a = cur->advance;
                if (a == 0) {
                    /* for tfm fonts */
                    if (boxtype == vlist_node)
                        a = height_plus_depth(f, cur->glyph);
                    else
                        a = char_width(f, cur->glyph);
                    assert(a >= 0);
                }
                b_max += a - c;
                prev_overlap = cur->end_overlap;
            } else {
                i = with_extenders;
                while (i > 0) {
                    c = cur->start_overlap;
                    if (min_overlap < c)
                        c = min_overlap;
                    if (prev_overlap < c)
                        c = prev_overlap;
                    a = cur->advance;
                    if (a == 0) {
                        /* for tfm fonts */
                        if (boxtype == vlist_node)
                            a = height_plus_depth(f, cur->glyph);
                        else
                            a = char_width(f, cur->glyph);
                        assert(a >= 0);
                    }
                    b_max += a - c;
                    prev_overlap = cur->end_overlap;
                    i--;
                }
            }
        }
    }

    /*
        assemble box using |with_extenders| copies of each extender, with
        appropriate glue wherever an overlap occurs
    */
    prev_overlap = 0;
    b_max = 0;
    s_max = 0;
    for (cur = ext; cur != NULL; cur = cur->next) {
        if (cur->extender == 0) {
            c = cur->start_overlap;
            if (prev_overlap < c)
                c = prev_overlap;
            d = c;
            if (min_overlap < c)
                c = min_overlap;
            if (d > 0) {
                stack_glue_into_box(b, -d, -c);
                s_max += (-c) - (-d);
                b_max -= d;
            }
            b_max += stack_into_box(b, f, cur->glyph);
            prev_overlap = cur->end_overlap;
            i--;
        } else {
            i = with_extenders;
            while (i > 0) {
                c = cur->start_overlap;
                if (prev_overlap < c)
                    c = prev_overlap;
                d = c;
                if (min_overlap < c)
                    c = min_overlap;
                if (d > 0) {
                    stack_glue_into_box(b, -d, -c);
                    s_max += (-c) - (-d);
                    b_max -= d;
                }
                b_max += stack_into_box(b, f, cur->glyph);
                prev_overlap = cur->end_overlap;
                i--;
            }
        }
    }

    /* set glue so as to stretch the connections if needed */

    d = 0;
    if (v > b_max && s_max > 0) {
        d = v-b_max;
        /* don't stretch more than |s_max| */
        if (d > s_max)
            d = s_max;
        glue_order(b) = normal;
        glue_sign(b) = stretching;
        glue_set(b) = unfloat(d/(float) s_max);
        b_max += d;
    }

    if (boxtype == vlist_node) {
        height(b) = b_max;
    } else {
        width(b) = b_max;
    }

    return b;
}

@ The |var_delimiter| function, which finds or constructs a sufficiently
  large delimiter, is the most interesting of the auxiliary functions that
  currently concern us. Given a pointer |d| to a delimiter field in some noad,
  together with a size code |s| and a vertical distance |v|, this function
  returns a pointer to a box that contains the smallest variant of |d| whose
  height plus depth is |v| or more. (And if no variant is large enough, it
  returns the largest available variant.) In particular, this routine will
  construct arbitrarily large delimiters from extensible components, if
  |d| leads to such characters.

  The value returned is a box whose |shift_amount| has been set so that
  the box is vertically centered with respect to the axis in the given size.
  If a built-up symbol is returned, the height of the box before shifting
  will be the height of its topmost component.

@c
static void endless_loop_error(internal_font_number g, int y)
{
    char s[256];
    const char *hlp[] = {
        "You managed to create a seemingly endless charlist chain in the current",
        "font. I have counted until 10000 already and still have not escaped, so"
        "I will jump out of the loop all by myself now. Fix your font!",
        NULL
    };
    snprintf(s, 256, "Math error: endless loop in charlist (U+%04x in %s)",
             (int) y, font_name(g));
    tex_error(s, hlp);
}

static pointer do_delimiter(pointer q, pointer d, int s, scaled v, boolean flat, int cur_style, boolean shift, boolean *stack, scaled *delta)
{
    pointer b;                  /* the box that will be constructed */
    internal_font_number f, g;  /* best-so-far and tentative font codes */
    int c, i, x, y;             /* best-so-far and tentative character codes */
    scaled u;                   /* height-plus-depth of a tentative character */
    scaled w;                   /* largest height-plus-depth so far */
    int z;                      /* runs through font family members */
    boolean large_attempt;      /* are we trying the ``large'' variant? */
    pointer att;                /* to save the current attribute list */
    boolean do_parts;
    extinfo *ext;
    att = null;
    f = null_font;
    c = 0;
    w = 0;
    do_parts = false;
    large_attempt = false;
    if (d == null)
        goto FOUND;
    z = small_fam(d);
    x = small_char(d);
    i = 0;
    while (true) {
        /*
            The search process is complicated slightly by the facts that some of the
            characters might not be present in some of the fonts, and they might not
            be probed in increasing order of height.
        */
        if ((z != 0) || (x != 0)) {
            g = fam_fnt(z, s);
            if (g != null_font) {
                y = x;
              CONTINUE:
                i++;
                if (char_exists(g, y)) {
                    if (flat)
                        u = char_width(g, y);
                    else
                        u = height_plus_depth(g, y);
                    if (u > w) {
                        f = g;
                        c = y;
                        w = u;
                        if (u >= v)
                            goto FOUND;
                    }
                    if (char_tag(g, y) == ext_tag) {
                        f = g;
                        c = y;
                        do_parts = true;
                        goto FOUND;
                    }
                    if (i > 10000) {
                        /* endless loop */
                        endless_loop_error(g, y);
                        goto FOUND;
                    }
                    if (char_tag(g, y) == list_tag) {
                        y = char_remainder(g, y);
                        goto CONTINUE;
                    }
                }
            }
        }
        if (large_attempt)
            goto FOUND; /* there were none large enough */
        large_attempt = true;
        z = large_fam(d);
        x = large_char(d);
    }
  FOUND:
    if (d != null) {
        att = node_attr(d);
        node_attr(d) = null;
        flush_node(d);
    }
    if (f != null_font) {
        /*
            When the following code is executed, |do_parts| will be true
            if a built-up symbol is supposed to be returned.
        */
        ext = NULL;
        if ((do_parts) && ((!flat && (ext = get_charinfo_vert_variants(char_info(f,c))) != NULL)
                       ||  ( flat && (ext = get_charinfo_hor_variants (char_info(f,c))) != NULL))) {
            if (flat) {
                b = get_delim_box(d, ext, f, v, att, cur_style, hlist_node);
            } else {
                b = get_delim_box(d, ext, f, v, att, cur_style, vlist_node);
            }
            if (delta != NULL) {
                if (do_new_math(f)) {
                    *delta = char_vert_italic(f,x);
                } else {
                    *delta = char_italic(f,x);
                }
            }
            if (stack != NULL)
                *stack = true ;
        } else {
            b = char_box(f, c, att);
            if (!do_new_math(f)) {
                /* italic gets added to width */
                width(b) += char_italic(f, c);
            }
            if (delta != NULL) {
                *delta = char_italic(f, c); /* was historically (f, x) */
            }
            if (stack != NULL)
                *stack = false ;
        }
    } else {
        b = new_null_box();
        reset_attributes(b, att);
        if (flat) {
            width(b) = 0;
        } else {
            /* use this width if no delimiter was found */
            width(b) = null_delimiter_space_par;
        }
        if (delta != NULL) {
            *delta = 0;
        }
        if (stack != NULL)
            *stack = false ;
    }
    if (!flat) {
        /* vertical variant */
        shift_amount(b) = half(height(b) - depth(b));
        if (shift) {
            shift_amount(b) -= math_axis_size(s);
        }
    }
    delete_attribute_ref(att);
    return b;
}

@ The next subroutine is much simpler; it is used for numerators and
denominators of fractions as well as for displayed operators and
their limits above and below. It takes a given box~|b| and
changes it so that the new box is centered in a box of width~|w|.
The centering is done by putting \.{\\hss} glue at the left and right
of the list inside |b|, then packaging the new box; thus, the
actual box might not really be centered, if it already contains
infinite glue.

The given box might contain a single character whose italic correction
has been added to the width of the box; in this case a compensating
kern is inserted.

@c
static pointer rebox(pointer b, scaled w)
{
    pointer p, q, r, att;       /* temporary registers for list manipulation */
    internal_font_number f;     /* font in a one-character box */
    scaled v;                   /* width of a character without italic correction */

    if ((width(b) != w) && (list_ptr(b) != null)) {
        if (type(b) == vlist_node) {
            p = hpack(b, 0, additional, -1);
            reset_attributes(p, node_attr(b));
            b = p;
        }
        p = list_ptr(b);
        att = node_attr(b);
        add_node_attr_ref(att);
        if ((is_char_node(p)) && (vlink(p) == null)) {
            f = font(p);
            v = char_width(f, character(p));
            if (v != width(b)) {
                q = new_kern(width(b) - v);
                reset_attributes(q, att);
                couple_nodes(p,q);
            }
        }
        list_ptr(b) = null;
        flush_node(b);
        b = new_glue(ss_glue);
        reset_attributes(b, att);
        couple_nodes(b,p);
        while (vlink(p) != null)
            p = vlink(p);
        q = new_glue(ss_glue);
        reset_attributes(q, att);
        couple_nodes(p,q);
        r = hpack(b, w, exactly, -1);
        reset_attributes(r, att);
        delete_attribute_ref(att);
        return r;
    } else {
        width(b) = w;
        return b;
    }
}

@ Here is a subroutine that creates a new glue specification from another
one that is expressed in `\.{mu}', given the value of the math unit.

@c
#define mu_mult(A) mult_and_add(n,(A),xn_over_d((A),f,unity),max_dimen)

static pointer math_glue(pointer g, scaled m)
{
    int n = x_over_n(m, unity); /* integer part of |m| */
    scaled f = tex_remainder;   /* fraction part of |m| */
    pointer p;                  /* the new glue specification */
    if (f < 0) {
        decr(n);
        f = f + unity;
    }
    p = new_node(glue_node, 0);
    width(p) = mu_mult(width(g)); /* convert \.{mu} to \.{pt} */
    stretch_order(p) = stretch_order(g);
    if (stretch_order(p) == normal)
        stretch(p) = mu_mult(stretch(g));
    else
        stretch(p) = stretch(g);
    shrink_order(p) = shrink_order(g);
    if (shrink_order(p) == normal)
        shrink(p) = mu_mult(shrink(g));
    else
        shrink(p) = shrink(g);
    return p;
}

static void math_glue_to_glue(pointer p, scaled m)
{
    int n = x_over_n(m, unity); /* integer part of |m| */
    scaled f = tex_remainder;   /* fraction part of |m| */
    if (f < 0) {
        decr(n);
        f = f + unity;
    }
    width(p) = mu_mult(width(p)); /* convert \.{mu} to \.{pt} */
    if (stretch_order(p) == normal)
        stretch(p) = mu_mult(stretch(p));
    if (shrink_order(p) == normal)
        shrink(p) = mu_mult(shrink(p));
    subtype(p) = normal;
}

@ The |math_kern| subroutine removes |mu_glue| from a kern node, given
the value of the math unit.

@c
static void math_kern(pointer p, scaled m)
{
    int n;    /* integer part of |m| */
    scaled f; /* fraction part of |m| */
    if (subtype(p) == mu_glue) {
        n = x_over_n(m, unity);
        f = tex_remainder;
        if (f < 0) {
            decr(n);
            f = f + unity;
        }
        width(p) = mu_mult(width(p));
        subtype(p) = italic_kern;
    }
}

@ @c
void run_mlist_to_hlist(halfword p, boolean penalties, int mstyle)
{
    int callback_id;
    int a, sfix;
    if (p == null) {
        vlink(temp_head) = null;
        return;
    }
    finalize_math_parameters();
    callback_id = callback_defined(mlist_to_hlist_callback);
    if (callback_id > 0) {
        sfix = lua_gettop(Luas);
        if (!get_callback(Luas, callback_id)) {
            lua_settop(Luas, sfix);
            return;
        }
        alink(p) = null ;
        nodelist_to_lua(Luas, p);
        lua_push_math_style_name(Luas, mstyle);
        lua_pushboolean(Luas, penalties);
        if (lua_pcall(Luas, 3, 1, 0) != 0) {            /* 3 args, 1 result */
            char errmsg[256]; /* temp hack ... we will have a formatted error */
            snprintf(errmsg, 255, "error: %s\n", lua_tostring(Luas, -1));
            errmsg[255]='\0';
            lua_settop(Luas, sfix);
            normal_error("mlist to hlist",errmsg); /* to be done */
            return;
        }
        a = nodelist_from_lua(Luas);
        /* alink(vlink(a)) = null; */
        lua_settop(Luas, sfix);
        vlink(temp_head) = a;
    } else if (callback_id == 0) {
        mlist_to_hlist(p, penalties, mstyle);
    } else {
        vlink(temp_head) = null;
    }
}

@ The recursion in |mlist_to_hlist| is due primarily to a subroutine
called |clean_box| that puts a given noad field into a box using a given
math style; |mlist_to_hlist| can call |clean_box|, which can call
|mlist_to_hlist|.
@^recursion@>

The box returned by |clean_box| is ``clean'' in the
sense that its |shift_amount| is zero.

@c
static pointer clean_box(pointer p, int s, int cur_style)
{
    pointer q;            /* beginning of a list to be boxed */
    pointer x;            /* box to be returned */
    pointer r;            /* temporary pointer */
    pointer mlist = null; /* beginning of mlist to be translated */
    switch (type(p)) {
    case math_char_node:
        mlist = new_noad();
        r = math_clone(p);
        nucleus(mlist) = r;
        break;
    case sub_box_node:
        q = math_list(p);
        goto FOUND;
        break;
    case sub_mlist_node:
        mlist = math_list(p);
        break;
    default:
        q = new_null_box();
        goto FOUND;
    }
    mlist_to_hlist(mlist, false, s);
    q = vlink(temp_head); /* recursive call */
    setup_cur_size(cur_style);
  FOUND:
    if (is_char_node(q) || (q == null))
        x = hpack(q, 0, additional, -1);
    else if ((vlink(q) == null) && (type(q) <= vlist_node) && (shift_amount(q) == 0))
        x = q; /* it's already clean */
    else
        x = hpack(q, 0, additional, -1);
    if (x != q && q != null)
        reset_attributes(x, node_attr(q));
    /* Here we save memory space in a common case. */
    q = list_ptr(x);
    if (is_char_node(q)) {
        r = vlink(q);
        if (r != null) {
            if (vlink(r) == null) {
                if (!is_char_node(r)) {
                    if (type(r) == kern_node) {
                        /* unneeded italic correction */
                        flush_node(r);
                        vlink(q) = null;
                    }
                }
            }
        }
    }
    return x;
}

@ It is convenient to have a procedure that converts a |math_char|
field to an ``unpacked'' form. The |fetch| routine sets |cur_f| and |cur_c|
to the font code and character code of a given noad field.
It also takes care of issuing error messages for
nonexistent characters; in such cases, |char_exists(cur_f,cur_c)| will be |false|
after |fetch| has acted, and the field will also have been reset to |null|.

The outputs of |fetch| are placed in global variables.

@c
internal_font_number cur_f; /* the |font| field of a |math_char| */
int cur_c;                  /* the |character| field of a |math_char| */

@ Here we unpack the |math_char| field |a|.

@c static void fetch(pointer a)
{
    cur_c = math_character(a);
    cur_f = fam_fnt(math_fam(a), cur_size);
    if (cur_f == null_font) {
        char *msg;
        const char *hlp[] = {
            "Somewhere in the math formula just ended, you used the",
            "stated character from an undefined font family. For example,",
            "plain TeX doesn't allow \\it or \\sl in subscripts. Proceed,",
            "and I'll try to forget that I needed that character.",
            NULL
        };
        msg = xmalloc(256);
        snprintf(msg, 255, "\\%s%d is undefined (character %d)",
                 math_size_string(cur_size), (int) math_fam(a), (int) cur_c);
        tex_error(msg, hlp);
        free(msg);
    } else if (!(char_exists(cur_f, cur_c))) {
        char_warning(cur_f, cur_c);
    }
}

@ We need to do a lot of different things, so |mlist_to_hlist| makes two
passes over the given mlist.

The first pass does most of the processing: It removes ``mu'' spacing from
glue, it recursively evaluates all subsidiary mlists so that only the
top-level mlist remains to be handled, it puts fractions and square roots
and such things into boxes, it attaches subscripts and superscripts, and
it computes the overall height and depth of the top-level mlist so that
the size of delimiters for a |fence_noad| will be known.
The hlist resulting from each noad is recorded in that noad's |new_hlist|
field, an integer field that replaces the |nucleus| or |thickness|.
@^recursion@>

The second pass eliminates all noads and inserts the correct glue and
penalties between nodes.

@c
static void assign_new_hlist(pointer q, pointer r)
{
    switch (type(q)) {
    case fraction_noad:
        math_list(numerator(q)) = null;
        flush_node(numerator(q));
        numerator(q) = null;
        math_list(denominator(q)) = null;
        flush_node(denominator(q));
        denominator(q) = null;
        break;
    case radical_noad:
    case simple_noad:
    case accent_noad:
        if (nucleus(q) != null) {
            math_list(nucleus(q)) = null;
            flush_node(nucleus(q));
            nucleus(q) = null;
        }
        break;
    }
    new_hlist(q) = r;
}

@ @c
#define choose_mlist(A) do { p=A(q); A(q)=null; } while (0)

@ Most of the actual construction work of |mlist_to_hlist| is done
by procedures with names like |make_fraction|, |make_radical|, etc. To
illustrate the general setup of such procedures, let's begin with a
couple of simple ones.

@c
static void make_over(pointer q, int cur_style, int cur_size, int cur_fam)
{
    pointer p;
    p = overbar(clean_box(nucleus(q), cramped_style(cur_style), cur_style),
                overbar_vgap(cur_style), overbar_rule(cur_style),
                overbar_kern(cur_style), node_attr(nucleus(q)), math_over_rule, cur_size, cur_fam);
    math_list(nucleus(q)) = p;
    type(nucleus(q)) = sub_box_node;
}

static void make_under(pointer q, int cur_style, int cur_size, int cur_fam)
{
    pointer p, x, y, r;         /* temporary registers for box construction */
    scaled delta;               /* overall height plus depth */
    x = clean_box(nucleus(q), cur_style, cur_style);
    p = new_kern(underbar_vgap(cur_style));
    reset_attributes(p, node_attr(q));
    couple_nodes(x,p);
    r = do_fraction_rule(underbar_rule(cur_style), node_attr(q), math_under_rule, cur_size, cur_fam);
    couple_nodes(p,r);
    y = vpackage(x, 0, additional, max_dimen, math_direction_par);
    reset_attributes(y, node_attr(q));
    delta = height(y) + depth(y) + underbar_kern(cur_style);
    height(y) = height(x);
    depth(y) = delta - height(y);
    math_list(nucleus(q)) = y;
    type(nucleus(q)) = sub_box_node;
}

static void make_vcenter(pointer q)
{
    pointer v;                  /* the box that should be centered vertically */
    scaled delta;               /* its height plus depth */
    v = math_list(nucleus(q));
    if (type(v) != vlist_node)
        confusion("vcenter");
    delta = height(v) + depth(v);
    height(v) = math_axis_size(cur_size) + half(delta);
    depth(v) = delta - height(v);
}

@ According to the rules in the \.{DVI} file specifications, we ensure alignment
@^square roots@>
between a square root sign and the rule above its nucleus by assuming that the
baseline of the square-root symbol is the same as the bottom of the rule. The
height of the square-root symbol will be the thickness of the rule, and the
depth of the square-root symbol should exceed or equal the height-plus-depth
of the nucleus plus a certain minimum clearance~|psi|. The symbol will be
placed so that the actual clearance is |psi| plus half the excess.

@c
static void make_hextension(pointer q, int cur_style)
{
    pointer e, p;
    halfword w;
    boolean stack = false;
    e = do_delimiter(q, left_delimiter(q), cur_size, radicalwidth(q), true, cur_style, true, &stack, NULL);
    w = width(e);
    if (!stack&& (radicalwidth(q) != 0) && (radicalwidth(q) != width(e))) {
        if (radicalmiddle(q)) {
            p = new_kern(half(radicalwidth(q)-w));
            reset_attributes(p, node_attr(q));
            couple_nodes(p,e);
            e = p;
            w = radicalwidth(q);
        } else if (radicalexact(q)) {
            w = radicalwidth(q);
        }
    }
    e = hpack(e, 0, additional, -1);
    width(e) = w ;
    reset_attributes(e, node_attr(q));
    math_list(nucleus(q)) = e;
    left_delimiter(q) = null;
}

static void make_radical(pointer q, int cur_style)
{
    pointer x, y, p, l1, l2;     /* temporary registers for box construction */
    scaled delta, clr, theta, h; /* dimensions involved in the calculation */
    x = clean_box(nucleus(q), cramped_style(cur_style), cur_style);
    clr = radical_vgap(cur_style);
    theta = radical_rule_par(cur_style);
    if (theta == undefined_math_parameter) {
        /* a real radical */
        theta = fraction_rule(cur_style);
        y = do_delimiter(q, left_delimiter(q), cur_size, height(x) + depth(x) + clr + theta, false, cur_style, true, NULL, NULL);
        /*
            If |y| is a composite then set |theta| to the height of its top
            character, else set it to the height of |y|.
        */
        l1 = list_ptr(y);
        if ((l1 != null) && (type(l1) == hlist_node)) {
            /* possible composite */
            l2 = list_ptr(l1);
            if ((l2 != null) && (type(l2) == glyph_node)) {
                /* top character */
                theta = char_height(font(l2), character(l2));
            } else {
                theta = height(y);
            }
        } else {
            theta = height(y);
        }
    } else {
        /* not really a radical but we use its node, historical sharing (like in mathml) */
        y = do_delimiter(q, left_delimiter(q), cur_size, height(x) + depth(x) + clr + theta, false, cur_style, true, NULL, NULL);
    }
    left_delimiter(q) = null;
    delta = (depth(y) + height(y) - theta) - (height(x) + depth(x) + clr);
    if (delta > 0) {
        /* increase the actual clearance */
        clr = clr + half(delta);
    }
    shift_amount(y) = (height(y) - theta) - (height(x) + clr);
    h = depth(y) + height(y);
    p = overbar(x, clr, theta, radical_kern(cur_style), node_attr(y), math_radical_rule, cur_size, small_fam(left_delimiter(q)));
    couple_nodes(y,p);
    if (degree(q) != null) {
        scaled wr, br, ar;
        pointer r = clean_box(degree(q), script_script_style, cur_style);
        reset_attributes(r, node_attr(degree(q)));
        wr = width(r);
        if (wr == 0) {
            flush_node(r);
        } else {
            br = radical_degree_before(cur_style);
            ar = radical_degree_after(cur_style);
            if (-ar > (wr + br))
                ar = -(wr + br);
            x = new_kern(ar);
            reset_attributes(x, node_attr(degree(q)));
            couple_nodes(x,y);
            shift_amount(r) =
                -((xn_over_d(h, radical_degree_raise(cur_style), 100)) -
                  depth(y) - shift_amount(y));
            couple_nodes(r,x);
            x = new_kern(br);
            reset_attributes(x, node_attr(degree(q)));
            couple_nodes(x,r);
            y = x;
        }
        /* for \.{\\Uroot ..{<list>}{}} : */
        math_list(degree(q)) = null;
        flush_node(degree(q));
    }
    p = hpack(y, 0, additional, -1);
    reset_attributes(p, node_attr(q));
    math_list(nucleus(q)) = p;
    type(nucleus(q)) = sub_box_node;
}

@ Construct a vlist box

@c
static pointer wrapup_over_under_delimiter(pointer x, pointer y, pointer q, scaled shift_up, scaled shift_down)
{
    pointer p;  /* temporary register for box construction */
    pointer v = new_null_box();
    type(v) = vlist_node;
    height(v) = shift_up + height(x);
    depth(v) = depth(y) + shift_down;
    reset_attributes(v, node_attr(q));
    p = new_kern((shift_up - depth(x)) - (height(y) - shift_down));
    reset_attributes(p, node_attr(q));
    couple_nodes(p,y);
    couple_nodes(x,p);
    list_ptr(v) = x;
    return v;
}

/* when exact use radicalwidth (y is delimiter) */

@ @c

#define fixup_widths(q,x,y) do { \
    if (width(y) >= width(x)) { \
        if (radicalwidth(q) != 0) { \
            shift_amount(x) += half(width(y)-width(x)) ; \
        } \
        width(x) = width(y); \
    } else { \
        if (radicalwidth(q) != 0) { \
            shift_amount(y) += half(width(x)-width(y)) ; \
        } \
        width(y) = width(x); \
    } \
} while (0)


#define check_radical(q,stack,r,t) do { \
    if (!stack && (width(r) >= width(t)) && (radicalwidth(q) != 0) && (radicalwidth(q) != width(r))) { \
        if (radicalleft(q)) { \
            halfword p = new_kern(radicalwidth(q)-width(r)); \
            reset_attributes(p, node_attr(q)); \
            couple_nodes(p,r); \
            r = hpack(p, 0, additional, -1); \
            width(r) = radicalwidth(q); \
            reset_attributes(r, node_attr(q)); \
        } else if (radicalmiddle(q)) { \
            halfword p = new_kern(half(radicalwidth(q)-width(r))); \
            reset_attributes(p, node_attr(q)); \
            couple_nodes(p,r); \
            r = hpack(p, 0, additional, -1); \
            width(r) = radicalwidth(q); \
            reset_attributes(r, node_attr(q)); \
        } else if (radicalright(q)) { \
            /* also kind of exact compared to vertical */ \
            r = hpack(r, 0, additional, -1); \
            width(r) = radicalwidth(q); \
            reset_attributes(r, node_attr(q)); \
        } \
    } \
} while (0)

#define check_widths(q,p) do { \
    if (radicalwidth(q) != 0) { \
        wd = radicalwidth(q); \
    } else { \
        wd = width(p); \
    } \
} while (0)

@ this has the |nucleus| box |x| as a limit above an extensible delimiter |y|

@c
static void make_over_delimiter(pointer q, int cur_style)
{
    pointer x, y, v; /* temporary registers for box construction */
    scaled shift_up, shift_down, clr, delta, wd;
    boolean stack;
    x = clean_box(nucleus(q), sub_style(cur_style), cur_style);
    check_widths(q,x);
    y = do_delimiter(q, left_delimiter(q), cur_size, wd, true, cur_style, true, &stack, NULL);
    left_delimiter(q) = null;
    check_radical(q,stack,y,x);
    fixup_widths(q, x, y);
    shift_up = over_delimiter_bgap(cur_style);
    shift_down = 0;
    clr = over_delimiter_vgap(cur_style);
    delta = clr - ((shift_up - depth(x)) - (height(y) - shift_down));
    if (delta > 0) {
        shift_up = shift_up + delta;
    }
    v = wrapup_over_under_delimiter(x, y, q, shift_up, shift_down);
    width(v) = width(x); /* this also equals |width(y)| */
    math_list(nucleus(q)) = v;
    type(nucleus(q)) = sub_box_node;
}

@ this has the extensible delimiter |x| as a limit below |nucleus| box |y|

@c
static void make_under_delimiter(pointer q, int cur_style)
{
    pointer x, y, v; /* temporary registers for box construction */
    scaled shift_up, shift_down, clr, delta, wd;
    boolean stack;
    y = clean_box(nucleus(q), sup_style(cur_style), cur_style);
    check_widths(q,y);
    x = do_delimiter(q, left_delimiter(q), cur_size, wd, true, cur_style, true, &stack, NULL);
    left_delimiter(q) = null;
    check_radical(q,stack,x,y);
    fixup_widths(q, x, y);
    shift_up = 0;
    shift_down = under_delimiter_bgap(cur_style);
    clr = under_delimiter_vgap(cur_style);
    delta = clr - ((shift_up - depth(x)) - (height(y) - shift_down));
    if (delta > 0) {
        shift_down = shift_down + delta;
    }
    v = wrapup_over_under_delimiter(x, y, q, shift_up, shift_down);
    width(v) = width(y); /* this also equals |width(y)| */
    math_list(nucleus(q)) = v;
    type(nucleus(q)) = sub_box_node;
}

@ this has the extensible delimiter |x| as a limit above |nucleus| box |y|

@c
static void make_delimiter_over(pointer q, int cur_style)
{
    pointer x, y, v; /* temporary registers for box construction */
    scaled shift_up, shift_down, clr, actual, wd;
    boolean stack;
    y = clean_box(nucleus(q), cur_style, cur_style);
    check_widths(q,y);
    x = do_delimiter(q, left_delimiter(q), cur_size + (cur_size == script_script_size ? 0 : 1), wd, true, cur_style, true, &stack, NULL);
    left_delimiter(q) = null;
    check_radical(q,stack,x,y);
    fixup_widths(q, x, y);
    shift_up = over_delimiter_bgap(cur_style)-height(x)-depth(x);
    shift_down = 0;
    clr = over_delimiter_vgap(cur_style);
    actual = shift_up - height(y);
    if (actual < clr) {
        shift_up = shift_up + (clr-actual);
    }
    v = wrapup_over_under_delimiter(x, y, q, shift_up, shift_down);
    width(v) = width(x); /* this also equals |width(y)| */
    math_list(nucleus(q)) = v;
    type(nucleus(q)) = sub_box_node;
}

@ this has the extensible delimiter |y| as a limit below a |nucleus| box |x|

@c
static void make_delimiter_under(pointer q, int cur_style)
{
    pointer x, y, v;            /* temporary registers for box construction */
    scaled shift_up, shift_down, clr, actual, wd;
    boolean stack;
    x = clean_box(nucleus(q), cur_style, cur_style);
    check_widths(q,x);
    y = do_delimiter(q, left_delimiter(q), cur_size + (cur_size == script_script_size ? 0 : 1), wd, true, cur_style, true, &stack, NULL);
    left_delimiter(q) = null;
    check_radical(q,stack,y,x);
    fixup_widths(q, x, y);
    shift_up = 0;
    shift_down = under_delimiter_bgap(cur_style) - height(y)-depth(y);
    clr = under_delimiter_vgap(cur_style);
    actual = shift_down - depth(x);
    if (actual<clr) {
       shift_down += (clr-actual);
    }
    v = wrapup_over_under_delimiter(x, y, q, shift_up, shift_down);
    width(v) = width(y); /* this also equals |width(y)| */
    math_list(nucleus(q)) = v;
    type(nucleus(q)) = sub_box_node;
}

@ Slants are not considered when placing accents in math mode. The accenter is
centered over the accentee, and the accent width is treated as zero with
respect to the size of the final box.

@c
#define TOP_CODE            1
#define BOT_CODE            2
#define OVERLAY_CODE        4
#define STRETCH_ACCENT_CODE 8

static boolean compute_accent_skew(pointer q, int flags, scaled *s)
{
    pointer p;                     /* temporary register for box construction */
    boolean s_is_absolute = false; /* will be true if a top-accent is placed in |s| */
    if (type(nucleus(q)) == math_char_node) {
        fetch(nucleus(q));
        if (do_new_math(cur_f)) {
            /*
                there is no bot_accent so let's assume similarity

                if (flags & (TOP_CODE | OVERLAY_CODE)) {
                    *s = char_top_accent(cur_f, cur_c);
                    if (*s != INT_MIN) {
                        s_is_absolute = true;
                    }
                } else {
                    *s = char_bot_accent(cur_f, cur_c);
                    if (*s != INT_MIN) {
                        s_is_absolute = true;
                    }
                }
            */
            *s = char_top_accent(cur_f, cur_c);
            if (*s != INT_MIN) {
                s_is_absolute = true;
            }
        } else {
            if (flags & TOP_CODE) {
                *s = get_kern(cur_f, cur_c, skew_char(cur_f));
            } else {
                *s = 0;
            }
        }
    } else if (type(nucleus(q)) == sub_mlist_node) {
        /*
            if |nucleus(q)| is a |sub_mlist_node| composed of an |accent_noad| we

            * use the positioning of the nucleus of that noad, recursing until
            * the inner most |accent_noad|. This way multiple stacked accents are
            * aligned to the inner most one.
        */
        p = math_list(nucleus(q));
        if (type(p) == accent_noad) {
            s_is_absolute = compute_accent_skew(p, flags, s);
        }
    }

    return s_is_absolute;
}

static void do_make_math_accent(pointer q, internal_font_number f, int c, int flags, int cur_style)
{
    pointer p, r, x, y;    /* temporary registers for box construction */
    scaled s;              /* amount to skew the accent to the right */
    scaled h;              /* height of character being accented */
    scaled delta;          /* space to remove between accent and accentee */
    scaled w;              /* width of the accentee, not including sub/superscripts */
    boolean s_is_absolute; /* will be true if a top-accent is placed in |s| */
    scaled fraction ;
    scaled ic = 0;
    scaled target ;
    extinfo *ext;
    pointer attr_p;
    attr_p = (flags & TOP_CODE ? top_accent_chr(q) : flags & BOT_CODE ? bot_accent_chr(q) : overlay_accent_chr(q));
    fraction = accentfraction(q);
    c = cur_c;
    f = cur_f;
    s = 1;
    if (fraction == 0) {
        fraction = 1000;
    }
    /* Compute the amount of skew, or set |s| to an alignment point */
    s_is_absolute = compute_accent_skew(q, flags, &s);
    x = clean_box(nucleus(q), cramped_style(cur_style), cur_style);
    w = width(x);
    h = height(x);
    if (do_new_math(cur_f) && !s_is_absolute) {
        s = half(w);
        s_is_absolute = true;
    }
    /* Switch to a larger accent if available and appropriate */
    y = null;
    ext = NULL;
    if (flags & OVERLAY_CODE) {
        if (fraction > 0) {
            target = xn_over_d(h,fraction,1000);
        } else {
            target = h;
        }
    } else {
        if (fraction > 0) {
            target = xn_over_d(w,fraction,1000);
        } else {
            target = w;
        }
    }
    if ((flags & STRETCH_ACCENT_CODE) && (char_width(f, c) < w)) {
      while (1) {
        if ((char_tag(f, c) == ext_tag) && ((ext = get_charinfo_hor_variants(char_info(f, c))) != NULL)) {
            /* a bit weird for an overlay but anyway, here we don't need a factor as we don't step */
            y = get_delim_box(q, ext, f, w, node_attr(attr_p), cur_style, hlist_node);
            break;
        } else if (char_tag(f, c) != list_tag) {
            break;
        } else {
            int yy = char_remainder(f, c);
            if (!char_exists(f, yy)) {
                break;
            } else if (flags & OVERLAY_CODE) {
                if (char_height(f, yy) > target) {
                   break;
                }
            } else {
                if (char_width(f, yy) > target)
                   break;
            }
            c = yy;
        }
      }
    }
    if (y == null) {
        y = char_box(f, c, node_attr(attr_p)); /* italic gets added to width */
    }
    if (flags & TOP_CODE) {
        if (h < accent_base_height(f)) {
            delta = h;
        } else {
            delta = accent_base_height(f);
        }
    } else if (flags & OVERLAY_CODE) {
        delta = half(height(y) + depth(y) + height(x) + depth(x)); /* center the accent vertically around the accentee */
    } else {
        delta = 0; /* hm */
    }
    if ((supscr(q) != null) || (subscr(q) != null)) {
        if (type(nucleus(q)) == math_char_node) {
            /* swap the subscript and superscript into box |x| */
            flush_node_list(x);
            x = new_noad();
            r = math_clone(nucleus(q));
            nucleus(x) = r;
            supscr(x) = supscr(q);
            supscr(q) = null;
            subscr(x) = subscr(q);
            subscr(q) = null;
            type(nucleus(q)) = sub_mlist_node;
            math_list(nucleus(q)) = x;
            x = clean_box(nucleus(q), cur_style, cur_style);
            delta = delta + height(x) - h;
            h = height(x);
        }
    } else if ((vlink(q) != null) && (type(nucleus(q)) == math_char_node)) {
        /* only pure math char nodes */
        internal_font_number f = fam_fnt(math_fam(nucleus(q)),cur_size);
        if (do_new_math(f)) {
            ic = char_italic(f,math_character(nucleus(q)));
        }
    }
    /* the top accents of both characters are aligned */
    if (s_is_absolute) {
        scaled sa;
        if (ext != NULL) {
            /* if the accent is extensible just take the center */
            sa = half(width(y));
        } else {
            /*
                there is no bot_accent so let's assume similarity

                if (flags & BOT_CODE) {
                    sa = char_bot_accent(f, c);
                } else {
                    sa = char_top_accent(f, c);
                }
            */
            sa = char_top_accent(f, c);
        }
        if (sa == INT_MIN) {
            /* just take the center */
            sa = half(width(y));
        }
        if (math_direction_par == dir_TRT) {
           shift_amount(y) = s + sa - width(y);
        } else {
           shift_amount(y) = s - sa;
        }
    } else {
        if (width(y)== 0) {
            shift_amount(y) = s + w;
        } else if (math_direction_par == dir_TRT) {
            shift_amount(y) = s + width(y); /* ok? */
        } else {
            shift_amount(y) = s + half(w - width(y));
        }
    }
    width(y) = 0;
    if (flags & (TOP_CODE | OVERLAY_CODE)) {
        p = new_kern(-delta);
        reset_attributes(p, node_attr(q));
        couple_nodes(p,x);
        couple_nodes(y,p);
    } else {
        couple_nodes(x,y);
        y = x;
    }
    r = vpackage(y, 0, additional, max_dimen, math_direction_par);
    reset_attributes(r, node_attr(q));
    width(r) = width(x);
    y = r;
    if (flags & (TOP_CODE | OVERLAY_CODE)) {
        if (height(y) < h) {
            /* make the height of box |y| equal to |h| */
            p = new_kern(h - height(y));
            reset_attributes(p, node_attr(q));
            try_couple_nodes(p,list_ptr(y));
            list_ptr(y) = p;
            height(y) = h;
        }
    } else {
        shift_amount(y) = -(h - height(y));
    }
    if (ic != 0) {
        /* old font codepath has ic built in, new font code doesn't */
        width(r) += ic ;
    }
    math_list(nucleus(q)) = y;
    type(nucleus(q)) = sub_box_node;
}

static void make_math_accent(pointer q, int cur_style)
{
    int topstretch = !(subtype(q) % 2);
    int botstretch = !(subtype(q) / 2);

    if (top_accent_chr(q) != null) {
        fetch(top_accent_chr(q));
        if (char_exists(cur_f, cur_c)) {
          do_make_math_accent(q, cur_f, cur_c, TOP_CODE | (topstretch ? STRETCH_ACCENT_CODE : 0), cur_style);
        }
        flush_node(top_accent_chr(q));
        top_accent_chr(q) = null;
    }
    if (bot_accent_chr(q) != null) {
        fetch(bot_accent_chr(q));
        if (char_exists(cur_f, cur_c)) {
          do_make_math_accent(q, cur_f, cur_c, BOT_CODE | (botstretch ? STRETCH_ACCENT_CODE : 0), cur_style);
        }
        flush_node(bot_accent_chr(q));
        bot_accent_chr(q) = null;
    }
    if (overlay_accent_chr(q) != null) {
        fetch(overlay_accent_chr(q));
        if (char_exists(cur_f, cur_c)) {
          do_make_math_accent(q, cur_f, cur_c, OVERLAY_CODE | STRETCH_ACCENT_CODE, cur_style);
        }
        flush_node(overlay_accent_chr(q));
        overlay_accent_chr(q) = null;
    }
}

@ The |make_fraction| procedure is a bit different because it sets
|new_hlist(q)| directly rather than making a sub-box.

@c
static void make_fraction(pointer q, int cur_style)
{
    pointer p, p1, p2, v, x, y, z, l, r, m; /* temporary registers for box construction */
    scaled delta, delta1, delta2, shift_up, shift_down, clr1, clr2;
    /* dimensions for box calculations */
    if (thickness(q) == default_code)
        thickness(q) = fraction_rule(cur_style);
    /*
        Create equal-width boxes |x| and |z| for the numerator and denominator,
        and compute the default amounts |shift_up| and |shift_down| by which they
        are displaced from the baseline
    */

    x = clean_box(numerator(q), num_style(cur_style), cur_style);
    z = clean_box(denominator(q), denom_style(cur_style), cur_style);

    if (middle_delimiter(q) != null) {
        delta = 0;
        m = do_delimiter(q, middle_delimiter(q), cur_size, delta, false, cur_style, true, NULL, NULL);
        middle_delimiter(q) = null;
    } else {
        m = null ;
        if (width(x) < width(z)) {
            x = rebox(x, width(z));
        } else {
            z = rebox(z, width(x));
        }
    }

    if (m != null) {
        shift_up = 0;
        shift_down = 0;
    } else if (thickness(q) == 0) {
        shift_up = stack_num_up(cur_style);
        shift_down = stack_denom_down(cur_style);
        /*
            the numerator and denominator must be separated by a certain minimum
            clearance, called |clr| in the following program. The difference between
            |clr| and the actual clearance is |2delta|.
        */
        clr1 = stack_vgap(cur_style);
        delta = half(clr1 - ((shift_up - depth(x)) - (height(z) - shift_down)));
        if (delta > 0) {
            shift_up = shift_up + delta;
            shift_down = shift_down + delta;
        }
    } else {
        shift_up = fraction_num_up(cur_style);
        shift_down = fraction_denom_down(cur_style);
        /*
            in the case of a fraction line, the minimum clearance depends on the actual
            thickness of the line.
        */
        clr1 = fraction_num_vgap(cur_style);
        clr2 = fraction_denom_vgap(cur_style);
        delta = half(thickness(q));
        if (fractionexact(q)) {
            delta1 = clr1 - ((shift_up   - depth(x) ) - (math_axis_size(cur_size) + delta));
            delta2 = clr2 - ((shift_down - height(z)) + (math_axis_size(cur_size) - delta));
        } else {
            clr1 = ext_xn_over_d(clr1, thickness(q), fraction_rule(cur_style));
            clr2 = ext_xn_over_d(clr2, thickness(q), fraction_rule(cur_style));
            delta1 = clr1 - ((shift_up   - depth(x) ) - (math_axis_size(cur_size) + delta));
            delta2 = clr2 - ((shift_down - height(z)) + (math_axis_size(cur_size) - delta));
        }
        if (delta1 > 0) {
            shift_up = shift_up + delta1;
        }
        if (delta2 > 0) {
            shift_down = shift_down + delta2;
        }
    }
    if (m != null) {
        /*
            construct a hlist box for the fraction, according to |hgap| and |vgap|
        */
        shift_up = skewed_fraction_vgap(cur_style);

        if (!fractionnoaxis(q)) {
            shift_up += half(math_axis_size(cur_size));
        }

        shift_down = shift_up;
        v = new_null_box();
        reset_attributes(v, node_attr(q));
        type(v) = hlist_node;
        list_ptr(v) = x;
        width(v) = width(x);
        height(v) = height(x) + shift_up;
        depth(v) = depth(x);
        shift_amount(v) = - shift_up;
        x = v;

        v = new_null_box();
        reset_attributes(v, node_attr(q));
        type(v) = hlist_node;
        list_ptr(v) = z;
        width(v) = width(z);
        height(v) = height(z);
        depth(v) = depth(z) + shift_down;
        shift_amount(v) = shift_down;
        z = v;

        v = new_null_box();
        reset_attributes(v, node_attr(q));
        type(v) = hlist_node;
        if (height(x) > height(z)) {
            height(v) = height(x);
        } else {
            height(v) = height(z);
        }
        if (depth(x) > depth(z)) {
            depth(v) = depth(x);
        } else {
            depth(v) = depth(z);
        }
        if (height(m) > height(v)) {
            height(v) = height(m);
        }
        if (depth(m) > depth(v)) {
            depth(v) = depth(m);
        }

        if (fractionexact(q)) {
            delta1 = -half(skewed_fraction_hgap(cur_style));
            delta2 = delta1;
            width(v) = width(x) + width(z) + width(m) - skewed_fraction_hgap(cur_style);
        } else {
            delta1 = half(skewed_fraction_hgap(cur_style)-width(m));
            delta2 = half(skewed_fraction_hgap(cur_style)+width(m));
            width(v) = width(x) + width(z) + skewed_fraction_hgap(cur_style);
            width(m) = 0;
        }

        p1 = new_kern(delta1);
        reset_attributes(p1, node_attr(q));
        p2 = new_kern(delta2);
        reset_attributes(p2, node_attr(q));

        couple_nodes(x,p1);
        couple_nodes(p1,m);
        couple_nodes(m,p2);
        couple_nodes(p2,z);

        list_ptr(v) = x;
    } else {
        /*
            construct a vlist box for the fraction, according to |shift_up| and |shift_down|
        */
        v = new_null_box();
        type(v) = vlist_node;
        height(v) = shift_up + height(x);
        depth(v) = depth(z) + shift_down;
        width(v) = width(x); /* this also equals |width(z)| */
        reset_attributes(v, node_attr(q));
        if (thickness(q) == 0) {
            p = new_kern((shift_up - depth(x)) - (height(z) - shift_down));
            couple_nodes(p,z);
        } else {
            y = do_fraction_rule(thickness(q), node_attr(q), math_fraction_rule, cur_size, math_rules_fam_par);
            p = new_kern((math_axis_size(cur_size) - delta) - (height(z) - shift_down));
            reset_attributes(p, node_attr(q));
            couple_nodes(y,p);
            couple_nodes(p,z);
            p = new_kern((shift_up - depth(x)) - (math_axis_size(cur_size) + delta));
            couple_nodes(p,y);
        }
        reset_attributes(p, node_attr(q));
        couple_nodes(x,p);
        list_ptr(v) = x;
    }
    /*
        put the fraction into a box with its delimiters, and make |new_hlist(q)|
        point to it
    */
    if (do_new_math(cur_f)) {
        if (math_use_old_fraction_scaling_par) {
            delta = fraction_del_size_old(cur_style);
        } else {
            delta = fraction_del_size_new(cur_style);
        }
        if (delta == undefined_math_parameter) {
            delta = get_delimiter_height(depth(v), height(v), true);
        }
    } else {
        delta = fraction_del_size_old(cur_style);
    }
    l = do_delimiter(q, left_delimiter(q), cur_size, delta, false, cur_style, true, NULL, NULL);
    left_delimiter(q) = null;
    r = do_delimiter(q, right_delimiter(q), cur_size, delta, false, cur_style, true, NULL, NULL);
    right_delimiter(q) = null;
    couple_nodes(l,v);
    couple_nodes(v,r);
    y = hpack(l, 0, additional, -1);
    reset_attributes(y, node_attr(q));
    assign_new_hlist(q, y);
}

@ If the nucleus of an |op_noad| is a single character, it is to be
centered vertically with respect to the axis, after first being enlarged
(via a character list in the font) if we are in display style.  The normal
convention for placing displayed limits is to put them above and below the
operator in display style.

The italic correction is removed from the character if there is a subscript
and the limits are not being displayed. The |make_op| routine returns the
value that should be used as an offset between subscript and superscript.

After |make_op| has acted, |subtype(q)| will be |limits| if and only if
the limits have been set above and below the operator. In that case,
|new_hlist(q)| will already contain the desired final box.

@c
static void make_scripts(pointer q, pointer p, scaled it, int cur_style, scaled supshift, scaled subshift);
static pointer check_nucleus_complexity(halfword q, scaled * delta, int cur_style);

static scaled make_op(pointer q, int cur_style)
{
    scaled delta = 0;            /* offset between subscript and superscript */
    scaled dummy = 0;
    pointer p, v, x, y, z, n;    /* temporary registers for box construction */
    int c;                       /* register for character examination */
    scaled shift_up, shift_down; /* dimensions for box calculation */
    boolean axis_shift = false;
    scaled ok_size;
    if ((subtype(q) == op_noad_type_normal) && (cur_style < text_style)) {
        subtype(q) = op_noad_type_limits;
    }
    if (type(nucleus(q)) == math_char_node) {
        fetch(nucleus(q));
        if (cur_style < text_style) {
            /* try to make it larger */
            ok_size = minimum_operator_size(cur_style);
            if (ok_size != undefined_math_parameter) {
                /* creating a temporary delimiter is the cleanest way */
                y = new_node(delim_node, 0);
                reset_attributes(y, node_attr(q));
                small_fam(y) = math_fam(nucleus(q));
                small_char(y) = math_character(nucleus(q));
                x = do_delimiter(q, y, text_size, ok_size, false, cur_style, true, NULL, &delta);
                if (do_new_math(cur_f)) {
                    /* we never added italic correction */
                } else if ((subscr(q) != null) && (subtype(q) != op_noad_type_limits)) {
                    /* remove italic correction */
                    width(x) -= delta;
                }
            } else {
                ok_size = height_plus_depth(cur_f, cur_c) + 1;
                while ((char_tag(cur_f, cur_c) == list_tag) && height_plus_depth(cur_f, cur_c) < ok_size) {
                    c = char_remainder(cur_f, cur_c);
                    if (!char_exists(cur_f, c))
                        break;
                    cur_c = c;
                    math_character(nucleus(q)) = c;
                }
                delta = char_italic(cur_f, cur_c);
                x = clean_box(nucleus(q), cur_style, cur_style);
                if (delta != 0) {
                    if (do_new_math(cur_f)) {
                        /* we never added italic correction */
                    } else if ((subscr(q) != null) && (subtype(q) != op_noad_type_limits)) {
                        /* remove italic correction */
                        width(x) = width(x) - delta;
                    }
                }
                axis_shift = true;
            }
        } else {
            /* normal size */
            delta = char_italic(cur_f, cur_c);
            x = clean_box(nucleus(q), cur_style, cur_style);
            if (delta != 0) {
                if (do_new_math(cur_f)) {
                    /* we never added italic correction */
                } else if ((subscr(q) != null) && (subtype(q) != op_noad_type_limits)) {
                    /* remove italic correction */
                    width(x) = width(x) - delta;
                }
            }
            axis_shift = true;
        }
        if (axis_shift) {
            /* center vertically */
            shift_amount(x) = half(height(x) - depth(x)) - math_axis_size(cur_size);
        }
        type(nucleus(q)) = sub_box_node;
        math_list(nucleus(q)) = x;
    }

    /* we now handle op_nod_type_no_limits here too */

    if (subtype(q) == op_noad_type_no_limits) {
        if (do_new_math(cur_f)) {
            /*
                if (delta != 0) {
                    delta = half(delta) ;
                }
            */
            p = check_nucleus_complexity(q, &dummy, cur_style);
            if ((subscr(q) == null) && (supscr(q) == null)) {
                assign_new_hlist(q, p);
            } else {
                /*
                    make_scripts(q, p, 0, cur_style, delta, -delta);
                */
                int mode = nolimits_mode_par; /* wins */
                /*
                    for easy configuration ... fonts are somewhat inconsistent and the
                    values for italic correction run from 30 to 60% of the width
                */
                switch (mode) {
                    case 0 :
                        /* full bottom correction */
                        make_scripts(q, p, 0, cur_style, 0, -delta);
                        break;
                    case 1 :
                        /* MathConstants driven */
                        make_scripts(q, p, 0, cur_style,
                             round_xn_over_d(delta, nolimit_sup_factor(cur_style), 1000),
                            -round_xn_over_d(delta, nolimit_sub_factor(cur_style), 1000));
                    case 2 :
                        /* no correction */
                        make_scripts(q, p, 0, cur_style, 0, 0);
                        break ;
                    case 3 :
                        /* half bottom correction */
                        make_scripts(q, p, 0, cur_style, 0, -half(delta));
                        break;
                    case 4 :
                        /* half bottom and top correction */
                        make_scripts(q, p, 0, cur_style, half(delta), -half(delta));
                        break;
                    default :
                        if (mode > 15) {
                            /* for quickly testing values */
                            make_scripts(q, p, 0, cur_style, 0, -round_xn_over_d(delta, mode, 1000));
                        } else {
                            make_scripts(q, p, 0, cur_style, 0, 0);
                        }
                        break;
                }
            }
            delta = 0;
        } else {
            /* similar code then the caller (before CHECK_DIMENSIONS) */
            p = check_nucleus_complexity(q, &delta, cur_style);
            if ((subscr(q) == null) && (supscr(q) == null)) {
                assign_new_hlist(q, p);
            } else {
                make_scripts(q, p, delta, cur_style, 0, 0);
            }
        }
    } else if (subtype(q) == op_noad_type_limits) {
        /* The following program builds a vlist box |v| for displayed limits. The
           width of the box is not affected by the fact that the limits may be skewed. */
        x = clean_box(supscr(q), sup_style(cur_style), cur_style);
        y = clean_box(nucleus(q), cur_style, cur_style);
        z = clean_box(subscr(q), sub_style(cur_style), cur_style);
        v = new_null_box();
        reset_attributes(v, node_attr(q));
        type(v) = vlist_node;
        if (do_new_math(cur_f)) {
            n = null;
            if (! math_no_italic_compensation_par) {
                n = nucleus(q);
                if (n != null) {
                    if ((type(n) == sub_mlist_node) || (type(n) == sub_box_node)) {
                        n = math_list(n);
                        if (n != null) {
                            if (type(n) == hlist_node) {
                                n = list_ptr(n); /* just a not scaled char */
                                while (n != null) {
                                    if (type(n) == glyph_node) {
                                        delta = char_italic(font(n),character(n));
                                    }
                                    n = vlink(n);
                                }
                            } else {
                                while (n != null) {
                                    if (type(n) == fence_noad) {
                                        if (delimiteritalic(n) > delta) {
                                            /* we can have dummies, the period ones */
                                            delta = delimiteritalic(n);
                                        }
                                    }
                                    n = vlink(n);
                                }
                            }
                        }
                    } else {
                        n = nucleus(q);
                        if (type(n) == math_char_node) {
                            delta = char_italic(fam_fnt(math_fam(n),cur_size),math_character(n));
                        }
                    }
                }
            }
        }
        width(v) = width(y);
        if (width(x) > width(v))
            width(v) = width(x);
        if (width(z) > width(v))
            width(v) = width(z);
        x = rebox(x, width(v));
        y = rebox(y, width(v));
        z = rebox(z, width(v));
        shift_amount(x) = half(delta);
        shift_amount(z) = -shift_amount(x);
        /* v is the still empty target */
        height(v) = height(y);
        depth(v) = depth(y);
        /*
            attach the limits to |y| and adjust |height(v)|, |depth(v)| to
            account for their presence

            we use |shift_up| and |shift_down| in the following program for the
            amount of glue between the displayed operator |y| and its limits |x| and
            |z|

            the vlist inside box |v| will consist of |x| followed by |y| followed
            by |z|, with kern nodes for the spaces between and around them

            b: baseline  v: minumum gap
        */

        if (supscr(q) == null) {
            list_ptr(x) = null;
            flush_node(x);
            list_ptr(v) = y;
        } else {
            shift_up = limit_above_bgap(cur_style) - depth(x);
            if (shift_up < limit_above_vgap(cur_style))
                shift_up = limit_above_vgap(cur_style);
            p = new_kern(shift_up);
            reset_attributes(p, node_attr(q));
            couple_nodes(p,y);
            couple_nodes(x,p);
            p = new_kern(limit_above_kern(cur_style));
            reset_attributes(p, node_attr(q));
            couple_nodes(p,x);
            list_ptr(v) = p;
            height(v) = height(v) + limit_above_kern(cur_style) + height(x) + depth(x) + shift_up;
        }
        if (subscr(q) == null) {
            list_ptr(z) = null;
            flush_node(z);
        } else {
            shift_down = limit_below_bgap(cur_style) - height(z);
            if (shift_down < limit_below_vgap(cur_style))
                shift_down = limit_below_vgap(cur_style);
            if (shift_down > 0) {
                p = new_kern(shift_down);
                reset_attributes(p, node_attr(q));
                couple_nodes(y,p);
                couple_nodes(p,z);
            }
            p = new_kern(limit_below_kern(cur_style));
            reset_attributes(p, node_attr(q));
            couple_nodes(z,p);
            depth(v) = depth(v) + limit_below_kern(cur_style) + height(z) + depth(z) + shift_down;
        }
        if (subscr(q) != null) {
            math_list(subscr(q)) = null;
            flush_node(subscr(q));
            subscr(q) = null;
        }
        if (supscr(q) != null) {
            math_list(supscr(q)) = null;
            flush_node(supscr(q));
            supscr(q) = null;
        }
        assign_new_hlist(q, v);
        if (do_new_math(cur_f)) {
            delta = 0;
        }
    }
    return delta;
}

@ A ligature found in a math formula does not create a ligature, because
there is no question of hyphenation afterwards; the ligature will simply be
stored in an ordinary |glyph_node|, after residing in an |ord_noad|.

The |type| is converted to |math_text_char| here if we would not want to
apply an italic correction to the current character unless it belongs
to a math font (i.e., a font with |space=0|).

No boundary characters enter into these ligatures.

@c
#define simple_char_noad(p) (\
    (p != null) && \
    (type(p) == simple_noad) && \
    (subtype(p) <= punct_noad_type) && \
    (type(nucleus(p)) == math_char_node) \
)

#define same_nucleus_fam(p,q) \
    (math_fam(nucleus(p)) == math_fam(nucleus(q)))

static void make_ord(pointer q)
{
    int a;           /* the left-side character for lig/kern testing */
    pointer p, r, s; /* temporary registers for list manipulation */
    scaled k;        /* a kern */
    liginfo lig;     /* a ligature */
  RESTART:
    if (subscr(q) == null && supscr(q) == null && type(nucleus(q)) == math_char_node) {
        p = vlink(q);
        if (simple_char_noad(p) && same_nucleus_fam(p,q)) {
            type(nucleus(q)) = math_text_char_node;
            fetch(nucleus(q));
            a = cur_c;
            /* add italic correction */
            if (do_new_math(cur_f) && (char_italic(cur_f,math_character(nucleus(q))) != 0)) {
                p = new_kern(char_italic(cur_f,math_character(nucleus(q))));
                reset_attributes(p, node_attr(q));
                couple_nodes(p,vlink(q));
                couple_nodes(q,p);
                return;
            }
            /* construct ligatures, quite unlikely in new math fonts */
            if ((has_kern(cur_f, a)) || (has_lig(cur_f, a))) {
                cur_c = math_character(nucleus(p));
                /*
                    if character |a| has a kern with |cur_c|, attach the kern after~|q|; or if
                    it has a ligature with |cur_c|, combine noads |q| and~|p| appropriately;
                    then |return| if the cursor has moved past a noad, or |goto restart|

                    note that a ligature between an |ord_noad| and another kind of noad
                    is replaced by an |ord_noad|, when the two noads collapse into one

                    we could make a parenthesis (say) change shape when it follows
                    certain letters. Presumably a font designer will define such
                    ligatures only when this convention makes sense
                */

                if (disable_lig_par == 0 && has_lig(cur_f, a)) {
                    lig = get_ligature(cur_f, a, cur_c);
                    if (is_valid_ligature(lig)) {
                        check_interrupt();      /* allow a way out of infinite ligature loop */
                        switch (lig_type(lig)) {
                        case 1:
                            /* \.{=:\char`\|} */
                        case 5:
                            /* \.{=:\char`\|>} */
                            math_character(nucleus(q)) = lig_replacement(lig);
                            break;
                        case 2:
                            /* \.{\char`\|=:} */
                        case 6:
                            /* \.{\char`\|=:>} */
                            math_character(nucleus(p)) = lig_replacement(lig);
                            break;
                        case 3:
                             /* \.{\char`\|=:\char`\|} */
                        case 7:
                             /* \.{\char`\|=:\char`\|>} */
                        case 11:
                             /* \.{\char`\|=:\char`\|>>} */
                            r = new_noad();
                            reset_attributes(r, node_attr(q));
                            s = new_node(math_char_node, 0);
                            reset_attributes(s, node_attr(q));
                            nucleus(r) = s;
                            math_character(nucleus(r)) = lig_replacement(lig);
                            math_fam(nucleus(r)) = math_fam(nucleus(q));
                            couple_nodes(q,r);
                            couple_nodes(r,p);
                            if (lig_type(lig) < 11)
                                type(nucleus(r)) = math_char_node;
                            else
                                /* prevent combination */
                                type(nucleus(r)) = math_text_char_node;
                            break;
                        default:
                            try_couple_nodes(q,vlink(p));
                            math_character(nucleus(q)) = lig_replacement(lig); /* \.{=:} */
                            s = math_clone(subscr(p));
                            subscr(q) = s;
                            s = math_clone(supscr(p));
                            supscr(q) = s;
                            math_reset(subscr(p)); /* just in case */
                            math_reset(supscr(p));
                            flush_node(p);
                            break;
                        }
                        if (lig_type(lig) > 3)
                            return;
                        type(nucleus(q)) = math_char_node;
                        goto RESTART;
                    }
                }
                if (disable_kern_par == 0 && has_kern(cur_f, a)) {
                    /* todo: should this use mathkerns? */
                    k = get_kern(cur_f, a, cur_c);
                    if (k != 0) {
                        p = new_kern(k);
                        reset_attributes(p, node_attr(q));
                        couple_nodes(p,vlink(q));
                        couple_nodes(q,p);
                        return;
                    }
                }
            }
        }
    }
}

@ If the fonts for the left and right bits of a mathkern are not
both new-style fonts, then return a sentinel value meaning:
please use old-style italic correction placement

@c
#define MATH_KERN_NOT_FOUND 0x7FFFFFFF

@ This function tries to find the kern needed for proper cut-ins.
The left side doesn't move, but the right side does, so the first
order of business is to create a staggered fence line on the
left side of the right character.

The microsoft spec says that there are four quadrants, but the
actual images say

@c
static scaled math_kern_at(internal_font_number f, int c, int side, int v)
{
    int h, k, numkerns;
    scaled *kerns_heights;
    scaled kern = 0;
    charinfo *co = char_info(f, c); /* known to exist */
    numkerns = get_charinfo_math_kerns(co, side);
#ifdef DEBUG
    fprintf(stderr, "  entries = %d, height = %d\n", numkerns, v);
#endif
    if (numkerns == 0)
        return kern;
    if (side == top_left_kern) {
        kerns_heights = co->top_left_math_kern_array;
    } else if (side == bottom_left_kern) {
        kerns_heights = co->bottom_left_math_kern_array;
    } else if (side == top_right_kern) {
        kerns_heights = co->top_right_math_kern_array;
    } else if (side == bottom_right_kern) {
        kerns_heights = co->bottom_right_math_kern_array;
    } else {
        confusion("math_kern_at");
        kerns_heights = NULL;   /* not reached */
    }
#ifdef DEBUG
    fprintf(stderr, "   entry 0: %d,%d\n", kerns_heights[0], kerns_heights[1]);
#endif
    if (v < kerns_heights[0])
        return kerns_heights[1];
    for (k = 0; k < numkerns; k++) {
        h = kerns_heights[(k * 2)];
        kern = kerns_heights[(k * 2) + 1];
#ifdef DEBUG
        if (k > 0)
            fprintf(stderr, "   entry %d: %d,%d\n", k, h, kern);
#endif
        if (h > v) {
            return kern;
        }
    }
    return kern;
}

@ @c
static scaled find_math_kern(internal_font_number l_f, int l_c,
                             internal_font_number r_f, int r_c,
                             int cmd, scaled shift)
{
    scaled corr_height_top = 0, corr_height_bot = 0;
    scaled krn_l = 0, krn_r = 0, krn = 0;
    if ((!do_new_math(l_f)) || (!do_new_math(r_f)) || (!char_exists(l_f, l_c)) || (!char_exists(r_f, r_c)))
        return MATH_KERN_NOT_FOUND;

    if (cmd == sup_mark_cmd) {
        corr_height_top = char_height(l_f, l_c);
        corr_height_bot = -char_depth(r_f, r_c) + shift; /* bottom of superscript */
        krn_l = math_kern_at(l_f, l_c, top_right_kern, corr_height_top);
        krn_r = math_kern_at(r_f, r_c, bottom_left_kern, corr_height_top);
#ifdef DEBUG
        fprintf(stderr, "SUPER Top LR = %d,%d (shift %d)\n", krn_l, krn_r, shift);
#endif
        krn = (krn_l + krn_r);
        krn_l = math_kern_at(l_f, l_c, top_right_kern, corr_height_bot);
        krn_r = math_kern_at(r_f, r_c, bottom_left_kern, corr_height_bot);
#ifdef DEBUG
        fprintf(stderr, "SUPER Bot LR = %d,%d\n", krn_l, krn_r);
#endif
        if ((krn_l + krn_r) < krn)
            krn = (krn_l + krn_r);
        return (krn);

    } else if (cmd == sub_mark_cmd) {
        corr_height_top = char_height(r_f, r_c) - shift; /* top of subscript */
        corr_height_bot = -char_depth(l_f, l_c);
        krn_l = math_kern_at(l_f, l_c, bottom_right_kern, corr_height_top);
        krn_r = math_kern_at(r_f, r_c, top_left_kern, corr_height_top);
#ifdef DEBUG
        fprintf(stderr, "SUB Top LR = %d,%d\n", krn_l, krn_r);
#endif
        krn = (krn_l + krn_r);
        krn_l = math_kern_at(l_f, l_c, bottom_right_kern, corr_height_bot);
        krn_r = math_kern_at(r_f, r_c, top_left_kern, corr_height_bot);
#ifdef DEBUG
        fprintf(stderr, "SUB Bot LR = %d,%d\n", krn_l, krn_r);
#endif
        if ((krn_l + krn_r) < krn)
            krn = (krn_l + krn_r);
        return (krn);

    } else {
        confusion("find_math_kern");
    }
    return 0; /* not reached */
}

@ just a small helper
@c
static pointer attach_hkern_to_new_hlist(pointer q, scaled delta2)
{
    pointer y;
    pointer z = new_kern(delta2);
    reset_attributes(z, node_attr(q));
    if (new_hlist(q) == null) {
        /* this is somewhat weird */
        new_hlist(q) = z;
    } else {
        y = new_hlist(q);
        while (vlink(y) != null)
            y = vlink(y);
        couple_nodes(y,z);
    }
    return new_hlist(q);
}

@
@c
#ifdef DEBUG
void dump_simple_field(pointer q)
{
    pointer p;
    printf("   [%d,  type=%d, vlink=%d] ", q, type(q), vlink(q));
    switch (type(q)) {
    case math_char_node:
        printf("mathchar ");
        break;
    case math_text_char_node:
        printf("texchar ");
        break;
    case sub_box_node:
        printf("box ");
        break;
    case sub_mlist_node:
        printf("mlist ");
        p = math_list(q);
        while (p != null) {
            dump_simple_field(p);
            p = vlink(p);
        }
        break;
    }
}

void dump_simple_node(pointer q)
{
    printf("node %d, type=%d, vlink=%d\n", q, type(q), vlink(q));
    printf("nucleus: ");
    dump_simple_field(nucleus(q));
    printf("\n");
    printf("sub: ");
    dump_simple_field(subscr(q));
    printf("\n");
    printf("sup: ");
    dump_simple_field(supscr(q));
    printf("\n\n");
}
#endif

@ The purpose of |make_scripts(q,it)| is to attach the subscript and/or
superscript of noad |q| to the list that starts at |new_hlist(q)|,
given that subscript and superscript aren't both empty. The superscript
will be horizontally shifted over |delta1|, the subscript over |delta2|.

We set |shift_down| and |shift_up| to the minimum amounts to shift the
baseline of subscripts and superscripts based on the given nucleus.

Note: We need to look at a character but also at the first one in a sub list
and there we ignore leading kerns and glue. Elsewhere is code that removes
kerns assuming that is italic correction. The heuristics are unreliable for
the new fonts so eventualy there will be an option to ignore such corrections.

@ @c
#define analyze_script(init,su_n,su_f,su_c) do {                                 \
    su_n = init;                                                                 \
    if (su_n != null) {                                                          \
        if (type(su_n) == sub_mlist_node && math_list(su_n)) {                   \
            su_n = math_list(su_n);                                              \
            if (su_n != null) {                                                  \
                while (su_n) {                                                   \
                    if ((type(su_n) == kern_node) || (type(su_n) == glue_node)) {\
                        su_n = vlink(su_n);                                      \
                    } else if (type(su_n) == simple_noad) {                      \
                        su_n = nucleus(su_n);                                    \
                        if (type(su_n) != math_char_node) {                      \
                            su_n = null;                                         \
                        }                                                        \
                        break;                                                   \
                    } else {                                                     \
                        su_n = null;                                             \
                        break;                                                   \
                    }                                                            \
                }                                                                \
            }                                                                    \
        }                                                                        \
        if ((su_n != null) && (type(su_n) == math_char_node)) {                  \
            fetch(su_n);                                                         \
            if (char_exists(cur_f, cur_c)) {                                     \
                su_f = cur_f;                                                    \
                su_c = cur_c;                                                    \
            } else {                                                             \
                su_n = null;                                                     \
            }                                                                    \
        }                                                                        \
    }                                                                            \
  } while (0)

static void make_scripts(pointer q, pointer p, scaled it, int cur_style, scaled supshift, scaled subshift)
{
    pointer x, y, z;                  /* temporary registers for box construction */
    scaled shift_up, shift_down, clr; /* dimensions in the calculation */
    scaled delta1, delta2;
    halfword sub_n, sup_n;
    internal_font_number sub_f, sup_f;
    int sub_c, sup_c;
    sub_n = null;
    sup_n = null;
    sub_f = 0;
    sup_f = 0;
    sub_c = 0;
    sup_c = 0;
    delta1 = it;
    delta2 = 0;

#ifdef DEBUG
    printf("it: %d\n", it);
    dump_simple_node(q);
    printf("p: node %d, type=%d, subtype=%d\n", p, type(p), subtype(p));
#endif
    switch (type(nucleus(q))) {
        case math_char_node:
        case math_text_char_node:
            if ((subscr(q) == null) && (delta1 != 0)) {
                /* todo: selective */
                x = new_kern(delta1); /* italic correction */
                reset_attributes(x, node_attr(nucleus(q)));
                couple_nodes(p,x);
                delta1 = 0;
            }
    }
    assign_new_hlist(q, p);
    if (is_char_node(p)) {
        shift_up = 0;
        shift_down = 0;
    } else {
        z = hpack(p, 0, additional, -1);
        shift_up = height(z) - sup_shift_drop(cur_style);  /* r18 */
        shift_down = depth(z) + sub_shift_drop(cur_style); /* r19 */
        list_ptr(z) = null;
        flush_node(z);
    }

    if (is_char_node(p)) {
        /* we look at the subscript character (_i) or first character in a list (_{ij}) */
        analyze_script(subscr(q),sub_n,sub_f,sub_c);
        /* we look at the superscript character (^i) or first character in a list (^{ij}) */
        analyze_script(supscr(q),sup_n,sup_f,sup_c);
    }

    if (supscr(q) == null) {
        /*
            construct a subscript box |x| when there is no superscript

            when there is a subscript without a superscript, the top of the subscript
            should not exceed the baseline plus four-fifths of the x-height.
        */
        x = clean_box(subscr(q), sub_style(cur_style), cur_style);
        width(x) = width(x) + space_after_script(cur_style);
        switch (scripts_mode_par) {
            case 1:
                shift_down = sub_shift_down(cur_style) ;
                break;
            case 2:
                shift_down = sub_sup_shift_down(cur_style) ;
                break;
            case 3:
                shift_down = sub_sup_shift_down(cur_style) ;
                break;
            case 4:
                shift_down = sub_shift_down(cur_style) + half(sub_sup_shift_down(cur_style)-sub_shift_down(cur_style)) ;
                break;
            case 5:
                shift_down = sub_shift_down(cur_style) ;
                break;
            default:
                if (shift_down < sub_shift_down(cur_style))
                    shift_down = sub_shift_down(cur_style);
                clr = height(x) - sub_top_max(cur_style);
                if (shift_down < clr)
                    shift_down = clr;
                break;
        }
        shift_amount(x) = shift_down;

        /* now find and correct for horizontal shift */
        if (sub_n != null) {
            delta2 = find_math_kern(font(p), character(p),sub_f,sub_c,sub_mark_cmd, shift_down);
            if (delta2 == MATH_KERN_NOT_FOUND) {
                delta2 = subshift ;
            } else {
                delta2 = delta2 + subshift ;
            }
        } else {
            delta2 = subshift ;
        }
        if (delta2 != 0) {
            p = attach_hkern_to_new_hlist(q, delta2);
        }

    } else {
        /*
            construct a superscript box |x|

            the bottom of a superscript should never descend below the baseline plus
            one-fourth of the x-height.
        */
        x = clean_box(supscr(q), sup_style(cur_style), cur_style);
        width(x) = width(x) + space_after_script(cur_style);
        switch (scripts_mode_par) {
            case 1:
                shift_up = sup_shift_up(cur_style);
                break;
            case 2:
                shift_up = sup_shift_up(cur_style) ;
                break;
            case 3:
                shift_up = sup_shift_up(cur_style) + sub_sup_shift_down(cur_style) - sub_shift_down(cur_style) ;
                break;
            case 4:
                shift_up = sup_shift_up(cur_style) + half(sub_sup_shift_down(cur_style)-sub_shift_down(cur_style)) ;
                break;
            case 5:
                shift_up = sup_shift_up(cur_style) + sub_sup_shift_down(cur_style)-sub_shift_down(cur_style) ;
                break;
            default:
                clr = sup_shift_up(cur_style);
                if (shift_up < clr)
                    shift_up = clr;
                clr = depth(x) + sup_bottom_min(cur_style);
                if (shift_up < clr)
                    shift_up = clr;
                break;
        }
        if (subscr(q) == null) {
            shift_amount(x) = -shift_up;
            /* now find and correct for horizontal shift */
            if (sup_n != null) {
                clr = find_math_kern(font(p),character(p),sup_f,sup_c,sup_mark_cmd,shift_up);
                if (clr == MATH_KERN_NOT_FOUND) {
                    clr = supshift ;
                } else {
                    clr = clr + supshift ;
                }
            } else {
                clr = supshift;
            }
            if (clr != 0) {
                p = attach_hkern_to_new_hlist(q, clr);
            }
        } else {
            /*
                construct a sub/superscript combination box |x|, with the superscript offset
                by |delta|

                when both subscript and superscript are present, the subscript must be
                separated from the superscript by at least four times |default_rule_thickness|

                if this condition would be violated, the subscript moves down, after which
                both subscript and superscript move up so that the bottom of the superscript
                is at least as high as the baseline plus four-fifths of the x-height
            */
            y = clean_box(subscr(q), sub_style(cur_style), cur_style);
            width(y) = width(y) + space_after_script(cur_style);
            switch (scripts_mode_par) {
                case 1:
                    shift_down = sub_shift_down(cur_style) ;
                    break;
                case 2:
                    shift_down = sub_sup_shift_down(cur_style) ;
                    break;
                case 3:
                    shift_down = sub_sup_shift_down(cur_style) ;
                    break;
                case 4:
                    shift_down = sub_shift_down(cur_style) + half(sub_sup_shift_down(cur_style)-sub_shift_down(cur_style)) ;
                    break;
                case 5:
                    shift_down = sub_shift_down(cur_style) ;
                    break;
                default:
                    if (shift_down < sub_sup_shift_down(cur_style))
                        shift_down = sub_sup_shift_down(cur_style);
                    clr = subsup_vgap(cur_style) - ((shift_up - depth(x)) - (height(y) - shift_down));
                    if (clr > 0) {
                        shift_down = shift_down + clr;
                        clr = sup_sub_bottom_max(cur_style) - (shift_up - depth(x));
                        if (clr > 0) {
                            shift_up = shift_up + clr;
                            shift_down = shift_down - clr;
                        }
                    }
                break;
            }
            /* now find and correct for horizontal shift */
            if (sub_n != null) {
                delta2 = find_math_kern(font(p), character(p),sub_f,sub_c,sub_mark_cmd, shift_down);
                if (delta2 == MATH_KERN_NOT_FOUND) {
                    delta2 = subshift ;
                } else {
                    delta2 = delta2 + subshift ;
                }
            } else {
                delta2 = subshift ;
            }
            if (delta2 != 0) {
                p = attach_hkern_to_new_hlist(q, delta2);
            }
            /*
                now the horizontal shift for the superscript; the superscript is also to be shifted
                by |delta1| (the italic correction)
            */
            clr = MATH_KERN_NOT_FOUND;
            if (sup_n != null) {
                clr = find_math_kern(font(p),character(p),sup_f,sup_c,sup_mark_cmd,shift_up);
            }

            /* delta can already have been applied and now be 0 */
            if (delta2 == MATH_KERN_NOT_FOUND)
                delta2 = - supshift ;
            else
                delta2 = delta2 - supshift ;
            if (clr != MATH_KERN_NOT_FOUND) {
                shift_amount(x) = clr + delta1 - delta2;
            } else {
                shift_amount(x) = delta1 - delta2;
            }
            /* todo: only if kern != 0 */
            p = new_kern((shift_up - depth(x)) - (height(y) - shift_down));
            reset_attributes(p, node_attr(q));
            couple_nodes(x,p);
            couple_nodes(p,y);
            /* we end up with funny dimensions */
            x = vpackage(x, 0, additional, max_dimen, math_direction_par);
            reset_attributes(x, node_attr(q));
            shift_amount(x) = shift_down;
        }
    }

    if (new_hlist(q) == null) {
        new_hlist(q) = x;
    } else {
        p = new_hlist(q);
        while (vlink(p) != null)
            p = vlink(p);
        couple_nodes(p,x);
    }
    if (subscr(q) != null) {
        math_list(subscr(q)) = null;
        flush_node(subscr(q));
        subscr(q) = null;
    }
    if (supscr(q) != null) {
        math_list(supscr(q)) = null;
        flush_node(supscr(q));
        supscr(q) = null;
    }
}

@ The |make_left_right| function constructs a left or right delimiter of
the required size and returns the value |open_noad| or |close_noad|. The
|left_noad_side| and |right_noad_side| will both be based on the original |style|,
so they will have consistent sizes.

@c
static small_number make_left_right(pointer q, int style, scaled max_d, scaled max_h)
{
    scaled delta;
    pointer tmp, lst;
    scaled hd_asked = 0;
    scaled ic = 0;
    boolean stack = false;
    boolean axis = false;
    /*
        scaled hd_done = 0;
        boolean fitting = true;
        boolean fence = false;
        int chr = 0;
        int cls = 0;
    */
    setup_cur_size(style);

    if ((delimiterheight(q)!=0) || (delimiterdepth(q)!=0)) {

        hd_asked = delimiterheight(q) + delimiterdepth(q);
        tmp = do_delimiter(q, delimiter(q), cur_size, hd_asked, false, style, false, &stack, &ic);
        delimiteritalic(q) = ic;

        /* beware, a stacked delimiter has a shift but no corrected height/depth (yet) */

        if (stack) {
            shift_amount(tmp) = delimiterdepth(q);
        }

        /*
            hd_done = height(tmp) + depth(tmp);
            fitting = stack || ((hd_done-hd_asked) == 0);

            if (type(delimiter(q)) == delim_node && (small_char(delimiter(q)) != 0)) {
                chr = small_char(delimiter(q));
                cls = get_math_code(chr).class_value ;
                fence = (cls == 4) || (cls == 5) ;
            }

            printf("delimiter stack %i fence %i fitting %i\n",stack,fence,fitting);
        */

        if (delimiterexact(q)) {
            delimiterheight(q) = height(tmp) - shift_amount(tmp);
            delimiterdepth(q)  = depth(tmp)  + shift_amount(tmp);
        }
        if (delimiteraxis(q)) {
            delimiterheight(q) += math_axis_size(cur_size);
            delimiterdepth(q)  -= math_axis_size(cur_size);
            shift_amount(tmp)  -= math_axis_size(cur_size);
        }
        lst = new_node(hlist_node,0);
        reset_attributes(lst, node_attr(q));
        box_dir(lst) = dir_TLT ;
        height(lst) = delimiterheight(q);
        depth(lst) = delimiterdepth(q);
        width(lst) = width(tmp);
        list_ptr(lst) = tmp;
        tmp = lst ;
    } else {
        axis = ! delimiternoaxis(q);
        delta = get_delimiter_height(max_d,max_h,axis);
        tmp = do_delimiter(q, delimiter(q), cur_size, delta, false, style, axis, &stack, &ic);
        delimiteritalic(q) = ic;
    }
    delimiter(q) = null;
    assign_new_hlist(q, tmp);
    if (delimiterclass(q) >= ord_noad_type) {
        if (delimiterclass(q) <= inner_noad_type) {
            return delimiterclass(q);
        } else {
            return ord_noad_type;
        }
    } else if (subtype(q) == no_noad_side) {
        return open_noad_type;
    } else if (subtype(q) == left_noad_side) {
        return open_noad_type;
    } else {
        return close_noad_type;
    }
}

@ @c
#define TEXT_STYLES(A,B) do {					\
    def_math_param(A,display_style,(B),level_one);		\
    def_math_param(A,cramped_display_style,(B),level_one);	\
    def_math_param(A,text_style,(B),level_one);			\
    def_math_param(A,cramped_text_style,(B),level_one);		\
  } while (0)

#define SCRIPT_STYLES(A,B) do {						\
    def_math_param(A,script_style,(B),level_one);			\
    def_math_param(A,cramped_script_style,(B),level_one);		\
    def_math_param(A,script_script_style,(B),level_one);		\
    def_math_param(A,cramped_script_script_style,(B),level_one);	\
  } while (0)

#define ALL_STYLES(A,B) do {			\
    TEXT_STYLES(A,(B));				\
    SCRIPT_STYLES(A,(B));			\
  } while (0)

#define SPLIT_STYLES(A,B,C) do {		\
    TEXT_STYLES(A,(B));				\
    SCRIPT_STYLES(A,(C));			\
  } while (0)


void initialize_math_spacing(void)
{
    /* *INDENT-OFF* */
    ALL_STYLES   (math_param_ord_ord_spacing,     0);
    ALL_STYLES   (math_param_ord_op_spacing,      thin_mu_skip_code);
    SPLIT_STYLES (math_param_ord_bin_spacing,     med_mu_skip_code, 0);
    SPLIT_STYLES (math_param_ord_rel_spacing,     thick_mu_skip_code, 0);
    ALL_STYLES   (math_param_ord_open_spacing,    0);
    ALL_STYLES   (math_param_ord_close_spacing,   0);
    ALL_STYLES   (math_param_ord_punct_spacing,   0);
    SPLIT_STYLES (math_param_ord_inner_spacing,   thin_mu_skip_code, 0);

    ALL_STYLES   (math_param_op_ord_spacing,      thin_mu_skip_code);
    ALL_STYLES   (math_param_op_op_spacing,       thin_mu_skip_code);
    ALL_STYLES   (math_param_op_bin_spacing,      0);
    SPLIT_STYLES (math_param_op_rel_spacing,      thick_mu_skip_code, 0);
    ALL_STYLES   (math_param_op_open_spacing,     0);
    ALL_STYLES   (math_param_op_close_spacing,    0);
    ALL_STYLES   (math_param_op_punct_spacing,    0);
    SPLIT_STYLES (math_param_op_inner_spacing,    thin_mu_skip_code, 0);

    SPLIT_STYLES (math_param_bin_ord_spacing,     med_mu_skip_code, 0);
    SPLIT_STYLES (math_param_bin_op_spacing,      med_mu_skip_code, 0);
    ALL_STYLES   (math_param_bin_bin_spacing,     0);
    ALL_STYLES   (math_param_bin_rel_spacing,     0);
    SPLIT_STYLES (math_param_bin_open_spacing,    med_mu_skip_code, 0);
    ALL_STYLES   (math_param_bin_close_spacing,   0);
    ALL_STYLES   (math_param_bin_punct_spacing,   0);
    SPLIT_STYLES (math_param_bin_inner_spacing,   med_mu_skip_code, 0);

    SPLIT_STYLES (math_param_rel_ord_spacing,     thick_mu_skip_code, 0);
    SPLIT_STYLES (math_param_rel_op_spacing,      thick_mu_skip_code, 0);
    ALL_STYLES   (math_param_rel_bin_spacing,     0);
    ALL_STYLES   (math_param_rel_rel_spacing,     0);
    SPLIT_STYLES (math_param_rel_open_spacing,    thick_mu_skip_code, 0);
    ALL_STYLES   (math_param_rel_close_spacing,   0);
    ALL_STYLES   (math_param_rel_punct_spacing,   0);
    SPLIT_STYLES (math_param_rel_inner_spacing,   thick_mu_skip_code, 0);

    ALL_STYLES   (math_param_open_ord_spacing,    0);
    ALL_STYLES   (math_param_open_op_spacing,     0);
    ALL_STYLES   (math_param_open_bin_spacing,    0);
    ALL_STYLES   (math_param_open_rel_spacing,    0);
    ALL_STYLES   (math_param_open_open_spacing,   0);
    ALL_STYLES   (math_param_open_close_spacing,  0);
    ALL_STYLES   (math_param_open_punct_spacing,  0);
    ALL_STYLES   (math_param_open_inner_spacing,  0);

    ALL_STYLES   (math_param_close_ord_spacing,   0);
    ALL_STYLES   (math_param_close_op_spacing,    thin_mu_skip_code);
    SPLIT_STYLES (math_param_close_bin_spacing,   med_mu_skip_code, 0);
    SPLIT_STYLES (math_param_close_rel_spacing,   thick_mu_skip_code, 0);
    ALL_STYLES   (math_param_close_open_spacing,  0);
    ALL_STYLES   (math_param_close_close_spacing, 0);
    ALL_STYLES   (math_param_close_punct_spacing, 0);
    SPLIT_STYLES (math_param_close_inner_spacing, thin_mu_skip_code, 0);

    SPLIT_STYLES (math_param_punct_ord_spacing,   thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_punct_op_spacing,    thin_mu_skip_code, 0);
    ALL_STYLES   (math_param_punct_bin_spacing,   0);
    SPLIT_STYLES (math_param_punct_rel_spacing,   thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_punct_open_spacing,  thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_punct_close_spacing, thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_punct_punct_spacing, thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_punct_inner_spacing, thin_mu_skip_code, 0);

    SPLIT_STYLES (math_param_inner_ord_spacing,   thin_mu_skip_code, 0);
    ALL_STYLES   (math_param_inner_op_spacing,    thin_mu_skip_code);
    SPLIT_STYLES (math_param_inner_bin_spacing,   med_mu_skip_code, 0);
    SPLIT_STYLES (math_param_inner_rel_spacing,   thick_mu_skip_code, 0);
    SPLIT_STYLES (math_param_inner_open_spacing,  thin_mu_skip_code, 0);
    ALL_STYLES   (math_param_inner_close_spacing, 0);
    SPLIT_STYLES (math_param_inner_punct_spacing, thin_mu_skip_code, 0);
    SPLIT_STYLES (math_param_inner_inner_spacing, thin_mu_skip_code, 0);
    /* *INDENT-ON* */
}

@ @c
#define both_types(A,B) ((A)*16+(B))

static pointer math_spacing_glue(int l_type, int r_type, int mstyle, scaled mmu)
{
    int x = -1;
    pointer z = null;
    if (l_type == op_noad_type_limits || l_type == op_noad_type_no_limits)
        l_type = op_noad_type_normal;
    if (r_type == op_noad_type_limits || r_type == op_noad_type_no_limits)
        r_type = op_noad_type_normal;
    switch (both_types(l_type, r_type)) {
    /* *INDENT-OFF* */
    case both_types(ord_noad_type,       ord_noad_type      ): x = get_math_param(math_param_ord_ord_spacing,mstyle); break;
    case both_types(ord_noad_type,       op_noad_type_normal): x = get_math_param(math_param_ord_op_spacing,mstyle); break;
    case both_types(ord_noad_type,       bin_noad_type      ): x = get_math_param(math_param_ord_bin_spacing,mstyle); break;
    case both_types(ord_noad_type,       rel_noad_type      ): x = get_math_param(math_param_ord_rel_spacing,mstyle); break;
    case both_types(ord_noad_type,       open_noad_type     ): x = get_math_param(math_param_ord_open_spacing,mstyle); break;
    case both_types(ord_noad_type,       close_noad_type    ): x = get_math_param(math_param_ord_close_spacing,mstyle); break;
    case both_types(ord_noad_type,       punct_noad_type    ): x = get_math_param(math_param_ord_punct_spacing,mstyle); break;
    case both_types(ord_noad_type,       inner_noad_type    ): x = get_math_param(math_param_ord_inner_spacing,mstyle); break;
    case both_types(op_noad_type_normal, ord_noad_type      ): x = get_math_param(math_param_op_ord_spacing,mstyle); break;
    case both_types(op_noad_type_normal, op_noad_type_normal): x = get_math_param(math_param_op_op_spacing,mstyle); break;
#if 0
    case both_types(op_noad_type_normal, bin_noad_type      ): x = get_math_param(math_param_op_bin_spacing,mstyle); break;
#endif
    case both_types(op_noad_type_normal, rel_noad_type      ): x = get_math_param(math_param_op_rel_spacing,mstyle); break;
    case both_types(op_noad_type_normal, open_noad_type     ): x = get_math_param(math_param_op_open_spacing,mstyle); break;
    case both_types(op_noad_type_normal, close_noad_type    ): x = get_math_param(math_param_op_close_spacing,mstyle); break;
    case both_types(op_noad_type_normal, punct_noad_type    ): x = get_math_param(math_param_op_punct_spacing,mstyle); break;
    case both_types(op_noad_type_normal, inner_noad_type    ): x = get_math_param(math_param_op_inner_spacing,mstyle); break;
    case both_types(bin_noad_type,       ord_noad_type      ): x = get_math_param(math_param_bin_ord_spacing,mstyle); break;
    case both_types(bin_noad_type,       op_noad_type_normal): x = get_math_param(math_param_bin_op_spacing,mstyle); break;
#if 0
    case both_types(bin_noad_type,       bin_noad_type      ): x = get_math_param(math_param_bin_bin_spacing,mstyle); break;
    case both_types(bin_noad_type,       rel_noad_type      ): x = get_math_param(math_param_bin_rel_spacing,mstyle); break;
#endif
    case both_types(bin_noad_type,       open_noad_type     ): x = get_math_param(math_param_bin_open_spacing,mstyle); break;
#if 0
    case both_types(bin_noad_type,       close_noad_type    ): x = get_math_param(math_param_bin_close_spacing,mstyle); break;
    case both_types(bin_noad_type,       punct_noad_type    ): x = get_math_param(math_param_bin_punct_spacing,mstyle); break;
#endif
    case both_types(bin_noad_type,       inner_noad_type    ): x = get_math_param(math_param_bin_inner_spacing,mstyle); break;
    case both_types(rel_noad_type,       ord_noad_type      ): x = get_math_param(math_param_rel_ord_spacing,mstyle); break;
    case both_types(rel_noad_type,       op_noad_type_normal): x = get_math_param(math_param_rel_op_spacing,mstyle); break;
#if 0
    case both_types(rel_noad_type,       bin_noad_type      ): x = get_math_param(math_param_rel_bin_spacing,mstyle); break;
#endif
    case both_types(rel_noad_type,       rel_noad_type      ): x = get_math_param(math_param_rel_rel_spacing,mstyle); break;
    case both_types(rel_noad_type,       open_noad_type     ): x = get_math_param(math_param_rel_open_spacing,mstyle); break;
    case both_types(rel_noad_type,       close_noad_type    ): x = get_math_param(math_param_rel_close_spacing,mstyle); break;
    case both_types(rel_noad_type,       punct_noad_type    ): x = get_math_param(math_param_rel_punct_spacing,mstyle); break;
    case both_types(rel_noad_type,       inner_noad_type    ): x = get_math_param(math_param_rel_inner_spacing,mstyle); break;
    case both_types(open_noad_type,      ord_noad_type      ): x = get_math_param(math_param_open_ord_spacing,mstyle); break;
    case both_types(open_noad_type,      op_noad_type_normal): x = get_math_param(math_param_open_op_spacing,mstyle); break;
#if 0
    case both_types(open_noad_type,      bin_noad_type      ): x = get_math_param(math_param_open_bin_spacing,mstyle); break;
#endif
    case both_types(open_noad_type,      rel_noad_type      ): x = get_math_param(math_param_open_rel_spacing,mstyle); break;
    case both_types(open_noad_type,      open_noad_type     ): x = get_math_param(math_param_open_open_spacing,mstyle); break;
    case both_types(open_noad_type,      close_noad_type    ): x = get_math_param(math_param_open_close_spacing,mstyle); break;
    case both_types(open_noad_type,      punct_noad_type    ): x = get_math_param(math_param_open_punct_spacing,mstyle); break;
    case both_types(open_noad_type,      inner_noad_type    ): x = get_math_param(math_param_open_inner_spacing,mstyle); break;
    case both_types(close_noad_type,     ord_noad_type      ): x = get_math_param(math_param_close_ord_spacing,mstyle); break;
    case both_types(close_noad_type,     op_noad_type_normal): x = get_math_param(math_param_close_op_spacing,mstyle); break;
    case both_types(close_noad_type,     bin_noad_type      ): x = get_math_param(math_param_close_bin_spacing,mstyle); break;
    case both_types(close_noad_type,     rel_noad_type      ): x = get_math_param(math_param_close_rel_spacing,mstyle); break;
    case both_types(close_noad_type,     open_noad_type     ): x = get_math_param(math_param_close_open_spacing,mstyle); break;
    case both_types(close_noad_type,     close_noad_type    ): x = get_math_param(math_param_close_close_spacing,mstyle); break;
    case both_types(close_noad_type,     punct_noad_type    ): x = get_math_param(math_param_close_punct_spacing,mstyle); break;
    case both_types(close_noad_type,     inner_noad_type    ): x = get_math_param(math_param_close_inner_spacing,mstyle); break;
    case both_types(punct_noad_type,     ord_noad_type      ): x = get_math_param(math_param_punct_ord_spacing,mstyle); break;
    case both_types(punct_noad_type,     op_noad_type_normal): x = get_math_param(math_param_punct_op_spacing,mstyle); break;
#if 0
    case both_types(punct_noad_type,     bin_noad_type      ): x = get_math_param(math_param_punct_bin_spacing,mstyle); break;
#endif
    case both_types(punct_noad_type,     rel_noad_type      ): x = get_math_param(math_param_punct_rel_spacing,mstyle); break;
    case both_types(punct_noad_type,     open_noad_type     ): x = get_math_param(math_param_punct_open_spacing,mstyle); break;
    case both_types(punct_noad_type,     close_noad_type    ): x = get_math_param(math_param_punct_close_spacing,mstyle); break;
    case both_types(punct_noad_type,     punct_noad_type    ): x = get_math_param(math_param_punct_punct_spacing,mstyle); break;
    case both_types(punct_noad_type,     inner_noad_type    ): x = get_math_param(math_param_punct_inner_spacing,mstyle); break;
    case both_types(inner_noad_type,     ord_noad_type      ): x = get_math_param(math_param_inner_ord_spacing,mstyle); break;
    case both_types(inner_noad_type,     op_noad_type_normal): x = get_math_param(math_param_inner_op_spacing,mstyle); break;
    case both_types(inner_noad_type,     bin_noad_type      ): x = get_math_param(math_param_inner_bin_spacing,mstyle); break;
    case both_types(inner_noad_type,     rel_noad_type      ): x = get_math_param(math_param_inner_rel_spacing,mstyle); break;
    case both_types(inner_noad_type,     open_noad_type     ): x = get_math_param(math_param_inner_open_spacing,mstyle); break;
    case both_types(inner_noad_type,     close_noad_type    ): x = get_math_param(math_param_inner_close_spacing,mstyle); break;
    case both_types(inner_noad_type,     punct_noad_type    ): x = get_math_param(math_param_inner_punct_spacing,mstyle); break;
    case both_types(inner_noad_type,     inner_noad_type    ): x = get_math_param(math_param_inner_inner_spacing,mstyle); break;
    /* *INDENT-ON* */
    }
    if (x < 0) {
        confusion("mathspacing");
    }
    if (x != 0) {
        if (x <= thick_mu_skip_code) {
            /* trap thin/med/thick settings cf. old TeX */
            z = math_glue(glue_par(x), mmu); /* allocates a glue */
            /* store a symbolic subtype */
            subtype(z) = (quarterword) (x + 1);
        } else {
            z = math_glue(x, mmu); /* allocates a glue */
        }
    }
    return z;
}

@ @c
static pointer check_nucleus_complexity(halfword q, scaled * delta, int cur_style)
{
    pointer p = null;
    switch (type(nucleus(q))) {
    case math_char_node:
    case math_text_char_node:
        fetch(nucleus(q));
        if (char_exists(cur_f, cur_c)) {
            /* we could look at neighbours */
            if (do_new_math(cur_f)) {
                *delta = 0 ; /* cf spec only the last one */
            } else {
                *delta = char_italic(cur_f, cur_c);
            }
            p = new_glyph(cur_f, cur_c);
            reset_attributes(p, node_attr(nucleus(q)));
            if (do_new_math(cur_f)) {
                if (! math_no_char_italic_par) {
                    /* keep italic, but bad with two successive letters */
                } else if (get_char_cat_code(cur_c) == 11) {
                    /* no italic correction in mid-word of text font */
                    *delta = 0;
                }
            } else {
                /* no italic correction in mid-word of text font */
                if (((type(nucleus(q))) == math_text_char_node) && (space(cur_f) != 0)) {
                    *delta = 0;
                }
            }
            /* so we only add italic correction when we have no scripts */
            if ((subscr(q) == null) && (supscr(q) == null) && (*delta != 0)) {
                pointer x = new_kern(*delta);
                reset_attributes(x, node_attr(nucleus(q)));
                couple_nodes(p,x);
                *delta = 0;
            }
            if (do_new_math(cur_f)) {
                *delta = char_italic(cur_f, cur_c); /* must be more selective */
            }
        }
        break;
    case sub_box_node:
        p = math_list(nucleus(q));
        break;
    case sub_mlist_node:
        mlist_to_hlist(math_list(nucleus(q)), false, cur_style);   /* recursive call */
        setup_cur_size(cur_style);
        p = hpack(vlink(temp_head), 0, additional, -1);
        reset_attributes(p, node_attr(nucleus(q)));
        break;
    default:
        confusion("mlist2");    /* this can't happen mlist2 */
    }
    return p;
}

@ Here is the overall plan of |mlist_to_hlist|, and the list of its
   local variables.

@c
void mlist_to_hlist(pointer mlist, boolean penalties, int cur_style)
{
    pointer q = mlist;                    /* runs through the mlist */
    pointer r = null;                     /* the most recent noad preceding |q| */
    int style = cur_style;                /* tuck global parameter away as local variable */
    int r_type = simple_noad;             /* the |type| of noad |r|, or |op_noad| if |r=null| */
    int r_subtype = op_noad_type_normal;  /* the |subtype| of noad |r| if |r_type| is |fence_noad| */
    int t;                                /* the effective |type| of noad |q| during the second pass */
    int t_subtype;                        /* the effective |subtype| of noad |q| during the second pass */
    pointer p = null;
    pointer z = null;
    int pen;                              /* a penalty to be inserted */
    scaled max_hl = 0;                    /* maximum height of the list translated so far */
    scaled max_d = 0;                     /* maximum depth of the list translated so far */
    scaled delta;                         /* italic correction offset for subscript and superscript */
    scaled cur_mu;                        /* the math unit width corresponding to |cur_size| */
    r_subtype = op_noad_type_normal;
    setup_cur_size(cur_style);
    cur_mu = x_over_n(get_math_quad_size(cur_size), 18);
    while (q != null) {
        /*
            we use the fact that no character nodes appear in an mlist, hence
            the field |type(q)| is always present.

            one of the things we must do on the first pass is change a |bin_noad| to
            an |ord_noad| if the |bin_noad| is not in the context of a binary operator

            the values of |r| and |r_type| make this fairly easy
        */
      RESWITCH:
        delta = 0;
        switch (type(q)) {
        case simple_noad:
            switch (subtype(q)) {
            case bin_noad_type:
                switch (r_type) {
                case simple_noad:
                    switch (r_subtype) {
                    case bin_noad_type:
                    case op_noad_type_normal:
                    case op_noad_type_limits:
                    case op_noad_type_no_limits:
                    case rel_noad_type:
                    case open_noad_type:
                    case punct_noad_type:
                        subtype(q) = ord_noad_type;
                        goto RESWITCH;
                        break;
                    }
                    break;
                case fence_noad:
                    if (r_subtype == left_noad_side) {
                        subtype(q) = ord_noad_type; /* so these can best be the same size */
                        goto RESWITCH;
                    }
                    break;
                }
                break;
            case over_noad_type:
                make_over(q, cur_style, cur_size, math_rules_fam_par);
                break;
            case under_noad_type:
                make_under(q, cur_style, cur_size, math_rules_fam_par);
                break;
            case vcenter_noad_type:
                make_vcenter(q);
                break;
            case rel_noad_type:
            case close_noad_type:
            case punct_noad_type:
                if (r_type == simple_noad && r_subtype == bin_noad_type) {
                    type(r) = simple_noad; /* assumes the same size .. can't this go */
                    subtype(r) = ord_noad_type;
                }
                break;
            case op_noad_type_normal:
            case op_noad_type_limits:
            case op_noad_type_no_limits:
                delta = make_op(q, cur_style);
                if ((subtype(q) == op_noad_type_limits) || (subtype(q) == op_noad_type_no_limits))
                    goto CHECK_DIMENSIONS;
                break;
            case ord_noad_type:
                make_ord(q);
                break;
            case open_noad_type:
            case inner_noad_type:
                break;
            }
            break;
        case fence_noad:
            if (subtype(q) != left_noad_side)
                if (r_type == simple_noad && r_subtype == bin_noad_type) {
                    type(r) = simple_noad; /* assumes the same size  */
                    subtype(r) = ord_noad_type;
                }
            goto DONE_WITH_NOAD;
            break;
        case fraction_noad:
            make_fraction(q, cur_style);
            goto CHECK_DIMENSIONS;
            break;
        case radical_noad:
            if (subtype(q) == 7)
                make_hextension(q, cur_style);
            else if (subtype(q) == 6)
                make_delimiter_over(q, cur_style);
            else if (subtype(q) == 5)
                make_delimiter_under(q, cur_style);
            else if (subtype(q) == 4)
                make_over_delimiter(q, cur_style);
            else if (subtype(q) == 3)
                make_under_delimiter(q, cur_style);
            else
                make_radical(q, cur_style);
            break;
        case accent_noad:
          make_math_accent(q, cur_style);
            break;
        case style_node:
            cur_style = subtype(q);
            setup_cur_size(cur_style);
            cur_mu = x_over_n(get_math_quad_style(cur_style), 18);
            goto DONE_WITH_NODE;
            break;
        case choice_node:
            switch (cur_style / 2) {
            case 0: /* |display_style=0| */
                choose_mlist(display_mlist);
                break;
            case 1: /* |text_style=2| */
                choose_mlist(text_mlist);
                break;
            case 2: /* |script_style=4| */
                choose_mlist(script_mlist);
                break;
            case 3: /* |script_script_style=6| */
                choose_mlist(script_script_mlist);
                break;
            }
            flush_node_list(display_mlist(q));
            flush_node_list(text_mlist(q));
            flush_node_list(script_mlist(q));
            flush_node_list(script_script_mlist(q));
            type(q) = style_node;
            subtype(q) = (quarterword) cur_style;
            if (p != null) {
                z = vlink(q);
                couple_nodes(q,p);
                while (vlink(p) != null)
                    p = vlink(p);
                try_couple_nodes(p,z);
            }
            goto DONE_WITH_NODE;
            break;
        case ins_node:
        case mark_node:
        case adjust_node:
        case boundary_node:
        case whatsit_node:
        case penalty_node:
        case disc_node:
            goto DONE_WITH_NODE;
            break;
        case rule_node:
            if (height(q) > max_hl)
                max_hl = height(q);
            if (depth(q) > max_d)
                max_d = depth(q);
            goto DONE_WITH_NODE;
            break;
        case glue_node:
            /*
                conditional math glue (`\.{\\nonscript}') results in a |glue_node|
                pointing to |zero_glue|, with |subtype(q)=cond_math_glue|; in such a case
                the node following will be eliminated if it is a glue or kern node and if the
                current size is different from |text_size|

                unconditional math glue (`\.{\\muskip}') is converted to normal glue by
                multiplying the dimensions by |cur_mu|

            */
            if (subtype(q) == mu_glue) {
                math_glue_to_glue(q, cur_mu);
            } else if ((cur_size != text_size) && (subtype(q) == cond_math_glue)) {
                p = vlink(q);
         	if (p != null)
                     if ((type(p) == glue_node) || (type(p) == kern_node)) {
                       if (vlink(p) != null) {
                            couple_nodes(q,vlink(p));
                            vlink(p) = null;
                        } else {
                            vlink(q) = null;
                        }
                        flush_node_list(p);
                     }
            }
	    goto DONE_WITH_NODE;
            break;
        case kern_node:
            math_kern(q, cur_mu);
            goto DONE_WITH_NODE;
            break;
        default:
            confusion("mlist1");
        }
        /*
            When we get to the following part of the program, we have ``fallen through''
            from cases that did not lead to |check_dimensions| or |done_with_noad| or
            |done_with_node|. Thus, |q|~points to a noad whose nucleus may need to be
            converted to an hlist, and whose subscripts and superscripts need to be
            appended if they are present.

            If |nucleus(q)| is not a |math_char|, the variable |delta| is the amount
            by which a superscript should be moved right with respect to a subscript
            when both are present.

        */
        p = check_nucleus_complexity(q, &delta, cur_style);

        if ((subscr(q) == null) && (supscr(q) == null)) {
            assign_new_hlist(q, p);
        } else {
            /* top, bottom */
            make_scripts(q, p, delta, cur_style, 0, 0);
        }
      CHECK_DIMENSIONS:
        z = hpack(new_hlist(q), 0, additional, -1);
        if (height(z) > max_hl)
            max_hl = height(z);
        if (depth(z) > max_d)
            max_d = depth(z);
        list_ptr(z) = null;
        /* only drop the \.{\\hbox} */
        flush_node(z);
      DONE_WITH_NOAD:
        r = q;
        r_type = type(r);
        r_subtype = subtype(r);
        if (r_type == fence_noad) {
            r_subtype = left_noad_side;
            cur_style = style;
            setup_cur_size(cur_style);
            cur_mu = x_over_n(get_math_quad_size(cur_size), 18); /* style */
        }
      DONE_WITH_NODE:
        q = vlink(q);
    }
    if (r_type == simple_noad && r_subtype == bin_noad_type) {
        type(r) = simple_noad;
        subtype(r) = ord_noad_type;
    }
    /*
        Make a second pass over the mlist, removing all noads and inserting the
        proper spacing and penalties.

        We have now tied up all the loose ends of the first pass of |mlist_to_hlist|.
        The second pass simply goes through and hooks everything together with the
        proper glue and penalties. It also handles the |fence_noad|s that
        might be present, since |max_hl| and |max_d| are now known. Variable |p| points
        to a node at the current end of the final hlist.
    */
    p = temp_head;
    vlink(p) = null;
    q = mlist;
    r_type = 0;
    r_subtype = 0;
    cur_style = style;
    setup_cur_size(cur_style);
    cur_mu = x_over_n(get_math_quad_size(cur_size), 18);
  NEXT_NODE:
    while (q != null) {
        /*
            If node |q| is a style node, change the style and |goto delete_q|;
            otherwise if it is not a noad, put it into the hlist,
            advance |q|, and |goto done|; otherwise set |s| to the size
            of noad |q|, set |t| to the associated type (|ord_noad..
            inner_noad|), and set |pen| to the associated penalty

            Just before doing the big |case| switch in the second pass, the program
            sets up default values so that most of the branches are short.
        */
        t = simple_noad;
        t_subtype = ord_noad_type;
        pen = inf_penalty;
        switch (type(q)) {
        case simple_noad:
            t_subtype = subtype(q);
            switch (t_subtype) {
            case bin_noad_type:
                pen = bin_op_penalty_par;
                break;
            case rel_noad_type:
                pen = rel_penalty_par;
                break;
            case vcenter_noad_type:
            case over_noad_type:
            case under_noad_type:
                t_subtype = ord_noad_type;
                break;
            }
        case radical_noad:
            break;
        case accent_noad:
            break;
        case fraction_noad:
            t = simple_noad;
            t_subtype = inner_noad_type;
            break;
        case fence_noad:
            t_subtype = make_left_right(q, style, max_d, max_hl);
            break;
        case style_node:
            /* Change the current style and |goto delete_q| */
            cur_style = subtype(q);
            setup_cur_size(cur_style);
            cur_mu = x_over_n(get_math_quad_style(cur_style), 18);
            goto DELETE_Q;
            break;
        case whatsit_node:
        case penalty_node:
        case rule_node:
        case disc_node:
        case adjust_node:
        case ins_node:
        case mark_node:
        case glue_node:
        case kern_node:
            couple_nodes(p,q);
            p = q;
            q = vlink(q);
            vlink(p) = null;
            goto NEXT_NODE;
            break;
        default:
            confusion("mlist3");
        }
        /* Append inter-element spacing based on |r_type| and |t| */
        if (r_type > 0) {
            /* not the first noad */
            z = math_spacing_glue(r_subtype, t_subtype, cur_style, cur_mu);
            if (z != null) {
                reset_attributes(z, node_attr(p));
                couple_nodes(p,z);
                p = z;
            }
        }
        /*
            Append any |new_hlist| entries for |q|, and any appropriate penalties

            We insert a penalty node after the hlist entries of noad |q| if |pen|
            is not an ``infinite'' penalty, and if the node immediately following |q|
            is not a penalty node or a |rel_noad| or absent entirely.
        */
        if (new_hlist(q) != null) {
            couple_nodes(p,new_hlist(q));
            do {
                p = vlink(p);
            } while (vlink(p) != null);
        }
        if (penalties && vlink(q) != null && pen < inf_penalty) {
            r_type = type(vlink(q));
            r_subtype = subtype(vlink(q));
            if (r_type != penalty_node && (r_type != simple_noad || r_subtype != rel_noad_type)) {
                z = new_penalty(pen);
                reset_attributes(z, node_attr(q));
                couple_nodes(p,z);
                p = z;
            }
        }
        if (type(q) == fence_noad && subtype(q) == right_noad_side) {
            t = simple_noad;
            t_subtype = open_noad_type;
        }
        r_type = t;
        r_subtype = t_subtype;
      DELETE_Q:
        r = q;
        q = vlink(q);
        /*
            The m-to-hlist conversion takes place in-place, so the various dependant
            fields may not be freed (as would happen if |flush_node| was called).

            A low-level |free_node| is easier than attempting to nullify such dependant
            fields for all possible node and noad types.
        */
        if (nodetype_has_attributes(type(r))) {
            delete_attribute_ref(node_attr(r));
        }
        reset_node_properties(r);
        free_node(r, get_node_size(type(r), subtype(r)));
    }
}
