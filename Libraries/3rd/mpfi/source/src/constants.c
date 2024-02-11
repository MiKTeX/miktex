/* constants.c -- Useful constants.

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
mpfi_const_log2 (mpfi_ptr a)
{
  mpfr_const_log2 (&(a->left), MPFI_RNDD);
  mpfr_const_log2 (&(a->right), MPFI_RNDU);

  return MPFI_FLAGS_BOTH_ENDPOINTS_INEXACT;
}

int
mpfi_const_pi (mpfi_ptr a)
{
  mpfr_const_pi (&(a->left), MPFI_RNDD);
  mpfr_const_pi (&(a->right), MPFI_RNDU);

  return MPFI_FLAGS_BOTH_ENDPOINTS_INEXACT;
}

int
mpfi_const_euler (mpfi_ptr a)
{
  mpfr_const_euler (&(a->left), MPFI_RNDD);
  mpfr_const_euler (&(a->right), MPFI_RNDU);

  return MPFI_FLAGS_BOTH_ENDPOINTS_INEXACT;
}

int
mpfi_const_catalan (mpfi_ptr a)
{
  mpfr_const_catalan (&(a->left), MPFI_RNDD);
  mpfr_const_catalan (&(a->right), MPFI_RNDU);

  return MPFI_FLAGS_BOTH_ENDPOINTS_INEXACT;
}
