## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_GRAPHICS_UTILITIES_FOLDER}/asymptote")

add_definitions(
  -DFFTWPP_SINGLE_THREAD
  -DGLEW_STATIC
)

add_definitions(
  -DCPLUSPLUSMAIN
  -Dmain=Main
)

if(MSVC)
  add_definitions(
    -DNOMINMAX
    -DUNICODE
    -D_UNICODE
  )
endif()  

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/symbol-headers
)

set(use_gc FALSE)

if(use_gc)
  add_definitions(
    -DUSEGC
  )
endif()

if(MIKTEX_NATIVE_WINDOWS AND FALSE)
  add_definitions(
    -D__MSDOS__
  )
endif()

#set(HAVE_LIBFFTW3 TRUE)
set(HAVE_GNU_GETOPT_H TRUE)
set(HAVE_LIBGL TRUE)
set(HAVE_LIBGLM TRUE)
set(HAVE_LIBGLUT TRUE)
set(HAVE_TGAMMA TRUE)
set(HAVE_UNORDERED_MAP TRUE)
set(PACKAGE_BUGREPORT "")
set(PACKAGE_NAME "miktex-asy")
set(PACKAGE_VERSION "${MIKTEX_COMP_ORIG_VERSION_STR}")
set(RETSIGTYPE void)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  asy-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/asy-version.h
)

configure_file(
  asy.rc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/asy.rc
)

set(camp_sources
  source/beziercurve.cc
  source/bezierpatch.cc
  source/camperror.cc
  source/drawfill.cc
  source/drawlabel.cc
  source/drawpath.cc
  source/drawpath3.cc
  source/drawsurface.cc
  source/flatguide.cc
  source/guide.cc
  source/knot.cc
  source/path.cc
  source/path3.cc
  source/pen.cc
  source/picture.cc
  source/pipestream.cc
  source/psfile.cc
  source/settings.cc
  source/texfile.cc
  source/util.cc
)

set(runtime_source_files
  source/runarray.cc
  source/runbacktrace.cc
  source/runfile.cc
  source/runhistory.cc
  source/runlabel.cc
  source/runmath.cc
  source/runpair.cc
  source/runpath.cc
  source/runpath3d.cc
  source/runpicture.cc
  source/runstring.cc
  source/runsystem.cc
  source/runtime.cc
  source/runtriple.cc
)

set(symbol_source_files
  ${runtime_source_files}
  source/builtin.cc
  source/gsl.cc
  source/types.cc
)

set(prc_sources
  source/prc/PRCbitStream.cc
  source/prc/PRCdouble.cc
  source/prc/oPRCFile.cc
  source/prc/writePRC.cc
)

set(core_sources
  ${camp_sources}
  ${prc_sources}
  ${symbol_source_files}
  source/Delaunay.cc
  source/absyn.cc
  source/access.cc
  source/application.cc
  source/array.cc
  source/callable.cc
  source/camp.tab.cc
  source/coder.cc
  source/coenv.cc
  source/constructor.cc
  source/dec.cc
  source/entry.cc
  source/env.cc
  source/envcompleter.cc
  source/errormsg.cc
  source/exp.cc
  source/fftw++asy.cc
  source/fileio.cc
  source/fundec.cc
  source/genv.cc
  source/glrender.cc
  source/impdatum.cc
  source/interact.cc
  source/jsfile.cc
  source/lex.yy.cc
  source/locate.cc
  source/name.cc
  source/newexp.cc
  source/parser.cc
  source/predicates.cc
  source/process.cc
  source/program.cc
  source/record.cc
  source/refaccess.cc
  source/shaders.cc
  source/simpson.cc
  source/stack.cc
  source/stm.cc
  source/symbol.cc
  source/tr.cc
  source/varinit.cc
  source/virtualfieldaccess.cc
)

set(core_headers
  source/Delaunay.h
  source/absyn.h
  source/access.h
  source/align.h
  source/angle.h
  source/application.h
  source/array.h
  source/arrayop.h
  source/bbox.h
  source/bbox3.h
  source/beziercurve.h
  source/bezierpatch.h
  source/builtin.h
  source/callable.h
  source/camp.tab.h
  source/camperror.h
  source/castop.h
  source/coder.h
  source/coenv.h
  source/common.h
  source/dec.h
  source/drawclipbegin.h
  source/drawclipend.h
  source/drawelement.h
  source/drawfill.h
  source/drawgrestore.h
  source/drawgroup.h
  source/drawgsave.h
  source/drawimage.h
  source/drawlabel.h
  source/drawlayer.h
  source/drawpath.h
  source/drawpath3.h
  source/drawsurface.h
  source/drawverbatim.h
  source/entry.h
  source/env.h
  source/envcompleter.h
  source/errormsg.h
  source/exp.h
  source/fftw++.h
  source/fileio.h
  source/flatguide.h
  source/fpu.h
  source/frame.h
  source/fundec.h
  source/genv.h
  source/glrender.h
  source/guide.h
  source/guideflags.h
  source/inst.h
  source/interact.h
  source/item.h
  source/jsfile.h
  source/knot.h
  source/lexical.h
  source/locate.h
  source/material.h
  source/mathop.h
  source/memory.h
  source/mod.h
  source/modifier.h
  source/name.h
  source/newexp.h
  source/opcodes.h
  source/pair.h
  source/parser.h
  source/path.h
  source/path3.h
  source/pen.h
  source/picture.h
  source/pipestream.h
  source/policy.h
  source/prcfile.h
  source/predicates.h
  source/primitives.h
  source/process.h
  source/profiler.h
  source/program.h
  source/psfile.h
  source/record.h
  source/refaccess.h
  source/rounding.h
  source/runarray.h
  source/runbacktrace.h
  source/runfile.h
  source/runhistory.h
  source/runlabel.h
  source/runmath.h
  source/runpair.h
  source/runpath.h
  source/runpath3d.h
  source/runpicture.h
  source/runstring.h
  source/runsystem.h
  source/runtime.h
  source/runtriple.h
  source/seconds.h
  source/settings.h
  source/shaders.h
  source/stack.h
  source/statistics.h
  source/stm.h
  source/symbol.h
  source/table.h
  source/texfile.h
  source/tr.h
  source/transform.h
  source/triple.h
  source/types.h
  source/util.h
  source/varinit.h
  source/virtualfieldaccess.h
  source/vm.h
  source/xstream.h
)

set(symbol_headers
  symbol-headers/allsymbols.h
  symbol-headers/builtin.symbols.h
  symbol-headers/gsl.symbols.h
  symbol-headers/opsymbols.h
  symbol-headers/runarray.symbols.h
  symbol-headers/runbacktrace.symbols.h
  symbol-headers/runfile.symbols.h
  symbol-headers/runhistory.symbols.h
  symbol-headers/runlabel.symbols.h
  symbol-headers/runmath.symbols.h
  symbol-headers/runpair.symbols.h
  symbol-headers/runpath.symbols.h
  symbol-headers/runpath3d.symbols.h
  symbol-headers/runpicture.symbols.h
  symbol-headers/runstring.symbols.h
  symbol-headers/runsystem.symbols.h
  symbol-headers/runtime.symbols.h
  symbol-headers/runtriple.symbols.h
  symbol-headers/types.symbols.h
)

set(miktex_sources
  ${CMAKE_CURRENT_BINARY_DIR}/asy-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  miktex/InProcPipe.h
  miktex/PipeStream.cpp
  miktex/PipeStream.h
  miktex/asy-first.h
  miktex/asy.cpp
  miktex/asy.h
)

set(asy_sources
  ${core_headers}
  ${core_sources}
  ${miktex_sources}
  ${symbol_headers}
  source/glew.c
  source/main.cc
  source/revision.cc
)

set_source_files_properties(source/glew.c
  PROPERTIES
    COMPILE_DEFINITIONS "GLEW_NO_GLU"
    INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND asy_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/asy.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}asy ${asy_sources})

set_property(TARGET ${MIKTEX_PREFIX}asy PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}asy
  PUBLIC
    ${app_dll_name}
    ${core_dll_name}
    ${glm_lib_name}
)

if(use_gc)
  target_link_libraries(${MIKTEX_PREFIX}asy
    PUBLIC
      ${gc_lib_name}
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}asy
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(USE_SYSTEM_OPENGL AND MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC opengl32)
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC glu32)
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC ${freeglut_dll_name})
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC ${opengl_registry_dll_name})
else()
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC MiKTeX::Imported::OPENGL)
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC MiKTeX::Imported::GLU)
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${MIKTEX_PREFIX}asy PUBLIC ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}asy
    PUBLIC
      ${getopt_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_link_flags(${MIKTEX_PREFIX}asy "wsetargv.obj")
endif()

install(TARGETS ${MIKTEX_PREFIX}asy DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

source_group(MiKTeX FILES ${miktex_sources})
