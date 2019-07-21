/* Copyright (C) 2000-2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _BASICS_H
#define _BASICS_H

#ifdef VMS		/* these three lines from Jacob Jansen, Open VMS port */
# include <vms_jackets.h>
#endif
#ifdef HAVE_CONFIG_H
/* cross-compiler woes */
#ifdef __MINGW32__
#define _CTYPE_H_
#endif
# include <w2c/config.h>
#if defined(MIKTEX)
#  if !defined(_INC_CTYPE)
     /* prevent inclusion of ctype.h */
#    define _INC_CTYPE 1
#  endif
#  if defined(MIKTEX_WINDOWS)
#    include <miktex/unxemu.h>
#  endif
#  include <assert.h>
#endif
#endif
#define gfree ff_gfree
#define grealloc ff_grealloc

#include <stdio.h>		/* for NULL */
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# include <inttypes.h>
#endif
#include <stdlib.h>		/* for free */
#include <limits.h>

#define true 1
#define false 0

#define forever for (;;)

typedef intptr_t	intpt;

#include <unibasics.h>

extern void *galloc(long size);
extern void *gcalloc(int cnt, long size);
extern void *grealloc(void *,long size);
extern void gfree(void *);
extern void galloc_set_trap(void (*)(void));
extern char *copy(const char *);
extern char *copyn(const char *,long);

#endif
