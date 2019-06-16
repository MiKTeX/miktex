/* memoryword.h
   
   Copyright 2009-2010 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


/* This header file is extra special because it is read in from
   within the pascal source */

#ifndef MEMORYWORD_H
#  define MEMORYWORD_H

/* texmfmem.h: the memory_word type, which is too hard to translate
   automatically from Pascal.  We have to make sure the byte-swapping
   that the (un)dumping routines do suffices to put things in the right
   place in memory.

   A memory_word can be broken up into a `twohalves' or a
   `fourquarters', and a `twohalves' can be further broken up.  Here is
   a picture.  ..._M = most significant byte, ..._L = least significant
   byte.
   
   
   BigEndian:
   twohalves.v:  RH_MM RH_ML RH_LM RH_LL LH_MM LH_ML LH_LM LH_LL
   twohalves.u:  ---------JUNK---------- ----B0----- ----B1-----
   fourquarters: ----B0----- ----B1----- ----B2----- ----B3-----
   twoints:      ---------CINT0--------- ---------CINT1---------
   
   LittleEndian:
   twohalves.v:  LH_LL LH_LM LH_ML LH_MM RH_LL RH_LM RH_ML RH_MM
   twohalves.u:  ----B1----- ----B0-----
   fourquarters: ----B3----- ----B2----- ----B1----- ----B0-----
   twoints:      ---------CINT1--------- ---------CINT0---------
   
*/


typedef union {
    struct {
#  ifdef WORDS_BIGENDIAN
        halfword RH, LH;
#  else
        halfword LH, RH;
#  endif
    } v;

    struct {                    /* Make B0,B1 overlap the most significant bytes of LH.  */
#  ifdef WORDS_BIGENDIAN
        halfword junk;
        quarterword B0, B1;
#  else                         /* not WORDS_BIGENDIAN */
        /* If 32-bit memory words, have to do something.  */
        quarterword B1, B0;
#  endif                        /* LittleEndian */
    } u;
} two_halves;

typedef struct {
    struct {
#  ifdef WORDS_BIGENDIAN
        quarterword B0, B1, B2, B3;
#  else
        quarterword B3, B2, B1, B0;
#  endif
    } u;
} four_quarters;

typedef struct {
#  ifdef WORDS_BIGENDIAN
    int CINT0, CINT1;
#  else
    int CINT1, CINT0;
#  endif
} two_ints;

typedef struct {
    glue_ratio GLUE;
} glues;

typedef union {
    two_halves hh;
    four_quarters qqqq;
    two_ints ii;
    glues gg;
} memory_word;

#  define b0 u.B0
#  define b1 u.B1
#  define b2 u.B2
#  define b3 u.B3

#  define rh v.RH
#  define lhfield v.LH

#  define cint ii.CINT0
#  define cint1 ii.CINT1

#  define gr gg.GLUE

/* the next five defines are needed for the prototypes in web2c's coerce.h */

#  define memoryword memory_word
#  define strnumber str_number
#  define packedASCIIcode packed_ASCII_code
#  define poolpointer pool_pointer

typedef FILE *word_file;

#  ifdef DEBUG
extern void print_word(memory_word w);
#  endif

#endif
