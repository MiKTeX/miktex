/* quadrant.c -- Internal function for trigonometric functions.

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

/* returns in quad the integer part of the division of x by Pi/2        */
/* the result is exact                                                  */
/* the returned value is the precision required to perform the division */
mpfr_prec_t
mpfi_quadrant (mpz_ptr quad, mpfr_srcptr x)
{
/* Assumption: x is neither a NaN nor an Infinite */
  int ok = 0;
  mpfr_prec_t prec;
  mpfi_t two_over_pi, tmp;

  prec = mpfr_get_prec (x);

  if (MPFR_IS_ZERO(x)) {
    mpz_set_ui(quad, 0);
  }
  else {
    mpfi_init2 (two_over_pi, prec);
    mpfi_init2 (tmp, prec);

    do {
      mpfi_const_pi (two_over_pi);
      mpfi_ui_div (two_over_pi, 2, two_over_pi);

      mpfi_mul_fr (tmp, two_over_pi, x);
      mpfr_floor (&(tmp->left), &(tmp->left));
      mpfr_floor (&(tmp->right), &(tmp->right));

      ok =  mpfr_cmp (&(tmp->left), &(tmp->right));
      if (ok != 0) {
        prec += BITS_PER_MP_LIMB;
        mpfi_set_prec (two_over_pi, prec);
        mpfi_set_prec (tmp, prec);
      }
    } while (ok != 0);

    mpfr_get_z (quad, &(tmp->left), MPFR_RNDN); /* exact */

    mpfi_clear (two_over_pi);
    mpfi_clear (tmp);
  }

 return prec;
}
