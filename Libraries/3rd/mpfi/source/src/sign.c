/* sign.c -- Default testing sign functions.

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

/* Customizable sign testing functions */

int (*mpfi_is_pos) (mpfi_srcptr)          = mpfi_is_pos_default;
int (*mpfi_is_strictly_pos) (mpfi_srcptr) = mpfi_is_strictly_pos_default;
int (*mpfi_is_nonneg) (mpfi_srcptr)       = mpfi_is_nonneg_default;
int (*mpfi_is_neg) (mpfi_srcptr)          = mpfi_is_neg_default;
int (*mpfi_is_strictly_neg) (mpfi_srcptr) = mpfi_is_strictly_neg_default;
int (*mpfi_is_nonpos) (mpfi_srcptr)       = mpfi_is_nonpos_default;
int (*mpfi_is_zero) (mpfi_srcptr)         = mpfi_is_zero_default;


int
mpfi_is_pos_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_POS (a));
}

int
mpfi_is_strictly_pos_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_STRICTLY_POS (a));
}

int
mpfi_is_nonneg_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_NONNEG (a));
}

int
mpfi_is_neg_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_NEG (a));
}

int
mpfi_is_strictly_neg_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_STRICTLY_NEG (a));
}

int
mpfi_is_nonpos_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return (MPFI_IS_NONPOS (a));
}

int
mpfi_is_zero_default (mpfi_srcptr a)
{
  if ( mpfi_nan_p (a) )
    return 0;

  return((mpfr_sgn (&(a->right)) == 0) &&
         (mpfr_sgn (&(a->left)) == 0));
}
