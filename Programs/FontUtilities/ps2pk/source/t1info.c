/* $XConsortium: t1info.c,v 1.9 92/03/20 16:00:13 eswu Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License, subject to the license given below, to use,
 * copy, modify, and distribute this software * and its
 * documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear
 * in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Carol H. Thompson  IBM Almaden Research Center
 *   Modeled on spinfo.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "types.h"
#include <stdio.h> 
#include <string.h> 
#include "util.h"
#include "ffilest.h"
#ifdef XSERVER
#include "FSproto.h"
#endif
#include "t1intf.h"

#ifdef WIN32
static void FontParseXLFDName(char *, FontScalablePtr, int);
#endif
static void FontComputeInfoAccelerators(FontInfoPtr);

#define DECIPOINTSPERINCH 722.7
#define DEFAULTRES 75
#define DEFAULTPOINTSIZE 120
 
enum scaleType {
    atom, pixel_size, point_size, resolution_x, resolution_y,
    average_width, scaledX, scaledY, unscaled, scaledXoverY,
    uncomputed
};
 
typedef struct _fontProp {
    const char *name;
    int32_t     atom;
    enum scaleType type;
} fontProp;
 
static fontProp fontNamePropTable[] = {  /* Example: */
    { "FOUNDRY", 0, atom },                  /* adobe */
    { "FAMILY_NAME", 0, atom },              /* times roman */
    { "WEIGHT_NAME", 0, atom },              /* bold */
    { "SLANT", 0, atom },                    /* i */
    { "SETWIDTH_NAME", 0, atom },            /* normal */
    { "ADD_STYLE_NAME", 0, atom },           /* */
    { "PIXEL_SIZE", 0, pixel_size },         /* 18 */
    { "POINT_SIZE", 0, point_size },         /* 180 */
    { "RESOLUTION_X", 0, resolution_x },     /* 72 */
    { "RESOLUTION_Y", 0, resolution_y },     /* 72 */
    { "SPACING", 0, atom },                  /* p */
    { "AVERAGE_WIDTH", 0, average_width },   /* 0 */
    { "CHARSET_REGISTRY", 0, atom },         /* ISO8859 */
    { "CHARSET_ENCODING", 0, atom }          /* 1 */
};
 
static fontProp extraProps[] = {
    { "FONT", 0, atom },
    { "COPYRIGHT", 0, atom }
};
 
/* this is a bit kludgy */
#define FONTPROP        0
#define COPYRIGHTPROP   1
 
#define NNAMEPROPS (sizeof(fontNamePropTable) / sizeof(fontProp))
#define NEXTRAPROPS (sizeof(extraProps) / sizeof(fontProp))
 
#define NPROPS  (NNAMEPROPS + NEXTRAPROPS)
 
/*ARGSUSED*/
static void
FillHeader(
    FontInfoPtr         pInfo,
    FontScalablePtr     Vals)
{
    /* OpenScalable in T1FUNCS sets the following:
    pInfo->firstCol,
    pInfo->firstRow,
    pInfo->lastCol, and
    pInfo->lastRow. */
    /* the following are ununsed
    pInfo->pad. */
 
    /* Items we should handle better someday +++ */
    pInfo->defaultCh = 0;
    pInfo->drawDirection = LeftToRight;
    /* our size is based only on Vals->pixel, so we won't
       create an anamorphic font (yet) */
    pInfo->anamorphic = 0;
    pInfo->inkMetrics = 0;  /* no ink metrics here */
    pInfo->cachable = 1;    /* no licensing (yet) */
}
 
static void
adjust_min_max(xCharInfo *minc, xCharInfo *maxc, xCharInfo *tmp)
{
#define MINMAX(field,ci) \
        if (minc->field > (ci)->field) \
             minc->field = (ci)->field; \
        if (maxc->field < (ci)->field) \
            maxc->field = (ci)->field;
 
    MINMAX(ascent, tmp);
    MINMAX(descent, tmp);
    MINMAX(leftSideBearing, tmp);
    MINMAX(rightSideBearing, tmp);
    MINMAX(characterWidth, tmp);
 
#undef  MINMAX
}
 
static void
ComputeBounds(
    FontInfoPtr         pInfo,
    CharInfoPtr         pChars,
    FontScalablePtr     Vals)
{
    int i;
    xCharInfo minchar, maxchar;
    int32_t width = 0;
    int numchars = 0;
    int totchars;
    int overlap;
    int maxlap;
 
    minchar.ascent = minchar.descent =
        minchar.leftSideBearing = minchar.rightSideBearing =
        minchar.characterWidth = 32767;
    minchar.attributes = 0;  /* What's this for? +++ */
    maxchar.ascent = maxchar.descent =
        maxchar.leftSideBearing = maxchar.rightSideBearing =
        maxchar.characterWidth = -32767;
    maxchar.attributes = 0;
 
    maxlap = -32767;
    totchars = pInfo->lastCol - pInfo->firstCol + 1;
    pInfo->allExist = 1;
    for (i = 0; i < totchars; i++,pChars++) {
        xCharInfo *pmetrics = &pChars->metrics;
 
        if (pmetrics->characterWidth) {
            width += pmetrics->characterWidth;
            numchars++;
            adjust_min_max(&minchar, &maxchar, pmetrics);
            overlap = pmetrics->rightSideBearing - pmetrics->characterWidth;
            if (overlap > maxlap) maxlap = overlap;
        }
        else pInfo->allExist = 0;
    }

    Vals->width = ((width * 10)+((numchars+1)/2)) / numchars;
    /* (We think the above average width value should be put into
        the Vals structure.  This may be wrong, and the proper
        behavior might be to regard the values in Vals as sacred,
        and for us to squirrel the computed number in a static, and
        then use that static in ComputeStdProps.) */
    pInfo->maxbounds = maxchar;
    pInfo->minbounds = minchar;
    pInfo->ink_maxbounds = maxchar;
    pInfo->ink_minbounds = minchar;
    pInfo->maxOverlap = maxlap + -(minchar.leftSideBearing);
 
    /* Set the pInfo flags */
    /* Properties set by FontComputeInfoAccelerators:
        pInfo->noOverlap;
        pInfo->terminalFont;
        pInfo->constantMetrics;
        pInfo->constantWidth;
        pInfo->inkInside;
 
    */
    FontComputeInfoAccelerators (pInfo);
}
 
static void
ComputeProps(
    FontInfoPtr         pInfo,
    FontScalablePtr     Vals,
    char                *Filename)
{
    int infoint;
    int infoBBox[4];
    int rc;
 
    QueryFontLib(Filename, "isFixedPitch", &infoint, &rc);
    if (!rc) {
        pInfo->constantWidth = infoint;
    }
    QueryFontLib((char *)0, "FontBBox", infoBBox, &rc);
    if (!rc) {
        pInfo->fontAscent = (infoBBox[3] * Vals->pixel) / 1000;
        pInfo->fontDescent = - (infoBBox[1] * Vals->pixel) / 1000;
    }
}
 
static void
ComputeStdProps(
    FontInfoPtr         pInfo,
    FontScalablePtr     Vals,
    char                *Filename,
    const char          *Fontname)
{
    FontPropPtr pp;
    int         i,
                nprops;
    fontProp   *fpt;
    char       *is_str;
    char       *ptr1,
               *ptr2;
    const char *infostrP;
    int       rc;
    char      scaledName[MAXFONTNAMELEN];
 
    strcpy (scaledName, Fontname);
#ifdef WIN32
    /* Fill in our copy of the fontname from the Vals structure */
    FontParseXLFDName (scaledName, Vals, FONT_XLFD_REPLACE_VALUE);
#endif
 
    /* This form of the properties is used by the X-client; the X-server
       doesn't care what they are. */
    nprops = pInfo->nprops = NPROPS;
    pInfo->isStringProp = (char *) Xalloc(sizeof(char) * nprops);
    pInfo->props = (FontPropPtr) Xalloc(sizeof(FontPropRec) * nprops);
    if (!pInfo->isStringProp || !pInfo->props) {
        Xfree(pInfo->isStringProp);
        pInfo->isStringProp = (char *) 0;
        Xfree(pInfo->props);
        pInfo->props = (FontPropPtr) 0;
        return;
    }
    (void) memset(pInfo->isStringProp, 0, (sizeof(char) * nprops));
 
    ptr2 = scaledName;
    for (i = NNAMEPROPS, pp = pInfo->props, fpt = fontNamePropTable, is_str = pInfo->isStringProp;
            i;
            i--, pp++, fpt++, is_str++) {
        ptr1 = ptr2 + 1;
        if (*ptr1 == '-')
            ptr2 = ptr1;
        else {
            if (i > 1)
                ptr2 = strchr(ptr1 + 1, '-');
            else
                ptr2 = strchr(ptr1 + 1, '\0');
        }
        pp->name = fpt->atom;
        switch (fpt->type) {
         case atom:  /* Just copy info from scaledName */
            *is_str = TRUE;
            pp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
            break;
         case pixel_size:
            pp->value = Vals->pixel;
            break;
         case point_size:
            pp->value = Vals->point;
            break;
         case resolution_x:
            pp->value = Vals->x;
            break;
         case resolution_y:
            pp->value = Vals->y;
            break;
         case average_width:
            pp->value = Vals->width;
            break;
         default: /* scaledX, scaledY, unscaled, scaledXoverY, and uncomputed */
            break;
        }
    }
 
    for (i = 0, fpt = extraProps;
          i < NEXTRAPROPS;
          i++, is_str++, pp++, fpt++) {
        pp->name = fpt->atom;
        switch (i) {
         case FONTPROP:
            /* Why do we need this property -- nice for debug anyway */
            *is_str = TRUE;
            pp->value = MakeAtom(scaledName, strlen(scaledName), TRUE);
            break;
         case COPYRIGHTPROP:
            *is_str = TRUE;
            QueryFontLib(Filename, "Notice", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "Copyright Notice not available";
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
        }
    }
}
 
void
T1FillFontInfo(
    FontPtr             pFont,
    FontScalablePtr     Vals,
    char                *Filename,
    const char          *Fontname)
{
    FontInfoPtr         pInfo = &pFont->info;
    struct type1font *p = (struct type1font *)pFont->fontPrivate;
 
    FillHeader(pInfo, Vals);
 
    ComputeBounds(pInfo, p->glyphs, Vals);
 
    ComputeProps(pInfo, Vals, Filename);
    ComputeStdProps(pInfo, Vals, Filename, Fontname);
}
 
/* Called once, at renderer registration time */
void
T1InitStdProps(void)
{
    int         i;
    fontProp   *t;
 
    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t = fontNamePropTable; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
    i = sizeof(extraProps) / sizeof(fontProp);
    for (t = extraProps; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
}

#ifdef WIN32
static void FontParseXLFDName(char *scaledName, FontScalablePtr Vals, int foo) { ; }
#endif
static void FontComputeInfoAccelerators(FontInfoPtr foo) { ; }
