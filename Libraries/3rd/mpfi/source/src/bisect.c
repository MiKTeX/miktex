/* bisect.c -- Split an interval into two halves.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2011,
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
mpfi_bisect (mpfi_ptr y1, mpfi_ptr y2, mpfi_srcptr y)
{
  mpfr_prec_t prec, prec1, prec2;
  mpfr_t centre;
  int inexact_centre;

  if ( MPFI_NAN_P (y) ) {
    mpfr_set_nan (&(y1->left));
    mpfr_set_nan (&(y1->right));
    mpfr_set_nan (&(y2->left));
    mpfr_set_nan (&(y2->right));
    MPFR_RET_NAN;
  }
  else if ( !mpfi_bounded_p (y) ) {
    mpfi_set (y1, y);
    mpfr_set_nan (&(y2->left));
    mpfr_set_nan (&(y2->right));
    MPFR_RET_NAN;
  }

  prec = mpfi_get_prec (y);
  prec1 = mpfi_get_prec (y1);
  prec2 = mpfi_get_prec (y2);
  if ( (prec1 >= prec2) && (prec1 >= prec))
    prec = prec1;
  else if ( (prec2 >= prec1) && (prec2 >= prec))
    prec = prec2;
  mpfr_init2 (centre, prec);

  inexact_centre = mpfi_mid (centre, y);

  mpfr_set (&(y1->left), &(y->left), MPFI_RNDD);
  mpfr_set (&(y2->right), &(y->right), MPFI_RNDU);
  mpfr_set (&(y1->right), centre, MPFI_RNDU); /* FIXME: double rounding
                                                        error */
  mpfr_set (&(y2->left), centre, MPFI_RNDD); /* FIXME: double rounding
                                                       error */

  /* do not allow +0 as upper bound. Note that left endpoint of y2
     cannot be -0 because if centre is zero its precision is
     sufficient for it to be an exact result (then center = +0). */
  if (mpfr_zero_p (&(y1->right)) && !mpfr_signbit (&(y1->right))) {
    mpfr_neg (&(y1->right), &(y1->right), MPFI_RNDD);
  }

  mpfr_clear (centre);
  return inexact_centre;
}
