/* blow.c -- Interval blow.

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

/* keeps the same center and multiplies the radius by (1+fact), the result may
   be grossly overestimated */

int
mpfi_blow (mpfi_ptr y, mpfi_srcptr x, double fact)
/* if c = mid (x) and r = rad (x), y = [c - (1+fact)*r , c + (1+fact)*r] */
{
  mpfr_prec_t prec;
  mpfr_t radius, factor;
  mpfr_t centre;
  int inex_diam, inex_div, inex_conv, inex_factor, inex_rad;
  int inex_centre, inex_left, inex_right;
  int inexact = 0;

  if (MPFI_NAN_P (x)) {
    mpfr_set_nan (&(y->left));
    mpfr_set_nan (&(y->right));
    MPFR_RET_NAN;
  }

  prec = mpfi_get_prec (x);
  mpfr_init2 (radius, prec);
  mpfr_init2 (factor, prec);
  mpfr_init2 (centre, prec);

  inex_diam = mpfi_diam_abs (radius, x);
  if (mpfr_zero_p (radius)) {
    /* x is a singleton */
    return mpfi_set (y, x);
  }
  inex_div = mpfr_div_2exp (radius, radius, 1, MPFI_RNDU); /* either underflow
                                                             or exact*/

  /* factor must be greater than 1 + |fact|, so it is not possible to perform
     this addition directly in C with the double precision since the usual
     rouding mode is rounding to nearest. */
  inex_conv = mpfr_set_d (factor, fact < 0.0 ? -fact : fact, MPFI_RNDU);
  inex_factor = mpfr_add_ui (factor, factor, 1, MPFI_RNDU);

  inex_rad = mpfr_mul (radius, radius, factor, MPFI_RNDU);
  inex_centre = mpfi_mid (centre, x);
  inex_left = mpfr_sub (&(y->left), centre, radius, MPFI_RNDD);
  inex_right = mpfr_add (&(y->right), centre, radius, MPFI_RNDU);

  mpfr_clear (radius);
  mpfr_clear (factor);
  mpfr_clear (centre);

  if ( MPFI_NAN_P (y) )
    MPFR_RET_NAN;

  /* do not allow -0 as lower bound */
  if (mpfr_zero_p (&(y->left)) && mpfr_signbit (&(y->left))) {
    mpfr_neg (&(y->left), &(y->left), MPFI_RNDU);
  }
  /* do not allow +0 as upper bound */
  if (mpfr_zero_p (&(y->right)) && !mpfr_signbit (&(y->right))) {
    mpfr_neg (&(y->right), &(y->right), MPFI_RNDD);
  }

  if (inex_diam || inex_div || inex_conv || inex_factor || inex_rad
      || inex_centre || inex_left)
    inexact += 1;
  if (inex_diam || inex_div || inex_conv || inex_factor || inex_rad
      || inex_centre || inex_right)
    inexact += 2;

  return inexact;
}
