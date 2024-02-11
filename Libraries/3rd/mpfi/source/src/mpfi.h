/* mpfi.h -- Include file for mpfi.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2012, 2018,
                     Spaces project, Inria Lorraine
                     and Salsa project, INRIA Rocquencourt,
                     and Arenaire project, Inria Rhone-Alpes, France
                     and Lab. ANO, USTL (Univ. of Lille),  France


This file is part of the MPFI Library.

The MPFI Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPFI Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPFI Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
MA 02110-1301, USA. */


#ifndef __MPFI_H__
#define __MPFI_H__

/* Machine independent implementations */

#include <gmp.h>
#include <mpfr.h>

/* Define MPFI version number */
#define MPFI_VERSION_MAJOR 1
#define MPFI_VERSION_MINOR 5
#define MPFI_VERSION_PATCHLEVEL 4
#define MPFI_VERSION_STRING "1.5.4"

typedef struct {
  __mpfr_struct left;
  __mpfr_struct right;
}__mpfi_struct;

typedef __mpfi_struct mpfi_t[1];
typedef __mpfi_struct *mpfi_ptr;
typedef const __mpfi_struct *mpfi_srcptr;

#ifdef __cplusplus
extern "C" {
#endif

/* Rounding                                     */
int     mpfi_round_prec (mpfi_ptr, mpfr_prec_t prec);


/* Initialization, destruction and assignment   */

/* initializations */
void    mpfi_init       (mpfi_ptr);
void    mpfi_init2      (mpfi_ptr, mpfr_prec_t);
void 	mpfi_inits 	(mpfi_ptr x, ...);
void 	mpfi_inits2 	(mpfr_prec_t p, mpfi_ptr x, ...);

void    mpfi_clear      (mpfi_ptr);
void 	mpfi_clears 	(mpfi_ptr x, ...);

/* mpfi bounds have the same precision */
mpfr_prec_t mpfi_get_prec (mpfi_srcptr);
void    mpfi_set_prec   (mpfi_ptr, mpfr_prec_t);


/* assignment functions                         */
int     mpfi_set        (mpfi_ptr, mpfi_srcptr);
int     mpfi_set_si     (mpfi_ptr, const long);
int     mpfi_set_ui     (mpfi_ptr, const unsigned long);
/* int     mpfi_set_sj     (mpfi_ptr, const intmax_t); */
/* int     mpfi_set_uj     (mpfi_ptr, const uintmax_t); */
int     mpfi_set_d      (mpfi_ptr, const double);
int     mpfi_set_flt    (mpfi_ptr, const float);
int     mpfi_set_ld     (mpfi_ptr, const long double);
int     mpfi_set_z      (mpfi_ptr, mpz_srcptr);
int     mpfi_set_q      (mpfi_ptr, mpq_srcptr);
int     mpfi_set_fr     (mpfi_ptr, mpfr_srcptr);
int     mpfi_set_str    (mpfi_ptr, const char *, int);

/* combined initialization and assignment functions */
int     mpfi_init_set       (mpfi_ptr, mpfi_srcptr);
int     mpfi_init_set_si    (mpfi_ptr, const long);
int     mpfi_init_set_ui    (mpfi_ptr, const unsigned long);
int     mpfi_init_set_d     (mpfi_ptr, const double);
int     mpfi_init_set_flt   (mpfi_ptr, const float);
int     mpfi_init_set_ld    (mpfi_ptr, const long double);
int     mpfi_init_set_z     (mpfi_ptr, mpz_srcptr);
int     mpfi_init_set_q     (mpfi_ptr, mpq_srcptr);
int     mpfi_init_set_fr    (mpfi_ptr, mpfr_srcptr);
int     mpfi_init_set_str   (mpfi_ptr, const char *, int);

/* swapping two intervals */
void    mpfi_swap (mpfi_ptr, mpfi_ptr);


/* Various useful interval functions            */
/* with scalar or interval results              */

/* absolute diameter                            */
int     mpfi_diam_abs   (mpfr_ptr, mpfi_srcptr);
/* relative diameter                            */
int     mpfi_diam_rel   (mpfr_ptr, mpfi_srcptr);
/* diameter: relative if the interval does not contain 0 */
/* absolute otherwise                                    */
int     mpfi_diam       (mpfr_ptr, mpfi_srcptr);
/* magnitude: the largest absolute value of any element */
int     mpfi_mag        (mpfr_ptr, mpfi_srcptr);
/* mignitude: the smallest absolute value of any element */
int     mpfi_mig        (mpfr_ptr, mpfi_srcptr);
/* middle of y                                           */
int     mpfi_mid        (mpfr_ptr, mpfi_srcptr);
/* picks randomly a point m in y */
void    mpfi_alea       (mpfr_ptr, mpfi_srcptr);
void    mpfi_urandom    (mpfr_ptr, mpfi_srcptr, gmp_randstate_t);
void    mpfi_nrandom    (mpfr_ptr, mpfi_srcptr, gmp_randstate_t);
void    mpfi_erandom    (mpfr_ptr, mpfi_srcptr, gmp_randstate_t);


/* Conversions                                  */

double  mpfi_get_d      (mpfi_srcptr);
void    mpfi_get_fr     (mpfr_ptr, mpfi_srcptr);


/* Basic arithmetic operations                  */

/* arithmetic operations between two interval operands */
int     mpfi_add        (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);
int     mpfi_sub        (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);
int     mpfi_mul        (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);
int     mpfi_div        (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);

/* arithmetic operations between an interval operand and a double prec. floating-point */
int     mpfi_add_d      (mpfi_ptr, mpfi_srcptr, const double);
int     mpfi_sub_d      (mpfi_ptr, mpfi_srcptr, const double);
int     mpfi_d_sub      (mpfi_ptr, const double, mpfi_srcptr);
int     mpfi_mul_d      (mpfi_ptr, mpfi_srcptr, const double);
int     mpfi_div_d      (mpfi_ptr, mpfi_srcptr, const double);
int     mpfi_d_div      (mpfi_ptr, const double, mpfi_srcptr);

/* arithmetic operations between an interval operand and an unsigned long integer */
int     mpfi_add_ui     (mpfi_ptr, mpfi_srcptr, const unsigned long);
int     mpfi_sub_ui     (mpfi_ptr, mpfi_srcptr, const unsigned long);
int     mpfi_ui_sub     (mpfi_ptr, const unsigned long, mpfi_srcptr);
int     mpfi_mul_ui     (mpfi_ptr, mpfi_srcptr, const unsigned long);
int     mpfi_div_ui     (mpfi_ptr, mpfi_srcptr, const unsigned long);
int     mpfi_ui_div     (mpfi_ptr, const unsigned long, mpfi_srcptr);

/* arithmetic operations between an interval operand and a long integer */
int     mpfi_add_si     (mpfi_ptr, mpfi_srcptr, const long);
int     mpfi_sub_si     (mpfi_ptr, mpfi_srcptr, const long);
int     mpfi_si_sub     (mpfi_ptr, const long, mpfi_srcptr);
int     mpfi_mul_si     (mpfi_ptr, mpfi_srcptr, const long);
int     mpfi_div_si     (mpfi_ptr, mpfi_srcptr, const long);
int     mpfi_si_div     (mpfi_ptr, const long, mpfi_srcptr);

/* arithmetic operations between an interval operand and a multiple prec. integer */
int     mpfi_add_z      (mpfi_ptr, mpfi_srcptr, mpz_srcptr);
int     mpfi_sub_z      (mpfi_ptr, mpfi_srcptr, mpz_srcptr);
int     mpfi_z_sub      (mpfi_ptr, mpz_srcptr, mpfi_srcptr);
int     mpfi_mul_z      (mpfi_ptr, mpfi_srcptr, mpz_srcptr);
int     mpfi_div_z      (mpfi_ptr, mpfi_srcptr, mpz_srcptr);
int     mpfi_z_div      (mpfi_ptr, mpz_srcptr, mpfi_srcptr);

/* arithmetic operations between an interval operand and a multiple prec. rational */
int     mpfi_add_q      (mpfi_ptr, mpfi_srcptr, mpq_srcptr);
int     mpfi_sub_q      (mpfi_ptr, mpfi_srcptr, mpq_srcptr);
int     mpfi_q_sub      (mpfi_ptr, mpq_srcptr, mpfi_srcptr);
int     mpfi_mul_q      (mpfi_ptr, mpfi_srcptr, mpq_srcptr);
int     mpfi_div_q      (mpfi_ptr, mpfi_srcptr, mpq_srcptr);
int     mpfi_q_div      (mpfi_ptr, mpq_srcptr, mpfi_srcptr);

/* arithmetic operations between an interval operand and a mult. prec. floating-pt nb */
int     mpfi_add_fr     (mpfi_ptr, mpfi_srcptr, mpfr_srcptr);
int     mpfi_sub_fr     (mpfi_ptr, mpfi_srcptr, mpfr_srcptr);
int     mpfi_fr_sub     (mpfi_ptr, mpfr_srcptr, mpfi_srcptr);
int     mpfi_mul_fr     (mpfi_ptr, mpfi_srcptr, mpfr_srcptr);
int     mpfi_div_fr     (mpfi_ptr, mpfi_srcptr, mpfr_srcptr);
int     mpfi_fr_div     (mpfi_ptr, mpfr_srcptr, mpfi_srcptr);

/* arithmetic operations taking a single interval operand */
int     mpfi_neg        (mpfi_ptr, mpfi_srcptr);
int     mpfi_sqr        (mpfi_ptr, mpfi_srcptr);
/* the inv function generates the whole real interval
   if 0 is in the interval defining the divisor */
int     mpfi_inv        (mpfi_ptr, mpfi_srcptr);
/* the sqrt of a (partially) negative interval is a NaN */
int     mpfi_sqrt       (mpfi_ptr, mpfi_srcptr);
int     mpfi_cbrt       (mpfi_ptr, mpfi_srcptr);
/* the first interval contains the absolute values of */
/* every element of the second interval */
int     mpfi_abs        (mpfi_ptr, mpfi_srcptr);
int     mpfi_inv        (mpfi_ptr, mpfi_srcptr);
int     mpfi_rec_sqrt   (mpfi_ptr, mpfi_srcptr);

/* extended division: returns 2 intervals if the denominator contains 0 */
int	mpfi_div_ext	(mpfi_ptr, mpfi_ptr, mpfi_srcptr, mpfi_srcptr);

/* various operations */
int     mpfi_mul_2exp   (mpfi_ptr, mpfi_srcptr, unsigned long);
int     mpfi_mul_2ui    (mpfi_ptr, mpfi_srcptr, unsigned long);
int     mpfi_mul_2si    (mpfi_ptr, mpfi_srcptr, long);
int     mpfi_div_2exp   (mpfi_ptr, mpfi_srcptr, unsigned long);
int     mpfi_div_2ui    (mpfi_ptr, mpfi_srcptr, unsigned long);
int     mpfi_div_2si    (mpfi_ptr, mpfi_srcptr, long);

/* Special functions                                        */
int     mpfi_log        (mpfi_ptr, mpfi_srcptr);
int     mpfi_exp        (mpfi_ptr, mpfi_srcptr);
int     mpfi_exp2       (mpfi_ptr, mpfi_srcptr);
int     mpfi_exp10      (mpfi_ptr, mpfi_srcptr);

int     mpfi_cos        (mpfi_ptr, mpfi_srcptr);
int     mpfi_sin        (mpfi_ptr, mpfi_srcptr);
int     mpfi_tan        (mpfi_ptr, mpfi_srcptr);
int     mpfi_acos       (mpfi_ptr, mpfi_srcptr);
int     mpfi_asin       (mpfi_ptr, mpfi_srcptr);
int     mpfi_atan       (mpfi_ptr, mpfi_srcptr);
int     mpfi_atan2      (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);

int     mpfi_sec        (mpfi_ptr, mpfi_srcptr);
int     mpfi_csc        (mpfi_ptr, mpfi_srcptr);
int     mpfi_cot        (mpfi_ptr, mpfi_srcptr);

int     mpfi_cosh       (mpfi_ptr, mpfi_srcptr);
int     mpfi_sinh       (mpfi_ptr, mpfi_srcptr);
int     mpfi_tanh       (mpfi_ptr, mpfi_srcptr);
int     mpfi_acosh      (mpfi_ptr, mpfi_srcptr);
int     mpfi_asinh      (mpfi_ptr, mpfi_srcptr);
int     mpfi_atanh      (mpfi_ptr, mpfi_srcptr);

int     mpfi_sech       (mpfi_ptr, mpfi_srcptr);
int     mpfi_csch       (mpfi_ptr, mpfi_srcptr);
int     mpfi_coth       (mpfi_ptr, mpfi_srcptr);

int     mpfi_log1p      (mpfi_ptr, mpfi_srcptr);
int     mpfi_log10p1    (mpfi_ptr, mpfi_srcptr);
int     mpfi_log2p1     (mpfi_ptr, mpfi_srcptr);
int     mpfi_expm1      (mpfi_ptr, mpfi_srcptr);
int     mpfi_exp2m1     (mpfi_ptr, mpfi_srcptr);
int     mpfi_exp10m1    (mpfi_ptr, mpfi_srcptr);

int     mpfi_log2       (mpfi_ptr, mpfi_srcptr);
int     mpfi_log10      (mpfi_ptr, mpfi_srcptr);

int     mpfi_hypot      (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);

int     mpfi_const_log2         (mpfi_ptr);
int     mpfi_const_pi           (mpfi_ptr);
int     mpfi_const_euler        (mpfi_ptr);
int     mpfi_const_catalan      (mpfi_ptr);

/* Comparison functions                                     */
/* Warning: the meaning of interval comparison is not clearly defined */
/* customizable comparison functions */

extern int    (*mpfi_cmp)       (mpfi_srcptr, mpfi_srcptr);

extern int    (*mpfi_cmp_d)     (mpfi_srcptr, const double);
extern int    (*mpfi_cmp_ui)    (mpfi_srcptr, const unsigned long);
extern int    (*mpfi_cmp_si)    (mpfi_srcptr, const long);
extern int    (*mpfi_cmp_z)     (mpfi_srcptr, mpz_srcptr);
extern int    (*mpfi_cmp_q)     (mpfi_srcptr, mpq_srcptr);
extern int    (*mpfi_cmp_fr)    (mpfi_srcptr, mpfr_srcptr);

extern int    (*mpfi_is_pos)    (mpfi_srcptr);
extern int    (*mpfi_is_nonneg) (mpfi_srcptr);
extern int    (*mpfi_is_neg)    (mpfi_srcptr);
extern int    (*mpfi_is_nonpos) (mpfi_srcptr);
extern int    (*mpfi_is_zero)   (mpfi_srcptr);
extern int    (*mpfi_is_strictly_pos) (mpfi_srcptr);
extern int    (*mpfi_is_strictly_neg) (mpfi_srcptr);

int     mpfi_has_zero   (mpfi_srcptr);

int     mpfi_nan_p      (mpfi_srcptr);
int     mpfi_inf_p      (mpfi_srcptr);
int     mpfi_bounded_p  (mpfi_srcptr);

/* Interval manipulation */

/* operations related to the internal representation by endpoints */

/* get left or right bound of the interval defined by the
   second argument and put the result in the first one */
int     mpfi_get_left   (mpfr_ptr, mpfi_srcptr);
int     mpfi_get_right  (mpfr_ptr, mpfi_srcptr);

int     mpfi_revert_if_needed  (mpfi_ptr);

/* Set operations on intervals */
/* "Convex hulls" */
/* extends the interval defined by the first argument
   so that it contains the second one */

int     mpfi_put        (mpfi_ptr, mpfi_srcptr);
int     mpfi_put_d      (mpfi_ptr, const double);
int     mpfi_put_si     (mpfi_ptr, const long);
int     mpfi_put_ui     (mpfi_ptr, const unsigned long);
int     mpfi_put_z      (mpfi_ptr, mpz_srcptr);
int     mpfi_put_q      (mpfi_ptr, mpq_srcptr);
int     mpfi_put_fr     (mpfi_ptr, mpfr_srcptr);

/* builds an interval whose left bound is the lower (round -infty)
   than the second argument and the right bound is greater
   (round +infty) than the third one */

int     mpfi_interv_d   (mpfi_ptr, const double, const double);
int     mpfi_interv_si  (mpfi_ptr, const long, const long);
int     mpfi_interv_ui  (mpfi_ptr, const unsigned long, const unsigned long);
int     mpfi_interv_z   (mpfi_ptr, mpz_srcptr, mpz_srcptr);
int     mpfi_interv_q   (mpfi_ptr, mpq_srcptr, mpq_srcptr);
int     mpfi_interv_fr  (mpfi_ptr, mpfr_srcptr, mpfr_srcptr);

/* Inclusion tests */
/* tests if the first argument is inside the interval
   defined by the second one */
int     mpfi_is_strictly_inside (mpfi_srcptr, mpfi_srcptr);
int     mpfi_is_inside        	(mpfi_srcptr, mpfi_srcptr);
int     mpfi_is_inside_d      	(const double, mpfi_srcptr);
int     mpfi_is_inside_ui     	(const unsigned long, mpfi_srcptr);
int     mpfi_is_inside_si     	(const long, mpfi_srcptr);
int     mpfi_is_inside_z      	(mpz_srcptr, mpfi_srcptr);
int     mpfi_is_inside_q      	(mpq_srcptr, mpfi_srcptr);
int     mpfi_is_inside_fr   	(mpfr_srcptr, mpfi_srcptr);

/* set operations */
int     mpfi_is_empty   (mpfi_srcptr);
int     mpfi_intersect  (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);
int     mpfi_union      (mpfi_ptr, mpfi_srcptr, mpfi_srcptr);

/* complement... : to do later */


/* Miscellaneous */

/* adds the second argument to the right bound of the first one
   and subtracts the second argument to the left bound of
   the first one */
int     mpfi_increase   (mpfi_ptr, mpfr_srcptr);
/* keeps the same center and multiply the radius by 2*(1+fact) */
int     mpfi_blow       (mpfi_ptr, mpfi_srcptr, double);
/* splits the interval into 2 halves */
int     mpfi_bisect     (mpfi_ptr, mpfi_ptr, mpfi_srcptr);

const char * mpfi_get_version (void);

/* Error handling */

extern int mpfi_error;
void    mpfi_reset_error (void);
void    mpfi_set_error   (const int);
int     mpfi_is_error    (void);
#ifdef __cplusplus
}
#endif

#define MPFI_ERROR(s)                           \
  do {                                          \
    if(!mpfi_error) {                           \
      mpfi_error = 1;                           \
      fprintf(stderr, "\n%s\n", s);             \
    }                                           \
  } while (0)

#define MPFI_FLAGS_BOTH_ENDPOINTS_EXACT       0
#define MPFI_FLAGS_LEFT_ENDPOINT_INEXACT      1
#define MPFI_FLAGS_RIGHT_ENDPOINT_INEXACT     2
#define MPFI_FLAGS_BOTH_ENDPOINTS_INEXACT     3

#define MPFI_BOTH_ARE_EXACT(x) ( (int)(x) == 0 )
#define MPFI_LEFT_IS_INEXACT(x) ( (int)(x)%2 )
#define MPFI_RIGHT_IS_INEXACT(x) ( (int)(x)/2 )
#define MPFI_BOTH_ARE_INEXACT(x) ( (int)(x) == 3 )

#define MPFI_REVERT_INEXACT_FLAGS(x) \
  ( ((x)==1) ? 2 : ((x)==2) ? 1 : x )

#define MPFI_NAN_P(a) ( MPFR_IS_NAN(&(a->left)) || MPFR_IS_NAN (&(a->right)) )
#define MPFI_INF_P(a) ( MPFR_IS_INF(&(a->left)) || MPFR_IS_INF (&(a->right)) )
#define MPFI_IS_ZERO(a)  (MPFI_NAN_P(a) ? 0 : ((mpfr_sgn(&(a->right))==0) && (mpfr_sgn(&(a->left))==0)))

#define MPFI_CLEAR(m) {mpfr_clear(&(m->right)); mpfr_clear(&(m->left));}

#endif /* __MPFI_H__ */
