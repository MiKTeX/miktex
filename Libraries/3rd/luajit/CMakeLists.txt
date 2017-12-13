## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/luajit")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dynasm
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/host
)

set(public_include_directories
  ${CMAKE_BINARY_DIR}/include
)

add_definitions(-DLUA_BUILD_AS_DLL -DLUAJIT_ENABLE_LUA52COMPAT)

ignore_warnings()

set(generated_headers
  ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_vmdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
)

set_source_files_properties(
  ${generated_headers}
  PROPERTIES GENERATED TRUE
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj
  PROPERTIES
    EXTERNAL_OBJECT TRUE
    GENERATED TRUE
)

set(minilua_sources
  source/src/host/minilua.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND minilua_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

add_executable(minilua EXCLUDE_FROM_ALL ${minilua_sources})

set_property(TARGET minilua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(dasmflags -D WIN -D JIT -D FFI -D P64)
else()
  set(dasmflags -D WIN -D JIT -D FFI)
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
  COMMAND minilua ${CMAKE_CURRENT_SOURCE_DIR}/source/dynasm/dynasm.lua
        -LN ${dasmflags}
        -o ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/vm_x86.dasc
  DEPENDS
        minilua
        ${CMAKE_CURRENT_SOURCE_DIR}/source/dynasm/dynasm.lua
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/vm_x86.dasc
  VERBATIM
)

set(buildvm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
  source/src/host/buildvm.c
  source/src/host/buildvm_asm.c
  source/src/host/buildvm_fold.c
  source/src/host/buildvm_lib.c
  source/src/host/buildvm_peobj.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND buildvm_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

add_executable(buildvm EXCLUDE_FROM_ALL ${buildvm_sources})

set_property(TARGET buildvm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set(ljlib_c
  ${CMAKE_CURRENT_SOURCE_DIR}/lbitlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_base.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_bit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_ffi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_io.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_jit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_math.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_os.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_package.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_string.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_table.c
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj
  COMMAND buildvm
        -m peobj
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj
  DEPENDS
        buildvm
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
  COMMAND buildvm
        -m bcdef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
        ${ljlib_c}
  DEPENDS
        buildvm
        ${ljlib_c}
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
  COMMAND buildvm
        -m ffdef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
        ${ljlib_c}
  DEPENDS
        buildvm
        ${ljlib_c}
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
  COMMAND buildvm
        -m libdef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
        ${ljlib_c}
  DEPENDS
        buildvm
        ${ljlib_c}
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
  COMMAND buildvm
        -m recdef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
        ${ljlib_c}
  DEPENDS
        buildvm
        ${ljlib_c}
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_vmdef.h
  COMMAND buildvm
        -m vmdef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_vmdef.h
        ${ljlib_c}
  DEPENDS
        buildvm
        ${ljlib_c}
  VERBATIM
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
  COMMAND buildvm
        -m folddef
        -o ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_fold.c
  DEPENDS
        buildvm
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_fold.c
  VERBATIM
)

set(ljcore_c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_aux.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lib_init.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_alloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_api.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_bc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_bcread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_bcwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_buf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_carith.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ccall.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ccallback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cconv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cdata.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_char.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_clib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cparse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_crecord.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ctype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_dispatch.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_err.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ffrecord.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_func.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_gc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_gdbjit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ir.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_lex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_lib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_load.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_mcode.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_meta.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_obj.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_dce.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_fold.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_loop.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_mem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_narrow.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_sink.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_opt_split.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_parse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_profile.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_record.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_snap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_state.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_str.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_strfmt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_strscan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_tab.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_trace.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_udata.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_vmevent.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_vmmath.c
)

set(headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lauxlib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_alloc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_arch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm_arm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm_mips.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm_ppc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_asm_x86.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_bc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_bcdump.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_buf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_carith.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ccall.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ccallback.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cconv.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cdata.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_char.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_clib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_cparse.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_crecord.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ctype.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_debug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_def.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_dispatch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_emit_arm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_emit_mips.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_emit_ppc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_emit_x86.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_err.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_errmsg.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ff.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ffrecord.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_frame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_func.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_gc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_gdbjit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ir.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_ircall.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_iropt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_jit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_lex.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_lib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_mcode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_meta.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_obj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_parse.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_profile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_record.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_snap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_state.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_str.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_strfmt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_strscan.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_tab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target_arm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target_arm64.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target_mips.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target_ppc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_target_x86.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_trace.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_traceerr.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_udata.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_vm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lj_vmevent.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lua.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/luaconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/luajit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lualib.h
)

set(${luajit_dll_name}_sources
  ${generated_headers}
  ${headers}
  ${ljlib_c}
  ${ljcore_c}
  ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libluajit.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libluajit.rc
  )
  set(${luajit_dll_name}_sources ${${luajit_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/libluajit.rc
  )
endif()

add_library(${luajit_dll_name} SHARED ${${luajit_dll_name}_sources})

set_property(TARGET ${luajit_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${luajit_dll_name}
  PUBLIC
    ${public_include_directories}
)

#target_link_libraries(${luajit_dll_name}
#  ${utf8wrap_dll_name}
#)

set_target_properties(${luajit_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

rebase(${luajit_dll_name})

install(TARGETS ${luajit_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

set(luajit_sources
  source/src/luajit.c
  luajit-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND luajit_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    luajit.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}luajit ${luajit_sources})

set_property(TARGET ${MIKTEX_PREFIX}luajit PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}luajit ${luajit_dll_name})

install(TARGETS ${MIKTEX_PREFIX}luajit DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(includes
  lauxlib.h
  lj_arch.h
  lj_def.h
  lj_obj.h
  lj_state.h
  lua.h
  luaconf.h
  luajit.h
  lualib.h
)

foreach(i ${includes})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${i}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${i}
    COPYONLY
  )
endforeach()
