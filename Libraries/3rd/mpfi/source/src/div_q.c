/* div_q.c -- Divide an interval by a rational number.

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
mpfi_div_q (mpfi_ptr a, mpfi_srcptr b, mpq_srcptr c)
{
  mpfr_t tmp;
  int inexact_left, inexact_right;
  int inexact = 0;

  if (MPFI_NAN_P (b))
    {
      mpfr_set_nan (&(a->left));
      mpfr_set_nan (&(a->right));
      MPFR_RET_NAN;
    }

  if (!mpq_sgn (c))             /* c = 0 */
    {
      if (mpfr_zero_p (&(b->left)))
        mpfr_set_nan (&(a->left));
      else
        mpfr_set_inf (&(a->left), -1);
      inexact_left = 0;

      if (mpfr_zero_p (&(b->right)))
        mpfr_set_nan (&(a->right));
      else
        mpfr_set_inf (&(a->right), +1);
      inexact_right = 0;
    }
  else if (mpq_sgn (c) < 0)     /* c < 0 */
    {
      mpfr_init2 (tmp, mpfr_get_prec (&(a->left)));
      inexact_left = mpfr_div_q (tmp, &(b->right), c, MPFI_RNDD);
      if (mpfr_inf_p (tmp) && !mpfr_inf_p (&(b->right)))
        {
          /* work around MPFR bug in mpfr_div_q (present in MPFR-3.0.0) */
          inexact_left = 1; /* overflow */
        }
      inexact_right = mpfr_div_q (&(a->right), &(b->left), c, MPFI_RNDU);
      if (mpfr_inf_p (&(a->right)) && !mpfr_inf_p (&(b->left)))
        {
          /* work around MPFR bug in mpfr_div_q */
          inexact_right = 1; /* overflow */
        }
      mpfr_set (&(a->left), tmp, MPFI_RNDD);    /* exact */
      mpfr_clear (tmp);
    }
  else                          /* c > 0 */
    {
      int mpfr_bug_work_around;
      mpfr_bug_work_around = !mpfr_inf_p (&(b->left));
      inexact_left = mpfr_div_q (&(a->left), &(b->left), c, MPFI_RNDD);
      if (mpfr_bug_work_around && mpfr_inf_p (&(a->left)))
        {
          /* work around MPFR bug in mpfr_div_q */
          inexact_left = 1; /* overflow */
        }

      mpfr_bug_work_around = !mpfr_inf_p (&(b->right));
      inexact_right = mpfr_div_q (&(a->right), &(b->right), c, MPFI_RNDU);
      if (mpfr_bug_work_around && mpfr_inf_p (&(a->right)))
        {
          /* work around MPFR bug in mpfr_div_q */
          inexact_right = 1; /* overflow */
        }
    }

  if (MPFI_NAN_P (a))
    MPFR_RET_NAN;

  /* no need to check to sign of the bounds in case they are 0 */
  if (inexact_left)
    inexact += 1;
  if (inexact_right)
    inexact += 2;

  return inexact;
}
