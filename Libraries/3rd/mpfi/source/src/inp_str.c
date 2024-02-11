/* inp_str.c -- Set an interval from an input stream.

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

size_t
mpfi_inp_str (mpfi_ptr x, FILE *s, int base)
{
  size_t blank = 0;
  size_t t;
  size_t pos;
  size_t size = 256;
  char *str;
  int c;

  void * (*mpfi_allocate_func) (size_t);
  void * (*mpfi_reallocate_func) (void *,size_t, size_t);
  void   (*mpfi_free_func) (void *, size_t);

  mp_get_memory_functions (&mpfi_allocate_func, &mpfi_reallocate_func,
                           &mpfi_free_func);

  c = fgetc (s);
  while (MPFI_ISSPACE (c)) {
    c = fgetc (s);
    ++blank;
  }

  if (c != '[') {
    /* one single number defining an interval */
    ungetc (c, s);
    t = mpfr_inp_str (&(x->left), s, base, MPFI_RNDD);
    mpfr_set (&(x->right), &(x->left), MPFI_RNDD);
    mpfr_nextabove (&(x->right));

    return t != 0 ? blank + t : 0;
  }
  else {
    /* interval given by two endpoints between square brackets */
    /* the interval is copied into a string and handled by mpfi_set_str */
    str = (char *)(*mpfi_allocate_func)(size);

    pos = 0;
    str[pos++] = '[';

    while (c != ']') {
      c = fgetc (s);
      if (c == EOF)
        break;
      str[pos++] = c;
      if (pos == size) {
        str = (char *)(*mpfi_reallocate_func)(str, size, 2 * size);
        size *= 2;
      }
    }
    str[pos]='\0';

    t = mpfi_set_str (x, str, base);
    (*mpfi_free_func)(str, size);

    return t == 0 ? blank + pos : 0;
  }
}
