/* revert_if_needed.c -- Helper function : swap endpoints so that
   left <= right.

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

/* ensures that the result is [a,b] with a<=b */
/* should be useless but bugs are not excluded... */
/* Returns 1 if endpoints have been exchanged     */

int
mpfi_revert_if_needed (mpfi_ptr a)
{
  if ( MPFI_NAN_P (a) )
    return 0;

  if ( mpfr_cmp (&(a->right), &(a->left)) < 0 ) {
    mpfr_swap (&(a->left), &(a->right));
    return 1;
  }
  else
    return 0;
}
