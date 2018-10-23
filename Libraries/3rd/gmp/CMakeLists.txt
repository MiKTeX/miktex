## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2018 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/gmp")

set(DEFN_LONG_LONG_LIMB "#define _LONG_LONG_LIMB 1")

math(EXPR GMP_LIMB_BITS "8 * ${LONG_LONG}")
set(GMP_NAIL_BITS 0)

if(${MIKTEX_TARGET_SYSTEM_TAG} STREQUAL "windows-x86")
  set(HAVE_HOST_CPU_FAMILY_x86 1)
  set(HAVE_LIMB_LITTLE_ENDIAN 1)
elseif(${MIKTEX_TARGET_SYSTEM_TAG} STREQUAL "windows-x64")
  set(HAVE_HOST_CPU_FAMILY_x86_64 1)
  set(HAVE_LIMB_LITTLE_ENDIAN 1)
elseif(${MIKTEX_TARGET_SYSTEM_TAG} STREQUAL "cygwin-x86")
  set(HAVE_HOST_CPU_FAMILY_x86 1)
  set(HAVE_LIMB_LITTLE_ENDIAN 1)
else()
  message(FATAL_ERROR "TODO: implement platform checks")
endif()

set(WANT_TMP_REENTRANT 1)
set(SIZEOF_MP_LIMB_T ${LONG_LONG})
set(VERSION "${MIKTEX_COMP_ORIG_VERSION_STR}")

add_definitions(-DNO_ASM -D__GMP_WITHIN_GMP)

ignore_warnings()

# set to TRUE, if we build the complete GMP library
set(gmp_complete FALSE)

set(fqsrcdir ${CMAKE_CURRENT_SOURCE_DIR}/source)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${fqsrcdir}
  ${fqsrcdir}/mpn/generic
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(
  source/gmp-h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmp.h
  @ONLY
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  gmp-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/gmp-version.h
)

set(generators
  gen-bases
  gen-fac
  gen-fib
  gen-jacobitab
  gen-psqr
  gen-trialdivtab
)

foreach(gen ${generators})
  add_executable(${gen}
    ${fqsrcdir}/${gen}.c
  )
  set_property(TARGET ${gen} PROPERTY FOLDER "${MIKTEX_CURRENT_FOLDER}/gen")
endforeach()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.h
    ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.c
    ${CMAKE_CURRENT_BINARY_DIR}/fac_table.h
    ${CMAKE_CURRENT_BINARY_DIR}/fib_table.h
    ${CMAKE_CURRENT_BINARY_DIR}/fib_table.c
    ${CMAKE_CURRENT_BINARY_DIR}/jacobitab.h
    ${CMAKE_CURRENT_BINARY_DIR}/perfsqr.h
    ${CMAKE_CURRENT_BINARY_DIR}/trialdivtab.h
  COMMAND gen-bases header ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.h
  COMMAND gen-bases table ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.c
  COMMAND gen-fac ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/fac_table.h
  COMMAND gen-fib header ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/fib_table.h
  COMMAND gen-fib table ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/fib_table.c
  COMMAND gen-jacobitab ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/jacobitab.h
  COMMAND gen-psqr ${GMP_LIMB_BITS} ${GMP_NAIL_BITS}
                > ${CMAKE_CURRENT_BINARY_DIR}/perfsqr.h
  COMMAND gen-trialdivtab ${GMP_LIMB_BITS} 8000
                > ${CMAKE_CURRENT_BINARY_DIR}/trialdivtab.h
)

set(generated_gmp_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.h
  ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.c
  ${CMAKE_CURRENT_BINARY_DIR}/fac_table.h
  ${CMAKE_CURRENT_BINARY_DIR}/fib_table.h
  ${CMAKE_CURRENT_BINARY_DIR}/fib_table.c
  ${CMAKE_CURRENT_BINARY_DIR}/jacobitab.h
  ${CMAKE_CURRENT_BINARY_DIR}/perfsqr.h
  ${CMAKE_CURRENT_BINARY_DIR}/trialdivtab.h
)

add_custom_target(gen-gmp-sources DEPENDS
  ${generated_gmp_sources}
)

set_property(TARGET gen-gmp-sources PROPERTY FOLDER "${MIKTEX_CURRENT_FOLDER}/gen")

set(basic_sources
  ${fqsrcdir}/assert.c
  ${fqsrcdir}/compat.c
  ${fqsrcdir}/errno.c
  ${fqsrcdir}/extract-dbl.c
  ${fqsrcdir}/invalid.c
  ${fqsrcdir}/memory.c
  ${fqsrcdir}/mp_bpl.c
  ${fqsrcdir}/mp_clz_tab.c
  ${fqsrcdir}/mp_dv_tab.c
  ${fqsrcdir}/mp_get_fns.c
  ${fqsrcdir}/mp_minv_tab.c
  ${fqsrcdir}/mp_set_fns.c
  ${fqsrcdir}/nextprime.c
  ${fqsrcdir}/primesieve.c
  ${fqsrcdir}/tal-reent.c
  ${fqsrcdir}/version.c
)

set(logops_n_ops
  and_n
  andn_n
  ior_n
  iorn_n
  nand_n
  nior_n
  xnor_n
  xor_n
)

foreach(op in ${logops_n_ops})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${op}.c)
    file(WRITE
      ${CMAKE_CURRENT_BINARY_DIR}/${op}.c
      "#define OPERATION_${op} 1\n#include <logops_n.c>\n"
    )
  endif()
endforeach()

set(popham_ops
  hamdist
  popcount
)

foreach(op in ${popham_ops})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${op}.c)
    file(WRITE
      ${CMAKE_CURRENT_BINARY_DIR}/${op}.c
      "#define OPERATION_${op} 1\n#include <popham.c>\n"
    )
  endif()
endforeach()

set(sec_aors_1_ops
  sec_add_1
  sec_sub_1
)

foreach(op in ${sec_aors_1_ops})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${op}.c)
    file(WRITE
      ${CMAKE_CURRENT_BINARY_DIR}/${op}.c
      "#define OPERATION_${op} 1\n#include <sec_aors_1.c>\n"
    )
  endif()
endforeach()

set(sec_div_ops
  sec_div_qr
  sec_div_r
)

foreach(op in ${sec_div_ops})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${op}.c)
    file(WRITE
      ${CMAKE_CURRENT_BINARY_DIR}/${op}.c
      "#define OPERATION_${op} 1\n#include <sec_div.c>\n"
    )
  endif()
endforeach()

set(sec_pi1_div_ops
  sec_pi1_div_qr
  sec_pi1_div_r
)

foreach(op in ${sec_pi1_div_ops})
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${op}.c)
    file(WRITE
      ${CMAKE_CURRENT_BINARY_DIR}/${op}.c
      "#define OPERATION_${op} 1\n#include <sec_pi1_div.c>\n"
    )
  endif()
endforeach()

set(mpn_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.c
  ${CMAKE_CURRENT_BINARY_DIR}/popcount.c

  ${fqsrcdir}/mpn/generic/add.c
  ${fqsrcdir}/mpn/generic/add_1.c
  ${fqsrcdir}/mpn/generic/add_n.c
  ${fqsrcdir}/mpn/generic/addmul_1.c
  ${fqsrcdir}/mpn/generic/bdiv_dbm1c.c
  ${fqsrcdir}/mpn/generic/bdiv_q.c
  ${fqsrcdir}/mpn/generic/binvert.c
  ${fqsrcdir}/mpn/generic/cmp.c
  ${fqsrcdir}/mpn/generic/com.c
  ${fqsrcdir}/mpn/generic/copyd.c
  ${fqsrcdir}/mpn/generic/copyi.c
  ${fqsrcdir}/mpn/generic/dcpi1_bdiv_q.c
  ${fqsrcdir}/mpn/generic/dcpi1_bdiv_qr.c
  ${fqsrcdir}/mpn/generic/dcpi1_div_q.c
  ${fqsrcdir}/mpn/generic/dcpi1_div_qr.c
  ${fqsrcdir}/mpn/generic/dcpi1_divappr_q.c
  ${fqsrcdir}/mpn/generic/div_q.c
  ${fqsrcdir}/mpn/generic/div_qr_2n_pi1.c
  ${fqsrcdir}/mpn/generic/dive_1.c
  ${fqsrcdir}/mpn/generic/divexact.c
  ${fqsrcdir}/mpn/generic/divrem.c
  ${fqsrcdir}/mpn/generic/divrem_1.c
  ${fqsrcdir}/mpn/generic/divrem_2.c
  ${fqsrcdir}/mpn/generic/gcd_subdiv_step.c
  ${fqsrcdir}/mpn/generic/gcdext.c
  ${fqsrcdir}/mpn/generic/gcdext_1.c
  ${fqsrcdir}/mpn/generic/gcdext_lehmer.c
  ${fqsrcdir}/mpn/generic/get_str.c
  ${fqsrcdir}/mpn/generic/hgcd.c
  ${fqsrcdir}/mpn/generic/hgcd2.c
  ${fqsrcdir}/mpn/generic/hgcd_appr.c
  ${fqsrcdir}/mpn/generic/hgcd_matrix.c
  ${fqsrcdir}/mpn/generic/hgcd_reduce.c
  ${fqsrcdir}/mpn/generic/hgcd_step.c
  ${fqsrcdir}/mpn/generic/invertappr.c
  ${fqsrcdir}/mpn/generic/lshift.c
  ${fqsrcdir}/mpn/generic/lshiftc.c
  ${fqsrcdir}/mpn/generic/matrix22_mul.c
  ${fqsrcdir}/mpn/generic/matrix22_mul1_inverse_vector.c
  ${fqsrcdir}/mpn/generic/mu_bdiv_q.c
  ${fqsrcdir}/mpn/generic/mu_div_q.c
  ${fqsrcdir}/mpn/generic/mu_div_qr.c
  ${fqsrcdir}/mpn/generic/mu_divappr_q.c
  ${fqsrcdir}/mpn/generic/mul.c
  ${fqsrcdir}/mpn/generic/mul_1.c
  ${fqsrcdir}/mpn/generic/mul_basecase.c
  ${fqsrcdir}/mpn/generic/mul_fft.c
  ${fqsrcdir}/mpn/generic/mul_n.c
  ${fqsrcdir}/mpn/generic/mullo_basecase.c
  ${fqsrcdir}/mpn/generic/mullo_n.c
  ${fqsrcdir}/mpn/generic/mulmod_bnm1.c
  ${fqsrcdir}/mpn/generic/neg.c
  ${fqsrcdir}/mpn/generic/nussbaumer_mul.c
  ${fqsrcdir}/mpn/generic/powlo.c
  ${fqsrcdir}/mpn/generic/powm.c
  ${fqsrcdir}/mpn/generic/pre_divrem_1.c
  ${fqsrcdir}/mpn/generic/redc_1.c
  ${fqsrcdir}/mpn/generic/redc_n.c
  ${fqsrcdir}/mpn/generic/rshift.c
  ${fqsrcdir}/mpn/generic/sbpi1_bdiv_q.c
  ${fqsrcdir}/mpn/generic/sbpi1_bdiv_qr.c
  ${fqsrcdir}/mpn/generic/sbpi1_div_q.c
  ${fqsrcdir}/mpn/generic/sbpi1_div_qr.c
  ${fqsrcdir}/mpn/generic/sbpi1_divappr_q.c
  ${fqsrcdir}/mpn/generic/scan1.c
  ${fqsrcdir}/mpn/generic/set_str.c
  ${fqsrcdir}/mpn/generic/sqr.c
  ${fqsrcdir}/mpn/generic/sqr_basecase.c
  ${fqsrcdir}/mpn/generic/sqrlo.c
  ${fqsrcdir}/mpn/generic/sqrlo_basecase.c
  ${fqsrcdir}/mpn/generic/sqrmod_bnm1.c
  ${fqsrcdir}/mpn/generic/sqrtrem.c
  ${fqsrcdir}/mpn/generic/sub.c
  ${fqsrcdir}/mpn/generic/sub_1.c
  ${fqsrcdir}/mpn/generic/sub_n.c
  ${fqsrcdir}/mpn/generic/submul_1.c
  ${fqsrcdir}/mpn/generic/tdiv_qr.c
  ${fqsrcdir}/mpn/generic/toom22_mul.c
  ${fqsrcdir}/mpn/generic/toom2_sqr.c
  ${fqsrcdir}/mpn/generic/toom32_mul.c
  ${fqsrcdir}/mpn/generic/toom33_mul.c
  ${fqsrcdir}/mpn/generic/toom3_sqr.c
  ${fqsrcdir}/mpn/generic/toom42_mul.c
  ${fqsrcdir}/mpn/generic/toom42_mulmid.c
  ${fqsrcdir}/mpn/generic/toom43_mul.c
  ${fqsrcdir}/mpn/generic/toom44_mul.c
  ${fqsrcdir}/mpn/generic/toom4_sqr.c
  ${fqsrcdir}/mpn/generic/toom53_mul.c
  ${fqsrcdir}/mpn/generic/toom63_mul.c
  ${fqsrcdir}/mpn/generic/toom6_sqr.c
  ${fqsrcdir}/mpn/generic/toom6h_mul.c
  ${fqsrcdir}/mpn/generic/toom8_sqr.c
  ${fqsrcdir}/mpn/generic/toom8h_mul.c
  ${fqsrcdir}/mpn/generic/toom_couple_handling.c
  ${fqsrcdir}/mpn/generic/toom_eval_dgr3_pm1.c
  ${fqsrcdir}/mpn/generic/toom_eval_dgr3_pm2.c
  ${fqsrcdir}/mpn/generic/toom_eval_pm1.c
  ${fqsrcdir}/mpn/generic/toom_eval_pm2.c
  ${fqsrcdir}/mpn/generic/toom_eval_pm2exp.c
  ${fqsrcdir}/mpn/generic/toom_eval_pm2rexp.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_12pts.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_16pts.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_5pts.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_6pts.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_7pts.c
  ${fqsrcdir}/mpn/generic/toom_interpolate_8pts.c
  ${fqsrcdir}/mpn/generic/zero_p.c
)

set(mpz_sources
  ${fqsrcdir}/mpz/abs.c
  ${fqsrcdir}/mpz/add.c
  ${fqsrcdir}/mpz/add_ui.c
  ${fqsrcdir}/mpz/aorsmul.c
  ${fqsrcdir}/mpz/aorsmul_i.c
  ${fqsrcdir}/mpz/cdiv_q_ui.c
  ${fqsrcdir}/mpz/cfdiv_q_2exp.c
  ${fqsrcdir}/mpz/clear.c
  ${fqsrcdir}/mpz/cmp.c
  ${fqsrcdir}/mpz/cmp_ui.c
  ${fqsrcdir}/mpz/cmpabs.c
  ${fqsrcdir}/mpz/divexact.c
  ${fqsrcdir}/mpz/fdiv_q.c
  ${fqsrcdir}/mpz/fdiv_q_ui.c
  ${fqsrcdir}/mpz/fdiv_qr.c
  ${fqsrcdir}/mpz/fits_slong.c
  ${fqsrcdir}/mpz/gcdext.c
  ${fqsrcdir}/mpz/get_si.c
  ${fqsrcdir}/mpz/get_ui.c
  ${fqsrcdir}/mpz/init.c
  ${fqsrcdir}/mpz/init2.c
  ${fqsrcdir}/mpz/invert.c
  ${fqsrcdir}/mpz/iset.c
  ${fqsrcdir}/mpz/iset_ui.c
  ${fqsrcdir}/mpz/mod.c
  ${fqsrcdir}/mpz/mul.c
  ${fqsrcdir}/mpz/mul_2exp.c
  ${fqsrcdir}/mpz/mul_si.c
  ${fqsrcdir}/mpz/mul_ui.c
  ${fqsrcdir}/mpz/n_pow_ui.c
  ${fqsrcdir}/mpz/neg.c
  ${fqsrcdir}/mpz/pow_ui.c
  ${fqsrcdir}/mpz/powm.c
  ${fqsrcdir}/mpz/powm_ui.c
  ${fqsrcdir}/mpz/realloc.c
  ${fqsrcdir}/mpz/realloc2.c
  ${fqsrcdir}/mpz/scan0.c
  ${fqsrcdir}/mpz/scan1.c
  ${fqsrcdir}/mpz/set.c
  ${fqsrcdir}/mpz/size.c
  ${fqsrcdir}/mpz/set_ui.c
  ${fqsrcdir}/mpz/sizeinbase.c
  ${fqsrcdir}/mpz/sqrt.c
  ${fqsrcdir}/mpz/sub.c
  ${fqsrcdir}/mpz/sub_ui.c
  ${fqsrcdir}/mpz/swap.c
  ${fqsrcdir}/mpz/tdiv_q.c
  ${fqsrcdir}/mpz/tdiv_q_2exp.c
  ${fqsrcdir}/mpz/tdiv_qr.c
  ${fqsrcdir}/mpz/tdiv_r.c
  ${fqsrcdir}/mpz/tdiv_r_2exp.c
  ${fqsrcdir}/mpz/tstbit.c
  ${fqsrcdir}/mpz/ui_pow_ui.c
)

set(gmp_sources
  ${basic_sources}
  ${mpn_sources}
  ${mpz_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/fac_table.h
  ${CMAKE_CURRENT_BINARY_DIR}/fib_table.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmp.h
  ${CMAKE_CURRENT_BINARY_DIR}/mp_bases.h
)

if(gmp_complete AND NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)

add_subdirectory(test)
