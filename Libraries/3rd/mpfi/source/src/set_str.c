/* set_str.c -- Set an interval to the value of a string.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010,
                     Spaces project, Inria Lorraine
                     and Salsa project, INRIA Rocquencourt,
                     and Arenaire project, Inria Rhone-Alpes, France
                     and Lab. ANO, USTL (Univ. of Lille),  France

This file is part of the MPFI Library.

The MPFI Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPFI Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPFI Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
MA 02110-1301, USA. */

#include "mpfi_io.h"
#include "mpfi-impl.h"

int
mpfi_set_str (mpfi_ptr x, const char *s, int base)
{
  const char *cur;
  char *end;

  cur = s;
  /* read the blanks */
  while (*cur && MPFI_ISSPACE (*cur)) ++cur;
  if (*cur == '\0') {
    /* Error: blank string */
    return 1;
  }

  /* Now *cur is the first non blank character */
  if (*cur == '[') {
    ++cur;

    /* read the blanks between '[' and the number itself */
    while (*cur && MPFI_ISSPACE (*cur)) ++cur;
    if (*cur == '\0') {
      /* Error: no number in string */
      return 1;
    }

    mpfr_strtofr (&(x->left), cur, &end, base, MPFI_RNDD);
    if (end == cur) {
      /* Error: no number in string */
      return 1;
    }
    cur = end;

    /* Read (possibly) blank characters between the first number and the comma */
    while (*cur && MPFI_ISSPACE (*cur) ) ++cur;
    if (*cur == '\0') {
      /* Error: only one number in string */
      return 1;
    }

    if (*cur != ',') {
      /* Error: missing comma */
      return 1;
    }
    ++cur;

    /* From now on, we are reading the second number */

    /* read (possibly) blank characters between the comma and the 2nd number */
    while (*cur && MPFI_ISSPACE (*cur)) ++cur;
    if (*cur == '\0') {
      /* Error: only one number in string */
      return 1;
    }

    /* Now *cur is the first character of the 2nd number */
    mpfr_strtofr (&(x->right), cur, &end, base, MPFI_RNDU);
    if (end == cur) {
      /* Error: only one number in string */
      return 1;
    }
    cur = end;

    /* Read (possibly) blank characters between the 2nd number and */
    /* closing square bracket */
    while (*cur && MPFI_ISSPACE (*cur)) ++cur;
    if (*cur == '\0') {
      /* Error: missing closing square bracket */
      return 1;
    }

    if (*cur != ']') {
      /* Missing closing square bracket */
      return 1;
    }

    /* Note that the string may contain any character after the */
    /* closing square bracket: they will be ignored */
  }
  else {
    /* Only one number to store as an interval */
    /* s[i] is the first non blank character and is not an */
    /* opening square bracket */

    /* Note that the whole string must be a valid number */
    if (mpfr_set_str (&(x->left), cur,  base, MPFI_RNDD))
      return 1;
    mpfr_set_str (&(x->right), cur, base, MPFI_RNDU);
  }

  return 0;
}

int
mpfi_init_set_str (mpfi_ptr x, const char *s, int base)
{
  mpfi_init (x);
  return (mpfi_set_str (x, s, base));
}
