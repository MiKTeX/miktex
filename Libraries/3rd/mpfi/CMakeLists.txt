## CMakeLists.txt
##
## Copyright (C) 2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/mpfi")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(HAVE_MPFR_Z_DIV 1)
set(HAVE_MPFR_Z_SUB 1)
set(MPFR_Q_SUB 1)
set(HAVE_STDARG 1)

configure_file(
    mpfi_config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/mpfi_config.h
    @ONLY
)

configure_file(
    miktex-mpfi-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-mpfi-version.h
)

set(mpfi_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/abs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/acos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/acosh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/add_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/alea.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/asin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/asinh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atan2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atanh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/bisect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/blow.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cbrt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/clear.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/clears.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmp_sym_pi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/constants.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cosh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/coth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/csc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/csch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/d_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/d_sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/diam.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_2exp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_2si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_2ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_ext.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/div_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/erandom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp10.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp10m1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/exp2m1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/expm1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fr_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fr_sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_endpoints.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_prec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/get_version.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/has_zero.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hypot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/increase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/init2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/inits.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/inits2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/inp_str.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/intersect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/interv_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/inv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/is_empty.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/is_inside.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log10.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log10p1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log1p.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/log2p1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mig.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mpfi-impl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mpfi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mpfi_io.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_2exp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_2si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_2ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mul_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/neg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/nrandom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/out_str.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/predicates.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/print_binary.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/put_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/q_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/q_sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/quadrant.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/rec_sqrt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/revert_if_needed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/round_prec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sech.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_flt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_ld.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_prec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_str.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/set_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/si_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/si_sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sign.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sinh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sqr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sqrt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_fr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_q.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_si.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/sub_z.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/swap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/tan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/tanh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ui_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ui_sub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/union.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/urandom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/z_div.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/z_sub.c 
)

set(public_headers
    mpfi.h
    mpfi-impl.h
    mpfi_io.h
)

foreach(hdr ${public_headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${hdr}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${hdr}
        COPYONLY
    )
endforeach()

add_subdirectory(static)
