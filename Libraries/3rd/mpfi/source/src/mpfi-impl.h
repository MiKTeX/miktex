/* mpfi-impl.h -- Include file for mpfi.

Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2010, 2018,
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

#ifndef __MPFI_IMPL_H__
#define __MPFI_IMPL_H__

#include "mpfi.h"
#if defined(MIKTEX)
#include "mpfi_config.h"
#else
#include "../mpfi_config.h"
#endif

#if MPFR_VERSION < MPFR_VERSION_NUM(3,0,0)
#define MPFI_RNDD GMP_RNDD
#define MPFI_RNDU GMP_RNDU
#else
#define MPFI_RNDD MPFR_RNDD
#define MPFI_RNDU MPFR_RNDU
#define GMP_RNDN  MPFR_RNDN
#endif

#define MPFI_IS_POS(x) ((mpfr_sgn((&(x->left)))>=0) && (mpfr_sgn((&(x->right)))>0))
#define MPFI_IS_STRICTLY_POS(x) ((mpfr_sgn((&(x->left)))>0) && (mpfr_sgn((&(x->right)))>0))
#define MPFI_IS_NONNEG(x) ((mpfr_sgn((&(x->left)))>=0) && (mpfr_sgn((&(x->right)))>=0))
#define MPFI_IS_NEG(x) ((mpfr_sgn((&(x->left)))<0) && (mpfr_sgn((&(x->right)))<=0))
#define MPFI_IS_STRICTLY_NEG(x) ((mpfr_sgn((&(x->left)))<0) && (mpfr_sgn((&(x->right)))<0))
#define MPFI_IS_NONPOS(x) ((mpfr_sgn((&(x->left)))<=0) && (mpfr_sgn((&(x->right)))<=0))
#define MPFI_IS_NULL(x) ((mpfr_sgn((&(x->left)))==0) && (mpfr_sgn((&(x->right)))==0))
#define MPFI_HAS_ZERO(x) ((mpfr_sgn((&(x->left)))<0) && (mpfr_sgn((&(x->right)))>0))
#define MPFI_HAS_ZERO_NONSTRICT(x) ((mpfr_sgn((&(x->left)))<=0) && (mpfr_sgn((&(x->right)))>=0))


#if defined(GMP_NUMB_BITS) /* GMP 4.1.2 or above */
# define BITS_PER_MP_LIMB  (GMP_NUMB_BITS+GMP_NAIL_BITS)
#elif defined (__GMP_BITS_PER_MP_LIMB) /* Older versions 4.x.x */
#define BITS_PER_MP_LIMB  __GMP_BITS_PER_MP_LIMB
#else
# error "Could not detect BITS_PER_MP_LIMB. Get GMP 4.1.0 at least."
#endif

#define MPFR_RET_NAN do {mpfr_set_nanflag(); return 0;} while (0)

#define MPFR_IS_NAN(x)  (mpfr_nan_p(x))
#define MPFR_IS_ZERO(x) (mpfr_sgn(x) == 0)
#define MPFR_IS_INF(x)  (mpfr_inf_p(x))

/* Internal functions */

#if defined (__cplusplus)
extern "C" {
#endif



mpfr_prec_t mpfi_quadrant (mpz_ptr, mpfr_srcptr);
int     mpfi_cmp_sym_pi (mpz_srcptr, mpfr_srcptr, mpfr_srcptr, mpfr_prec_t);

/* default comparison functions */
int    	mpfi_cmp_default    (mpfi_srcptr, mpfi_srcptr);
int    	mpfi_cmp_d_default  (mpfi_srcptr, const double);
int    	mpfi_cmp_ui_default (mpfi_srcptr, const unsigned long);
int    	mpfi_cmp_si_default (mpfi_srcptr, const long);
int    	mpfi_cmp_z_default  (mpfi_srcptr, mpz_srcptr);
int    	mpfi_cmp_q_default  (mpfi_srcptr, mpq_srcptr);
int    	mpfi_cmp_fr_default (mpfi_srcptr, mpfr_srcptr);

int    	mpfi_is_pos_default          (mpfi_srcptr);
int    	mpfi_is_nonneg_default       (mpfi_srcptr);
int    	mpfi_is_neg_default          (mpfi_srcptr);
int    	mpfi_is_nonpos_default       (mpfi_srcptr);
int    	mpfi_is_zero_default         (mpfi_srcptr);
int    	mpfi_is_strictly_neg_default (mpfi_srcptr);
int    	mpfi_is_strictly_pos_default (mpfi_srcptr);

#if defined (__cplusplus)
}
#endif

/* Debug macros */
#ifdef WARN_IF_REVERTED_ENDPOINTS
/* include stdio.h when using this macro */
# define WARNING_REVERTED_ENDPOINTS(a, fname)                           \
  do {									\
  fprintf (stdout, "Pb endpoints in reverse order in %s\n", fname);     \
  mpfi_out_str (stdout, 10, 0, a);                                      \
  fprintf (stdout, "\n");						\
  } while (0)
#else
# define WARNING_REVERTED_ENDPOINTS(a, fname)
#endif

#endif /* __MPFI_IMPL_H__ */
