/* mid.c -- Middle of the interval.

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

/* Middle of y
   With an IEEE-compliant arithmetic, this formula provides more
   accurate results than x->left + 0.5*(x->right - x->left) or
   x->right - 0.5*(x->right - x->left)  */

int
mpfi_mid (mpfr_ptr m, mpfi_srcptr y)
{
  int inexact_add, inexact_div2=0;
  mpfr_t tmp_l, tmp_r;

  inexact_add = mpfr_add (m, &(y->left), &(y->right), MPFR_RNDN);

  /*  when one of the bound is infinite: nothing to be done */
  if ( ! (MPFR_IS_INF(&(y->left)) || MPFR_IS_INF(&y->right)) )
    {
    /* In case of overflow: first division by 2 and then addition  */
    if (MPFR_IS_INF(m))
      {
      mpfr_init2(tmp_l, mpfi_get_prec(y));
      mpfr_div_2ui(tmp_l, &(y->left), 1, MPFR_RNDN); /* should be exact*/
      mpfr_init2(tmp_r, mpfi_get_prec(y));
      mpfr_div_2ui(tmp_r, &(y->right), 1, MPFR_RNDN); /* should be exact*/
      inexact_add = mpfr_add (m, tmp_l, tmp_r, MPFR_RNDN);
      mpfr_clear (tmp_l);
      mpfr_clear (tmp_r);
      }
    else
      {
      inexact_div2 = mpfr_div_2ui (m, m, 1, MPFR_RNDN);
      }
    }

  /* Hope it copes correctly with an underflow in the division by 2... */
  if (inexact_div2)
    return inexact_div2;
  else
    return inexact_add;
}
