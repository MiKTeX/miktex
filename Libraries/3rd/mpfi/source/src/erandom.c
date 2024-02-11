/* erandom.c -- Random element in the interval, following an exponential distribution.
                without any guarantee: to be checked

Copyright 2018
                     AriC project, Inria Rhone-Alpes, France

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

/* Picks randomly a point m in y */
void
mpfi_erandom (mpfr_ptr m, mpfi_srcptr y, gmp_randstate_t state)
{
  mpfr_prec_t prec, tmp_prec;
  mpfr_t diam, fact;


  if (MPFI_NAN_P(y)) {
    mpfr_set_nan (m);
    return;
  }

  if (mpfr_equal_p (&(y->left), &(y->right))) {
    mpfr_set (m, &(y->left), MPFR_RNDN);
    return;
  }

  prec = mpfr_get_prec (m);
  tmp_prec = mpfi_get_prec(y);
  if (tmp_prec > prec)
    {
    prec = tmp_prec;
    }
  mpfr_init2 (diam, prec);
  mpfr_init2 (fact, prec);

  mpfi_diam_abs (diam, y);
  mpfr_erandom (fact, state,  MPFR_RNDN); 
  mpfr_sub_d (fact, fact, 0.5, MPFR_RNDN);
  if (mpfr_cmp_ui (fact, 0) < 0)
    mpfr_set_ui(fact, 0, MPFR_RNDN);
  else if (mpfr_cmp_ui(fact, 1) > 0)
    mpfr_set_ui(fact, 1, MPFR_RNDN);  /* now fact lies between 0 and 1 */

  if (mpfr_cmp_ui (diam, 1) <= 0) {
    /* the picked point lies at a relative distance "fact" of the left
       endpoint: m = inf + (sup - inf) * fact  */
    mpfr_mul (fact, fact, diam, MPFR_RNDN);
    /* FIXME: because of possible cancelation, the random distribution is
       not uniform among the floating-point numbers in y */
    mpfr_add (m, &(y->left), fact, MPFR_RNDN);
  }
  else {
    mpfr_exp_t e;
    if (mpfr_cmp_abs (&(y->left), &(y->right)) < 0) {
      e = mpfr_inf_p (&(y->right)) ? mpfr_get_emax ()
        : mpfr_get_exp (&(y->right));
    }
    else {
      e = mpfr_inf_p (&(y->left)) ? mpfr_get_emax ()
        : mpfr_get_exp (&(y->left));
    }
    e += 1;
    /* resize fact in [0, 2^e] where e = 1 + max{exp(left), exp(right)} */
    mpfr_mul_2exp (fact, fact, e, MPFR_RNDN);
    mpfr_set (m, &(y->left), MPFR_RNDN);
    if (mpfr_inf_p (m)) {
      mpfr_nextabove (m);
    }
    /* m may be outside y */
    mpfr_add (m, m, fact, MPFR_RNDN);
  }
  mpfr_clear (fact);
  mpfr_clear (diam);

  /* Ensure that m belongs to y (if the precision is sufficient) */
  if (mpfr_cmp (m, &(y->left)) < 0)
    mpfr_set (m, &(y->left), MPFR_RNDU);

  if (mpfr_cmp (&(y->right), m) < 0)
    mpfr_set (m, &(y->right), MPFR_RNDD);
}
