/* div_ext.c -- Divide two intervals: 
	when the denominator contains 0,
	returns two intervals and a gap between them.

  The returned integer is the number of returned intervals.

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
mpfi_div_ext (mpfi_ptr res1, mpfi_ptr res2, mpfi_srcptr op1, mpfi_srcptr op2)
{
  mpfr_t tmp1, tmp2;
  int nb_res = 0, tmp;

  if ( MPFI_NAN_P (op1) || MPFI_NAN_P (op2) ) {
    mpfr_set_nan (&(res1->left));
    mpfr_set_nan (&(res1->right));
    mpfr_set_nan (&(res2->left));
    mpfr_set_nan (&(res2->right));
    return 0;
  }

  if (MPFI_HAS_ZERO (op2)) {
    if ( MPFR_IS_INF(&(op2->left)) && MPFR_IS_INF(&(op2->right)) ) {
      mpfr_set_inf (&(res1->left), -1);
      mpfr_set_inf (&(res1->right), 1);
      mpfr_set_nan (&(res2->left));
      mpfr_set_nan (&(res2->right));
      nb_res = 1;
    }
    else if ( MPFI_HAS_ZERO_NONSTRICT (op1) ) {
      mpfr_set_inf (&(res1->left), -1);
      mpfr_set_inf (&(res1->right), 1);
      mpfr_set_nan (&(res2->left));
      mpfr_set_nan (&(res2->right));
      nb_res = 1;
    }
    else if ( MPFI_IS_STRICTLY_NEG (op1) ) {
      mpfr_init2 (tmp1, mpfi_get_prec(res1));
      mpfr_init2 (tmp2, mpfi_get_prec(res2));
      if ( mpfr_number_p (&(op2->left)) ) {
        tmp = mpfr_div (tmp2, &(op1->right), &(op2->left), MPFI_RNDD);
      }
      else { /* denominator has infinite left endpoint */
        mpfr_set_zero (tmp2, 1);
      }

      if ( mpfr_number_p (&(op2->right)) ) {
        tmp = mpfr_div ( tmp1, &(op1->right), &(op2->right), MPFI_RNDU);
      }
      else { /* denominator has infinite right endpoint */
        mpfr_set_zero( tmp1, -1);
      }

      mpfr_set_inf (&(res1->left), -1);
      mpfr_set (&(res1->right), tmp1, MPFI_RNDU);
      mpfr_set (&(res2->left), tmp2, MPFI_RNDD);
      mpfr_set_inf (&(res2->right), 1);

      mpfr_clear(tmp1);
      mpfr_clear(tmp2);

      nb_res = 2;
    }
    else { /* if ( MPFI_IS_STRICTLY_POS (op1) ) */
      mpfr_init2 (tmp1, mpfi_get_prec(res1));
      mpfr_init2 (tmp2, mpfi_get_prec(res2));
      if ( mpfr_number_p (&(op2->left)) ) {
        tmp = mpfr_div (tmp1, &(op1->left), &(op2->left), MPFI_RNDU);
      }
      else { /* denominator has infinite left endpoint */
        mpfr_set_zero (tmp1, -1);
      }

      if ( mpfr_number_p (&(op2->right)) ) {
        tmp = mpfr_div ( tmp2, &(op1->left), &(op2->right), MPFI_RNDD);
      }
      else { /* denominator has infinite right endpoint */
        mpfr_set_zero( tmp2, 1);
      }
      mpfr_set_inf (&(res1->left), -1);
      mpfr_set (&(res1->right), tmp1, MPFI_RNDU);
      mpfr_set (&(res2->left), tmp2, MPFI_RNDD);
      mpfr_set_inf (&(res2->right), 1);

      mpfr_clear(tmp1);
      mpfr_clear(tmp2);

      nb_res = 2;
    }
  }

  else {
    tmp = mpfi_div(res1, op1, op2);
    mpfr_set_nan (&(res2->left));
    mpfr_set_nan (&(res2->right));
    return 1;
  }

  if (nb_res >= 1) {
    /* do not allow -0 as lower bound */
    if (mpfr_zero_p (&(res1->left)) && mpfr_signbit (&(res1->left))) {
      mpfr_neg (&(res1->left), &(res1->left), MPFI_RNDU);
    }
    /* do not allow +0 as upper bound */
    if (mpfr_zero_p (&(res1->right)) && !mpfr_signbit (&(res1->right))) {
      mpfr_neg (&(res1->right), &(res1->right), MPFI_RNDD);
    }
  }
  if (nb_res >= 2) {
    /* do not allow -0 as lower bound */
    if (mpfr_zero_p (&(res2->left)) && mpfr_signbit (&(res2->left))) {
      mpfr_neg (&(res2->left), &(res2->left), MPFI_RNDU);
    }
    /* do not allow +0 as upper bound */
    if (mpfr_zero_p (&(res2->right)) && !mpfr_signbit (&(res2->right))) {
      mpfr_neg (&(res2->right), &(res2->right), MPFI_RNDD);
    }
  }

  return nb_res;
}
