## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2021 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/fontconfig")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  source/src
  source
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -DHAVE_CONFIG_H
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set(fqsrcdir ${CMAKE_CURRENT_SOURCE_DIR}/source)

set(public_include_directories
  ${fqsrcdir}/fontconfig
  ${fqsrcdir}
)

set(interface_definitions
  -DHAVE_LIBFONTCONFIG
)

###############################################################################
## fc-case

set(fc-case_sources source/fc-case/fc-case.c)

if (MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-case_sources
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

add_executable(fc-case ${fc-case_sources})

set_property(TARGET fc-case PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(fc-case
  ${app_dll_name}
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(fc-case
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fccase.h
  COMMAND $<TARGET_FILE:fc-case> ${fqsrcdir}/fc-case/CaseFolding.txt
        < ${fqsrcdir}/fc-case/fccase.tmpl.h
        > ${CMAKE_CURRENT_BINARY_DIR}/fccase.h
  MAIN_DEPENDENCY source/fc-case/CaseFolding.txt
  DEPENDS
        fc-case
        source/fc-case/fccase.tmpl.h
  VERBATIM
)

add_custom_target(gen-fccase-header ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/fccase.h
)

set_property(TARGET gen-fccase-header PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

###############################################################################
## fc-lang

set(orth_files
  aa.orth
  ab.orth
  af.orth
  ak.orth
  am.orth
  an.orth
  ar.orth
  as.orth
  ast.orth
  av.orth
  ay.orth
  az_az.orth
  az_ir.orth
  ba.orth
  be.orth
  ber_dz.orth
  ber_ma.orth
  bg.orth
  bh.orth
  bho.orth
  bi.orth
  bin.orth
  bm.orth
  bn.orth
  bo.orth
  br.orth
  brx.orth
  bs.orth
  bua.orth
  byn.orth
  ca.orth
  ce.orth
  ch.orth
  chm.orth
  chr.orth
  co.orth
  crh.orth
  cs.orth
  csb.orth
  cu.orth
  cv.orth
  cy.orth
  da.orth
  de.orth
  doi.orth
  dv.orth
  dz.orth
  ee.orth
  el.orth
  en.orth
  eo.orth
  es.orth
  et.orth
  eu.orth
  fa.orth
  fat.orth
  ff.orth
  fi.orth
  fil.orth
  fj.orth
  fo.orth
  fr.orth
  fur.orth
  fy.orth
  ga.orth
  gd.orth
  gez.orth
  gl.orth
  gn.orth
  gu.orth
  gv.orth
  ha.orth
  haw.orth
  he.orth
  hi.orth
  hne.orth
  ho.orth
  hr.orth
  hsb.orth
  ht.orth
  hu.orth
  hy.orth
  hz.orth
  ia.orth
  id.orth
  ie.orth
  ig.orth
  ii.orth
  ik.orth
  io.orth
  is.orth
  it.orth
  iu.orth
  ja.orth
  jv.orth
  ka.orth
  kaa.orth
  kab.orth
  ki.orth
  kj.orth
  kk.orth
  kl.orth
  km.orth
  kn.orth
  ko.orth
  kok.orth
  kr.orth
  ks.orth
  ku_am.orth
  ku_iq.orth
  ku_ir.orth
  ku_tr.orth
  kum.orth
  kv.orth
  kw.orth
  kwm.orth
  ky.orth
  la.orth
  lah.orth
  lb.orth
  lez.orth
  lg.orth
  li.orth
  ln.orth
  lo.orth
  lt.orth
  lv.orth
  mai.orth
  mg.orth
  mh.orth
  mi.orth
  mk.orth
  ml.orth
  mn_cn.orth
  mn_mn.orth
  mni.orth
  mo.orth
  mr.orth
  ms.orth
  mt.orth
  my.orth
  na.orth
  nb.orth
  nds.orth
  ne.orth
  ng.orth
  nl.orth
  nn.orth
  no.orth
  nqo.orth
  nr.orth
  nso.orth
  nv.orth
  ny.orth
  oc.orth
  om.orth
  or.orth
  os.orth
  ota.orth
  pa.orth
  pa_pk.orth
  pap_an.orth
  pap_aw.orth
  pl.orth
  ps_af.orth
  ps_pk.orth
  pt.orth
  qu.orth
  quz.orth
  rm.orth
  rn.orth
  ro.orth
  ru.orth
  rw.orth
  sa.orth
  sah.orth
  sat.orth
  sc.orth
  sco.orth
  sd.orth
  se.orth
  sel.orth
  sg.orth
  sh.orth
  shs.orth
  si.orth
  sid.orth
  sk.orth
  sl.orth
  sm.orth
  sma.orth
  smj.orth
  smn.orth
  sms.orth
  sn.orth
  so.orth
  sq.orth
  sr.orth
  ss.orth
  st.orth
  su.orth
  sv.orth
  sw.orth
  syr.orth
  ta.orth
  te.orth
  tg.orth
  th.orth
  ti_er.orth
  ti_et.orth
  tig.orth
  tk.orth
  tl.orth
  tn.orth
  to.orth
  tr.orth
  ts.orth
  tt.orth
  tw.orth
  ty.orth
  tyv.orth
  ug.orth
  uk.orth
  ur.orth
  uz.orth
  ve.orth
  vi.orth
  vo.orth
  vot.orth
  wa.orth
  wal.orth
  wen.orth
  wo.orth
  xh.orth
  yap.orth
  yi.orth
  yo.orth
  za.orth
  zh_cn.orth
  zh_hk.orth
  zh_mo.orth
  zh_sg.orth
  zh_tw.orth
  zu.orth
  und_zmth.orth
  und_zsye.orth
)

set(fc-lang_sources
  ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  source/fc-lang/fc-lang.c
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  PROPERTIES GENERATED TRUE
)

if (MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-lang_sources
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

add_executable(fc-lang ${fc-lang_sources})

set_property(TARGET fc-lang PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(fc-lang gen-fccase-header)

target_link_libraries(fc-lang
  ${app_dll_name}
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(fc-lang
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
  COMMAND $<TARGET_FILE:fc-lang> -d ${fqsrcdir}/fc-lang ${orth_files}
        < ${fqsrcdir}/fc-lang/fclang.tmpl.h
        > ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
  MAIN_DEPENDENCY source/fc-lang/fclang.tmpl.h
  DEPENDS
        fc-lang
#       ${orth_files}
        source/fc-lang/fclang.tmpl.h
  VERBATIM
)

add_custom_target(gen-fclang-header ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
)

set_property(TARGET gen-fclang-header PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

###############################################################################
## lib

configure_file(
  fontconfig-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/fontconfig-version.h
)

set(${fontconfig_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  ${CMAKE_CURRENT_BINARY_DIR}/fccase.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
  ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
  ${CMAKE_CURRENT_BINARY_DIR}/fontconfig-version.h

  source/src/fcatomic.c
  source/src/fcatomic.h
  source/src/fccache.c
  source/src/fccfg.c
  source/src/fccharset.c
  source/src/fccompat.c
  source/src/fcdbg.c
  source/src/fcdefault.c
  source/src/fcdeprecate.h
  source/src/fcdir.c
  source/src/fcformat.c
  source/src/fcfreetype.c
  source/src/fcfs.c
  source/src/fcftint.h
  source/src/fchash.c
  source/src/fcinit.c
  source/src/fcint.h
  source/src/fclang.c
  source/src/fclist.c
  source/src/fcmatch.c
  source/src/fcmatrix.c
  source/src/fcmutex.h
  source/src/fcname.c
  source/src/fcobjs.c
  source/src/fcobjs.h
  source/src/fcobjshash.h
  source/src/fcpat.c
  source/src/fcptrlist.c
  source/src/fcrange.c
  source/src/fcserialize.c
  source/src/fcstat.c
  source/src/fcstdint.h
  source/src/fcstr.c
  source/src/fcweight.c
  source/src/fcwindows.h
  source/src/fcxml.c
  source/src/ftglue.c
  source/src/ftglue.h

  fcmiktex.cpp
  fcmiktex.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/fc-case/fccase.h
  ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
  PROPERTIES GENERATED TRUE
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    fontconfig.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/fontconfig.rc
  )
  list(APPEND ${fontconfig_dll_name}_sources
    fontconfig.def
    ${CMAKE_CURRENT_BINARY_DIR}/fontconfig.rc
  )
endif()

add_library(${fontconfig_dll_name} SHARED ${${fontconfig_dll_name}_sources})

prevent_all_warnings_on_bad_code_target(${fontconfig_dll_name})

set_property(TARGET ${fontconfig_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${fontconfig_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${fontconfig_dll_name}
  PRIVATE
    -DDLL_EXPORT
  INTERFACE
    ${interface_definitions}
)

target_include_directories(${fontconfig_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${fontconfig_dll_name}
  PRIVATE
    ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${fontconfig_dll_name}
    PRIVATE
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(USE_SYSTEM_EXPAT)
  target_link_libraries(${fontconfig_dll_name} PRIVATE MiKTeX::Imported::EXPAT)
else()
  target_link_libraries(${fontconfig_dll_name} PRIVATE ${expat_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${fontconfig_dll_name} PUBLIC MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${fontconfig_dll_name} PUBLIC ${freetype2_dll_name})
endif()

add_dependencies(${fontconfig_dll_name}
  gen-fccase-header
  gen-fclang-header
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
  VERBATIM
)

add_executable(fcarch source/src/fcarch.c)

set_property(TARGET fcarch PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(fcarch
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(fcarch
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${fontconfig_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

###############################################################################
## fc-cache

set(fc-cache_sources
  ${CMAKE_CURRENT_BINARY_DIR}/fcarch.h
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-cache-version.h
  source/fc-cache/fc-cache.c
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/fc-arch/fcarch.h
  PROPERTIES GENERATED TRUE
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-cache_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-cache.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-cache ${fc-cache_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-cache PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-cache
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-cache
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-cache DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-cat

set(fc-cat_sources
  ${CMAKE_CURRENT_BINARY_DIR}/fcarch.h
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-cat-version.h
  source/fc-cat/fc-cat.c
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/fcarch.h
  PROPERTIES GENERATED TRUE
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-cat_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-cat.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-cat ${fc-cat_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-cat PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-cat
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-cat
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-cat DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-conflist

set(fc-conflist_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-conflist-version.h
  source/fc-conflist/fc-conflist.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-conflist_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-conflist.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-conflist ${fc-conflist_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-conflist PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-conflist
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-conflist
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-conflist DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-list

set(fc-list_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-list-version.h
  source/fc-list/fc-list.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-list_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-list.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-list ${fc-list_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-list PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-list
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-list
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()
  
install(TARGETS ${MIKTEX_PREFIX}fc-list DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-match

set(fc-match_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-match-version.h
  source/fc-match/fc-match.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-match_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-match.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-match ${fc-match_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-match PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-match
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-match
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-match DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-pattern

set(fc-pattern_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-pattern-version.h
  source/fc-pattern/fc-pattern.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-pattern_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-pattern.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-pattern ${fc-pattern_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-pattern PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-pattern
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-pattern
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-pattern DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-query

set(fc-query_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-query-version.h
  source/fc-query/fc-query.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-query_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-query.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-query ${fc-query_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-query PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-query
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-query
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-query DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-scan

set(fc-scan_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-scan-version.h
  source/fc-scan/fc-scan.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-scan_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-scan.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-scan ${fc-scan_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-scan PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-scan
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-scan
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-scan DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-validate

set(fc-validate_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-validate-version.h
  source/fc-validate/fc-validate.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fc-validate_sources
    ${MIKTEX_COMMON_MANIFEST}
    fc-validate.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fc-validate ${fc-validate_sources})

set_property(TARGET ${MIKTEX_PREFIX}fc-validate PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}fc-validate
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}fc-validate
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}fc-validate DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
