/*
 *
 *  This file is part of
 *	MakeIndex - A formatter and format independent index processor
 *
 *  Copyright (C) 1998-2011 by the TeX Live project.
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
#include    "genind.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

static FIELD_PTR curr = NULL;
static FIELD_PTR prev = NULL;
static FIELD_PTR begin = NULL;
static FIELD_PTR the_end = NULL;
static FIELD_PTR range_ptr;
static int level = 0;
static int prev_level = 0;
static char *encap = NULL;
static char *prev_encap = NULL;
static int in_range = FALSE;
static int encap_range = FALSE;
static char buff[2 * ARGUMENT_MAX];
static char line[2 * ARGUMENT_MAX];	/* output buffer */
static int ind_lc = 0;			/* overall line count */
static int ind_ec = 0;			/* erroneous line count */
static int ind_indent;

static	void	flush_line (int print);
static	void	insert_page (void);
static	int	make_entry (int n);
static	void	make_item (const char* term);
static  unsigned char first_letter (char* term);
static	void	new_entry (void);
static	void	old_entry (void);
static	int	page_diff (struct KFIELD *a,struct KFIELD *b);
static	void	put_header (int let);
static	void	wrap_line (int print);

void
gen_ind(void)
{
    int     n;
    int     tmp_lc;

    MESSAGE1("Generating output file %s...", ind_fn);
    PUT(preamble);
    ind_lc += prelen;
    if (init_page)
	insert_page();

    /* reset counters for putting out dots */
    idx_dc = 0;
    for (n = 0; n < idx_gt; n++) {
	if (idx_key[n]->type != DUPLICATE)
	    if (make_entry(n)) {
		IDX_DOT(DOT_MAX);
	    }
    }
    tmp_lc = ind_lc;
    if (in_range) {
	curr = range_ptr;
	IND_ERROR1("Unmatched range opening operator %c.\n", idx_ropen);
    }
    prev = curr;
    flush_line(TRUE);
    PUT(delim_t);
    PUT(postamble);
    tmp_lc = ind_lc + postlen;
    if (ind_ec == 1) {
	DONE(tmp_lc, "lines written", ind_ec, "warning");
    } else {
	DONE(tmp_lc, "lines written", ind_ec, "warnings");
    }
}


static int
make_entry(int n)
{
    int     let;

    /* determine current and previous pointer */
    prev = curr;
    curr = idx_key[n];
    /* check if current entry is in range */
    if ((*curr->encap == idx_ropen) || (*curr->encap == idx_rclose))
	encap = &(curr->encap[1]);
    else
	encap = curr->encap;

    /* determine the current nesting level */
    if (n == 0) {
	prev_level = level = 0;
	let = *curr->sf[0];
	put_header(let);
	make_item(NIL);
    } else {
	prev_level = level;
	for (level = 0; level < FIELD_MAX; level++)
	    if (STRNEQ(curr->sf[level], prev->sf[level]) ||
		STRNEQ(curr->af[level], prev->af[level]))
		break;
	if (level < FIELD_MAX)
	    new_entry();

        /* Repeat test from just below to see if we are already in an
           open range.  If so, we don't want to output anything.  (It
           ends up being output as an erroneous \(.  See
           tests/nested-range-bb.tex.)  */           
	else if (! (*curr->encap == idx_ropen && in_range)) {
	    old_entry();
	}
    }

    if (*curr->encap == idx_ropen)
	if (in_range) {
	    IND_ERROR1("Extra range opening operator %c.\n", idx_ropen);
	} else {
	    in_range = TRUE;
	    range_ptr = curr;
	}
    else if (*curr->encap == idx_rclose)
	if (in_range) {
	    in_range = FALSE;
	    if (STRNEQ(&(curr->encap[1]), "") &&
		STRNEQ(prev_encap, &(curr->encap[1]))) {
IND_ERROR1("Range closing operator has an inconsistent encapsulator %s.\n",
			  &(curr->encap[1]));
	    }
	} else {
	    IND_ERROR1("Unmatched range closing operator %c.\n", idx_rclose);
	}
    else if ((*curr->encap != NUL) &&
	     STRNEQ(curr->encap, prev_encap) && in_range)
	IND_ERROR1("Inconsistent page encapsulator %s within range.\n",
		  curr->encap);
    return (1);
}


static void
make_item(const char *term)
{
    int     i;

    if (level > prev_level) {
	/* ascending level */
	if (*curr->af[level] == NUL)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s%s", term, item_u[level], curr->sf[level]);
#else
	    sprintf(line, "%s%s%s", term, item_u[level], curr->sf[level]);
#endif
	else
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s%s", term, item_u[level], curr->af[level]);
#else
	    sprintf(line, "%s%s%s", term, item_u[level], curr->af[level]);
#endif
	ind_lc += ilen_u[level];
    } else {
	/* same or descending level */
	if (*curr->af[level] == NUL)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s%s", term, item_r[level], curr->sf[level]);
#else
	    sprintf(line, "%s%s%s", term, item_r[level], curr->sf[level]);
#endif
	else
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s%s", term, item_r[level], curr->af[level]);
#else
	    sprintf(line, "%s%s%s", term, item_r[level], curr->af[level]);
#endif
	ind_lc += ilen_r[level];
    }

    i = level + 1;
    while (i < FIELD_MAX && *curr->sf[i] != NUL) {
	PUT(line);
	if (*curr->af[i] == NUL)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s", item_x[i], curr->sf[i]);
#else
	    sprintf(line, "%s%s", item_x[i], curr->sf[i]);
#endif
	else
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s", item_x[i], curr->af[i]);
#else
	    sprintf(line, "%s%s", item_x[i], curr->af[i]);
#endif
	ind_lc += ilen_x[i];
	level = i;		/* Added at 2.11 <brosig@gmdzi.gmd.de> */
	i++;
    }

    ind_indent = 0;
    strcat(line, delim_p[level]);
    SAVE;
}

static unsigned char
first_letter(char *term)
{
    if (thai_sort)
        return strchr("\340\341\342\343\344", term[0]) ? term[1] : term[0];

    return TOLOWER(term[0]);
}

static void
new_entry(void)
{
    int let = -1; /* see comment below */
    FIELD_PTR ptr;
#ifdef HAVE_SETLOCALE
    char *prev_locale;
    prev_locale = setlocale(LC_CTYPE, NULL);
    setlocale(LC_CTYPE, "");
#endif

    if (in_range) {
	ptr = curr;
	curr = range_ptr;
	IND_ERROR1("Unmatched range opening operator %c.\n", idx_ropen);
	in_range = FALSE;
	curr = ptr;
    }
    flush_line(TRUE);

    /* beginning of a new group? */

    if (((curr->group != ALPHA) && (curr->group != prev->group) &&
	 (prev->group == SYMBOL)) ||
	((curr->group == ALPHA) &&
	 ((unsigned char)(let = first_letter(curr->sf[0])) 
	  != first_letter(prev->sf[0]))) ||
	(german_sort &&
	 (curr->group != ALPHA) && (prev->group == ALPHA))) {
	PUT(delim_t);
	PUT(group_skip);
	ind_lc += skiplen;
	/* beginning of a new letter? */
	/* Although we may use let unassigned here, it doesn't matter,
	   because put_header will not use its arg except in case
	   ALPHA, when it is assigned in the midst of condition above.
	   Let's not perturb the logic, just initialize it (above) to
	   avoid the warning.  */
	put_header(let);
	make_item(NIL);
    } else
	make_item(delim_t);
#ifdef HAVE_SETLOCALE
    setlocale(LC_CTYPE, prev_locale);
#endif
}


static void
old_entry(void)
{
    int     diff;

    /* current entry identical to previous one: append pages */
    diff = page_diff(the_end, curr);

    if ((prev->type == curr->type) && (diff != -1) &&
	(((diff == 0) && (prev_encap != NULL) && STREQ(encap, prev_encap)) ||
	 (merge_page && (diff == 1) &&
	  (prev_encap != NULL) && STREQ(encap, prev_encap)) ||
	 in_range)) {
	the_end = curr;
	/* extract in-range encaps out */
	if (in_range &&
	    (*curr->encap != NUL) &&
	    (*curr->encap != idx_rclose) &&
	    STRNEQ(curr->encap, prev_encap)) {
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s%s%s%s", encap_p, curr->encap,
#else
	    sprintf(buff, "%s%s%s%s%s", encap_p, curr->encap,
#endif
		    encap_i, curr->lpg, encap_s);
	    wrap_line(FALSE);
	}
	if (in_range)
	    encap_range = TRUE;
    } else {
	flush_line(FALSE);
	if ((diff == 0) && (prev->type == curr->type)) {
IND_ERROR(
"Conflicting entries: multiple encaps for the same page under same key.\n");
	} else if (in_range && (prev->type != curr->type)) {
IND_ERROR(
"Illegal range formation: starting & ending pages are of different types.\n");
	} else if (in_range && (diff == -1)) {
IND_ERROR(
"Illegal range formation: starting & ending pages cross chap/sec breaks.\n");
	}
	SAVE;
    }
}


static int
page_diff(FIELD_PTR a,FIELD_PTR b)
{
    short   i;

    if (a->count != b->count)
	return (-1);
    for (i = 0; i < a->count - 1; i++)
	if (a->npg[i] != b->npg[i])
	    return (-1);
    return (b->npg[b->count - 1] - a->npg[a->count - 1]);
}

static void
put_header(int let)
{
    if (headings_flag)
    {
	PUT(heading_pre);
	ind_lc += headprelen;
	switch (curr->group)
	{
	case SYMBOL:
	    if (headings_flag > 0)
	    {
		PUT(symhead_pos);
	    }
	    else
	    {
		PUT(symhead_neg);
	    }
	    break;
	case ALPHA:
	    if (headings_flag > 0)
	    {
		let = TOUPPER(let);
		PUTC(let);
	    }
	    else
	    {
		let = TOLOWER(let);
		PUTC(let);
	    }
	    break;
	default:
	    if (headings_flag > 0)
	    {
		PUT(numhead_pos);
	    }
	    else
	    {
		PUT(numhead_neg);
	    }
	    break;
	}
	PUT(heading_suf);
	ind_lc += headsuflen;
    }
}


/* changes for 2.12 (May 20, 1993) by Julian Reschke (jr@ms.maus.de):
   Use keywords suffix_2p, suffix_3p or suffix_mp for one, two or
   multiple page ranges (when defined) */

static void
flush_line(int print)
{
    char    tmp[sizeof(buff)];

    if (page_diff(begin, the_end) != 0)
	if (encap_range || (page_diff(begin, prev) > (*suffix_2p ? 0 : 1)))
	{
		int diff = page_diff(begin, the_end);
		
		if ((diff == 1) && *suffix_2p)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
		    SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s", begin->lpg, suffix_2p);
#else
		    sprintf(buff, "%s%s", begin->lpg, suffix_2p);
#endif
		else if ((diff == 2) && *suffix_3p)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
		    SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s", begin->lpg, suffix_3p);
#else
		    sprintf(buff, "%s%s", begin->lpg, suffix_3p);
#endif
		else if ((diff >= 2) && *suffix_mp)
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
		    SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s", begin->lpg, suffix_mp);
#else
		    sprintf(buff, "%s%s", begin->lpg, suffix_mp);
#endif
		else
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
		    SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s%s", begin->lpg, delim_r, the_end->lpg);
#else
		    sprintf(buff, "%s%s%s", begin->lpg, delim_r, the_end->lpg);
#endif

	    encap_range = FALSE;
	}
	else
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	        SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s%s", begin->lpg, delim_n, the_end->lpg);
#else
	   	sprintf(buff, "%s%s%s", begin->lpg, delim_n, the_end->lpg);
#endif
    else
    {
	encap_range = FALSE; /* might be true from page range on same page */
#if defined(MIKTEX) && defined(HAVE_STRCPY_S)
	STRCPY(buff, begin->lpg);
#else
	strcpy(buff, begin->lpg);
#endif
    }

    if (*prev_encap != NUL)
    {
#if defined(MIKTEX) && defined(HAVE_STRCPY_S)
        STRCPY (tmp, buff);
#else
	strcpy(tmp, buff);
#endif
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	SPRINTF_S(buff, sizeof(buff) / sizeof(buff[0]), "%s%s%s%s%s",
#else
	sprintf(buff, "%s%s%s%s%s",
#endif
		encap_p, prev_encap, encap_i, tmp, encap_s);
    }
    wrap_line(print);
}

static void
wrap_line(int print)
{
    int     len;

    len = strlen(line) + strlen(buff) + ind_indent;
    if (print) {
	if (len > linemax) {
	    PUTLN(line);
	    PUT(indent_space);
	    ind_indent = indent_length;
	} else
	    PUT(line);
	PUT(buff);
    } else {
	if (len > linemax) {
	    PUTLN(line);
#if defined(MIKTEX) && defined(HAVE_SPRINTF_S)
	    SPRINTF_S(line, sizeof(line) / sizeof(line[0]), "%s%s%s", indent_space, buff, delim_n);
#else
	    sprintf(line, "%s%s%s", indent_space, buff, delim_n);
#endif
	    ind_indent = indent_length;
	} else {
	    strcat(buff, delim_n);
	    strcat(line, buff);
	}
    }
}


static void
insert_page(void)
{
    int     i = 0;
    int     j = 0;
    int     page = 0;

    if (even_odd >= 0) {
	/* find the rightmost digit */
	while (pageno[i++] != NUL);
	j = --i;
	/* find the leftmost digit */
	while (isdigit((unsigned char)pageno[--i]) && i > 0);
	if (!isdigit((unsigned char)pageno[i]))
	    i++;
	/* convert page from literal to numeric */
	page = strtoint(&pageno[i]) + 1;
	/* check even-odd numbering */
	if (((even_odd == 1) && (page % 2 == 0)) ||
	    ((even_odd == 2) && (page % 2 == 1)))
	    page++;
	pageno[j + 1] = NUL;
	/* convert page back to literal */
	while (page >= 10) {
	    pageno[j--] = TOASCII(page % 10);
	    page = page / 10;
	}
	pageno[j] = TOASCII(page);
	if (i < j) {
	    while (pageno[j] != NUL)
		pageno[i++] = pageno[j++];
	    pageno[i] = NUL;
	}
    }
    PUT(setpage_open);
    PUT(pageno);
    PUT(setpage_close);
    ind_lc += setpagelen;
}
