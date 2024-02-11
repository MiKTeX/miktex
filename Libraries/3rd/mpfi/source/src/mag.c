/* mag.c -- Element with largest absolute value.

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

/* Magnitude: the largest absolute value of any element */
int
mpfi_mag (mpfr_ptr m, mpfi_srcptr x)
{
  int inexact;

  if (MPFI_NAN_P (x)) {
    mpfr_set_nan (m);
    MPFR_RET_NAN;
  }

  if (mpfi_is_nonneg_default (x)) {
    inexact = mpfr_set (m, &(x->right), MPFI_RNDU);
    if (mpfr_zero_p (m) && mpfr_signbit (m))
      /* fix sign of zero */
      mpfr_neg (m, m, MPFI_RNDU);
  }
  else if (mpfi_is_nonpos_default (x)) {
    inexact = mpfr_neg (m, &(x->left), MPFI_RNDU);
  }
  else { /* x contains 0 */
    inexact = mpfr_neg (m, &(x->left), MPFI_RNDU);
    if (mpfr_cmp (m, &(x->right)) < 0)
      inexact = mpfr_set (m, &(x->right), MPFI_RNDU);
  }

  return inexact;
}
