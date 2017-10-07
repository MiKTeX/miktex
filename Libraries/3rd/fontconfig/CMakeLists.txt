## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_FONT_UTILITIES_FOLDER}/fontconfig")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  source/src
  source
)

add_definitions(
  -DHAVE_CONFIG_H
  -DDONT_USE_MIKTEX_EXIT
  -D_UNICODE -DUNICODE
)

set(fqsrcdir ${CMAKE_CURRENT_SOURCE_DIR}/source)

set(public_include_directories
  ${fqsrcdir}/fontconfig
  ${fqsrcdir}
)

set(interface_definitions
  -DHAVE_LIBFONTCONFIG
)

ignore_warnings()

###############################################################################
## fc-case

add_executable(fc-case source/fc-case/fc-case.c)

set_property(TARGET fc-case PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-case asInvoker)
merge_compatibility_manifest(fc-case)

target_link_libraries(fc-case
  ${app_dll_name}
  $(core_dll_name)
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

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

add_executable(fc-lang ${fc-lang_sources})

set_property(TARGET fc-lang PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(fc-lang gen-fccase-header)

merge_trustinfo_manifest(fc-lang asInvoker)
merge_compatibility_manifest(fc-lang)

target_link_libraries(fc-lang
  ${app_dll_name}
  $(core_dll_name)
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

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
## fc-glyphname

add_executable(fc-glyphname source/fc-glyphname/fc-glyphname.c)

set_property(TARGET fc-glyphname PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-glyphname asInvoker)
merge_compatibility_manifest(fc-glyphname)

target_link_libraries(fc-glyphname
  ${app_dll_name}
  $(core_dll_name)
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fcglyphname.h
  COMMAND $<TARGET_FILE:fc-glyphname> ${fqsrcdir}/fc-glyphname/zapfdingbats.txt
        < ${fqsrcdir}/fc-glyphname/fcglyphname.tmpl.h
        > ${CMAKE_CURRENT_BINARY_DIR}/fcglyphname.h
  MAIN_DEPENDENCY source/fc-glyphname/zapfdingbats.txt
  DEPENDS
        fc-glyphname
        ${fqsrcdir}/fc-glyphname/fcglyphname.tmpl.h
  VERBATIM
)

add_custom_target(gen-fcglyphname-header ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/fcglyphname.h
)

set_property(TARGET gen-fcglyphname-header PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

###############################################################################
## lib

set(${fontconfig_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/fccase.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcglyphname.h
  ${CMAKE_CURRENT_BINARY_DIR}/fclang.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
  source/src/fcatomic.c
  source/src/fcatomic.h
  source/src/fcblanks.c
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
  fontconfig-version.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/fc-case/fccase.h
  ${CMAKE_CURRENT_BINARY_DIR}/fcglyphname.h
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
  set(${fontconfig_dll_name}_sources ${${fontconfig_dll_name}_sources}
    fontconfig.def
    ${CMAKE_CURRENT_BINARY_DIR}/fontconfig.rc
  )
endif()

add_library(${fontconfig_dll_name} SHARED ${${fontconfig_dll_name}_sources})

set_property(TARGET ${fontconfig_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${fontconfig_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
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
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
)

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

rebase(${fontconfig_dll_name})

add_dependencies(${fontconfig_dll_name}
  gen-fccase-header
  gen-fcglyphname-header
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
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

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
  set(fc-cache_sources ${fc-cache_sources}
    fc-cache.rc
  )
endif()

add_executable(fc-cache ${fc-cache_sources})

set_property(TARGET fc-cache PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-cache asInvoker)
merge_compatibility_manifest(fc-cache)

target_link_libraries(fc-cache
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-cache DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

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
  set(fc-cat_sources ${fc-cat_sources}
    fc-cat.rc
  )
endif()

add_executable(fc-cat ${fc-cat_sources})

set_property(TARGET fc-cat PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-cat asInvoker)
merge_compatibility_manifest(fc-cat)

target_link_libraries(fc-cat
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-cat DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-list

set(fc-list_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-list-version.h
  source/fc-list/fc-list.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-list_sources ${fc-list_sources}
    fc-list.rc
  )
endif()

add_executable(fc-list ${fc-list_sources})

set_property(TARGET fc-list PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-list asInvoker)
merge_compatibility_manifest(fc-list)

target_link_libraries(fc-list
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-list DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-match

set(fc-match_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-match-version.h
  source/fc-match/fc-match.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-match_sources ${fc-match_sources}
    fc-match.rc
  )
endif()

add_executable(fc-match ${fc-match_sources})

set_property(TARGET fc-match PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-match asInvoker)
merge_compatibility_manifest(fc-match)

target_link_libraries(fc-match
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-match DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-pattern

set(fc-pattern_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-pattern-version.h
  source/fc-pattern/fc-pattern.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-pattern_sources ${fc-pattern_sources}
    fc-pattern.rc
  )
endif()

add_executable(fc-pattern ${fc-pattern_sources})

set_property(TARGET fc-pattern PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-pattern asInvoker)
merge_compatibility_manifest(fc-pattern)

target_link_libraries(fc-pattern
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-pattern DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-query

set(fc-query_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-query-version.h
  source/fc-query/fc-query.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-query_sources ${fc-query_sources}
    fc-query.rc
  )
endif()

add_executable(fc-query ${fc-query_sources})

set_property(TARGET fc-query PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-query asInvoker)
merge_compatibility_manifest(fc-query)

target_link_libraries(fc-query
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-query DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-scan

set(fc-scan_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-scan-version.h
  source/fc-scan/fc-scan.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-scan_sources ${fc-scan_sources}
    fc-scan.rc
  )
endif()

add_executable(fc-scan ${fc-scan_sources})

set_property(TARGET fc-scan PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-scan asInvoker)
merge_compatibility_manifest(fc-scan)

target_link_libraries(fc-scan
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-scan DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

###############################################################################
## fc-validate

set(fc-validate_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  fc-validate-version.h
  source/fc-validate/fc-validate.c
)

if(MIKTEX_NATIVE_WINDOWS)
  set(fc-validate_sources ${fc-validate_sources}
    fc-validate.rc
  )
endif()

add_executable(fc-validate ${fc-validate_sources})

set_property(TARGET fc-validate PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(fc-validate asInvoker)
merge_compatibility_manifest(fc-validate)

target_link_libraries(fc-validate
  ${app_dll_name}
  ${core_dll_name}
  ${fontconfig_dll_name}
  ${getopt_dll_name}
  ${unxemu_dll_name}
  ${utf8wrap_dll_name}
)

install(TARGETS fc-validate DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
