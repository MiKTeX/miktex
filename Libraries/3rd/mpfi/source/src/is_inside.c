/* is_inside.c -- Test whether the first operand is in the second one.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2011,
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
mpfi_is_strictly_inside (mpfi_srcptr a, mpfi_srcptr b)
{
  return mpfr_cmp (&(b->left), &(a->left)) < 0
    && mpfr_cmp (&(a->right), &(b->right)) < 0;
}

int
mpfi_is_inside (mpfi_srcptr a, mpfi_srcptr b)
{
  /* Returns 0 if one of the operands is a NaN */
  if (MPFI_NAN_P (a) || MPFI_NAN_P (b))
    return 0;

  return mpfr_cmp (&(b->left), &(a->left)) <= 0
    && mpfr_cmp (&(a->right), &(b->right)) <= 0;
}

int
mpfi_is_inside_d (const double a, mpfi_srcptr b)
{
  int res;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (b));
  mpfi_set_d (tmp, a);
  res = mpfi_is_inside (tmp, b);
  MPFI_CLEAR (tmp);

  return res;
}

int
mpfi_is_inside_ui (const unsigned long a, mpfi_srcptr b)
{
  /* Returns 0 if one of the operands is a NaN */
  if (MPFI_NAN_P (b))
    return 0;

  return mpfr_cmp_ui (&(b->left), a) <= 0
    &&  mpfr_cmp_ui (&(b->right), a) >= 0;
}

int
mpfi_is_inside_si (const long a, mpfi_srcptr b)
{
  /* Returns 0 if one of the operands is a NaN */
  if (MPFI_NAN_P (b))
    return 0;

  return mpfr_cmp_si (&(b->left), a) <= 0
    &&  mpfr_cmp_si (&(b->right), a) >= 0;
}

int
mpfi_is_inside_z (mpz_srcptr a, mpfi_srcptr b)
{
  int res;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (b));
  mpfi_set_z (tmp, a);
  res = mpfi_is_inside (tmp, b);
  MPFI_CLEAR (tmp);

  return res;
}

int
mpfi_is_inside_q (mpq_srcptr a, mpfi_srcptr b)
{
  int res;
  mpfi_t tmp;
  /* Returns 0 if one of the operands is a NaN */
  if (MPFI_NAN_P (b))
    return 0;

  mpfi_init2 (tmp, mpfi_get_prec (b));
  mpfi_set_q (tmp, a);
  res = mpfi_is_inside (tmp, b);
  MPFI_CLEAR (tmp);

  return res;
}

int
mpfi_is_inside_fr (mpfr_srcptr a, mpfi_srcptr b)
{
  /* Returns 0 if one of the operands is a NaN */
  if (mpfr_nan_p (a) || MPFI_NAN_P (b))
    return 0;

  return mpfr_cmp (a, &(b->left)) >= 0
    &&  mpfr_cmp (a, &(b->right)) <= 0;
}
