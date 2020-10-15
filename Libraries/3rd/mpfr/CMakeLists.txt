## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/mpfr")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(HAVE_STDARG 1)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  @ONLY
)

configure_file(
  source/src/mparam_h.in
  ${CMAKE_CURRENT_BINARY_DIR}/mparam.h
  @ONLY
)

configure_file(
  mpfr-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/mpfr-version.h
)

set(mpfr_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/abort_prec_max.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add1sp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_d.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/agm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atan2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cache.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/clear.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/clears.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmpabs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp_si.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/comparisons.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/const_catalan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/const_euler.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/const_log2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/const_pi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/constant.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_2si.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_2ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exceptions.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp3.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp_2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/extract.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fits_sint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fits_slong.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fits_ulong.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/free_cache.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_d.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_si.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_str.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_z.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_z_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/gmp_op.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/init.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/init2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/inits2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/int_ceil_log2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/isinteger.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/isnum.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/isqrt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/min_prec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mpfr-gmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mpn_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mp_clz_tab.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_2si.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_2ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mulders.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/nbits_ulong.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/neg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/next.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/pool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/powerof2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/rem1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/rint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/round_near_x.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/round_p.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/round_prec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/scale2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_si.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_d.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_dfl_prec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_f.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_prec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_q.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_rnd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_si_2exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_str.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_ui_2exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_z.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_z_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_zero.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/setmax.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/setmin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sgn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sin_cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/si_op.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sqr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sqrt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sqrt_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/strtofr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub1sp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_ui.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/swap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ubf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ui_div.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ui_sub.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/version.c
)

set(public_headers
  mpfr.h
  mpf2mpfr.h
)

foreach(hdr ${public_headers})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${hdr}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${hdr}
    COPYONLY
  )
endforeach()

add_subdirectory(test)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
