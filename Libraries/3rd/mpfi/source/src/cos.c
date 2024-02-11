/* cos.c -- Cosine of an interval.

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
mpfi_cos (mpfi_ptr a, mpfi_srcptr b)
{
  int inexact_left, inexact_right, inexact=0;
  mpfr_prec_t prec, prec_left, prec_right;
  mpfr_t tmp;
  mpz_t z, zmod4;
  mpz_t quad_left, quad_right;
  int ql_mod4, qr_mod4;

  if (MPFI_NAN_P (b)) {
    mpfr_set_nan (&(a->left));
    mpfr_set_nan (&(a->right));
    MPFR_RET_NAN;
  }

  if (MPFI_INF_P (b)) {
    /* the two endpoints are the same infinite */
    if ( mpfr_cmp (&(b->left), &(b->right)) == 0) {
      mpfr_set_nan (&(a->left));
      mpfr_set_nan (&(a->right));
      MPFR_RET_NAN;
    }
    mpfr_set_si (&(a->left), -1, MPFI_RNDD);
    mpfr_set_si (&(a->right), 1, MPFI_RNDU);
    return 0;
  }

  mpz_init (quad_left);
  mpz_init (quad_right);
  mpz_init (z);
  /* quad_left gives the quadrant where the left endpoint of b is */
  /* quad_left = floor (2 b->left / pi)                            */
  /* idem for quad_right and b->right                             */
  prec_left = mpfi_quadrant (quad_left, &(b->left));
  prec_right = mpfi_quadrant (quad_right, &(b->right));

  /* if there is at least one period in b, then a = [-1, 1] */
  mpz_sub (z, quad_right, quad_left);
  if (mpz_cmp_ui (z, 4) >= 0) {
    mpfr_set_si (&(a->left), -1, MPFI_RNDD);
    mpfr_set_si (&(a->right), 1, MPFI_RNDU);
    inexact = 0;
  }
  else {
    /* there is less than one period in b */
    /* let us discuss according to the position (quadrant) of the endpoints of
       b  */

    /* computing precision = maximal precision required to determine the
       relative position of the endpoints of b and of integer multiples of
       Pi / 2 */
    prec = mpfi_get_prec (a);
    if (prec_left > prec) prec = prec_left;
    if (prec_right > prec) prec = prec_right;

    mpz_add (z, quad_left, quad_right);
    /* z = quad_right + quad_left */

    mpz_init (zmod4);

    /* qr_mod4 gives the quadrant where the right endpoint of b is */
    /* qr_mod4 = floor (2 b->right / pi) mod 4 */
    mpz_mod_ui (zmod4, quad_right, 4);
    qr_mod4 = mpz_get_ui (zmod4);

    /* quad_left gives the quadrant where the left endpoint of b is */
    /* quad_left = floor (2 b->left / pi) mod 4 */
    mpz_mod_ui (zmod4, quad_left, 4);
    ql_mod4 = mpz_get_ui (zmod4);


    switch (qr_mod4) {
    case 0:
      switch (ql_mod4) {
      case 0:
        mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
	inexact_left = mpfr_cos (tmp, &(b->right), MPFI_RNDD);
	inexact_right = mpfr_cos (&(a->right), &(b->left), MPFI_RNDU);
	mpfr_set (&(a->left), tmp, MPFI_RNDD); /* exact */
        mpfr_clear (tmp);
	break;
      case 1:
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      case 2:
	inexact_left = mpfr_cos (&(a->left), &(b->left), MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      case 3:
	mpz_add_ui (z, z, 1);
	if (mpfi_cmp_sym_pi (z, &(b->right), &(b->left), prec) >= 0)
	  inexact_left = mpfr_cos (&(a->left), &(b->left), MPFI_RNDD);
	else
	  inexact_left = mpfr_cos (&(a->left), &(b->right), MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      }
      break;
    case 1:
      switch (ql_mod4) {
      case 0:
      case 1:
        mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
	inexact_left = mpfr_cos (tmp, &(b->right), MPFI_RNDD);
	inexact_right = mpfr_cos (&(a->right), &(b->left), MPFI_RNDU);
	mpfr_set (&(a->left), tmp, MPFI_RNDD); /* exact */
        mpfr_clear (tmp);
	break;
      case 2:
	mpz_add_ui (z, z, 1);
	if (mpfi_cmp_sym_pi (z, &(b->right), &(b->left), prec) >= 0)
	  inexact_left = mpfr_cos (&(a->left), &(b->left), MPFI_RNDD);
	else
	  inexact_left = mpfr_cos (&(a->left), &(b->right), MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      case 3:
	inexact_left = mpfr_cos (&(a->left), &(b->right), MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      }
      break;
    case 2:
      switch (ql_mod4) {
      case 0:
	inexact_right = mpfr_cos (&(a->right), &(b->left), MPFI_RNDU);
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	break;
      case 1:
	mpz_add_ui (z, z, 1);
	if (mpfi_cmp_sym_pi (z, &(b->left), &(b->right), prec) >= 0)
	  inexact_right = mpfr_cos (&(a->right), &(b->left), MPFI_RNDU);
	else
	  inexact_right = mpfr_cos (&(a->right), &(b->right), MPFI_RNDU);
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	break;
      case 2:
	inexact_left = mpfr_cos (&(a->left), &(b->left), MPFI_RNDD);
	inexact_right = mpfr_cos (&(a->right), &(b->right), MPFI_RNDU);
	break;
      case 3:
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	inexact_right = mpfr_set_si (&(a->right), 1, MPFI_RNDU);
	break;
      }
      break;
    case 3:
      switch (ql_mod4) {
      case 0:
	mpz_add_ui (z, z, 1);
	if (mpfi_cmp_sym_pi (z, &(b->right), &(b->left), prec) >= 0)
	  inexact_right = mpfr_cos (&(a->right), &(b->left), MPFI_RNDU);
	else
	  inexact_right = mpfr_cos (&(a->right), &(b->right), MPFI_RNDU);
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	break;
      case 1:
	inexact_right = mpfr_cos (&(a->right), &(b->right), MPFI_RNDU);
	inexact_left = mpfr_set_si (&(a->left), -1, MPFI_RNDD);
	break;
      case 2:
      case 3:
	inexact_left = mpfr_cos (&(a->left), &(b->left), MPFI_RNDD);
	inexact_right = mpfr_cos (&(a->right), &(b->right), MPFI_RNDU);
	break;
      }
      break;
    }

    if (inexact_left) inexact = 1;
    if (inexact_right) inexact += 2;

    mpz_clear (zmod4);
  }

  mpz_clear (quad_left);
  mpz_clear (quad_right);
  mpz_clear (z);

  return inexact;
}
