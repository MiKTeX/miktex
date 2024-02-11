/* mul_fr.c -- Multiply an interval and a floating-point number.

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
mpfi_mul_fr (mpfi_ptr a, mpfi_srcptr b, mpfr_srcptr c)
{
  mpfi_t tmp;
  int inexact = 0;

 if ( MPFI_NAN_P (b) || mpfr_nan_p(c) )
    {
      mpfr_set_nan (&(a->left));
      mpfr_set_nan (&(a->right));
      MPFR_RET_NAN;
    }

  mpfi_init2 (tmp, mpfr_get_prec (c));
  mpfi_set_fr (tmp, c); /* exact */
  inexact = mpfi_mul (a, b, tmp);
  MPFI_CLEAR (tmp);

  if (MPFI_NAN_P (a))
    MPFR_RET_NAN;

  return inexact;
}
