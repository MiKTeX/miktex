/* set_ld.c -- Assign a floating-point long double to an interval.

Copyright 2018
                     AriC project, Inria Grenoble - Rhone-Alpes, France

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
mpfi_set_ld (mpfi_ptr a, const long double b)
{
  int inexact_left, inexact_right, inexact=0;

  inexact_left = mpfr_set_ld (&(a->left), b, MPFI_RNDD);
  inexact_right = mpfr_set_ld (&(a->right), b, MPFI_RNDU);

  if ( MPFI_NAN_P (a) )
    MPFR_RET_NAN;

  if (b == 0.0) {
    /* fix signed zero so as to return [+0, -0] */
    mpfr_setsign (&(a->left), &(a->left), 0, MPFI_RNDU);
    mpfr_setsign (&(a->right), &(a->right), 1, MPFI_RNDD);
  }

  if (inexact_left)
    inexact += 1;
  if (inexact_right)
    inexact += 2;

  return inexact;
}

/* Combined initialization and assignment      */

int
mpfi_init_set_ld (mpfi_ptr a, const long double b)
{
  int inexact_left, inexact_right, inexact = 0;

  inexact_left = mpfr_init_set_ld (&(a->left), b, MPFI_RNDD);
  inexact_right = mpfr_init_set_ld (&(a->right), b, MPFI_RNDU);

  if ( MPFI_NAN_P (a) )
    MPFR_RET_NAN;

  if (b == 0.0) {
    /* fix signed zero so as to return [+0, -0] */
    mpfr_setsign (&(a->left), &(a->left), 0, MPFI_RNDU);
    mpfr_setsign (&(a->right), &(a->right), 1, MPFI_RNDD);
  }

  if (inexact_left)
    inexact += 1;
  if (inexact_right)
    inexact += 2;

  return inexact;
}
