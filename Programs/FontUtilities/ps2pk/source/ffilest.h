/*
 * $XConsortium: fontfilest.h,v 1.3 91/07/16 20:15:16 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _FONTFILEST_H_
#define _FONTFILEST_H_

#ifdef XSERVER
#include    <X11/Xos.h>
#else
#include    "types.h"
#include    "Xstuff.h"
#endif

#include    "fontmisc.h"
#include    "fontstruct.h"
#include    "fontfile.h"
#include    "fontxlfd.h"

typedef struct _FontName {
    const char	*name;
    short	length;
    short	ndashes;
} FontNameRec;

typedef struct _FontScaled {
    FontScalableRec	vals;
    FontEntryPtr	bitmap;
    FontPtr		pFont;
} FontScaledRec;

typedef struct _FontScalableExtra {
    FontScalableRec	defaults;
    int			numScaled;
    int			sizeScaled;
    FontScaledPtr	scaled;
    pointer		private;
} FontScalableExtraRec;

typedef struct _FontScalableEntry {
    FontRendererPtr	    renderer;
    char		    *fileName;
    FontScalableExtraPtr   extra;
} FontScalableEntryRec;

/*
 * This "can't" work yet - the returned alias string must be permanent,
 * but this layer would need to generate the appropriate name from the
 * resolved scalable + the XLFD values passed in.  XXX
 */

typedef struct _FontScaleAliasEntry {
    char		*resolved;
} FontScaleAliasEntryRec;

typedef struct _FontBitmapEntry {
    FontRendererPtr	renderer;
    char		*fileName;
    FontPtr		pFont;
} FontBitmapEntryRec;

typedef struct _FontAliasEntry {
    char	*resolved;
} FontAliasEntryRec;

typedef struct _FontBCEntry {
    FontScalableRec	    vals;
    FontEntryPtr	    entry;
} FontBCEntryRec;

typedef struct _FontEntry {
    FontNameRec	name;
    int		type;
    union _FontEntryParts {
	FontScalableEntryRec	scalable;
	FontBitmapEntryRec	bitmap;
	FontAliasEntryRec	alias;
	FontBCEntryRec		bc;
    } u;
} FontEntryRec;

typedef struct _FontTable {
    int		    used;
    int		    size;
    FontEntryPtr    entries;
    Bool	    sorted;
} FontTableRec;

typedef struct _FontDirectory {
    char	   *directory;
    uint32_t       dir_mtime;
    uint32_t       alias_mtime;
    FontTableRec   scalable;
    FontTableRec   nonScalable;
} FontDirectoryRec;

typedef struct _FontRenderer {
    const char    *fileSuffix;
    int	    fileSuffixLen;
    int	    (*OpenBitmap)(/* fpe, pFont, flags, entry, fileName, format, fmask */);
    int	    (*OpenScalable)(/* fpe, pFont, flags, entry, fileName, vals, format, fmask */);
    int	    (*GetInfoBitmap)(/* fpe, pFontInfo, entry, fileName */);
    int	    (*GetInfoScalable)(/* fpe, pFontInfo, entry, fileName, vals */);
    int	    number;
} FontRendererRec;

typedef struct _FontRenders {
    int		    number;
    FontRendererPtr *renderers;
} FontRenderersRec, *FontRenderersPtr;

typedef struct _BitmapInstance {
    FontScalableRec	vals;
    FontBitmapEntryPtr	bitmap;
} BitmapInstanceRec, *BitmapInstancePtr;

typedef struct _BitmapScalablePrivate {
    int			numInstances;
    BitmapInstancePtr	instances;
} BitmapScalablePrivateRec, *BitmapScalablePrivatePtr;

typedef struct _BitmapSources {
    FontPathElementPtr	*fpe;
    int			size;
    int			count;
} BitmapSourcesRec, *BitmapSourcesPtr;

extern BitmapSourcesRec	FontFileBitmapSources;

extern void Type1CloseFont(FontPtr);

#endif /* _FONTFILEST_H_ */
