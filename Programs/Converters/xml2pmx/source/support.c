/*
 * support.c
 * 
 * This file is part of the Oxford Oberon-2 compiler
 * Copyright (c) 2006--2016 J. M. Spivey
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "obx.h"
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/* Assorted runtime support routines */

void panic(const char *msg, ...) {
     va_list va;

     mybool bug = FALSE;

     if (*msg == '*') {
	  bug = TRUE; msg++;
     }

     fflush(stdout);
     fprintf(stderr, "Fatal error: ");
     va_start(va, msg);
     vfprintf(stderr, msg, va);
     va_end(va);
     fprintf(stderr, "\n");
     if (bug)
	  fprintf(stderr, "Please report bugs to %s\n", PACKAGE_BUGREPORT);
     fflush(stderr);
     error_exit(3);
}


/* The DIV and MOD instructions must give the correct results, even if 
   C is wrong.  Correct means that b * (a DIV b) + a MOD b = a, and 
   (-a) DIV (-b) = a DIV b, and if b > 0 then 0 <= a MOD b < b. */

void int_div(value *sp) {
     int a = sp[1].i, b = sp[0].i;
     int quo = a / b;
     int rem = a - b * quo;
     if (rem != 0 && (rem ^ b) < 0) quo--;
     sp[1].i = quo;
}

void int_mod(value *sp) {
     int a = sp[1].i, b = sp[0].i;
     int rem = a % b;
     if (rem != 0 && (rem ^ b) < 0) rem += b;
     sp[1].i = rem;
}

void long_div(value *sp) {
     longint a = get_long(&sp[2]), b = get_long(&sp[0]);
     longint quo = a / b;
     longint rem = a - b * quo;
     if (rem != 0 && (rem ^ b) < 0) quo--;
     put_long(&sp[2], quo);
}

void long_mod(value *sp) {
     longint a = get_long(&sp[2]), b = get_long(&sp[0]);
     longint rem = a % b;
     if (rem != 0 && (rem ^ b) < 0) rem += b;
     put_long(&sp[2], rem);
}

void long_flo(value *sp) {
     put_double(sp, get_long(sp));
}

#ifndef M64X32
void long_add(value *sp) {
     put_long(sp+2, get_long(sp+2) + get_long(sp));
}

void long_sub(value *sp) {
     put_long(sp+2, get_long(sp+2) - get_long(sp));
}

void long_mul(value *sp) {
     put_long(sp+2, get_long(sp+2) * get_long(sp));
}

void long_neg(value *sp) {
     put_long(sp, -get_long(sp));
}

void long_cmp(value *sp) {
     longint a = get_long(sp+2), b = get_long(sp);
     sp[3].i = (a < b ? -1 : a > b ? 1 : 0);
}

void long_ext(value *sp) {
     put_long(sp-1, (longint) sp[0].i);
}

void long_zcheck(value *sp) {
     if (get_long(sp+2) == 0)
          runtime_error(E_DIV, sp[0].i, ptrcast(value, sp[1].a), NULL);
}
#endif

#ifndef FLOATOPS
void flo_add(value *sp) { sp[1].f = sp[1].f + sp[0].f; }
void flo_sub(value *sp) { sp[1].f = sp[1].f - sp[0].f; }
void flo_mul(value *sp) { sp[1].f = sp[1].f * sp[0].f; }
void flo_div(value *sp) { sp[1].f = sp[1].f / sp[0].f; }
void flo_neg(value *sp) { sp[0].f = - sp[0].f; }
void flo_float(value *sp) { sp[0].f = (float) sp[0].i; }
void flo_fix(value *sp) { sp[0].i = (int) sp[0].f; }

void flo_cmpl(value *sp) {
     float a = sp[1].f, b = sp[0].f;
     sp[1].i = (a > b ? 1 : a == b ? 0 : -1);
}

void flo_cmpg(value *sp) {
     float a = sp[1].f, b = sp[0].f;
     sp[1].i = (a < b ? -1 : a == b ? 0 : 1);
}     
     
void dbl_add(value *sp) {
     put_double(sp+2, get_double(sp+2) + get_double(sp));
}

void dbl_sub(value *sp) {
     put_double(sp+2, get_double(sp+2) - get_double(sp));
}

void dbl_mul(value *sp) {
     put_double(sp+2, get_double(sp+2) * get_double(sp));
}

void dbl_div(value *sp) {
     put_double(sp+2, get_double(sp+2) / get_double(sp));
}

void dbl_neg(value *sp) {
     put_double(sp, - get_double(sp));
}

void dbl_float(value *sp) {
     put_double(sp-1, (double) sp[0].i);
}

void dbl_fix(value *sp) {
     sp[1].i = (int) get_double(sp);
}

void dbl_cmpl(value *sp) {
     double a = get_double(sp+2), b = get_double(sp);
     sp[3].i = (a > b ? 1 : a == b ? 0 : -1);
}

void dbl_cmpg(value *sp) {
     double a = get_double(sp+2), b = get_double(sp);
     sp[3].i = (a < b ? -1 : a == b ? 0 : 1);
}

void dbl_widen(value *sp) {
     put_double(sp-1, (double) sp[0].f);
}

void flo_trunc(value *sp) {
     sp[1].f = (float) get_double(sp);
}

void flo_zcheck(value *sp) {
     if (sp[2].f == 0.0f)
          runtime_error(E_FDIV, sp[0].i, ptrcast(value, sp[1].a), NULL);
}

void dbl_zcheck(value *sp) {
     if (get_double(sp+2) == 0.0)
          runtime_error(E_FDIV, sp[0].i, ptrcast(value, sp[1].a), NULL);
}
#endif


/* Conversions between int and floating point */

#ifndef GCOV
/* These are not done inline in interp() because that upsets the
   gcc optimiser on i386, adding overhead to every instruction. */
double flo_conv(int x) { 
     return (double) x; 
}

double flo_convq(longint x) {
     return (double) x;
}
#endif

/* obcopy -- like strncpy, but guarantees termination with zero */
void obcopy(char *dst, int dlen, const char *src, int slen, value *bp) {
     if (slen == 0 || dlen < slen) {
          strncpy(dst, src, dlen);
          if (dst[dlen-1] != '\0')
               liberror("string copy overflows destination");
     } else {
          strncpy(dst, src, slen);
          if (dst[slen-1] != '\0')
               liberror("source was not null-terminated");
          memset(&dst[slen], '\0', dlen-slen);
     }
}

#ifndef UNALIGNED_MEM
double get_double(value *v) {
     dblbuf dd;
     dd.n.lo = v[0].i;
     dd.n.hi = v[1].i;
     return dd.d;
}

void put_double(value *v, double x) {
     dblbuf dd;
     dd.d = x;
     v[0].i = dd.n.lo;
     v[1].i = dd.n.hi;
}

longint get_long(value *v) {
     dblbuf dd;
     dd.n.lo = v[0].i;
     dd.n.hi = v[1].i;
     return dd.q;
}

void put_long(value *v, longint x) {
     dblbuf dd;
     dd.q = x;
     v[0].i = dd.n.lo;
     v[1].i = dd.n.hi;
}
#endif

/* find_symbol -- find a procedure from its CP. Works for modules too. */
proc find_symbol(word p, proc *table, int nelem) {
     int a = 0, b = nelem;

     if (p == 0) return NULL;
     if (nelem == 0 || p < table[0]->p_addr) return NULL;

     /* Binary search */
     /* Inv: 0 <= a < b <= nelem, table[a] <= x < table[b], 
	where table[nelem] = infinity */
     while (a+1 != b) {
	  int m = (a+b)/2;
	  if (table[m]->p_addr <= p)
	       a = m;
	  else
	       b = m;
     }

     return table[a];
}

#ifdef WINDOWS
#ifdef OBXDEB
#define OBGETC 1
#endif
#endif

/* obgetc -- version of getc that compensates for Windows quirks */
int obgetc(FILE *fp) {
#ifdef OBGETC
     /* Even if Ctrl-C is trapped, it causes a getc() call on the console
	to return EOF. */
     for (;;) {
	  int c = getc(fp);
	  if (c == EOF && intflag && prim_bp != NULL) {
	       value *cp = valptr(prim_bp[CP]);
	       debug_break(cp , prim_bp, NULL, "interrupt");
	       continue;
	  }
	  return c;
     }
#else
     return getc(fp);
#endif
}

/* get_errno -- fetch the value of errno, for use as a primitive */
int get_errno(void) {
     return errno;
}
