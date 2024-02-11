/* log10p1.c -- Logarithm, in base 10, of (an interval plus 1).

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2022
                     Spaces project, Inria Lorraine
                     and Salsa project, INRIA Rocquencourt,
                     and Arenaire project, Inria Rhone-Alpes, France
                     and AriC project, Inria Grenoble Rhone-Alpes and Inria Lyon, France
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

/* Computes the log, in base 10, of (an interval plus 1)              */
int
mpfi_log10p1 (mpfi_ptr a, mpfi_srcptr b)
{
  mpfr_prec_t tmp_prec_left, tmp_prec_right;
  mpfr_t tmp_left, tmp_right;
  int inexact_log, inexact_p1; /* for versions of MPFR < 4.2 */
  int inexact_left, inexact_right, inexact=0;

  #if ( defined(MPFR_VERSION_MAJOR) &&  (MPFR_VERSION_MAJOR >= 4) && defined(MPFR_VERSION_MINOR) & (MPFR_VERSION_MINOR >= 2) )
    {
    inexact_left = mpfr_log10p1 (&(a->left), &(b->left), MPFI_RNDD);
    inexact_right = mpfr_log10p1 (&(a->right), &(b->right), MPFI_RNDU);

    if (inexact_left)
      inexact += 1;
    if (inexact_right)
      inexact += 2;
    }
  #else
    {
    /* Without any guarantee that the bounds are the tightest = correctly rounded ones */
    inexact_p1 = mpfi_add_d(a, a, 1.0);
    inexact_log = mpfi_log10(a, b);
    if ( (inexact_log %2) || (inexact_p1%2) )
      inexact += 1;
    if ( (inexact_log >= 2) || (inexact_p1 >= 2) )
      inexact += 2;
    }
  #endif


  /* do not allow +0 as upper bound */
  if (mpfr_zero_p (&(a->right)) && !mpfr_signbit (&(a->right))) {
    mpfr_neg (&(a->right), &(a->right), MPFI_RNDD);
  }

  if ( MPFI_NAN_P (a) )
    MPFR_RET_NAN;

  return inexact;
}
