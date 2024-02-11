/* exp2m1.c -- Exponential, in base 2, of an interval, minus 1.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2022
                     Spaces project, Inria Lorraine
                     and Salsa project, INRIA Rocquencourt,
                     and Arenaire project, Inria Rhone-Alpes, France
                     and AriC project, Inria Grenoble Rhone-Alpes and Inria Lyon, France
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


/* temporary, for versions of MPFR < 4.2 */
/* computes the ceiling of the logarithm, in base 2, of prec */
mpfr_prec_t log_prec_exp2m1 (mpfr_prec_t prec)
{ 
  mpfr_prec_t res=1;
  while (res < prec)
    res *=2;
  return res;
}

/* Computes the exp, in base 2, of an interval, minus 1              */
int
mpfi_exp2m1 (mpfi_ptr a, mpfi_srcptr b)
{
  mpfr_prec_t tmp_prec_left, tmp_prec_right, tmp_prec;
  mpfi_t tmp;
  int inexact_exp, inexact_m1; /* for versions of MPFR < 4.2 */
  int inexact_left, inexact_right, inexact_set, inexact=0;

  #if ( defined(MPFR_VERSION_MAJOR) &&  (MPFR_VERSION_MAJOR >= 4) && defined(MPFR_VERSION_MINOR) & (MPFR_VERSION_MINOR >= 2) )
    {
    inexact_left = mpfr_exp2m1 (&(a->left), &(b->left), MPFI_RNDD);
    inexact_right = mpfr_exp2m1 (&(a->right), &(b->right), MPFI_RNDU);

    if (inexact_left)
      inexact += 1;
    if (inexact_right)
      inexact += 2;
    }
  #else
    {
    /* Without any guarantee that the bounds are the tightest = correctly rounded ones */
/*
    tmp_prec_left = mpfr_get_exp( &(b->left));
    if (tmp_prec_left < 0)
      tmp_prec_left = -tmp_prec_left;
    tmp_prec_left += 1 + mpfr_get_prec( &(b->left) );
    if ( tmp_prec_left < mpfr_get_prec( &(a->left)) + 15)
      tmp_prec_left = mpfr_get_prec( &(a->left) ) + 15;
    tmp_prec_right = mpfr_get_exp( &(b->right));
    if (tmp_prec_right < 0)
      tmp_prec_right = -tmp_prec_right;
    tmp_prec_right += 1 + mpfr_get_prec( &(b->right) );
    if ( tmp_prec_right < mpfr_get_prec( &(a->right)) + 15)
      tmp_prec_right = mpfr_get_prec( &(a->right) ) + 15;
    if (tmp_prec_left > tmp_prec_right)
      tmp_prec = tmp_prec_left + 1;
    else
      tmp_prec = tmp_prec_right +1;
*/
    tmp_prec_left = mpfr_get_prec( &(a->left) );
    tmp_prec_right = mpfr_get_prec( &(a->right) );
    if (tmp_prec_left > tmp_prec_right)
      tmp_prec = tmp_prec_left;
    else
      tmp_prec = tmp_prec_right;
    tmp_prec += 2*log_prec_exp2m1(tmp_prec);
    mpfi_init2(tmp, tmp_prec);
    inexact_exp = mpfi_exp2(tmp, b);
    inexact_m1 = mpfi_sub_d(a, tmp, 1.0);
    if ( (inexact_exp %2) || (inexact_m1%2) )
      inexact += 1;
    if ( (inexact_exp >= 2) || (inexact_m1 >= 2) )
      inexact += 2;
    mpfi_clear(tmp);
    }
  #endif


  /* do not allow +0 as upper bound */
  if (mpfr_zero_p (&(a->right)) && !mpfr_signbit (&(a->right))) {
    mpfr_neg (&(a->right), &(a->right), MPFI_RNDD);
  }

  if ( MPFI_NAN_P (a) )
    MPFR_RET_NAN;

  return inexact;
}
