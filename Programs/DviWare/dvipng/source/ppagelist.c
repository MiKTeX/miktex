/* ppagelist.c */

/************************************************************************

  Part of the dvipng distribution

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <http://www.gnu.org/licenses/>.

  Copyright (C) 2002-2010 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

/* Some code at the end of this file is adapted from dvips */

static int32_t first=PAGE_FIRSTPAGE, last=PAGE_LASTPAGE;
static bool    abspage=false, reverse=false;
bool           no_ppage=true;

/* dvips' behaviour:
 * -pp outputs _all_ pages with the correct numbers,
 * -p, -l outputs from the first occurrence of firstpage to the first
 * occurrence of lastpage. Using '=' means absolute pagenumbers
 */

void FirstPage(int32_t page, bool data)
{
  first=page;
  abspage |= data;
}
void LastPage(int32_t page,bool data)
{
  last=page;
  abspage |= data;
}
void Reverse(bool new)
{
  reverse=new;
}
/*-->NextPPage*/
/**********************************************************************/
/****************************  NextPPage  *****************************/
/**********************************************************************/
/* Return the page in turn on our queue */
/* (Implicitly, PAGE_POST is never in the pagelist) */
bool InPageList(int32_t i);

struct page_list* NextPPage(void* dvi, struct page_list* page)
{
  if (! reverse) { /*********** normal order */
    if (page == NULL) { /* first call: find first page */ 
      if (no_ppage)
	return(NULL);
      page=FindPage(dvi,first,abspage);
    } else              /* later calls: count up, except "last" page */ 
      page=(last==page->count[abspage ? 0 : 10]) ? NULL : NextPage(dvi,page);
    /* seek for pages in pagelist */ 
    while (page!=NULL && ! InPageList(page->count[0]))
      page=(last==page->count[abspage ? 0 : 10]) ? NULL : NextPage(dvi,page);
  } else { /******************** reverse order */
    if (page == NULL) { /* first call: find "last" page */ 
      if (no_ppage)
	return(NULL);
      page=FindPage(dvi,last,abspage);
    } else              /* later calls: count down, except "first" page */
      page=(first==page->count[abspage ? 0 : 10]) ? NULL : PrevPage(dvi,page);
    /* seek for pages in pagelist */ 
    while (page!=NULL && ! InPageList(page->count[0])) 
      page=(first==page->count[abspage ? 0 : 10]) ? NULL : PrevPage(dvi,page);
  }
  return(page);
}

struct pp_list {
    struct pp_list *next;	/* next in a series of alternates */
    int32_t ps_low, ps_high;	/* allowed range */
} *ppages = 0;	/* the list of allowed pages */

/*-->InPageList*/
/**********************************************************************/
/******************************  InPageList  **************************/
/**********************************************************************/
/* Return true iff i is one of the desired output pages */

bool InPageList(int32_t i)
{
  register struct pp_list *pl = ppages;

  while (pl) {
    if ( i >= pl -> ps_low && i <= pl -> ps_high)
      return(true);		/* success */
    pl = pl -> next;
  }
  return(false);
}

static void ListPage(int32_t pslow, int32_t pshigh)
{
  register struct pp_list   *pl;

  /* Some added code, we want to reuse the list */
  no_ppage=false;
  pl = ppages;
  while (pl != NULL && pl->ps_low <= pl->ps_high)
    pl = pl->next;
  if (pl == NULL) {
    if ((pl = (struct pp_list *)malloc(sizeof(struct pp_list)))
	==NULL) 
      Fatal("cannot malloc memory for page queue");
    pl -> next = ppages;
    ppages = pl;
  }
  pl -> ps_low = pslow;
  pl -> ps_high = pshigh;
}

/* Parse a string representing a list of pages.  Return 0 iff ok.  As a
   side effect, the page selection(s) is (are) prepended to ppages. */

bool ParsePages(const char *s)
{
  const char *c;	/* conversion start */
  char *t;
  long int ps_low = PAGE_MINPAGE, ps_high = PAGE_MAXPAGE;

  while (*s==' ' || *s=='\t') s++;
  while (*s!='\0') {
    if (*s=='-' || *s==':') { /* range with no starting value */
      ps_low = PAGE_MINPAGE;
      c=s+1;
      ps_high = strtol(c,&t,10);
      s = t;
      if (c==s) ps_high=PAGE_MAXPAGE; /* no number */
      while (*s==' ' || *s=='\t') s++;
      if (*s=='-' || *s==':') { /* Oh, range with negative starting value */
	ps_low = -ps_high;
	c=s+1;
	ps_high = strtol(c,&t,10);
	s = t;
	if (c==s) ps_high=PAGE_MAXPAGE; /* no number */
      }
    } else { /* range with starting value, or singleton */
      c=s;
      ps_low = ps_high = strtol(c,&t,10);
      s = t;
      if (c==s) 
	return(true);
      if (*s=='-' || *s==':') { /* range */
	c=s+1;
	ps_high = strtol(c,&t,10);
	s = t;
	if (c==s) ps_high=PAGE_MAXPAGE; /* no number */
      }
    }
    ListPage(ps_low, ps_high);
    while (*s==' ' || *s=='\t' || *s==',') s++;
  }
  return(false);
}

/* Addition, we want to be able to clear the pplist */
void ClearPpList(void)
{
  register struct pp_list *pl = ppages;

  while (pl) {
    ppages=ppages->next;
    free(pl);
    pl = ppages;
  }
  first=PAGE_FIRSTPAGE;
  last=PAGE_LASTPAGE;
  abspage = false;
  no_ppage=true;
}

