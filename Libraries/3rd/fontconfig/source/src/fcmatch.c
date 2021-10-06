/*
 * fontconfig/src/fcmatch.c
 *
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"

static double
FcCompareNumber (const FcValue *value1, const FcValue *value2, FcValue *bestValue)
{
    double  v1, v2, v;

    switch ((int) value1->type) {
    case FcTypeInteger:
	v1 = (double) value1->u.i;
	break;
    case FcTypeDouble:
	v1 = value1->u.d;
	break;
    default:
	return -1.0;
    }
    switch ((int) value2->type) {
    case FcTypeInteger:
	v2 = (double) value2->u.i;
	break;
    case FcTypeDouble:
	v2 = value2->u.d;
	break;
    default:
	return -1.0;
    }
    v = v2 - v1;
    if (v < 0)
	v = -v;
    *bestValue = FcValueCanonicalize (value2);
    return v;
}

static double
FcCompareString (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    *bestValue = FcValueCanonicalize (v2);
    return (double) FcStrCmpIgnoreCase (FcValueString(v1), FcValueString(v2)) != 0;
}

static double
FcCompareFamily (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    /* rely on the guarantee in FcPatternObjectAddWithBinding that
     * families are always FcTypeString. */
    const FcChar8* v1_string = FcValueString(v1);
    const FcChar8* v2_string = FcValueString(v2);

    *bestValue = FcValueCanonicalize (v2);

    if (FcToLower(*v1_string) != FcToLower(*v2_string) &&
	*v1_string != ' ' && *v2_string != ' ')
       return 1.0;

    return (double) FcStrCmpIgnoreBlanksAndCase (v1_string, v2_string) != 0;
}

static double
FcComparePostScript (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    const FcChar8 *v1_string = FcValueString (v1);
    const FcChar8 *v2_string = FcValueString (v2);
    int n;
    size_t len;

    *bestValue = FcValueCanonicalize (v2);

    if (FcToLower (*v1_string) != FcToLower (*v2_string) &&
	*v1_string != ' ' && *v2_string != ' ')
	return 1.0;

    n = FcStrMatchIgnoreCaseAndDelims (v1_string, v2_string, (const FcChar8 *)" -");
    len = strlen ((const char *)v1_string);

    return (double)(len - n) / (double)len;
}

static double
FcCompareLang (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    FcLangResult    result;
    FcValue value1 = FcValueCanonicalize(v1), value2 = FcValueCanonicalize(v2);

    switch ((int) value1.type) {
    case FcTypeLangSet:
	switch ((int) value2.type) {
	case FcTypeLangSet:
	    result = FcLangSetCompare (value1.u.l, value2.u.l);
	    break;
	case FcTypeString:
	    result = FcLangSetHasLang (value1.u.l,
				       value2.u.s);
	    break;
	default:
	    return -1.0;
	}
	break;
    case FcTypeString:
	switch ((int) value2.type) {
	case FcTypeLangSet:
	    result = FcLangSetHasLang (value2.u.l, value1.u.s);
	    break;
	case FcTypeString:
	    result = FcLangCompare (value1.u.s,
				    value2.u.s);
	    break;
	default:
	    return -1.0;
	}
	break;
    default:
	return -1.0;
    }
    *bestValue = FcValueCanonicalize (v2);
    switch (result) {
    case FcLangEqual:
	return 0;
    case FcLangDifferentCountry:
	return 1;
    case FcLangDifferentLang:
    default:
	return 2;
    }
}

static double
FcCompareBool (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    if (v2->type != FcTypeBool || v1->type != FcTypeBool)
	return -1.0;

    if (v2->u.b != FcDontCare)
	*bestValue = FcValueCanonicalize (v2);
    else
	*bestValue = FcValueCanonicalize (v1);

    return (double) ((v2->u.b ^ v1->u.b) == 1);
}

static double
FcCompareCharSet (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    *bestValue = FcValueCanonicalize (v2); /* TODO Improve. */
    return (double) FcCharSetSubtractCount (FcValueCharSet(v1), FcValueCharSet(v2));
}

static double
FcCompareRange (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    FcValue value1 = FcValueCanonicalize (v1);
    FcValue value2 = FcValueCanonicalize (v2);
    double b1, e1, b2, e2, d;

    switch ((int) value1.type) {
    case FcTypeInteger:
        b1 = e1 = value1.u.i;
	break;
    case FcTypeDouble:
        b1 = e1 = value1.u.d;
	break;
    case FcTypeRange:
	b1 = value1.u.r->begin;
	e1 = value1.u.r->end;
	break;
    default:
	return -1;
    }
    switch ((int) value2.type) {
    case FcTypeInteger:
        b2 = e2 = value2.u.i;
	break;
    case FcTypeDouble:
        b2 = e2 = value2.u.d;
	break;
    case FcTypeRange:
	b2 = value2.u.r->begin;
	e2 = value2.u.r->end;
	break;
    default:
	return -1;
    }

    if (e1 < b2)
      d = b2;
    else if (e2 < b1)
      d = e2;
    else
      d = (FC_MAX (b1, b2) + FC_MIN (e1, e2)) * .5;

    bestValue->type = FcTypeDouble;
    bestValue->u.d = d;

    /* If the ranges overlap, it's a match, otherwise return closest distance. */
    if (e1 < b2 || e2 < b1)
	return FC_MIN (fabs (b2 - e1), fabs (b1 - e2));
    else
	return 0.0;
}

static double
FcCompareSize (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    FcValue value1 = FcValueCanonicalize (v1);
    FcValue value2 = FcValueCanonicalize (v2);
    double b1, e1, b2, e2;

    switch ((int) value1.type) {
    case FcTypeInteger:
        b1 = e1 = value1.u.i;
	break;
    case FcTypeDouble:
        b1 = e1 = value1.u.d;
	break;
    case FcTypeRange:
	abort();
	b1 = value1.u.r->begin;
	e1 = value1.u.r->end;
	break;
    default:
	return -1;
    }
    switch ((int) value2.type) {
    case FcTypeInteger:
        b2 = e2 = value2.u.i;
	break;
    case FcTypeDouble:
        b2 = e2 = value2.u.d;
	break;
    case FcTypeRange:
	b2 = value2.u.r->begin;
	e2 = value2.u.r->end;
	break;
    default:
	return -1;
    }

    bestValue->type = FcTypeDouble;
    bestValue->u.d = (b1 + e1) * .5;

    /* If the ranges overlap, it's a match, otherwise return closest distance. */
    if (e1 < b2 || e2 < b1)
	return FC_MIN (fabs (b2 - e1), fabs (b1 - e2));
    if (b2 != e2 && b1 == e2) /* Semi-closed interval. */
        return 1e-15;
    else
	return 0.0;
}

static double
FcCompareFilename (const FcValue *v1, const FcValue *v2, FcValue *bestValue)
{
    const FcChar8 *s1 = FcValueString (v1), *s2 = FcValueString (v2);
    *bestValue = FcValueCanonicalize (v2);
    if (FcStrCmp (s1, s2) == 0)
	return 0.0;
    else if (FcStrCmpIgnoreCase (s1, s2) == 0)
	return 1.0;
    else if (FcStrGlobMatch (s1, s2))
	return 2.0;
    else
	return 3.0;
}


/* Define priorities to -1 for objects that don't have a compare function. */

#define PRI_NULL(n)				\
    PRI_ ## n ## _STRONG = -1,			\
    PRI_ ## n ## _WEAK = -1,
#define PRI1(n)
#define PRI_FcCompareFamily(n)		PRI1(n)
#define PRI_FcCompareString(n)		PRI1(n)
#define PRI_FcCompareNumber(n)		PRI1(n)
#define PRI_FcCompareBool(n)		PRI1(n)
#define PRI_FcCompareFilename(n)	PRI1(n)
#define PRI_FcCompareCharSet(n)		PRI1(n)
#define PRI_FcCompareLang(n)		PRI1(n)
#define PRI_FcComparePostScript(n)	PRI1(n)
#define PRI_FcCompareRange(n)		PRI1(n)
#define PRI_FcCompareSize(n)		PRI1(n)

#define FC_OBJECT(NAME, Type, Cmp)	PRI_##Cmp(NAME)

typedef enum _FcMatcherPriorityDummy {
#include "fcobjs.h"
} FcMatcherPriorityDummy;

#undef FC_OBJECT


/* Canonical match priority order. */

#undef PRI1
#define PRI1(n)					\
    PRI_ ## n,					\
    PRI_ ## n ## _STRONG = PRI_ ## n,		\
    PRI_ ## n ## _WEAK = PRI_ ## n

typedef enum _FcMatcherPriority {
    PRI1(FILE),
    PRI1(FONTFORMAT),
    PRI1(VARIABLE),
    PRI1(SCALABLE),
    PRI1(COLOR),
    PRI1(FOUNDRY),
    PRI1(CHARSET),
    PRI_FAMILY_STRONG,
    PRI_POSTSCRIPT_NAME_STRONG,
    PRI1(LANG),
    PRI_FAMILY_WEAK,
    PRI_POSTSCRIPT_NAME_WEAK,
    PRI1(SYMBOL),
    PRI1(SPACING),
    PRI1(SIZE),
    PRI1(PIXEL_SIZE),
    PRI1(STYLE),
    PRI1(SLANT),
    PRI1(WEIGHT),
    PRI1(WIDTH),
    PRI1(DECORATIVE),
    PRI1(ANTIALIAS),
    PRI1(RASTERIZER),
    PRI1(OUTLINE),
    PRI1(FONTVERSION),
    PRI_END
} FcMatcherPriority;

#undef PRI1

typedef struct _FcMatcher {
    FcObject object;
    double   (*compare) (const FcValue *v1, const FcValue *v2, FcValue *bestValue);
    int      strong, weak;
} FcMatcher;

/*
 * Order is significant, it defines the precedence of
 * each value, earlier values are more significant than
 * later values
 */
#define FC_OBJECT(NAME, Type, Cmp)	{ FC_##NAME##_OBJECT,	Cmp,	PRI_##NAME##_STRONG,	PRI_##NAME##_WEAK },
static const FcMatcher _FcMatchers [] = {
    { FC_INVALID_OBJECT, NULL, -1, -1 },
#include "fcobjs.h"
};
#undef FC_OBJECT

static const FcMatcher*
FcObjectToMatcher (FcObject object,
		   FcBool   include_lang)
{
    if (include_lang)
    {
	switch (object) {
	case FC_FAMILYLANG_OBJECT:
	case FC_STYLELANG_OBJECT:
	case FC_FULLNAMELANG_OBJECT:
	    object = FC_LANG_OBJECT;
	    break;
	}
    }
    if (object > FC_MAX_BASE_OBJECT ||
	!_FcMatchers[object].compare ||
	_FcMatchers[object].strong == -1 ||
	_FcMatchers[object].weak == -1)
	return NULL;

    return _FcMatchers + object;
}

static FcBool
FcCompareValueList (FcObject	     object,
		    const FcMatcher *match,
		    FcValueListPtr   v1orig,	/* pattern */
		    FcValueListPtr   v2orig,	/* target */
		    FcValue         *bestValue,
		    double          *value,
		    int             *n,
		    FcResult        *result)
{
    FcValueListPtr  v1, v2;
    double    	    v, best, bestStrong, bestWeak;
    int		    j, k, pos = 0;

    if (!match)
    {
	if (bestValue)
	    *bestValue = FcValueCanonicalize(&v2orig->value);
	if (n)
	    *n = 0;
	return FcTrue;
    }

    best = 1e99;
    bestStrong = 1e99;
    bestWeak = 1e99;
    j = 0;
    for (v1 = v1orig; v1; v1 = FcValueListNext(v1))
    {
	for (v2 = v2orig, k = 0; v2; v2 = FcValueListNext(v2), k++)
	{
	    FcValue matchValue;
	    v = (match->compare) (&v1->value, &v2->value, &matchValue);
	    if (v < 0)
	    {
		*result = FcResultTypeMismatch;
		return FcFalse;
	    }
	    v = v * 1000 + j;
	    if (v < best)
	    {
		if (bestValue)
		    *bestValue = matchValue;
		best = v;
		pos = k;
	    }
	    if (v1->binding == FcValueBindingStrong)
	    {
		if (v < bestStrong)
		    bestStrong = v;
	    }
	    else
	    {
		if (v < bestWeak)
		    bestWeak = v;
	    }
	}
	j++;
    }
    if (FcDebug () & FC_DBG_MATCHV)
    {
	printf (" %s: %g ", FcObjectName (object), best);
	FcValueListPrint (v1orig);
	printf (", ");
	FcValueListPrint (v2orig);
	printf ("\n");
    }
    if (value)
    {
	int weak    = match->weak;
	int strong  = match->strong;
	if (weak == strong)
	    value[strong] += best;
	else
	{
	    value[weak] += bestWeak;
	    value[strong] += bestStrong;
	}
    }
    if (n)
	*n = pos;

    return FcTrue;
}

/*
 * Return a value indicating the distance between the two lists of
 * values
 */

static FcBool
FcCompare (FcPattern	*pat,
	   FcPattern	*fnt,
	   double	*value,
	   FcResult	*result)
{
    int		    i, i1, i2;

    for (i = 0; i < PRI_END; i++)
	value[i] = 0.0;

    i1 = 0;
    i2 = 0;
    while (i1 < pat->num && i2 < fnt->num)
    {
	FcPatternElt *elt_i1 = &FcPatternElts(pat)[i1];
	FcPatternElt *elt_i2 = &FcPatternElts(fnt)[i2];

	i = FcObjectCompare(elt_i1->object, elt_i2->object);
	if (i > 0)
	    i2++;
	else if (i < 0)
	    i1++;
	else
	{
	    const FcMatcher *match = FcObjectToMatcher (elt_i1->object, FcFalse);
	    if (!FcCompareValueList (elt_i1->object, match,
				     FcPatternEltValues(elt_i1),
				     FcPatternEltValues(elt_i2),
				     NULL, value, NULL, result))
		return FcFalse;
	    i1++;
	    i2++;
	}
    }
    return FcTrue;
}

FcPattern *
FcFontRenderPrepare (FcConfig	    *config,
		     FcPattern	    *pat,
		     FcPattern	    *font)
{
    FcPattern	    *new;
    int		    i;
    FcPatternElt    *fe, *pe;
    FcValue	    v;
    FcResult	    result;
    FcBool	    variable = FcFalse;
    FcStrBuf        variations;

    assert (pat != NULL);
    assert (font != NULL);

    FcPatternObjectGetBool (font, FC_VARIABLE_OBJECT, 0, &variable);
    assert (variable != FcDontCare);
    if (variable)
	FcStrBufInit (&variations, NULL, 0);

    new = FcPatternCreate ();
    if (!new)
	return NULL;
    for (i = 0; i < font->num; i++)
    {
	fe = &FcPatternElts(font)[i];
	if (fe->object == FC_FAMILYLANG_OBJECT ||
	    fe->object == FC_STYLELANG_OBJECT ||
	    fe->object == FC_FULLNAMELANG_OBJECT)
	{
	    /* ignore those objects. we need to deal with them
	     * another way */
	    continue;
	}
	if (fe->object == FC_FAMILY_OBJECT ||
	    fe->object == FC_STYLE_OBJECT ||
	    fe->object == FC_FULLNAME_OBJECT)
	{
	    FcPatternElt    *fel, *pel;

	    FC_ASSERT_STATIC ((FC_FAMILY_OBJECT + 1) == FC_FAMILYLANG_OBJECT);
	    FC_ASSERT_STATIC ((FC_STYLE_OBJECT + 1) == FC_STYLELANG_OBJECT);
	    FC_ASSERT_STATIC ((FC_FULLNAME_OBJECT + 1) == FC_FULLNAMELANG_OBJECT);

	    fel = FcPatternObjectFindElt (font, fe->object + 1);
	    pel = FcPatternObjectFindElt (pat, fe->object + 1);

	    if (fel && pel)
	    {
		/* The font has name languages, and pattern asks for specific language(s).
		 * Match on language and and prefer that result.
		 * Note:  Currently the code only give priority to first matching language.
		 */
		int n = 1, j;
		FcValueListPtr l1, l2, ln = NULL, ll = NULL;
		const FcMatcher *match = FcObjectToMatcher (pel->object, FcTrue);

		if (!FcCompareValueList (pel->object, match,
					 FcPatternEltValues (pel),
					 FcPatternEltValues (fel), NULL, NULL, &n, &result))
		{
		    FcPatternDestroy (new);
		    return NULL;
		}

		for (j = 0, l1 = FcPatternEltValues (fe), l2 = FcPatternEltValues (fel);
		     l1 != NULL || l2 != NULL;
		     j++, l1 = l1 ? FcValueListNext (l1) : NULL, l2 = l2 ? FcValueListNext (l2) : NULL)
		{
		    if (j == n)
		    {
			if (l1)
			    ln = FcValueListPrepend (ln,
						     FcValueCanonicalize (&l1->value),
						     FcValueBindingStrong);
			if (l2)
			    ll = FcValueListPrepend (ll,
						     FcValueCanonicalize (&l2->value),
						     FcValueBindingStrong);
		    }
		    else
		    {
			if (l1)
			    ln = FcValueListAppend (ln,
						    FcValueCanonicalize (&l1->value),
						    FcValueBindingStrong);
			if (l2)
			    ll = FcValueListAppend (ll,
						    FcValueCanonicalize (&l2->value),
						    FcValueBindingStrong);
		    }
		}
		FcPatternObjectListAdd (new, fe->object, ln, FcFalse);
		FcPatternObjectListAdd (new, fel->object, ll, FcFalse);

		continue;
	    }
	    else if (fel)
	    {
		/* Pattern doesn't ask for specific language.  Copy all for name and
		 * lang. */
		FcValueListPtr l1, l2;

		l1 = FcValueListDuplicate (FcPatternEltValues (fe));
		l2 = FcValueListDuplicate (FcPatternEltValues (fel));
		FcPatternObjectListAdd (new, fe->object, l1, FcFalse);
		FcPatternObjectListAdd (new, fel->object, l2, FcFalse);

		continue;
	    }
	}

	pe = FcPatternObjectFindElt (pat, fe->object);
	if (pe)
	{
	    const FcMatcher *match = FcObjectToMatcher (pe->object, FcFalse);
	    if (!FcCompareValueList (pe->object, match,
				     FcPatternEltValues(pe),
				     FcPatternEltValues(fe), &v, NULL, NULL, &result))
	    {
		FcPatternDestroy (new);
		return NULL;
	    }
	    FcPatternObjectAdd (new, fe->object, v, FcFalse);

	    /* Set font-variations settings for standard axes in variable fonts. */
	    if (variable &&
		FcPatternEltValues(fe)->value.type == FcTypeRange &&
		(fe->object == FC_WEIGHT_OBJECT ||
		 fe->object == FC_WIDTH_OBJECT ||
		 fe->object == FC_SIZE_OBJECT))
	    {
		double num;
		FcChar8 temp[128];
		const char *tag = "    ";
		assert (v.type == FcTypeDouble);
		num = v.u.d;
		if (variations.len)
		    FcStrBufChar (&variations, ',');
		switch (fe->object)
		{
		    case FC_WEIGHT_OBJECT:
			tag = "wght";
			num = FcWeightToOpenType (num);
			break;

		    case FC_WIDTH_OBJECT:
			tag = "wdth";
			break;

		    case FC_SIZE_OBJECT:
			tag = "opsz";
			break;
		}
		sprintf ((char *) temp, "%4s=%g", tag, num);
		FcStrBufString (&variations, temp);
	    }
	}
	else
	{
	    FcPatternObjectListAdd (new, fe->object,
				    FcValueListDuplicate (FcPatternEltValues (fe)),
				    FcTrue);
	}
    }
    for (i = 0; i < pat->num; i++)
    {
	pe = &FcPatternElts(pat)[i];
	fe = FcPatternObjectFindElt (font, pe->object);
	if (!fe &&
	    pe->object != FC_FAMILYLANG_OBJECT &&
	    pe->object != FC_STYLELANG_OBJECT &&
	    pe->object != FC_FULLNAMELANG_OBJECT)
	{
	    FcPatternObjectListAdd (new, pe->object,
				    FcValueListDuplicate (FcPatternEltValues(pe)),
				    FcFalse);
	}
    }

    if (variable && variations.len)
    {
	FcChar8 *vars = NULL;
	if (FcPatternObjectGetString (new, FC_FONT_VARIATIONS_OBJECT, 0, &vars) == FcResultMatch)
	{
	    FcStrBufChar (&variations, ',');
	    FcStrBufString (&variations, vars);
	    FcPatternObjectDel (new, FC_FONT_VARIATIONS_OBJECT);
	}

	FcPatternObjectAddString (new, FC_FONT_VARIATIONS_OBJECT, FcStrBufDoneStatic (&variations));
	FcStrBufDestroy (&variations);
    }

    FcConfigSubstituteWithPat (config, new, pat, FcMatchFont);
    return new;
}

static FcPattern *
FcFontSetMatchInternal (FcFontSet   **sets,
			int	    nsets,
			FcPattern   *p,
			FcResult    *result)
{
    double    	    score[PRI_END], bestscore[PRI_END];
    int		    f;
    FcFontSet	    *s;
    FcPattern	    *best;
    int		    i;
    int		    set;

    for (i = 0; i < PRI_END; i++)
	bestscore[i] = 0;
    best = 0;
    if (FcDebug () & FC_DBG_MATCH)
    {
	printf ("Match ");
	FcPatternPrint (p);
    }
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	for (f = 0; f < s->nfont; f++)
	{
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Font %d ", f);
		FcPatternPrint (s->fonts[f]);
	    }
	    if (!FcCompare (p, s->fonts[f], score, result))
		return 0;
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Score");
		for (i = 0; i < PRI_END; i++)
		{
		    printf (" %g", score[i]);
		}
		printf ("\n");
	    }
	    for (i = 0; i < PRI_END; i++)
	    {
		if (best && bestscore[i] < score[i])
		    break;
		if (!best || score[i] < bestscore[i])
		{
		    for (i = 0; i < PRI_END; i++)
			bestscore[i] = score[i];
		    best = s->fonts[f];
		    break;
		}
	    }
	}
    }
    if (FcDebug () & FC_DBG_MATCH)
    {
	printf ("Best score");
	for (i = 0; i < PRI_END; i++)
	    printf (" %g", bestscore[i]);
	printf ("\n");
	FcPatternPrint (best);
    }
    if (FcDebug () & FC_DBG_MATCH2)
    {
#if !defined(MIKTEX)
	char *env = getenv ("FC_DBG_MATCH_FILTER");
#endif
	FcObjectSet *os = NULL;

#if !defined(MIKTEX)
	if (env)
	{
	    char *ss, *s;
	    char *p;
	    FcBool f = FcTrue;

	    ss = s = strdup (env);
	    os = FcObjectSetCreate ();
	    while (f)
	    {
		size_t len;
		char *x;

		if (!(p = strchr (s, ',')))
		{
		    f = FcFalse;
		    len = strlen (s);
		}
		else
		{
		    len = (p - s);
		}
		x = malloc (sizeof (char) * (len + 1));
		if (x)
		{
		    strcpy (x, s);
		    if (FcObjectFromName (x) > 0)
			FcObjectSetAdd (os, x);
		    s = p + 1;
		    free (x);
		}
	    }
	    free (ss);
	}
#endif
	FcPatternPrint2 (p, best, os);
	if (os)
	    FcObjectSetDestroy (os);
    }
    /* assuming that 'result' is initialized with FcResultNoMatch
     * outside this function */
    if (best)
	*result = FcResultMatch;

    return best;
}

FcPattern *
FcFontSetMatch (FcConfig    *config,
		FcFontSet   **sets,
		int	    nsets,
		FcPattern   *p,
		FcResult    *result)
{
    FcPattern	    *best;

    assert (sets != NULL);
    assert (p != NULL);
    assert (result != NULL);

    *result = FcResultNoMatch;

    if (!config)
    {
	config = FcConfigGetCurrent ();
	if (!config)
	    return 0;
    }
    best = FcFontSetMatchInternal (sets, nsets, p, result);
    if (best)
	return FcFontRenderPrepare (config, p, best);
    else
	return NULL;
}

FcPattern *
FcFontMatch (FcConfig	*config,
	     FcPattern	*p,
	     FcResult	*result)
{
    FcFontSet	*sets[2];
    int		nsets;
    FcPattern   *best;

    assert (p != NULL);
    assert (result != NULL);

    *result = FcResultNoMatch;

    if (!config)
    {
	config = FcConfigGetCurrent ();
	if (!config)
	    return 0;
    }
    nsets = 0;
    if (config->fonts[FcSetSystem])
	sets[nsets++] = config->fonts[FcSetSystem];
    if (config->fonts[FcSetApplication])
	sets[nsets++] = config->fonts[FcSetApplication];

    best = FcFontSetMatchInternal (sets, nsets, p, result);
    if (best)
	return FcFontRenderPrepare (config, p, best);
    else
	return NULL;
}

typedef struct _FcSortNode {
    FcPattern	*pattern;
    double	score[PRI_END];
} FcSortNode;

static int
FcSortCompare (const void *aa, const void *ab)
{
    FcSortNode  *a = *(FcSortNode **) aa;
    FcSortNode  *b = *(FcSortNode **) ab;
    double	*as = &a->score[0];
    double	*bs = &b->score[0];
    double	ad = 0, bd = 0;
    int         i;

    i = PRI_END;
    while (i-- && (ad = *as++) == (bd = *bs++))
	;
    return ad < bd ? -1 : ad > bd ? 1 : 0;
}

static FcBool
FcSortWalk (FcSortNode **n, int nnode, FcFontSet *fs, FcCharSet **csp, FcBool trim)
{
    FcBool ret = FcFalse;
    FcCharSet *cs;
    int i;

    cs = 0;
    if (trim || csp)
    {
	cs = FcCharSetCreate ();
	if (cs == NULL)
	    goto bail;
    }

    for (i = 0; i < nnode; i++)
    {
	FcSortNode	*node = *n++;
	FcBool		adds_chars = FcFalse;

	/*
	 * Only fetch node charset if we'd need it
	 */
	if (cs)
	{
	    FcCharSet	*ncs;

	    if (FcPatternGetCharSet (node->pattern, FC_CHARSET, 0, &ncs) !=
		FcResultMatch)
	        continue;

	    if (!FcCharSetMerge (cs, ncs, &adds_chars))
		goto bail;
	}

	/*
	 * If this font isn't a subset of the previous fonts,
	 * add it to the list
	 */
	if (!i || !trim || adds_chars)
	{
	    FcPatternReference (node->pattern);
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Add ");
		FcPatternPrint (node->pattern);
	    }
	    if (!FcFontSetAdd (fs, node->pattern))
	    {
		FcPatternDestroy (node->pattern);
		goto bail;
	    }
	}
    }
    if (csp)
    {
	*csp = cs;
	cs = 0;
    }

    ret = FcTrue;

bail:
    if (cs)
	FcCharSetDestroy (cs);

    return ret;
}

void
FcFontSetSortDestroy (FcFontSet *fs)
{
    FcFontSetDestroy (fs);
}

FcFontSet *
FcFontSetSort (FcConfig	    *config FC_UNUSED,
	       FcFontSet    **sets,
	       int	    nsets,
	       FcPattern    *p,
	       FcBool	    trim,
	       FcCharSet    **csp,
	       FcResult	    *result)
{
    FcFontSet	    *ret;
    FcFontSet	    *s;
    FcSortNode	    *nodes;
    FcSortNode	    **nodeps, **nodep;
    int		    nnodes;
    FcSortNode	    *new;
    int		    set;
    int		    f;
    int		    i;
    int		    nPatternLang;
    FcBool    	    *patternLangSat;
    FcValue	    patternLang;

    assert (sets != NULL);
    assert (p != NULL);
    assert (result != NULL);

    /* There are some implementation that relying on the result of
     * "result" to check if the return value of FcFontSetSort
     * is valid or not.
     * So we should initialize it to the conservative way since
     * this function doesn't return NULL anymore.
     */
    if (result)
	*result = FcResultNoMatch;

    if (FcDebug () & FC_DBG_MATCH)
    {
	printf ("Sort ");
	FcPatternPrint (p);
    }
    nnodes = 0;
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	nnodes += s->nfont;
    }
    if (!nnodes)
	return FcFontSetCreate ();

    for (nPatternLang = 0;
	 FcPatternGet (p, FC_LANG, nPatternLang, &patternLang) == FcResultMatch;
	 nPatternLang++)
	;
	
    /* freed below */
    nodes = malloc (nnodes * sizeof (FcSortNode) +
		    nnodes * sizeof (FcSortNode *) +
		    nPatternLang * sizeof (FcBool));
    if (!nodes)
	goto bail0;
    nodeps = (FcSortNode **) (nodes + nnodes);
    patternLangSat = (FcBool *) (nodeps + nnodes);

    new = nodes;
    nodep = nodeps;
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	if (!s)
	    continue;
	for (f = 0; f < s->nfont; f++)
	{
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Font %d ", f);
		FcPatternPrint (s->fonts[f]);
	    }
	    new->pattern = s->fonts[f];
	    if (!FcCompare (p, new->pattern, new->score, result))
		goto bail1;
	    if (FcDebug () & FC_DBG_MATCHV)
	    {
		printf ("Score");
		for (i = 0; i < PRI_END; i++)
		{
		    printf (" %g", new->score[i]);
		}
		printf ("\n");
	    }
	    *nodep = new;
	    new++;
	    nodep++;
	}
    }

    nnodes = new - nodes;

    qsort (nodeps, nnodes, sizeof (FcSortNode *),
	   FcSortCompare);

    for (i = 0; i < nPatternLang; i++)
	patternLangSat[i] = FcFalse;

    for (f = 0; f < nnodes; f++)
    {
	FcBool	satisfies = FcFalse;
	/*
	 * If this node matches any language, go check
	 * which ones and satisfy those entries
	 */
	if (nodeps[f]->score[PRI_LANG] < 2000)
	{
	    for (i = 0; i < nPatternLang; i++)
	    {
		FcValue	    nodeLang;
		
		if (!patternLangSat[i] &&
		    FcPatternGet (p, FC_LANG, i, &patternLang) == FcResultMatch &&
		    FcPatternGet (nodeps[f]->pattern, FC_LANG, 0, &nodeLang) == FcResultMatch)
		{
		    FcValue matchValue;
		    double  compare = FcCompareLang (&patternLang, &nodeLang, &matchValue);
		    if (compare >= 0 && compare < 2)
		    {
			if (FcDebug () & FC_DBG_MATCHV)
			{
			    FcChar8 *family;
			    FcChar8 *style;

			    if (FcPatternGetString (nodeps[f]->pattern, FC_FAMILY, 0, &family) == FcResultMatch &&
				FcPatternGetString (nodeps[f]->pattern, FC_STYLE, 0, &style) == FcResultMatch)
				printf ("Font %s:%s matches language %d\n", family, style, i);
			}
			patternLangSat[i] = FcTrue;
			satisfies = FcTrue;
			break;
		    }
		}
	    }
	}
	if (!satisfies)
	{
	    nodeps[f]->score[PRI_LANG] = 10000.0;
	}
    }

    /*
     * Re-sort once the language issues have been settled
     */
    qsort (nodeps, nnodes, sizeof (FcSortNode *),
	   FcSortCompare);

    ret = FcFontSetCreate ();
    if (!ret)
	goto bail1;

    if (!FcSortWalk (nodeps, nnodes, ret, csp, trim))
	goto bail2;

    free (nodes);

    if (FcDebug() & FC_DBG_MATCH)
    {
	printf ("First font ");
	FcPatternPrint (ret->fonts[0]);
    }
    if (ret->nfont > 0)
	*result = FcResultMatch;

    return ret;

bail2:
    FcFontSetDestroy (ret);
bail1:
    free (nodes);
bail0:
    return 0;
}

FcFontSet *
FcFontSort (FcConfig	*config,
	    FcPattern	*p,
	    FcBool	trim,
	    FcCharSet	**csp,
	    FcResult	*result)
{
    FcFontSet	*sets[2];
    int		nsets;

    assert (p != NULL);
    assert (result != NULL);

    *result = FcResultNoMatch;

    if (!config)
    {
	config = FcConfigGetCurrent ();
	if (!config)
	    return 0;
    }
    nsets = 0;
    if (config->fonts[FcSetSystem])
	sets[nsets++] = config->fonts[FcSetSystem];
    if (config->fonts[FcSetApplication])
	sets[nsets++] = config->fonts[FcSetApplication];
    return FcFontSetSort (config, sets, nsets, p, trim, csp, result);
}
#define __fcmatch__
#include "fcaliastail.h"
#undef __fcmatch__
