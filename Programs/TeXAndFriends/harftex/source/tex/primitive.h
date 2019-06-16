/* primitive.h

   Copyright 2008-2009 Taco Hoekwater <taco@luatex.org>

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


#ifndef LUATEX_PRIMITIVE_H
#  define LUATEX_PRIMITIVE_H 1

/* This enum is a list of origins for primitive commands */

typedef enum {
    /* Can be modified as suggested ?*/
    tex_command    =  1,  /*  1 */
    etex_command   =  2,  /*  2 */
    pdftex_command = 16,  /*  4 */ /* obsolete */
    luatex_command = 32,  /*  8 */
    core_command   = 64,  /* 16 */
    no_command     = 128, /* 32 */
    umath_command  = 256, /* 64 */ /* obsolete */
} command_origin;

#  define hash_size 65536       /* maximum number of control sequences; it should be at most about |(fix_mem_max-fix_mem_min)/10| */
#  define hash_prime 55711      /* a prime number equal to about 85\pct! of |hash_size| */

extern two_halves *hash;        /* the hash table */
extern halfword hash_used;      /* allocation pointer for |hash| */
extern int hash_extra;          /* |hash_extra=hash| above |eqtb_size| */
extern halfword hash_top;       /* maximum of the hash array */
extern halfword hash_high;      /* pointer to next high hash location */
extern boolean no_new_control_sequence; /* are new identifiers legal? */
extern int cs_count;            /* total number of known identifiers */

#  define cs_next(a) hash[(a)].lhfield  /* link for coalesced lists */
#  define cs_text(a) hash[(a)].rh
                                /* string number for control sequence name */

#  define undefined_primitive 0
#  define prim_size 2100        /* maximum number of primitives */
#  define prim_prime 1777       /* about 85\pct! of |primitive_size| */

extern void init_primitives(void);
extern void ini_init_primitives(void);

extern halfword compute_pool_hash(pool_pointer j, pool_pointer l,
                                  halfword prime_number);
extern pointer prim_lookup(str_number s);

extern boolean is_primitive(str_number csname);

extern quarterword get_prim_eq_type(int p);
extern halfword get_prim_equiv(int p);
extern str_number get_prim_text(int p);
extern quarterword get_prim_origin(int p);

extern void dump_primitives(void);
extern void undump_primitives(void);

#  define primitive_tex(a,b,c,d)    primitive((a),(b),(c),(d),tex_command)
#  define primitive_etex(a,b,c,d)   primitive((a),(b),(c),(d),etex_command)
#  define primitive_pdftex(a,b,c,d) primitive((a),(b),(c),(d),pdftex_command)
#  define primitive_luatex(a,b,c,d) primitive((a),(b),(c),(d),luatex_command)
#  define primitive_umath(a,b,c,d)  primitive((a),(b),(c),(d),umath_command)
#  define primitive_core(a,b,c,d)   primitive((a),(b),(c),(d),core_command)
#  define primitive_no(a,b,c,d)     primitive((a),(b),(c),(d),no_command)

extern void primitive(const char *ss, quarterword c, halfword o, halfword off,
                      int cmd_origin);
extern void primitive_def(const char *s, size_t l, quarterword c, halfword o);
extern void print_cmd_chr(quarterword cmd, halfword chr_code);

extern pointer string_lookup(const char *s, size_t l);
extern pointer id_lookup(int j, int l);

#endif                          /* LUATEX_PRIMITIVE_H */
