/* diam.c -- Various diameter functions.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2018,
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

/* Absolute diameter                            */
int
mpfi_diam_abs (mpfr_ptr diam, mpfi_srcptr interv)
{
  return (mpfr_sub (diam, &(interv->right), &(interv->left), MPFI_RNDU));
}

/* Relative diameter                                     */
/* Always computes an overestimation                     */
/* return value: 0 if the result is exact, > 0 otherwise */
int
mpfi_diam_rel (mpfr_ptr diam, mpfi_srcptr interv)
{
  mpfr_t center;
  int inexact_mid, inexact_sub = 0, inexact = 0;

  mpfr_init2 (center, mpfr_get_prec (diam));

  /* if interv is bounded, compute d/|c| where d is the absolute diameter of
     interv and c is its midpoint
     if interv is not bounded, d and c are infinite, their quotient does not
     exist */
  inexact_sub = mpfr_sub (diam, &(interv->right), &(interv->left), MPFI_RNDU);

  inexact_mid = mpfi_mid (center, interv);
  if (mpfr_sgn (center) * inexact_mid > 0 && !mpfr_inf_p (center)) {
    /* the absolute value of center have to be underestimated for an
       overestimated quotient, except for the infinite cases where the
       division below should return NaN.
       Note that in case of underflow, mpfr_sgn(center)==0 and the
       if-condition fails, preventing the next operation to actually increase
       the absolute value of center */
    if (mpfr_sgn (center) > 0)
      mpfr_nextbelow (center);
    else
      mpfr_nextabove (center);
  }
  mpfr_abs (center, center, MPFI_RNDD); /* always exact */

  inexact = mpfr_div (diam, diam, center, MPFI_RNDU);
  mpfr_clear (center);

  if (mpfr_nan_p (diam))
    MPFR_RET_NAN;

  if (inexact || inexact_sub || inexact_mid)
    return 1;
  else
    return 0;
}

/* Diameter: relative if the interval does not contain 0 */
/* absolute otherwise                                    */
int
mpfi_diam (mpfr_ptr diam, mpfi_srcptr interv)
{

  if (!mpfi_has_zero (interv)) {
    return (mpfi_diam_rel (diam, interv));
  }
  else {
    return (mpfi_diam_abs (diam, interv));
  }
}
