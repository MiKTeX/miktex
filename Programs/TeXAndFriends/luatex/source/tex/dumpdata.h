/* dumpdata.h
   
   Copyright 2009, 2012 Taco Hoekwater <taco@luatex.org>

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


#ifndef DUMPDATA_H
#  define DUMPDATA_H

extern str_number format_ident;
extern str_number format_name;  /* principal file name */
extern FILE *fmt_file;          /* for input or output of format information */

extern void store_fmt_file(void);
#if defined(MIKTEX)
extern boolean load_fmt_file(const char *, boolean);
#else
extern boolean load_fmt_file(const char *);
#endif

/* (Un)dumping.  These are called from the change file.  */
#  define        dump_things(base, len) \
  do_zdump ((char *) &(base), sizeof (base), (int) (len), DUMP_FILE)
#  define        undump_things(base, len) \
  do_zundump ((char *) &(base), sizeof (base), (int) (len), DUMP_FILE)

extern void do_zdump(char *, int, int, FILE *);
extern void do_zundump(char *, int, int, FILE *);

/* Like do_undump, but check each value against LOW and HIGH.  The
   slowdown isn't significant, and this improves the chances of
   detecting incompatible format files.  In fact, Knuth himself noted
   this problem with Web2c some years ago, so it seems worth fixing.  We
   can't make this a subroutine because then we lose the type of BASE.  */
#  define undump_checked_things(low, high, base, len)			\
    do {								\
      unsigned i;							\
    undump_things (base, len);						\
    for (i = 0; i < (len); i++) {                                       \
	if ((&(base))[i] < (low) || (&(base))[i] > (high)) {		\
	  FATAL5 ("Item %u (=%ld) of .fmt array at %lx <%ld or >%ld",	\
                i, (unsigned long) (&(base))[i], (unsigned long) &(base), \
                (unsigned long) low, (unsigned long) high);			\
      }                                                                 \
    }									\
  } while (0)

/* Like undump_checked_things, but only check the upper value. We use
   this when the base type is unsigned, and thus all the values will be
   greater than zero by definition.  */
#  define undump_upper_check_things(high, base, len)			\
    do {								\
      unsigned i;							\
    undump_things (base, len);						\
    for (i = 0; i < (len); i++) {                                       \
	if ((&(base))[i] > (high)) {					\
	  FATAL4 ("Item %u (=%ld) of .fmt array at %lx >%ld",		\
                i, (unsigned long) (&(base))[i], (unsigned long) &(base), \
                (unsigned long) high);					\
      }                                                                 \
    }									\
  } while (0)

/* Use the above for all the other dumping and undumping.  */
#  define generic_dump(x) dump_things (x, 1)
#  define generic_undump(x) undump_things (x, 1)

#  define dump_wd   generic_dump
#  define dump_hh   generic_dump
#  define dump_qqqq generic_dump
#  define undump_wd   generic_undump
#  define undump_hh   generic_undump
#  define	undump_qqqq generic_undump

/* `dump_int' is called with constant integers, so we put them into a
   variable first.  */
#  define	dump_int(x)							\
  do									\
    {									\
      int x_val = (x);					     	        \
      generic_dump (x_val);						\
    }									\
  while (0)

/* web2c/regfix puts variables in the format file loading into
   registers.  Some compilers aren't willing to take addresses of such
   variables.  So we must kludge.  */
#  if defined(REGFIX) || defined(WIN32)
#    define undump_int(x)							\
  do									\
    {									\
      int x_val;							\
      generic_undump (x_val);						\
      x = x_val;							\
    }									\
  while (0)
#  else
#    define	undump_int generic_undump
#  endif                        /* not (REGFIX || WIN32) */




#endif
