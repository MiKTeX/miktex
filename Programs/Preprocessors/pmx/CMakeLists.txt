## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/pmx")

add_definitions(
  -DNON_UNIX_STDIO
  -DMSDOS
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c
)

## libf2c headers
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/f2c.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/fio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/fmt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/fp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/lio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rawio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/signal1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/sysdep1.h
)

## MISC
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/abort_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/cabs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/derf_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/derfc_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/erf_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/erfc_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/exit_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/f77vers.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/getarg_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/getenv_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i77vers.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/iargc_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_paus.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_rnge.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_stop.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/sig_die.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/signal_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/system_.c
)

## MISC (not used for pmx)
list(APPEND libf2c_a_extra
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/uninit.c
)

## POW
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_ci.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_dd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_di.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_hh.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_ii.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_ri.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_zi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_zz.c
)

## CX
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_div.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_log.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_sin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/c_sqrt.c
)

## DCX
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_div.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_log.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_sin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/z_sqrt.c
)

## REAL
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_acos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_asin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_atan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_atn2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_cnjg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_cosh.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_dim.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_imag.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_int.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_lg10.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_log.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_mod.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_nint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_sign.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_sin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_sinh.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_sqrt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_tan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/r_tanh.c
)

## DBL
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_acos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_asin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_atan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_atn2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_cnjg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_cos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_cosh.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_dim.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_exp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_imag.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_int.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_lg10.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_log.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_mod.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_nint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_prod.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_sign.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_sin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_sinh.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_sqrt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_tan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/d_tanh.c
)

## INT
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_dim.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_dnnt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_indx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_len.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_mod.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_nint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/i_sign.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/lbitbits.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/lbitshft.c
)

## HALF
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_abs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_dim.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_dnnt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_indx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_len.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_mod.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_nint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/h_sign.c
)

## CMP
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/hl_ge.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/hl_gt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/hl_le.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/hl_lt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/l_ge.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/l_gt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/l_le.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/l_lt.c
)

## EFL
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/ef1asc_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/ef1cmc_.c
)

## CHAR
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/f77_aloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_cat.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_cmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/s_copy.c
)

## I77
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/backspac.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/close.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/dfe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/dolio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/due.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/endfile.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/err.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/fmt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/fmtlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/ftell_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/iio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/ilnw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/inquire.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/lread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/lwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/open.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rdfmt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rewind.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rsfe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rsli.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/rsne.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/sfe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/sue.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/typesize.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/uio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/wref.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/wrtfmt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/wsfe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/wsle.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/wsne.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/xwsne.c
)

## TIME
list(APPEND libf2c_a_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/dtime_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/etime_.c
)

## Handle DOS line endings
if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libf2c_a_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/w32getc.c
  )
endif()

## QINT (unused)
list(APPEND libf2c_a_extra
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/ftell64_.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/pow_qq.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/qbitbits.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/qbitshft.c
)

## unused
list(APPEND libf2c_a_extra
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libf2c/signbit.c
)

add_library(f2c STATIC ${libf2c_a_sources})

set(programs
  pmxab
  scor2prt
)

foreach(p ${programs})
  set(${p}_sources
    pmx-version.h
    source/${p}.c
  )
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${p}_sources
      ${p}.rc
    )
  endif()
  add_executable(${p} ${${p}_sources})
  set_property(TARGET ${p} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  merge_trustinfo_manifest(${p} asInvoker)
  merge_compatibility_manifest(${p})
  target_link_libraries(${p}
    f2c
  )
  install(TARGETS ${p} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endforeach()
