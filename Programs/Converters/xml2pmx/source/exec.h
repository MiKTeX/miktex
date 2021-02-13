/*
 * exec.h
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

/* 
The binary file output by the linker and input by the run-time
interpreter consists of several segments followed by a trailer that
describes the layout of the segments.  The segments may be preceded
by a block of arbitrary data, but they must appear in the following order:

    CODE, DATA, RELOC, SYMTAB

By using a trailer rather than a header, we allow for a binary file
also to be a valid file in another format that uses a header -- like
the executable format of most operating systems including various
Unixes and MS--DOS.  In that format, our code is just junk beyond the
end of the file, and it is usually ignored.  So we can make
self-contained executables by concatenating the interpreter with the
code for a program. This idea was stolen from CAML Light.  
*/

/* Codes for the segments: they must appear in the object file
   in this order */
#define NSEGS 4
#define S_CODE 0		/* Bytecode */
#define S_DATA 1		/* Initialized data */
#define S_BSS 2			/* Below stack storage */
#define S_STACK 3		/* Stack */

typedef uchar word4[4];

typedef struct {
     word4 magic;		/* Magic number 'OBCX' */
     word4 sig;			/* Signature */
     word4 primsig;		/* Checksum of primitives */
     word4 start;		/* Offset of data start from end of file */
     word4 entry;		/* Entry point */
     word4 gcmap;		/* Global pointer map */
     word4 libdir;		/* Location of dynamic libraries */
     word4 segment[NSEGS];	/* Segment sizes */
     word4 nprocs, nmods, nsyms; /* No. of procs, modules, symbols */
} trailer;


/* Layout of relocation data */
#define WORD_SIZE 4
#define CODES_PER_BYTE 4
#define BITS_PER_CODE 2
#define CODE_MASK ((1 << BITS_PER_CODE) - 1)

#define reloc_bits(buf, i) (buf[(i)/CODES_PER_BYTE] >> \
	((i) % CODES_PER_BYTE * BITS_PER_CODE) & CODE_MASK)

#define IBIT 0x80000000

/* Relocation codes */
#define R_NONE 0
#define R_WORD 1
#define R_ADDR 2
#define R_SUBR 3

/* Symbol tags */
#define X_NONE 0
#define X_MODULE 1
#define X_PROC 2
#define X_DATA 3
#define X_LINE 4
#define X_SYM 5

/* Fixed primitives */
#define INTERP 0		/* Index of interpreter as primitive */
#define DLTRAP 1                /* Index of dynlink trap */
