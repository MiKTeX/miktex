/* out_str.c -- Output an interval representation in a stream.

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
mpfi_out_str (FILE *f, int base, size_t n_digits, mpfi_srcptr a)
{
  size_t res_left, res_right;
  fprintf (f, "[");
  res_left = mpfr_out_str (f, base, n_digits, &(a->left), MPFI_RNDD);
  fprintf (f, ",");
  res_right = mpfr_out_str (f, base, n_digits, &(a->right), MPFI_RNDU);
  fprintf (f, "]");
  if ( (res_left > 0) && (res_right > 0) )
    return res_left + res_right + 3; /* 3 stands for "[", "," and "]" */
  else
    return 0;
}
