/* mpfi_io.h -- Header for mpfi_io.c.

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


#ifndef __MPFI_IO_H__
#define __MPFI_IO_H__

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mpfi.h>

/*#define isblank isspace*/
#define MPFI_ISSPACE isspace

#ifdef __cplusplus
extern "C" {
#endif
size_t mpfi_out_str(FILE *stream, int base, size_t n_digits, mpfi_srcptr op);
size_t mpfi_inp_str(mpfi_ptr x,FILE *stream,int base);

void mpfi_print_binary(mpfi_srcptr);
#ifdef __cplusplus
}
#endif

#endif /* __MPFI_IO_H__ */
