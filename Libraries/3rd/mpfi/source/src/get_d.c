/* get_d.c -- Return the center of an interval as a double.

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

double
mpfi_get_d (mpfi_srcptr a)
{
  double res;
  mpfr_t tmp;

  mpfr_init2 (tmp, 53); /* Double rounding may occur for subnormal numbers */
  mpfi_mid (tmp, a);
  res = mpfr_get_d (tmp, MPFR_RNDN);
  mpfr_clear (tmp);

  return res;
}
