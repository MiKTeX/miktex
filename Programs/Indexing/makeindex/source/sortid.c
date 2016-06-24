/*
 *
 *  This file is part of
 *	MakeIndex - A formatter and format independent index processor
 *
 *  Copyright (C) 1998-2012 by the TeX Live project.
 *  Copyright (C) 1989 by Chen & Harrison International Systems, Inc.
 *  Copyright (C) 1988 by Olivetti Research Center
 *  Copyright (C) 1987 by Regents of the University of California
 *
 *  Author:
 *	Pehong Chen
 *	Chen & Harrison International Systems, Inc.
 *	Palo Alto, California
 *	USA
 *
 *  Contributors:
 *	Please refer to the CONTRIB file that comes with this release
 *	for a list of people who have contributed to this and/or previous
 *	release(s) of MakeIndex.
 *
 *  All rights reserved by the copyright holders.  See the copyright
 *  notice distributed with this software for a complete description of
 *  the conditions under which it is made available.
 *
 */

#include    "mkind.h"
#include    "qsort.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

static	long	idx_gc;

static int check_mixsym (const char *x, const char *y);
static int compare (const void *va, const void *vb);
static int compare_one (const char *x, const char *y);
static int compare_page (const FIELD_PTR *a, const FIELD_PTR *b);
static int compare_string (const unsigned char *a, const unsigned char *b);
static int new_strcmp (const unsigned char *a, const unsigned char *b,
           int option);

void
sort_idx(void)
{
#ifdef HAVE_SETLOCALE
    char *prev_locale;
#endif

    MESSAGE("Sorting entries...");
#ifdef HAVE_SETLOCALE
    prev_locale = setlocale(LC_COLLATE, NULL);
    setlocale(LC_COLLATE, "");
#endif
    idx_dc = 0;
    idx_gc = 0L;
    qqsort(idx_key, (size_t)idx_gt, sizeof(FIELD_PTR), compare);
#ifdef HAVE_SETLOCALE
    setlocale(LC_COLLATE, prev_locale);
#endif
    MESSAGE1("done (%ld comparisons).\n", idx_gc);
}

static int
compare(const void *va, const void *vb)
{
#if defined(MIKTEX)
    const FIELD_PTR *a = (FIELD_PTR*)va;
    const FIELD_PTR *b = (FIELD_PTR*)vb;
#else
    const FIELD_PTR *a = va;
    const FIELD_PTR *b = vb;
#endif
    int     i;
    int     dif;

    idx_gc++;
    IDX_DOT(CMP_MAX);

    for (i = 0; i < FIELD_MAX; i++) {
	/* compare the sort fields */
	if ((dif = compare_one((*a)->sf[i], (*b)->sf[i])) != 0)
	    break;

	/* compare the actual fields */
	if ((dif = compare_one((*a)->af[i], (*b)->af[i])) != 0)
	    break;
    }

    /* both key aggregates are identical, compare page numbers */
    if (i == FIELD_MAX)
	dif = compare_page(a, b);
    return (dif);
}

static int
compare_one(const char *x, const char *y)
{
    int     m;
    int     n;

    if ((x[0] == NUL) && (y[0] == NUL))
	return (0);

    if (x[0] == NUL)
	return (-1);

    if (y[0] == NUL)
	return (1);

    m = group_type(x);
    n = group_type(y);

    /* both pure digits */
    if ((m >= 0) && (n >= 0))
	return (m - n);

    /* x digit, y non-digit */
    if (m >= 0) {
	if (german_sort)
	    return (1);
	else
	    return ((n == -1) ? 1 : -1);
    }
    /* x non-digit, y digit */
    if (n >= 0) {
	if (german_sort)
	    return (-1);
	else
	    return ((m == -1) ? -1 : 1);
    }
    /* strings started with a symbol (including digit) */
    if ((m == SYMBOL) && (n == SYMBOL))
	return (check_mixsym(x, y));

    /* x symbol, y non-symbol */
    if (m == SYMBOL)
	return (-1);

    /* x non-symbol, y symbol */
    if (n == SYMBOL)
	return (1);

    /* strings with a leading letter, the ALPHA type */
    return (compare_string((const unsigned char*)x, (const unsigned char*)y));
}

static int
check_mixsym(const char *x, const char *y)
{
    int     m;
    int     n;

    m = ISDIGIT(x[0]);
    n = ISDIGIT(y[0]);

    if (m && !n)
	return (1);

    if (!m && n)
	return (-1);

    return (locale_sort ? strcoll(x, y) : strcmp(x, y));
}


static int
compare_string(const unsigned char *a, const unsigned char *b)
{
    int     i = 0;
    int     j = 0;
    int     al;
    int     bl;

    if (locale_sort) return strcoll((const char *)a, (const char *)b);

    while ((a[i] != NUL) || (b[j] != NUL)) {
	if (a[i] == NUL)
	    return (-1);
	if (b[j] == NUL)
	    return (1);
	if (letter_ordering) {
	    if (a[i] == SPC)
		i++;
	    if (b[j] == SPC)
		j++;
	}
	al = TOLOWER(a[i]);
	bl = TOLOWER(b[j]);

	if (al != bl)
	    return (al - bl);
	i++;
	j++;
    }
    if (german_sort)
	return (new_strcmp(a, b, GERMAN));
    else
	return (strcmp((const char*)a, (const char*)b));
}

static int
compare_page(const FIELD_PTR *a, const FIELD_PTR *b)
{
    int     m = 0;
    short   i = 0;

    while ((i < (*a)->count) && (i < (*b)->count) &&
	   ((m = (*a)->npg[i] - (*b)->npg[i]) == 0))
    {
	i++;
    }
    if (m == 0)
    {				/* common leading page numbers match */
	if ((i == (*a)->count) && (i == (*b)->count))
	{			/* all page numbers match */
	    /***********************************************************
	    We have identical entries, except possibly in encap fields.
	    The ordering is tricky here.  Consider the following input
	    sequence of index names, encaps, and page numbers:

		foo|(	2
		foo|)	6
		foo|(	6
		foo|)	10

	    This might legimately occur when a page range ends, and
	    subsequently, a new range starts, on the same page.  If we
	    just order by range_open and range_close (here, parens),
	    then we will produce

		foo|(	2
		foo|(	6
		foo|)	6
		foo|)	10

	    This will later generate the index entry

		foo, 2--6, \({6}, 10

	    which is not only wrong, but has also introduced an illegal
	    LaTeX macro, \({6}, because the merging step treated this
	    like a \see{6} entry.

	    The solution is to preserve the original input order, which
	    we can do by treating range_open and range_close as equal,
	    and then ordering by input line number.  This will then
	    generate the correct index entry

		foo, 2--10

	    Ordering inconsistencies from missing range open or close
	    entries, or mixing roman and arabic page numbers, will be
	    detected later.
	    ***********************************************************/

#define isrange(c) ( ((c) == idx_ropen) || ((c) == idx_rclose) )

	    /* Order two range values by input line number */

	    if (isrange(*(*a)->encap) && isrange(*(*b)->encap))
		m = (*a)->lc - (*b)->lc;

	    /* Handle identical encap fields; neither is a range delimiter */

	    else if (STREQ((*a)->encap, (*b)->encap))
	    {
		/* If neither are yet marked duplicate, mark the second
		of them to be ignored. */
		if (((*a)->type != DUPLICATE) &&
		    ((*b)->type != DUPLICATE))
		    (*b)->type = DUPLICATE;
		/* leave m == 0 to show equality */
	    }

	    /* Encap fields differ: only one may be a range delimiter, */
	    /* or else neither of them is.   If either of them is a range */
	    /* delimiter, order by input line number; otherwise, order */
	    /* by name. */

	    else
	    {
		if ( isrange(*(*a)->encap) || isrange(*(*b)->encap) )
		    m = (*a)->lc - (*b)->lc; /* order by input line number */
		else			/* order non-range items by */
					/* their encap strings */
		    m = compare_string((const unsigned char*)((*a)->encap),
				       (const unsigned char*)((*b)->encap));
	    }
	}
	else if ((i == (*a)->count) && (i < (*b)->count))
	    m = -1;
	else if ((i < (*a)->count) && (i == (*b)->count))
	    m = 1;
    }
    return (m);
}


static int
new_strcmp(const unsigned char *s1, const unsigned char *s2, int option)
{
    int     i;

    i = 0;
    while (s1[i] == s2[i])
	if (s1[i++] == NUL)
	    return (0);
    if (option)			       /* ASCII */
	return (isupper(s1[i]) ? -1 : 1);
    else			       /* GERMAN */
	return (isupper(s1[i]) ? 1 : -1);
}
