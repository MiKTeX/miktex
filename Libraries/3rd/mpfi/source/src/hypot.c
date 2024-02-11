/* hypot.c -- Euclidean distance.

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
mpfi_hypot (mpfi_ptr a, mpfi_srcptr b, mpfi_srcptr c)
{
  int inexact_left, inexact_right, inexact=0;
  mpfi_t tmp;
  int bmax_left_p, cmax_left_p;

  if (MPFI_NAN_P (b) || MPFI_NAN_P (c)) {
    /* hypot (y, +-inf) = hypot (+-inf, y) = +inf even if y is NaN */
    if (MPFI_NAN_P (b)
        && mpfr_inf_p (&(c->left)) && mpfr_inf_p (&(c->right))
        && mpfr_equal_p (&(c->left), &(c->right))) {
      mpfi_abs (a, c);
      return inexact;
    }
    else if (MPFI_NAN_P (c)
             && mpfr_inf_p (&(b->left)) && mpfr_inf_p (&(b->right))
             && mpfr_equal_p (&(b->left), &(b->right))) {
      mpfi_abs (a, b);
      return inexact;
    }

    mpfr_set_nan (&(a->left));
    mpfr_set_nan (&(a->right));
    MPFR_RET_NAN;
  }

  mpfi_init2 (tmp, mpfi_get_prec (a));

  bmax_left_p = mpfr_cmp_abs (&(b->left), &(b->right)) > 0;
  cmax_left_p = mpfr_cmp_abs (&(c->left), &(c->right)) > 0;

  if (MPFI_HAS_ZERO_NONSTRICT (b)) {
    if (MPFI_HAS_ZERO_NONSTRICT (c))
      inexact_left  = mpfr_set_ui (&(tmp->left), 0, MPFI_RNDU);
    else
      inexact_left  = mpfr_abs (&(tmp->left), cmax_left_p ? &(c->right) : &(c->left), MPFI_RNDD);
  }
  else {
    if (MPFI_HAS_ZERO_NONSTRICT (c))
      inexact_left  = mpfr_abs (&(tmp->left), bmax_left_p ? &(b->right) : &(b->left), MPFI_RNDD);
    else
      inexact_left = mpfr_hypot (&(tmp->left), bmax_left_p ? &(b->right) : &(b->left), cmax_left_p ? &(c->right) : &(c->left), MPFI_RNDD);
  }

  inexact_right = mpfr_hypot (&(tmp->right), bmax_left_p ? &(b->left) : &(b->right), cmax_left_p ? &(c->left) : &(c->right), MPFI_RNDU);

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
