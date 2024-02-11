/* cmp.c -- Default comparison functions.

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

/* Warning: there is no canonical order =>      */
/* interval comparison is not clearly defined   */

/* Customizable comparison functions */
/* Since the mpfi_cmp_* are based on mpfi_cmp, only mpfi_cmp needs to be modified */

int (*mpfi_cmp)    (mpfi_srcptr, mpfi_srcptr)         = mpfi_cmp_default;
int (*mpfi_cmp_d)  (mpfi_srcptr, const double)        = mpfi_cmp_d_default;
int (*mpfi_cmp_ui) (mpfi_srcptr, const unsigned long) = mpfi_cmp_ui_default;
int (*mpfi_cmp_si) (mpfi_srcptr, const long)          = mpfi_cmp_si_default;
int (*mpfi_cmp_z)  (mpfi_srcptr, mpz_srcptr)          = mpfi_cmp_z_default;
int (*mpfi_cmp_q)  (mpfi_srcptr, mpq_srcptr)          = mpfi_cmp_q_default;
int (*mpfi_cmp_fr) (mpfi_srcptr, mpfr_srcptr)         = mpfi_cmp_fr_default;

/* Default comparison functions */
/* They return 1 if one (at least) of their operands is invalid (contains NaN) */

int
mpfi_cmp_default (mpfi_srcptr a, mpfi_srcptr b)
{
  if ( MPFI_NAN_P (a) || MPFI_NAN_P (b) )
    return 1;
  return ( (mpfr_cmp (&(a->right), &(b->left)) < 0) ? -1
	   :(mpfr_cmp (&(b->right), &(a->left)) < 0) );
}

int
mpfi_cmp_d_default (mpfi_srcptr a, const double b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_d (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}

int
mpfi_cmp_ui_default (mpfi_srcptr a, const unsigned long b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_ui (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}

int
mpfi_cmp_si_default (mpfi_srcptr a, const long b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_si (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}

int
mpfi_cmp_z_default (mpfi_srcptr a, mpz_srcptr b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_z (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}

int
mpfi_cmp_q_default (mpfi_srcptr a, mpq_srcptr b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_q (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}

int
mpfi_cmp_fr_default (mpfi_srcptr a, mpfr_srcptr b)
{
  int res = 0;
  mpfi_t tmp;

  mpfi_init2 (tmp, mpfi_get_prec (a));
  mpfi_set_fr (tmp, b);
  res = mpfi_cmp (a, tmp);
  MPFI_CLEAR (tmp);

  return (res);
}
