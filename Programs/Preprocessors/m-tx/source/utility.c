/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "utility.pas" */


#include "p2c.h"


#define UTILITY_G
#include "utility.h"


#ifndef STRINGS_H
#include "strings.h"
#endif


#define blank           ' '
#define dummy           '\0'


short wordCount(Char *s)
{
  short i, n, FORLIM;

  if (*s == '\0')
    return 0;
  if (s[0] == blank)
    n = 0;
  else
    n = 1;
  FORLIM = strlen(s);
  for (i = 1; i <= FORLIM - 1; i++) {
    if (s[i-1] == blank && s[i] != blank)
      n++;
  }
  return n;
}


Char *GetNextWord(Char *Result, Char *s, Char Delim, Char Term)
{
  /* A delimiter is a character that separates words, but forms no part
     of them.  A terminator is a character that appears at the end of
     a word. */
  short n = 1;
  short start, last;

  last = strlen(s);
  while (n <= last && s[n-1] == Delim)
    n++;
  start = n;
  while (n <= last && s[n-1] != Term && s[n-1] != Delim)
    n++;
  if (n <= last && s[n-1] == Term)
    n++;
  substr_(Result, s, start, n - start);
  while (n <= last && s[n-1] == Delim)
    n++;
  predelete(s, n - 1);
  return Result;
}


Char *NextWord(Char *Result, Char *s_, Char Delim, Char Term)
{
  Char s[256];

  strcpy(s, s_);
  return (GetNextWord(Result, s, Delim, Term));
}


Char *plural(Char *Result, short n)
{
  if (n == 1)
    return strcpy(Result, "");
  else
    return strcpy(Result, "s");
}


short curtail(Char *s, Char c)
{
  short Result = 0;
  short l;

  l = strlen(s);
  if (s[l-1] == c) {
    shorten(s, l - 1);
    return l;
  }
  return Result;
}


Char *toString(Char *Result, short n)
{
  Char s[256];

  sprintf(s, "%d", n);
  return strcpy(Result, s);
}


short digit(Char c)
{
  return (c - '0');
}


boolean equalsIgnoreCase(Char *s1_, Char *s2_)
{
  Char s1[256], s2[256];

  strcpy(s1, s1_);
  strcpy(s2, s2_);
  toUpper(s1);
  toUpper(s2);
  return (strcmp(s1, s2) == 0);
}


boolean startsWithIgnoreCase(Char *s1_, Char *s2_)
{
  Char s1[256], s2[256];

  strcpy(s1, s1_);
  strcpy(s2, s2_);
  toUpper(s1);
  toUpper(s2);
  return (startsWith(s1, s2));
}


boolean startsWithBracedWord(Char *P_)
{
  Char P[256];
  Char w[256];

  strcpy(P, P_);
  GetNextWord(w, P, blank, dummy);
  return (w[0] == '{' && w[strlen(w) - 1] == '}');
}


void trim(Char *s)
{
  short k;

  k = posNot(blank, s);
  if (k > 1)
    predelete(s, k - 1);
  else if (k == 0)
    *s = '\0';
}


boolean endsWith(Char *s1_, Char *s2)
{
  Char s1[256];
  short l1, l2;

  strcpy(s1, s1_);
  l1 = strlen(s1);
  l2 = strlen(s2);
  if (l1 < l2)
    return false;
  predelete(s1, l1 - l2);
  return (strcmp(s1, s2) == 0);
}


/* Local variables for grep: */
struct LOC_grep {
  Char *source, *pattern;
  short p1[10], p2[10];
  short i, p, s, index;
  boolean matching;
} ;

Local void remember(short s1, short s2, struct LOC_grep *LINK)
{
  if (LINK->index > 9)
    _Escape(9999);
  LINK->p1[LINK->index] = s1;
  LINK->p2[LINK->index] = s2;
  LINK->s = s2 + 1;
  LINK->index++;
}

Local void matchnum(struct LOC_grep *LINK)
{
  boolean allowsign = false, allowpoint = false, quit = false;
  short s0;

  LINK->matching = false;
  s0 = LINK->s;
  if (LINK->p < strlen(LINK->pattern)) {
    if (LINK->pattern[LINK->p] == '#') {
      LINK->p++;
      allowsign = true;
      if (LINK->p < strlen(LINK->pattern)) {
	if (LINK->pattern[LINK->p] == '#') {
	  LINK->p++;
	  allowpoint = true;
	}
      }
    }
  }
  if (allowsign &&
      (LINK->source[LINK->s-1] == '-' || LINK->source[LINK->s-1] == '+')) {
    LINK->s++;
    if (LINK->s > strlen(LINK->source))
      return;
  }
  while (!quit && LINK->s <= strlen(LINK->source)) {
    if (LINK->source[LINK->s-1] == '.') {
      if (!allowpoint)
	quit = true;
      else {
	LINK->s++;
	allowpoint = false;
      }
    }
    if (isdigit(LINK->source[LINK->i-1])) {
      LINK->s++;
      LINK->matching = true;
    } else
      quit = true;
  }
  if (LINK->matching)
    remember(s0, LINK->s - 1, LINK);
}

Local void matchmeta(struct LOC_grep *LINK)
{
  if (LINK->p < strlen(LINK->pattern))
    LINK->p++;
  if (LINK->source[LINK->s-1] == LINK->pattern[LINK->p-1]) {
    LINK->s++;
    LINK->p++;
  } else
    LINK->matching = false;
}

Local void subgrep(struct LOC_grep *LINK)
{
  LINK->matching = true;
  if (LINK->pattern[LINK->p-1] == '*') {
    remember(LINK->s, strlen(LINK->source), LINK);
    LINK->p++;
    return;
  }
  if (LINK->pattern[LINK->p-1] == '?') {
    remember(LINK->s, LINK->s, LINK);
    LINK->p++;
    return;
  }
  if (LINK->pattern[LINK->p-1] == '#') {
    matchnum(LINK);
    return;
  }
  if (LINK->pattern[LINK->p-1] == '\\') {
    matchmeta(LINK);
    return;
  }
  if (LINK->source[LINK->s-1] == LINK->pattern[LINK->p-1]) {
    LINK->s++;
    LINK->p++;
  } else
    LINK->matching = false;
}


/*--- Match/Replace package --- */

/* Search and replace.  Stops when pattern no longer matches source.
          Pattern wildcards:
  ?   Any single character
  *   Any string
  #   An unsigned integer
  ##  A signed integer
  ### A signed number maybe with a decimal part
          Pattern metacharacters:
  \x  where x is any character, stands for that character
          Target wildcards
  \0 to \9  Value of corresponding source wildcard
          Target metacharacters
  \   When not followed by 0..9 or \, stands for itself
  \\  Backslash
*/
void grep(Char *source_, Char *pattern_, Char *target)
{
  struct LOC_grep V;
  short j, t, reg;
  Char product[256];
  boolean trigger = false;
  short FORLIM, FORLIM1;
  Char STR2[256];

  V.source = source_;
  V.pattern = pattern_;
  V.index = 0;
  V.s = 1;
  V.p = 1;
  for (V.i = 0; V.i <= 9; V.i++) {
    V.p1[V.i] = 1;
    V.p2[V.i] = 0;
  }
  while (V.matching && V.p <= strlen(V.pattern) && V.s <= strlen(V.source))
    subgrep(&V);
  *product = '\0';
  FORLIM = strlen(target);
  for (t = 0; t <= FORLIM - 1; t++) {
    if (trigger) {
      reg = digit(target[t]);
      if ((unsigned)reg <= 9) {
	FORLIM1 = V.p2[reg];
	for (j = V.p1[reg] - 1; j <= FORLIM1 - 1; j++)
	  sprintf(product + strlen(product), "%c", V.source[j]);
      } else if (target[t] == '\\')
	strcat(product, "\\");
      else
	sprintf(product + strlen(product), "\\%c", target[t]);
      trigger = false;
    } else if (target[t] == '\\' && t + 1 < strlen(target))
      trigger = true;
    else
      sprintf(product + strlen(product), "%c", target[t]);
  }
  strcpy(V.source, substr_(STR2, V.source, V.s, strlen(V.source)));
  strcpy(V.pattern, substr_(STR2, V.pattern, V.p, strlen(V.pattern)));
  strcpy(target, product);
}


/* "match" tests whether the source matches the pattern exactly */
boolean match(Char *source_, Char *pattern_)
{
  Char source[256], pattern[256];
  static Char target[256] = "";

  strcpy(source, source_);
  strcpy(pattern, pattern_);
  grep(source, pattern, target);
  return (*source == '\0' && *pattern == '\0');
}


/* "translate" replaces the pattern by the target in the source. */
Char *translate(Char *Result, Char *source_, Char *pattern_, Char *target_)
{
  Char source[256], pattern[256], target[256];

  strcpy(source, source_);
  strcpy(pattern, pattern_);
  strcpy(target, target_);
  grep(source, pattern, target);
  return strcpy(Result, target);
}




/* End. */
