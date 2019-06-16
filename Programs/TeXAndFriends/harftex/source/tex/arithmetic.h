/* arithmetic.h
   
   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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


#ifndef ARITHMETIC_H
#  define ARITHMETIC_H

/* |incr| and |decr| are in texmfmp.h */

                                                                                                                                                                   /* #define incr(A) (A)++ *//* increase a variable by unity */
                                                                                                                                                                   /* #define decr(A) (A)-- *//* decrease a variable by unity */

#  define negate(A) (A)=-(A)    /* change the sign of a variable */

#  undef half
extern int half(int x);

/*
Fixed-point arithmetic is done on {\sl scaled integers\/} that are multiples
of $2^{-16}$. In other words, a binary point is assumed to be sixteen bit
positions from the right end of a binary computer word.
*/

#  define unity 0200000         /* $2^{16}$, represents 1.00000 */
#  define two   0400000         /* $2^{17}$, represents 2.00000 */

typedef unsigned int nonnegative_integer;       /* $0\L x<2^{31}$ */

extern scaled round_decimals(int k);
extern void print_scaled(scaled s);

extern boolean arith_error;
extern scaled tex_remainder;

extern scaled mult_and_add(int n, scaled x, scaled y, scaled max_answer);

#  define nx_plus_y(A,B,C) mult_and_add((A),(B),(C),07777777777)
#  define mult_integers(A,B) mult_and_add((A),(B),0,017777777777)

extern scaled x_over_n(scaled x, int n);
extern scaled xn_over_d(scaled x, int n, int d);

#  define inf_bad 10000         /* infinitely bad value */

extern halfword badness(scaled t, scaled s);

#  define set_glue_ratio_zero(A) (A)=0.0        /* store the representation of zero ratio */
#  define set_glue_ratio_one(A) (A)=1.0 /* store the representation of unit ratio */
#  define float_cast(A) (float)(A)      /* convert from |glue_ratio| to type |real| */
#  define unfloat(A) (glue_ratio)(A)    /* convert from |real| to type |glue_ratio| */
#  define float_constant(A) (float)A    /* convert |integer| constant to |real| */
#  define float_round round

extern void initialize_arithmetic(void);

extern scaled random_seed;      /* the default random seed */

extern void init_randoms(int seed);
extern int unif_rand(int x);
extern int norm_rand(void);

extern int fix_int(int val, int min, int max);

#endif
