/*
 * interp.c
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

/* This file is the skeleton of the bytecode interpreter; the parts
   specific to each instruction are inserted from the file
   'keiko.iset' by the script 'iset.tcl'.  There are three places that
   code is inserted, each marked by two dollar signs.  In order of
   appearance, they are:

   1. A jump table for quick dispatching (used if JTABLE is defined).

   2. Macro definitions used in the action routines.

   3. Action routines for each instruction, forming the cases in a big
      switch. */

#include <math.h>
#include <string.h>
#include "obx.h"
#include "keiko.h"

#ifdef HAVE_INDEXED_JUMPS
#define JTABLE 1
#endif

#ifdef TRACE
#define DISASS 1
#undef JTABLE
#define do_find_proc if (dflag > 1) thisproc = find_proc(dsegaddr(cp))
#else
#define do_find_proc
#endif

#ifdef PROFILE
#undef JTABLE
#endif

#ifdef UNALIGNED_MEM
#define getdbl get_double
#define putdbl put_double
#define getlong get_long
#define putlong put_long
#else
static inline double getdbl(value *v) {
     dblbuf dd;
     dd.n.lo = v[0].i;
     dd.n.hi = v[1].i;
     return dd.d;
}

static inline void putdbl(value *v, double x) {
     dblbuf dd;
     dd.d = x;
     v[0].i = dd.n.lo;
     v[1].i = dd.n.hi;
}

static inline longint getlong(value *v) {
     dblbuf dd;
     dd.n.lo = v[0].i;
     dd.n.hi = v[1].i;
     return dd.q;
}

static inline void putlong(value *v, longint x) {
     dblbuf dd;
     dd.q = x;
     v[0].i = dd.n.lo;
     v[1].i = dd.n.hi;
}
#endif

/* Macros used in action routines */

#define error(msg, n)   runtime_error(msg, n, bp, pc0)

#define local(n)        ((uchar *) bp + (n))
#define parent(a, t)    indir(pointer(bp[SL]) + a, t)
#define indir(p, t)     (* (t *) (p))
#define subs(p, n, t)   ((t *) (p))[n]
#define const(n)        cp[CP_CONST+n]
#define jump(lab)       pc = pc0 + lab

#define load(x, t)      indir(pointer(x), t)
#define store(x, y, t)  indir(pointer(y), t) = x
#define ldl(a, t)       indir(local(a), t)
#define stl(a, x, t)    indir(local(a), t) = x
#define ldg(a, t)       indir(pointer(const(a)), t)
#define stg(a, x, t)    indir(pointer(const(a)), t) = x
#define ldn(a, x)       indir((char *) pointer(x) + a, int)
#define stn(a, x, y)    indir((char *) pointer(y) + a, int) = x
#define ldi(x, y, t)    subs(pointer(x), y.i, t)
#define sti(x, y, z, t) subs(pointer(y), z.i, t) = x

#define dup(n, sp)      sp--; sp[0] = sp[n+1]
#define swap(sp)        sp[-1] = sp[1]; sp[1] = sp[0]; sp[0] = sp[-1]
#define slide(nargs)    sp += HEAD + nargs; cond_break();

#define ror(a, b)       ((((unsigned) a) >> b) | (((unsigned) a) << (32-b)))

#define fcmpl(a, b)     (a > b ? 1 : a == b ? 0 : -1)
#define fcmpg(a, b)     (a < b ? -1 : a == b ? 0 : 1)
#define lcmp(a, b)      (a < b ? -1 : a > b ? 1 : 0)

#ifdef WORDS_BIGENDIAN
#define alignx(a, n)    (a << (32-n))
#else
#define alignx(a, n)    a
#endif

#ifdef PROFILE
#define prof_charge(n)  ticks += n
#else
#define prof_charge(n)
#endif

#ifdef OBXDEB
#define cond_break() \
     if (one_shot && *pc != K_LNUM_2 && *pc != K_BREAK_2) \
          debug_break(cp, bp, pc, "stop")
#else
#define cond_break()
#endif

/* interp -- main loop of the interpreter */
value *interp(value *sp0) {
     register value *cp = valptr(sp0[CP]);
     uchar *pc = codeptr(cp[CP_CODE].a);
     register uchar *pc0 = NULL;
     register value *sp = sp0;
     register value *rp = NULL;
     register uchar ir = 0;
#ifdef PROFILE
     register counter ticks = 0;
#endif
     register value *bp = NULL;
     value *base = sp0;
#ifdef TRACE
     proc thisproc = NULL;
#endif

#ifdef JTABLE
     /* Save time by using gcc's label array feature */
#define __o__(op, inst, patt, arg, len) &&lbl_ ## op,
     static void *jtable[256] = { __OPCODES__(__o__) };
#endif

#ifdef JTABLE
/* Each action ends with an indexed jump to the next */
#define ACTION(op) lbl_ ## op:
#define ALSO(op)
#define DEFAULT
#define NEXT       goto *jtable[ir = *(pc0 = pc)]
#else
/* Actions are just cases in a big switch */
#define ACTION(op) case K_ ## op:
#define ALSO(op)   case K_ ## op:
#define DEFAULT    default:
#define NEXT       break
#endif

     level++;

enter:
     do_find_proc;

#ifdef PROFILE
     prof_enter(dsegaddr(cp), ticks, PROF_CALL);
#endif

     bp = sp;								
     sp = (value *) ((uchar *) bp - cp[CP_FRAME].i);			
     if ((uchar *) sp < stack + SLIMIT) error(E_STACK, 0);		

     /* Preserve the static link if the routine starts with SAVELINK */
     memset(sp, 0, (*pc == K_SAVELINK ? cp[CP_FRAME].i - 4 : cp[CP_FRAME].i));

#ifdef JTABLE
     NEXT;
#else
     while (TRUE) {
#ifdef TRACE
	  if (dflag > 1) {
	       printf("pc=%s+%ld(%p) sp=%p bp=%p cp=%p",
		      thisproc->p_name,
                      (long) (pc - codeptr(cp[CP_CODE].a)),
                      pc, sp, bp, cp);
	       fflush(stdout);
	       for (int i = 0; i < 8; i++) printf(" %x", sp[i].i);
	       printf("\n");
	       printf("%6ld: %s\n", (long) (pc-imem), fmt_inst(pc));
	       fflush(stdout);
	  }
#endif

#ifdef PROFILE
	  ticks++;
#endif

	  switch (ir = *(pc0 = pc)) {
#endif

#include "action.c"

	  ACTION(ILLEGAL)
	  DEFAULT
	       panic("*illegal instruction %d", ir);
               return NULL;
#ifndef JTABLE
	  }
     }
#endif
}
