/* atan2.c -- Arc tangent of two intervals.

Copyright 2009, 2010,
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
mpfi_atan2 (mpfi_ptr a, mpfi_srcptr b, mpfi_srcptr c)
{
  int inexact_left, inexact_right, inexact=0;
  mpfi_t tmp;

  if ( MPFI_NAN_P (b) || MPFI_NAN_P (c) ) {
    mpfr_set_nan (&(a->left));
    mpfr_set_nan (&(a->right));
    MPFR_RET_NAN;
  }

  mpfi_init2 (tmp, mpfi_get_prec (a));

  if (MPFI_IS_POS (b)) {
    if (MPFI_IS_POS (c)) {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->left), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->right), &(c->left), MPFI_RNDU);
    }
    else if (MPFI_IS_NEG (c)) {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->right), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->left), &(c->left), MPFI_RNDU);
    }
    else {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->left), &(c->right), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->left), &(c->left), MPFI_RNDU);
    }
  }
  else if (MPFI_IS_NEG (b)) {
    if (MPFI_IS_POS (c)) {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->left), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->right), &(c->right), MPFI_RNDU);
    }
    else if (MPFI_IS_NEG (c)) {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->right), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->left), &(c->right), MPFI_RNDU);
    }
    else {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->right), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->right), &(c->right), MPFI_RNDU);
    }
  }
  else {
    if (MPFI_IS_POS (c)) {
      inexact_left = mpfr_atan2 (&(tmp->left), &(b->left), &(c->left), MPFI_RNDD);
      inexact_right = mpfr_atan2 (&(tmp->right), &(b->right), &(c->left), MPFI_RNDU);
    }
    else {
      inexact_left = -mpfr_const_pi (&(tmp->left), MPFI_RNDU);
      mpfr_neg (&(tmp->left), &(tmp->left), MPFI_RNDD);
      inexact_right = mpfr_const_pi (&(tmp->right), MPFI_RNDU);
    }
  }

  if (mpfr_sgn (&(tmp->left)) == 0) {
    /* +0 as left endpoint */
    mpfr_setsign (&(tmp->left), &(tmp->left), 0, MPFI_RNDU);
  }
  if (mpfr_sgn (&(tmp->right)) == 0) {
    /* -0 as right endpoint */
    mpfr_setsign (&(tmp->right), &(tmp->right), 1, MPFI_RNDD);
  }

  mpfi_set (a, tmp);
  mpfi_clear (tmp);

  if (inexact_left)
    inexact += 1;
  if (inexact_right)
    inexact += 2;

  return inexact;
}
