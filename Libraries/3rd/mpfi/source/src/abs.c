/* abs.c -- Absolute value.

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

#include "mpfi-impl.h"

int
mpfi_abs (mpfi_ptr a, mpfi_srcptr b)
{
  /* the result a contains the absolute values of every element of b */
  int inexact_right, inexact = 0;

  if (MPFI_NAN_P (b)) {
    mpfr_set_nan (&(a->left));
    mpfr_set_nan (&(a->right));
    MPFR_RET_NAN;
  }

  if (MPFI_IS_NONNEG (b))
    inexact = mpfi_set (a, b);
  else if (MPFI_IS_NONPOS (b))
    inexact = mpfi_neg (a, b);
  else { /* b contains 0 */
    if (mpfr_cmpabs (&(b->left), &(b->right)) < 0) {
      inexact_right = mpfr_set (&(a->right), &(b->right), MPFI_RNDU);
    }
    else {
      inexact_right = mpfr_neg (&(a->right), &(b->left), MPFI_RNDU);
    }
    mpfr_set_si (&(a->left), 0, MPFI_RNDD);

    if (inexact_right)
      inexact += 2;
  }

  return inexact;
}
