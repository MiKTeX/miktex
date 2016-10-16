/* Copyright (C) 2007-2012 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include "fontforgevw.h"
#ifdef __need_size_t
/* This is a bug on the mac, someone defines this and leaves it defined */
/*  that means when I load stddef.h it only defines size_t and doesn't */
/*  do offset_of, which is what I need */
# undef __need_size_t
#endif

#define MCD(name) { #name, offsetof(struct MATH,name), -1 }
#define MCDD(name,devtab_name) { #name, offsetof(struct MATH,name), offsetof(struct MATH,devtab_name) }

struct math_constants_descriptor math_constants_descriptor[] = {
    MCD(ScriptPercentScaleDown),
    MCD(ScriptScriptPercentScaleDown),
    MCD(DelimitedSubFormulaMinHeight),
    MCD(DisplayOperatorMinHeight),
    MCDD(MathLeading,MathLeading_adjust),
    MCDD(AxisHeight,AxisHeight_adjust),
    MCDD(AccentBaseHeight,AccentBaseHeight_adjust),
    MCDD(FlattenedAccentBaseHeight,FlattenedAccentBaseHeight_adjust),
    MCDD(SubscriptShiftDown,SubscriptShiftDown_adjust),
    MCDD(SubscriptTopMax,SubscriptTopMax_adjust),
    MCDD(SubscriptBaselineDropMin,SubscriptBaselineDropMin_adjust),
    MCDD(SuperscriptShiftUp,SuperscriptShiftUp_adjust),
    MCDD(SuperscriptShiftUpCramped,SuperscriptShiftUpCramped_adjust),
    MCDD(SuperscriptBottomMin,SuperscriptBottomMin_adjust),
    MCDD(SuperscriptBaselineDropMax,SuperscriptBaselineDropMax_adjust),
    MCDD(SubSuperscriptGapMin,SubSuperscriptGapMin_adjust),
    MCDD(SuperscriptBottomMaxWithSubscript,SuperscriptBottomMaxWithSubscript_adjust),
    MCDD(SpaceAfterScript,SpaceAfterScript_adjust),
    MCDD(UpperLimitGapMin,UpperLimitGapMin_adjust),
    MCDD(UpperLimitBaselineRiseMin,UpperLimitBaselineRiseMin_adjust),
    MCDD(LowerLimitGapMin,LowerLimitGapMin_adjust),
    MCDD(LowerLimitBaselineDropMin,LowerLimitBaselineDropMin_adjust),
    MCDD(StackTopShiftUp,StackTopShiftUp_adjust),
    MCDD(StackTopDisplayStyleShiftUp,StackTopDisplayStyleShiftUp_adjust),
    MCDD(StackBottomShiftDown,StackBottomShiftDown_adjust),
    MCDD(StackBottomDisplayStyleShiftDown,StackBottomDisplayStyleShiftDown_adjust),
    MCDD(StackGapMin,StackGapMin_adjust),
    MCDD(StackDisplayStyleGapMin,StackDisplayStyleGapMin_adjust),
    MCDD(StretchStackTopShiftUp,StretchStackTopShiftUp_adjust),
    MCDD(StretchStackBottomShiftDown,StretchStackBottomShiftDown_adjust),
    MCDD(StretchStackGapAboveMin,StretchStackGapAboveMin_adjust),
    MCDD(StretchStackGapBelowMin,StretchStackGapBelowMin_adjust),
    MCDD(FractionNumeratorShiftUp,FractionNumeratorShiftUp_adjust),
    MCDD(FractionNumeratorDisplayStyleShiftUp,FractionNumeratorDisplayStyleShiftUp_adjust),
    MCDD(FractionDenominatorShiftDown,FractionDenominatorShiftDown_adjust),
    MCDD(FractionDenominatorDisplayStyleShiftDown,FractionDenominatorDisplayStyleShiftDown_adjust),
    MCDD(FractionNumeratorGapMin,FractionNumeratorGapMin_adjust),
    MCDD(FractionNumeratorDisplayStyleGapMin,FractionNumeratorDisplayStyleGapMin_adjust),
    MCDD(FractionRuleThickness,FractionRuleThickness_adjust),
    MCDD(FractionDenominatorGapMin,FractionDenominatorGapMin_adjust),
    MCDD(FractionDenominatorDisplayStyleGapMin,FractionDenominatorDisplayStyleGapMin_adjust),
    MCDD(SkewedFractionHorizontalGap,SkewedFractionHorizontalGap_adjust),
    MCDD(SkewedFractionVerticalGap,SkewedFractionVerticalGap_adjust),
    MCDD(OverbarVerticalGap,OverbarVerticalGap_adjust),
    MCDD(OverbarRuleThickness,OverbarRuleThickness_adjust),
    MCDD(OverbarExtraAscender,OverbarExtraAscender_adjust),
    MCDD(UnderbarVerticalGap,UnderbarVerticalGap_adjust),
    MCDD(UnderbarRuleThickness,UnderbarRuleThickness_adjust),
    MCDD(UnderbarExtraDescender,UnderbarExtraDescender_adjust),
    MCDD(RadicalVerticalGap,RadicalVerticalGap_adjust),
    MCDD(RadicalDisplayStyleVerticalGap,RadicalDisplayStyleVerticalGap_adjust),
    MCDD(RadicalRuleThickness,RadicalRuleThickness_adjust),
    MCDD(RadicalExtraAscender,RadicalExtraAscender_adjust),
    MCDD(RadicalKernBeforeDegree,RadicalKernBeforeDegree_adjust),
    MCDD(RadicalKernAfterDegree,RadicalKernAfterDegree_adjust),
    MCD(RadicalDegreeBottomRaisePercent),
    MCD(MinConnectorOverlap),
    MATH_CONSTANTS_DESCRIPTOR_EMPTY
};
