/*
 * loader.c
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
#include "keiko.h"
#include "exec.h"
#include <string.h>

/* get_int -- get a 4-byte value in portable byte order */
static int get_int(uchar *p) {
     return (p[3]<<24) + (p[2]<<16) + (p[1]<<8) + p[0];
}

/* Here is the still centre of the whirling vortex that is byte-order
   independence.  The compiler output, Kieko assembly language, is
   plain text.  The assembler/linker translates this into a byte-order
   independent file of object code.  

   The bytecode in this file contains one and two byte embedded
   constants that are in little-endian order, and the bytecode
   interpreter puts the bytes together where necessary, respecting the
   little-endian order in the code even on a big-endian machine.  (It
   has to address bytecode one byte a time anyway, because of
   alignment restrictions.)

   The data segment in the object code consists of 4-byte words, and
   these are relocated when the program is loaded.  Some of these
   words contain character data for string constants, and they require
   no relocation.  Some words contain integer or floating-point
   constants, and they are relocated by swapping the byte order if
   necessary.  Finally, some words contain addresses in the data or
   code segment, and they are relocated by swapping the byte order as
   needed, and adding the base address of the segment in question.
   Thus in the running program, both the memory and the evaluation
   stack contain only values in native byte order -- and all pointers
   are represented as absolute addresses, enabling the program to live
   in harmony with a conservative garbage collector.

   One final twist: double-precision values are always stored as two
   words, with each word in native byte order, but with the less
   significant word first, even on a big-endian machine.  This is ok,
   because these values are always loaded and stored one word at a
   time, and assembled into native order immediately before doing
   arithmetic. */

#define REL_BLOCK 4096

static void reloc(int base, const uchar rbuf[], int size) {
     int i;
     value *p;
     unsigned m;

     for (i = 0; i < size; i += WORD_SIZE) {
          int rbits = reloc_bits(rbuf, i/WORD_SIZE);

#ifdef DEBUG
          if (dflag > 3)
               printf("Reloc %d %d\n", base+i, rbits);
#endif

          if (rbits == R_NONE) continue;

          p = (value *) &dmem[base+i];
          m = get_int((uchar *) p);

          switch (rbits) {
          case R_WORD:
               (*p).i = m;
               break;
          case R_ADDR:
               if ((m & IBIT) == 0)
                    (*p).a = dsegaddr(dmem + m);
               else
                    (*p).a = codeaddr(imem + (m & ~IBIT));
               break;
          case R_SUBR:
               switch (m) {
               case INTERP: (*p).a = interpreter; break;
               case DLTRAP: (*p).a = dyntrap; break;
               default:
                    panic("bad subr code %x\n", m);
               }
               break;
          }
     }
}

#ifndef PRELOAD
static FILE *binfp;

static int binread(void *buf, int size) {
     return fread(buf, 1, size, binfp);
}

/* relocate -- read relocation data */
static void relocate(int size) {
     uchar rbuf[REL_BLOCK];
     int n;

     for (int base = 0; base < size; base += n) {
	  n = min(size - base, REL_BLOCK * CODES_PER_BYTE * WORD_SIZE);
	  int nbytes = (n/WORD_SIZE+CODES_PER_BYTE-1)/CODES_PER_BYTE;
	  binread(rbuf, nbytes);
          reloc(base, rbuf, n);
     }
}
	       
static int bingetc(void) {
     char buf[1];
     if (binread(buf, 1) == 0) return EOF;
     return buf[0];
}

/* read_int -- input a 4-byte value in portable byte order */
static int read_int() {
     uchar buf[4];
     binread(buf, 4);
     return get_int(buf);
}

/* read_string -- input a null-terminated string, allocate space dynamically */
static char *read_string() {
     int n = 0;
     int c;
     char *p;
     char buf[256];
     
     do {
	  c = bingetc();
	  if (c == EOF) panic("*unexpected EOF");
	  buf[n++] = c;
     } while (c != '\0');

     p = scratch_alloc_atomic(n);
     strcpy(p, buf);
     return p;
}

/* read_symbols -- read symbol table */
static void read_symbols(int dseg) {
     word addr;
     int chksum, nlines;
#ifdef DEBUG
     const char *kname;
#define debug_kind(n) kname = n
#else
#define debug_kind(n)
#endif
	  
     for (int i = 0; i < nsyms; i++) {
	  int kind = read_int();
	  char *name = read_string(); 

	  switch (kind) {
	  case X_MODULE:
	       debug_kind("Module");
	       addr = dsegaddr(dmem + read_int()); 
	       chksum = read_int();
	       nlines = read_int();
	       make_module(name, addr, chksum, nlines);
	       break;

	  case X_PROC:
	       debug_kind("Proc");
	       addr = dsegaddr(dmem + read_int()); 
	       make_proc(name, addr);
	       break;
		    
	  case X_DATA:
	       debug_kind("Data");
	       addr = dsegaddr(dmem + read_int()); 
	       make_symbol("data", name, addr);
	       break;

	  case X_LINE:
	       debug_kind("Line");
	       addr = codeaddr(imem + read_int());
	       make_symbol("line", name, addr);
	       break;

	  default:
	       debug_kind("Unknown"); 
	       addr = 0;
	       panic("*bad symbol %s", name);
	  }

#ifdef DEBUG
	  if (dflag >= 1) printf("%s %s = %#x\n", kname, name, addr);
#endif
     }

     fix_sizes(dseg);
}

/* load_file -- load a file of object code */
void load_file(FILE *bfp) {
     /* Get trailer */
     trailer t;
     fseek(bfp, - (long) sizeof(trailer), SEEK_END);
     int nread = fread(&t, 1, sizeof(trailer), bfp);
     if (nread != sizeof(trailer)) panic("couldn't read trailer");

     /* Check magic numbers */
     if (nread < sizeof(trailer))
	  panic("couldn't read trailer");
     if (strncmp((char *) t.magic, MAGIC, 4) != 0)
	  panic("bad magic number\n%s",
		"[The program you are running is not a valid"
		" Oberon bytecode file]");
     if (get_int(t.sig) != SIG)
	  panic("bad signature %#0.8x\n%s\n%s", get_int(t.sig),
		"[Although this appears to be an Oberon bytecode file,",
		"  it needs a different version of the runtime system]");

     /* Decode the other data */
     int seglen[NSEGS];
     for (int i = 0; i < NSEGS; i++)
	  seglen[i] = get_int(t.segment[i]);

     code_size = seglen[S_CODE];
     stack_size = seglen[S_STACK];

     nmods = get_int(t.nmods); nprocs = get_int(t.nprocs); 
     nsyms = get_int(t.nsyms);
     int start = get_int(t.start);

#ifdef DEBUG
     if (dflag >= 1) {
	  printf("csize = %d, dsize = %d, bss = %d, stk = %d\n", 
		 seglen[S_CODE], seglen[S_DATA], 
		 seglen[S_BSS], seglen[S_STACK]);
	  printf("nmods = %d, nprocs = %d, nsyms = %d\n",
		 nmods, nprocs, nsyms);
     }
#endif

     fseek(bfp, start, SEEK_END);
     binfp = bfp;

     /* Load the code */
     imem = scratch_alloc_atomic(seglen[S_CODE]);
     binread(imem, seglen[S_CODE]);

     /* Load and relocate the data */
     dmem = scratch_alloc(seglen[S_DATA]+seglen[S_BSS]);
#ifdef SEGMEM
     data_vbase = map_segment(dmem, seglen[S_DATA]+seglen[S_BSS]);
#endif
     binread(dmem, seglen[S_DATA]);
     relocate(seglen[S_DATA]);
     memset(dmem+seglen[S_DATA], 0, seglen[S_BSS]);

     /* Allocate stack */
     stack = scratch_alloc(stack_size);
#ifdef SEGMEM
     stack_vbase = map_segment(stack, stack_size);
#endif
     /* Save the entry point, pointer map and library path */
     entry = (value *) &dmem[get_int(t.entry)];
     gcmap = (value *) &dmem[get_int(t.gcmap)];
     if (get_int(t.libdir) != 0)
	  libpath = (char *) &dmem[get_int(t.libdir)];

     /* Read the symbols */
     if (nsyms > 0) read_symbols(seglen[S_DATA]);
}
#else
extern const uchar preload_imem[], preload_dmem[], preload_reloc[];
extern const unsigned preload_segsize[];
extern const unsigned preload_entry, preload_gcmap, preload_libdir;
extern const unsigned preload_nprocs, preload_nmods;
extern const struct _sym { int kind; char *name; int val; } preload_syms[];

#define seglen preload_segsize

/* load_image -- unpack preloaded image */
void load_image(void) {
     int i;
     
     code_size = seglen[S_CODE];
     stack_size = seglen[S_STACK];
     nmods = preload_nmods;
     nprocs = preload_nprocs;
     nsyms = nmods+nprocs;

     imem = (uchar *) preload_imem;
     dmem = scratch_alloc(seglen[S_DATA]+seglen[S_BSS]);
#ifdef SEGMEM
     data_vbase = map_segment(dmem, seglen[S_DATA]+seglen[S_BSS]);
#endif
     memcpy(dmem, preload_dmem, seglen[S_DATA]);
     reloc(0, preload_reloc, seglen[S_DATA]);
     memset(dmem+seglen[S_DATA], 0, seglen[S_BSS]);
     stack = scratch_alloc(stack_size);
#ifdef SEGMEM
     stack_vbase = map_segment(stack, stack_size);
#endif

     for (i = 0; i < nsyms; i++) {
          const struct _sym *s = &preload_syms[i];
          switch (s->kind) {
          case X_PROC:
               make_proc(s->name, dsegaddr(dmem + s->val));
               break;
          case X_MODULE:
               make_module(s->name, dsegaddr(dmem + s->val), 0, 0);
               break;
          default:
               panic("Bad symbol code");
          }
     }

     fix_sizes(seglen[S_DATA]);
     entry = (value *) &dmem[preload_entry];
     gcmap = (value *) &dmem[preload_gcmap];

     if (preload_libdir != 0)
	  libpath = (char *) &dmem[preload_libdir];
}
#endif
