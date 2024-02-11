/* div.c -- Divide two intervals.

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
mpfi_div (mpfi_ptr a, mpfi_srcptr b, mpfi_srcptr c)
{
  mpfr_t tmp;
  int inexact_left = 0, inexact_right = 0, inexact = 0;

  if ( MPFI_NAN_P (b) || MPFI_NAN_P (c) ) {
    mpfr_set_nan (&(a->left));
    mpfr_set_nan (&(a->right));
    MPFR_RET_NAN;
  }

  if (MPFI_HAS_ZERO_NONSTRICT (c)) {
    if ( MPFI_HAS_ZERO (c) || MPFI_HAS_ZERO (b) ) { /* a = ]-oo, +oo [ */
      mpfr_set_inf (&(a->left), -1);
      mpfr_set_inf (&(a->right), 1);
    }
    else if (MPFI_IS_NONNEG (c)) {  /* c >= 0 and its left endpoint is 0 */
      if (MPFI_IS_NONNEG (b)) {                    /* a = [ bl/cr, +oo [ */
        inexact_left = mpfr_div (&(a->left), &(b->left), &(c->right), MPFI_RNDD);
        mpfr_set_inf (&(a->right), 1);
      }
      else { /* b <= 0 */                         /* a = ] -oo, br/cr ] */
        inexact_right = mpfr_div (&(a->right), &(b->right), &(c->right), MPFI_RNDU);
        mpfr_set_inf (&(a->left), -1);
      }
    }
    else { /* c <= 0 and its right endpoint is 0 */
      if (MPFI_IS_NONNEG (b)) {                    /* a = ] -oo, bl/cl ] */
        inexact_right = mpfr_div (&(a->right), &(b->left), &(c->left), MPFI_RNDU);
        mpfr_set_inf (&(a->left), -1);
      }
      else { /* b <= 0 */                         /* a = [ br/cl, +oo [ */
        inexact_left = mpfr_div (&(a->left), &(b->right), &(c->left), MPFI_RNDD);
        mpfr_set_inf (&(a->right), 1);
      }
    }
  }
  else if (MPFI_IS_POS (c)) {
    if (MPFI_IS_NONNEG (b)) {                       /* a = [ bl/cr, br/cl ] */
      mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
      inexact_left  = mpfr_div (tmp, &(b->left), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_div (&(a->right), &(b->right), &(c->left), MPFI_RNDU);
      mpfr_set (&(a->left), tmp, MPFI_RNDD); /* exact */
      mpfr_clear (tmp);
    }
    else if (MPFI_IS_NONPOS (b)) {                 /* a = [ bl/cl, br/cr ] */
      inexact_left  = mpfr_div (&(a->left), &(b->left), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_div (&(a->right), &(b->right), &(c->right), MPFI_RNDU);
    }
    else { /* b contains 0 in its interior */     /* a = [ bl/cl, br/cl ] */
      mpfr_init2 (tmp, mpfr_get_prec (&(a->right)));
      inexact_right = mpfr_div (tmp, &(b->right), &(c->left), MPFI_RNDU);
      inexact_left  = mpfr_div (&(a->left), &(b->left), &(c->left), MPFI_RNDD);
      mpfr_set (&(a->right), tmp, MPFI_RNDU); /* exact */
      mpfr_clear (tmp);
    }
  }
  else { /* c < 0 */
    mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
    if (MPFI_IS_NONNEG (b)) {                       /* a = [ br/cr, bl/cl ] */
      inexact_left  = mpfr_div (tmp, &(b->right), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_div (&(a->right), &(b->left), &(c->left), MPFI_RNDU);
    }
    else if (MPFI_IS_NONPOS (b)) {                 /* a = [ br/cl, bl/cr ] */
      inexact_left  = mpfr_div (tmp, &(b->right), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_div (&(a->right), &(b->left), &(c->right), MPFI_RNDU);
    }
    else { /* b contains 0 in its interior */     /* a = [ br/cr, bl/cr ] */
      inexact_left  = mpfr_div (tmp, &(b->right), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_div (&(a->right), &(b->left), &(c->right), MPFI_RNDU);
    }
    mpfr_set (&(a->left), tmp, MPFI_RNDD); /* exact */
    mpfr_clear (tmp);
  }

  if (MPFI_NAN_P (a))
    MPFR_RET_NAN;

  if (inexact_left)
    inexact += 1;
  if (inexact_right)
    inexact += 2;

  /* do not allow -0 as lower bound */
  if (mpfr_zero_p (&(a->left)) && mpfr_signbit (&(a->left))) {
    mpfr_neg (&(a->left), &(a->left), MPFI_RNDU);
  }
  /* do not allow +0 as upper bound */
  if (mpfr_zero_p (&(a->right)) && !mpfr_signbit (&(a->right))) {
    mpfr_neg (&(a->right), &(a->right), MPFI_RNDD);
  }

  return inexact;
}
