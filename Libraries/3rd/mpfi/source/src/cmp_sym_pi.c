/* cmp_sym_pi.c -- Internal function.

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

/* compares z * Pi/2 - x and y where z is an integer (mpz)              */
/* the result is exact                                                  */
int
mpfi_cmp_sym_pi (mpz_srcptr z, mpfr_srcptr x, mpfr_srcptr y, mpfr_prec_t prec_init)
{
  /* Assumption: x and y are neither NaN nor Infinite */
  mpfr_prec_t prec;
  mpfi_t pi_over_two, tmp;
  int not_ok;


  prec = prec_init;
  mpfi_init2 (pi_over_two, prec);
  mpfi_init2 (tmp, prec);

  if (mpz_cmp_ui (z, 0) == 0) {
    not_ok=0;
    mpfi_set_prec (tmp, mpfr_get_prec (x));
    mpfi_set_fr (tmp, x);
    mpfi_neg (tmp, tmp);
  }
  else {
    do {
      mpfi_const_pi (pi_over_two);
      mpfi_div_2exp (pi_over_two, pi_over_two, 1);

      mpfi_mul_z (tmp, pi_over_two, z);
      mpfi_sub_fr (tmp, tmp, x);

      not_ok = mpfi_is_inside_fr (y, tmp);
      if (not_ok) {
	prec += BITS_PER_MP_LIMB;
	mpfi_set_prec (pi_over_two, prec);
	mpfi_set_prec (tmp, prec);
      }
    } while (not_ok);
  }

  not_ok = mpfi_cmp_fr_default (tmp, y);
  mpfi_clear (pi_over_two);
  mpfi_clear (tmp);

  return not_ok;
}
