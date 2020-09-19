// -*- related-file-name: "../include/lcdf/globmatch.hh" -*-

/* globmatch.{cc,hh} -- glob_match() function for shell globbing
 *
 * Copyright (c) 2000-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/globmatch.hh>
#include <lcdf/string.hh>
#include <lcdf/vector.hh>

bool
glob_match(const String& str, const String& pattern)
{
    const char* sdata = str.data();
    const char* pdata = pattern.data();
    int slen = str.length();
    int plen = pattern.length();
    int spos = 0, ppos = 0;
    Vector<int> glob_ppos, glob_spos1, glob_spos2;

    while (1) {
	while (ppos < plen)
	    switch (pdata[ppos]) {

	      case '?':
		if (spos >= slen)
		    goto done;
		spos++;
		ppos++;
		break;

	      case '*':
		glob_ppos.push_back(ppos + 1);
		glob_spos1.push_back(spos);
		glob_spos2.push_back(slen);
		spos = slen;
		ppos++;
		break;

	      case '[': {
		  if (spos >= slen)
		      goto done;

		  // find end of character class
		  int p = ppos + 1;
		  bool negated = false;
		  if (p < plen && pdata[p] == '^') {
		      negated = true;
		      p++;
		  }
		  int first = p;
		  if (p < plen && pdata[p] == ']')
		      p++;
		  while (p < plen && pdata[p] != ']')
		      p++;
		  if (p >= plen) // not a character class at all
		      goto ordinary;

		  // parse character class
		  bool in = false;
		  for (int i = first; i < p && !in; i++) {
		      int c1 = pdata[i];
		      int c2 = c1;
		      if (i < p - 2 && pdata[i+1] == '-') {
			  c2 = pdata[i+2];
			  i += 2;
		      }
		      if (sdata[spos] >= c1 && sdata[spos] <= c2)
			  in = true;
		  }

		  if ((negated && in) || (!negated && !in))
		      goto done;
		  ppos = p + 1;
		  spos++;
		  break;
	      }

	      default:
	      ordinary:
		if (spos >= slen || sdata[spos] != pdata[ppos])
		    goto done;
		spos++;
		ppos++;
		break;

	    }

      done:
	if (spos == slen && ppos == plen)
	    return true;
	while (glob_ppos.size() && glob_spos1.back() == glob_spos2.back()) {
	    glob_ppos.pop_back();
	    glob_spos1.pop_back();
	    glob_spos2.pop_back();
	}
	if (glob_ppos.size()) {
	    glob_spos2.back()--;
	    spos = glob_spos2.back();
	    ppos = glob_ppos.back();
	} else
	    return false;
    }
}
