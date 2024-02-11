/* print_binary.c -- Print binary representation of an interval in stdout.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2018,
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


#include "mpfi_io.h"
#include "mpfi-impl.h"

void
mpfi_print_binary (mpfi_srcptr x)
{
  printf ("[ ");
  mpfr_printf ("%Rf", &(x->left));
  printf (" , ");
  mpfr_printf ("%Rf", &(x->right));
  printf (" ]");
}
