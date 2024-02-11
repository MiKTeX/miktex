/* q_sub.c -- Subtract an interval by an rational number.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2011, 2012,
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
#include "mpfi_config.h"

#ifndef HAVE_MPFR_Q_SUB
static int
mpfr_q_sub (mpfr_ptr x, mpq_srcptr z, mpfr_srcptr y, mpfr_rnd_t rnd)
{
  /* mpfr_q_sub does not exist (at least up to version 3.0) */

  /* Here we use the fact that x and y do not point to the same variable */
  int inex;

  inex = mpfr_sub_q (x, y, z, rnd == MPFI_RNDD ? MPFI_RNDU : MPFI_RNDD);
  mpfr_neg (x, x, MPFI_RNDD);

  return -inex;
}
#endif /* HAVE_MPFR_Q_SUB */

int
mpfi_q_sub (mpfi_ptr a, mpq_srcptr b, mpfi_srcptr c)
{
  mpfr_t tmp;
  int inexact_left, inexact_right, inexact = 0;

  if (MPFI_IS_ZERO (c)) {
    return mpfi_set_q (a, b);
  }
  else if (!mpq_sgn(b)) {
    return mpfi_neg (a, c);
  }
  else {
    mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
    inexact_left = mpfr_q_sub (tmp, b, &(c->right), MPFI_RNDD);
    inexact_right = mpfr_q_sub (&(a->right), b, &(c->left), MPFI_RNDU);
    mpfr_set (&(a->left), tmp, MPFI_RNDD); /* exact */
    mpfr_clear (tmp);

    /* do not allow -0 as lower bound */
    if (mpfr_zero_p (&(a->left)) && mpfr_signbit (&(a->left))) {
      mpfr_neg (&(a->left), &(a->left), MPFI_RNDU);
    }
    /* do not allow +0 as upper bound */
    if (mpfr_zero_p (&(a->right)) && !mpfr_signbit (&(a->right))) {
      mpfr_neg (&(a->right), &(a->right), MPFI_RNDD);
    }

    if (MPFI_NAN_P (a))
      MPFR_RET_NAN;
    if (inexact_left)
      inexact += 1;
    if (inexact_right)
      inexact += 2;

    return inexact;
    }

}

