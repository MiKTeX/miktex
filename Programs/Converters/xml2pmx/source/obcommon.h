/*
 * obcommon.h
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

#ifndef EXTERN
#define EXTERN extern
#endif

typedef unsigned char uchar;

typedef int mybool;
#define TRUE 1
#define FALSE 0

#ifndef min
#define min(x, y) ((x) <= (y) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) ((x) <= (y) ? (y) : (x))
#endif

#define align(x, k) (((x)+((k)-1)) & ~((k)-1))

#ifdef WORDS_BIGENDIAN
#define align_byte(x) (((unsigned) (x))>>24)
#define align_short(x) ((x)>>16)
#else
#define align_byte(x) (x)
#define align_short(x) (x)
#endif

#include <stdint.h>

typedef int64_t longint;

typedef union {
#ifdef WORDS_BIGENDIAN
     struct { int hi, lo; } n;
#else
     struct { int lo, hi; } n;
#endif
     double d;
     longint q;
} dblbuf;

/* Stack frame layout */
#define BP 0			/* Saved base pointer */
#define PC 1			/* Saved program counter */
#define CP 2			/* Saved context pointer */
#define SL -1			/* Static link */
#define HEAD 3
#define FRAME_SHIFT 16		/* Shift for pointer map of stack frame */

/* Constant pool layout */
#define CP_PRIM 0		/* Primitive address (64 bits) */
#define CP_CODE 1		/* Bytecode address */
#define CP_SIZE 2		/* Size of bytecode */
#define CP_FRAME 3		/* Frame size */
#define CP_STACK 4		/* Stack size */
#define CP_MAP 5		/* Frame map */
#define CP_STKMAP 6		/* Stack map table */
#define CP_CONST 7		/* First constant */

/* Descriptor layout */
#define DESC_MAP 0		/* Pointer map */
#define DESC_DEPTH 1		/* Inheritance depth of record */
#define DESC_ANCES 2		/* Ancestor list of record */
#define DESC_BOUND 1		/* First bound for flex array */

/* Tokens used in pointer maps: all must be congruent to 2 modulo 4 */
#define GC_BASE 2
#define GC_REPEAT 6
#define GC_END 10
#define GC_MAP 14
#define GC_FLEX 18
#define GC_BLOCK 22
#define GC_MARK 26
#define GC_POINTER 30

/* Error codes */
#define E_CAST 1
#define E_ASSIGN 2
#define E_CASE 3
#define E_WITH 4
#define E_ASSERT 5
#define E_RETURN 6
#define E_BOUND 7
#define E_NULL 8
#define E_DIV 9
#define E_FDIV 10
#define E_STACK 11
#define E_GLOB 12
