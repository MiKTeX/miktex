## CMakeLists.txt: top-level CMake file for building MiKTeX
##
## Copyright (C) 2006-2020 Christian Schenk
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

###############################################################################
## CMake controls

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_COLOR_MAKEFILE FALSE)
#set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_LINK_DEPENDS_NO_SHARED 1)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET
    "10.12"
    CACHE STRING
    "The earliest version of MacOS X that MiKTeX will run on."
  )
  #set(CMAKE_EXE_LINKER_FLAGS "-Wl,-no_weak_imports")
endif()

###############################################################################
## CMake policies

# CMP0071: Let AUTOMOC and AUTOUIC process GENERATED files.
list(APPEND cmake_policies_new
  CMP0071
)

foreach(p ${cmake_policies_new})
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

###############################################################################
## the name of the game

set(MIKTEX_PRODUCT_NAME "MiKTeX")
string(TOLOWER ${MIKTEX_PRODUCT_NAME} MIKTEX_PRODUCT_TAG)

include(miktex-version.cmake)
include(miktex-local-version.cmake OPTIONAL)

if(MIKTEX_PATCH_VERSION EQUAL 0)
  set(miktex_project_version ${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION})
else()
  set(miktex_project_version ${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}.${MIKTEX_PATCH_VERSION})
endif()

project(${MIKTEX_PRODUCT_TAG}
  VERSION ${miktex_project_version}
)

###############################################################################
## global C/C++ options

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

###############################################################################
## system type

if(WIN32 AND NOT UNIX)
  set(MIKTEX_NATIVE_WINDOWS TRUE)
else()
  set(MIKTEX_UNIX_ALIKE TRUE)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "BSD/OS")
  set(system_name "openbsd")
elseif(CMAKE_SYSTEM_NAME MATCHES "HP-UX")
  set(system_name "hpux")
elseif(CMAKE_SYSTEM_NAME MATCHES "GNU/kFreeBSD")
  set(MIKTEX_FREEBSD TRUE)
  set(system_name "freebsd")
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(MIKTEX_FREEBSD TRUE)
  set(system_name "freebsd")
elseif(CMAKE_SYSTEM_NAME MATCHES "SCO_SV")
  set(system_name "scosv")
elseif(CMAKE_SYSTEM_NAME MATCHES "UNIX_SV")
  set(system_name "unixsv")
elseif(CMAKE_SYSTEM_NAME MATCHES "CYGWIN_NT-.+")
  set(system_name "cygwin")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(MIKTEX_LINUX TRUE)
  set(system_name "linux")
else()
  string(TOLOWER "${CMAKE_SYSTEM_NAME}" system_name)
endif()

if(MIKTEX_NATIVE_WINDOWS)
  if(CMAKE_CL_64)
    set(system_cpu "x64")
    set(MIKTEX_MANIFEST_PROCESSOR_ARCHITECTURE "amd64")
  else()
    set(system_cpu "x86")
    set(MIKTEX_MANIFEST_PROCESSOR_ARCHITECTURE "x86")
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86")
  set(system_cpu "x86")
else()
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" system_cpu)
endif()

#set(MIKTEX_TARGET_SYSTEM_TAG "${system_cpu}-${system_name}")
set(MIKTEX_TARGET_SYSTEM_TAG "${system_name}-${system_cpu}")

set(MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN "win")
set(MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX "unx")

if(MIKTEX_NATIVE_WINDOWS)
  set(MIKTEX_SHORTEST_TARGET_SYSTEM_TAG ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN})
else()
  set(MIKTEX_SHORTEST_TARGET_SYSTEM_TAG ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX})
endif()

###############################################################################
## macOS: bundle vs Homebrew detection

string(SUBSTRING ${CMAKE_INSTALL_PREFIX} 0 17 homebrew_cellar)
if(homebrew_cellar STREQUAL "/usr/local/Cellar")
  set(MIKTEX_HOMEBREW TRUE)
endif()

if(APPLE AND NOT MIKTEX_HOMEBREW)
  set(MIKTEX_MACOS_BUNDLE TRUE)
endif()

###############################################################################
## handle install prefix standards

if(CMAKE_INSTALL_PREFIX STREQUAL "/usr")
  set(MIKTEX_SELF_CONTAINED_DEFAULT FALSE)
elseif(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
  set(MIKTEX_SELF_CONTAINED_DEFAULT FALSE)
elseif(MIKTEX_HOMEBREW)
  set(MIKTEX_SELF_CONTAINED_DEFAULT FALSE)
elseif(MIKTEX_UNIX_ALIKE AND NOT MIKTEX_MACOS_BUNDLE)
  set(MIKTEX_SELF_CONTAINED_DEFAULT TRUE)
endif()

###############################################################################
## user options

option(
  MIKTEX_SELF_CONTAINED
  "Build and install a self-contained MiKTeX distribution."
  ${MIKTEX_SELF_CONTAINED_DEFAULT}
)

option(
  MIKTEX_CORE_AUTO_ADMIN
  "Turn on adminstrator mode for shared installations and elevated programs."
  FALSE
)

option(
  MIKTEX_MPM_AUTO_ADMIN
  "Install packages for all users."
  FALSE
)

option(
  MIKTEX_MPM_AUTO_INSTALL
  "Install missing packages automatically (on-the-fly)."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_APR
  "Use the system Apache Portable Runtime (APR) library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_APRUTIL
  "Use the system Apache Portable Runtime (APR-util) library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_BZIP2
  "Use the system bzip2 compression library."
  ${MIKTEX_UNIX_ALIKE}
)

if(MIKTEX_MACOS_BUNDLE)
  set(use_system_cairo_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_cairo_default TRUE)
endif()

option(
  USE_SYSTEM_CAIRO
  "Use the system Cairo 2D graphics library."
  ${use_system_cairo_default}
)

option(
  USE_SYSTEM_CURL
  "Use the system libcurl URL transfer library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_EXPAT
  "Use the system Expat XML parser library."
  ${MIKTEX_UNIX_ALIKE}
)

if(MIKTEX_MACOS_BUNDLE)
  set(use_system_fontconfig_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_fontconfig_default TRUE)
endif()

option(
  USE_SYSTEM_FMT
  "Use the system fmt library."
  FALSE
)

option(
  USE_SYSTEM_FONTCONFIG
  "Use the system Fontconfig library."
  ${use_system_fontconfig_default}
)

option(
  USE_SYSTEM_FREETYPE2
  "Use the system FreeType2 font engine library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_FRIBIDI
  "Use the system GNU FriBiDi library."
  ${MIKTEX_UNIX_ALIKE}
)

if(MIKTEX_MACOS_BUNDLE)
  set(use_system_gd_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_gd_default TRUE)
endif()

option(
  USE_SYSTEM_GD
  "Use the system GD graphics library."
  ${use_system_gd_default}
)

option(
  USE_SYSTEM_GMP
  "Use the system GNU Multiple Precision Arithmetic library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_GRAPHITE2
  "Use the system Graphite2 library."
  ${MIKTEX_UNIX_ALIKE}
)

if(MIKTEX_HOMEBREW OR MIKTEX_MACOS_BUNDLE)
  set(use_system_harfbuzz_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_harfbuzz_default TRUE)
endif()

option(
  USE_SYSTEM_HARFBUZZ
  "Use the system Harfbuzz text shaping library."
  ${use_system_harfbuzz_default}
)

if(MIKTEX_HOMEBREW OR MIKTEX_MACOS_BUNDLE)
  set(use_system_harfbuzz_icu_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_harfbuzz_icu_default TRUE)
endif()

option(
  USE_SYSTEM_HARFBUZZ_ICU
  "Use the system Harfbuzz (ICU) text shaping library."
  ${use_system_harfbuzz_icu_default}
)

option(
  USE_SYSTEM_HUNSPELL
  "Use the system Hunspell spell checker library."
  ${MIKTEX_UNIX_ALIKE}
)

# we always depend on the system ICU development files
set(
  USE_SYSTEM_ICU
  TRUE
)

option(
  USE_SYSTEM_JPEG
  "Use the system JPEG library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_LOG4CXX
  "Use the system Log4cxx logging framework library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_LZMA
  "Use the system LZMA compression library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_MPFR
  "Use the system GNU Multiple Precision Floating-Point Reliably (MPFR) library."
  ${MIKTEX_UNIX_ALIKE}
)

if(MIKTEX_HOMEBREW OR MIKTEX_MACOS_BUNDLE)
  set(use_system_mspack_default FALSE)
elseif(MIKTEX_UNIX_ALIKE)
  set(use_system_mspack_default TRUE)
endif()

option(
  USE_SYSTEM_MSPACK
  "Use the system mspack compression library."
  ${use_system_mspack_default}
)

option(
  USE_SYSTEM_OPENSSL_CRYPTO
  "Use the system OpenSSL/LibreSSL crypto library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_PIXMAN
  "Use the system Pixman pixel manipulation library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_PNG
  "Use the system PNG library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_POPPLER
  "Use the system Poppler PDF Rendering library."
  FALSE
)

option(
  USE_SYSTEM_POPPLER_QT5
  "Use the system Poppler PDF Rendring (Qt5 Wrapper) library."
  FALSE
)

option(
  USE_SYSTEM_POPT
  "Use the system popt (command-line options parser) library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_URIPARSER
  "Use the system uriparser (URI parser) library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_ZLIB
  "Use the system zlib compression library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  USE_SYSTEM_ZZIP
  "Use the system Zzip compression library."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  WITH_ASYMPTOTE
  "Build asymptote."
  ${MIKTEX_NATIVE_WINDOWS}
)

option(
  WITH_BOOTSTRAPPING
  "Install files necessary for initializing a bare MiKTeX installation."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  WITH_CODE_SIGNING
  "Sign MiKTeX executables."
  FALSE
)

option(
  WITH_PACKAGE_DB_SIGNING
  "MiKTeX package database files must be signed."
  TRUE
)

option(
  WITH_LIBRESSL_CRYPTO
  "Use the LibreSSL crypto library supplied with MiKTeX."
  ${WIN32}
)

option(
  WITH_TRAPMF
  "Build trapmf."
  FALSE
)

option(
  WITH_TRIPTEX
  "Build triptex."
  FALSE
)

option(
  WITH_UI_QT
  "Build Qt components."
  TRUE
)

option(
  WITH_UI_MFC
  "Build MFC components."
  ${MSVC}
)

option(
  WITH_COM
  "Build COM components."
  ${MSVC}
)

option(
  WITH_MPC
  "Build mpc (MiKTeX Package Composer)."
  TRUE
)

option(
  WITH_SYNCTEX
  "Enable SyncTeX extensions."
  TRUE
)

option(
  LINK_EVERYTHING_STATICALLY
  "Don't build and use shared libraries."
  FALSE
)

option(
  INSTALL_MIKTEX_HEADERS
  "Install the MiKTeX header files."
  FALSE
)

option(
  INSTALL_STATIC_LIBRARIES
  "Install static libraries."
  FALSE
)

option(
  WITH_CONTEXT_SUPPORT
  "Enable ConTeXt support."
  FALSE
)

option(
  WITH_TPMS
  "Build package manifest files."
  ${WIN32}
)

option(
  WITH_CONFIG_FILES
  "Build configuration files."
  ${WIN32}
)

option(
  WITH_MIKTEX_DOC
  "Build miktex-doc package."
  ${WIN32}
)

option(
  WITH_SCRIPTS
  "Build script files."
  ${WIN32}
)

option(
  WITH_MAN_PAGES
  "Build man pages."
  ${MIKTEX_UNIX_ALIKE}
)

option(
  WITH_RUNGS
  "Create rungs, a symbolic link to the miktex-rungs executable."
  TRUE
)

option(
  WITH_KPSEWHICH
  "Create kpsewhich, a symbolic link to the miktex-kpsewhich executable."
  TRUE
)

option(
  WITH_MKTEXLSR
  ""
  ${WIN32}
)

option(
  WITH_TEXHASH
  ""
  ${WIN32}
)

option(
  WITH_TEXLINKS
  ""
  ${WIN32}
)

option(
  WITH_TEXDOC
  ""
  ${WIN32}
)

option(
  WITH_UPDMAP
  ""
  ${WIN32}
)

if(WITH_ASYMPTOTE AND MIKTEX_NATIVE_WINDOWS)
  set(USE_SYSTEM_OPENGL TRUE)
  set(WITH_LIBATOMIC_OPS TRUE)
  set(WITH_GC TRUE)
endif()
  
###############################################################################
## fixed values

set(MIKTEX_HQ_BASE_URL_NO_SLASH "https://miktex.org")
set(MIKTEX_HQ_BASE_URL "${MIKTEX_HQ_BASE_URL_NO_SLASH}/")

set(MIKTEX_DOC_BASE_URL_NO_SLASH "https://docs.miktex.org")
set(MIKTEX_DOC_BASE_URL "${MIKTEX_DOC_BASE_URL_NO_SLASH}/")

set(MIKTEX_PREFIX "miktex-")
set(MIKTEX_ADMIN_SUFFIX "_admin")
set(MIKTEX_STANDALONE_SUFFIX "_standalone")

## autotools compatibility names
set(PACKAGE_NAME ${MIKTEX_PRODUCT_NAME})
set(top_srcdir "${CMAKE_SOURCE_DIR}")

if(MIKTEX_NATIVE_WINDOWS)
  set(MIKTEX_DEV_NULL "nul")
else()
  set(MIKTEX_DEV_NULL "/dev/null")
endif()

if(MIKTEX_MACOS_BUNDLE)
  set(MIKTEX_CODESIGN_IDENTITY "Developer ID")
endif()

###############################################################################
## version information

string(TIMESTAMP MIKTEX_DATETIME_STR UTC)

if(MIKTEX_MONTH_VERSION EQUAL 1)
  set(MIKTEX_MONTH_NAME_VERSION "January")
elseif(MIKTEX_MONTH_VERSION EQUAL 2)
  set(MIKTEX_MONTH_NAME_VERSION "February")
elseif(MIKTEX_MONTH_VERSION EQUAL 3)
  set(MIKTEX_MONTH_NAME_VERSION "March")
elseif(MIKTEX_MONTH_VERSION EQUAL 4)
  set(MIKTEX_MONTH_NAME_VERSION "April")
elseif(MIKTEX_MONTH_VERSION EQUAL 5)
  set(MIKTEX_MONTH_NAME_VERSION "May")
elseif(MIKTEX_MONTH_VERSION EQUAL 6)
  set(MIKTEX_MONTH_NAME_VERSION "June")
elseif(MIKTEX_MONTH_VERSION EQUAL 7)
  set(MIKTEX_MONTH_NAME_VERSION "July")
elseif(MIKTEX_MONTH_VERSION EQUAL 8)
  set(MIKTEX_MONTH_NAME_VERSION "August")
elseif(MIKTEX_MONTH_VERSION EQUAL 9)
  set(MIKTEX_MONTH_NAME_VERSION "September")
elseif(MIKTEX_MONTH_VERSION EQUAL 10)
  set(MIKTEX_MONTH_NAME_VERSION "October")
elseif(MIKTEX_MONTH_VERSION EQUAL 11)
  set(MIKTEX_MONTH_NAME_VERSION "November")
elseif(MIKTEX_MONTH_VERSION EQUAL 12)
  set(MIKTEX_MONTH_NAME_VERSION "December")
endif()  

if(MIKTEX_LINUX)
  set(MIKTEX_LINUX_DIST
    "ubuntu"
    CACHE STRING
    "The Linux distribution (ubuntu, debian, fedora, ...)."
  )
  set(MIKTEX_LINUX_DIST_VERSION
    "16.04"
    CACHE STRING
    "The Linux distribution version (16.04, 9, 28, ...)."
  )
  set(MIKTEX_LINUX_DIST_CODE_NAME
    "xenial"
    CACHE STRING
    "The Linux distribution code name (xenial, stretch, ...)."
  )
endif()

## the release state; one of:
##   0 (stable)
##   1 (next)
##   4 (debug)
set(MIKTEX_RELEASE_STATE
  0
  CACHE STRING
  "The MiKTeX release state."
)

## sanity check
if(MIKTEX_RELEASE_STATE EQUAL 0)
elseif(MIKTEX_RELEASE_STATE EQUAL 1)
elseif(MIKTEX_RELEASE_STATE EQUAL 4)
else()
  message("${MIKTEX_RELEASE_STATE} is an unknown release state.")
  message("Falling back to release state 4 (debug).")
  set(MIKTEX_RELEASE_STATE
      4
      CACHE STRING
      "MiKTeX release state; one of 0 (stable), 1 (next), 4 (debug)."
      FORCE)
endif()

find_package(Git)

add_custom_target(gen-git-info
  COMMAND ${CMAKE_COMMAND}
    -D GIT_FOUND=${Git_FOUND}
    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
    -D SOURCE_DIR=${CMAKE_SOURCE_DIR}
    -D GITINFO_H_IN=${CMAKE_SOURCE_DIR}/include/miktex/GitInfo.h.in
    -D GITINFO_H=${CMAKE_BINARY_DIR}/include/miktex/GitInfo.h
    -P ${CMAKE_CURRENT_SOURCE_DIR}/gen-git-info.cmake
)

set(MIKTEX_MAJOR_MINOR_STR
  "${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}"
)

set(MIKTEX_MAJOR_MINOR_PATCH_STR
  "${MIKTEX_MAJOR_VERSION}.${MIKTEX_MINOR_VERSION}.${MIKTEX_PATCH_VERSION}"
)

set(MIKTEX_CONFIG_MAJOR_MINOR_STR
  "${MIKTEX_CONFIG_MAJOR_VERSION}.${MIKTEX_CONFIG_MINOR_VERSION}"
)

set(MIKTEX_PACKAGES_MAJOR_MINOR_STR
  "${MIKTEX_PACKAGES_MAJOR_VERSION}.${MIKTEX_PACKAGES_MINOR_VERSION}"
)

if(MIKTEX_WINDOWS)
  set(MIKTEX_COM_MAJOR_MINOR_STR
    "${MIKTEX_COM_MAJOR_VERSION}.${MIKTEX_COM_MINOR_VERSION}"
  )
  set(MIKTEX_COM_MAJOR_MINOR_ID_STR
    "${MIKTEX_COM_MAJOR_VERSION}_${MIKTEX_COM_MINOR_VERSION}"
  )
  math(
    EXPR
    MIKTEX_COM_MAJOR_MINOR_INT
    "(100 * ${MIKTEX_COM_MAJOR_VERSION}) + ${MIKTEX_COM_MINOR_VERSION}"
  )
endif()

math(
  EXPR
  MIKTEX_MAJOR_MINOR_INT
  "(100 * ${MIKTEX_MAJOR_VERSION}) + ${MIKTEX_MINOR_VERSION}"
)

if(MIKTEX_RELEASE_STATE EQUAL 0)
  if(MIKTEX_PATCH_VERSION EQUAL 0)
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_STR}")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_STR}")
  else()
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR}")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR}")
  endif()
elseif(MIKTEX_RELEASE_STATE EQUAL 1)
  if(MIKTEX_PATCH_VERSION EQUAL 0)
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_STR} NEXT")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_STR}-next")
  else()
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR} NEXT")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR}-next")
  endif()
elseif(MIKTEX_RELEASE_STATE EQUAL 4)
  if(MIKTEX_PATCH_VERSION EQUAL 0)
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_STR} debug")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_STR}-debug")
  else()
    set(default_miktex_display_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR} debug")
    set(default_miktex_version_str "${MIKTEX_MAJOR_MINOR_PATCH_STR}-debug")
  endif()
endif()

set(MIKTEX_DISPLAY_VERSION_STR
  "${default_miktex_display_version_str}"
  CACHE STRING
  "The MiKTeX version presented to the user."
  FORCE
)

set(MIKTEX_VERSION_STR
  "${default_miktex_version_str}"
  CACHE STRING
  "The MiKTeX version."
  FORCE
)

if(MIKTEX_RELEASE_STATE EQUAL 1)
  set(MIKTEX_DEFAULT_PACKAGE_STREAM "next")
else()
  set(MIKTEX_DEFAULT_PACKAGE_STREAM "stable")
endif()

###############################################################################
## Endianess

include(TestBigEndian)
test_big_endian(MIKTEX_WORDS_BIGENDIAN)

###############################################################################
## CMake modules

include(Admin/COM/guids.cmake)

include(FindPackageHandleStandardArgs)
include(GNUInstallDirs)

include(AddLinkFlags)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckVariableExists)
include(ConfigNames)
include(CreateDllWrapper)
include(CreateSymbolicLinks)
include(CreateWebApp)
include(DefaultCharTypeIsUnsigned)
include(DelayLoad)
include(DisableRuntimeChecks)
include(ExecutableNames)
include(InstallPaths)
include(GenerateExportHeader)
include(LibraryNames)
include(RuntimePaths)
include(SetSharedLibraryVersion)
include(SourcePaths)
include(UseStaticCRT)
include(IgnoreWarnings)

###############################################################################
## build sandbox

set(MIKTEX_SANDBOX_SOURCE_DIR "${CMAKE_SOURCE_DIR}/sandbox")
set(MIKTEX_SANDBOX_BINARY_DIR "${CMAKE_BINARY_DIR}/sandbox")
set(MIKTEX_SANDBOX_USER_CONFIG_DIR "${MIKTEX_SANDBOX_BINARY_DIR}" CACHE PATH "MiKTeX user config root.")
set(MIKTEX_SANDBOX_USER_DATA_DIR "${MIKTEX_SANDBOX_BINARY_DIR}" CACHE PATH "MiKTeX user data root.")
set(MIKTEX_SANDBOX_USER_INSTALL_DIR "${MIKTEX_SANDBOX_BINARY_DIR}" CACHE PATH "MiKTeX user install root.")
set(MIKTEX_SANDBOX_USER_ROOTS "" CACHE PATH  "MiKTeX user roots")

configure_file(
  sandbox/miktex/config/miktexstartup.ini.in
  ${MIKTEX_SANDBOX_BINARY_DIR}/miktex/config/miktexstartup.ini
)

configure_file(
  sandbox/miktex/config/miktex.ini.in
  ${MIKTEX_SANDBOX_BINARY_DIR}/miktex/config/miktex.ini
)

configure_file(
  Admin/ConfigFiles/win/log4cxx.win.xml
  ${MIKTEX_SANDBOX_BINARY_DIR}/miktex/config/win/log4cxx.win.xml
  COPYONLY
)

configure_file(
  Admin/ConfigFiles/unx/log4cxx.unx.xml
  ${MIKTEX_SANDBOX_BINARY_DIR}/miktex/config/unx/log4cxx.unx.xml
  COPYONLY
)

set(MIKTEX_AUTO_MAINTENANCE_LOCK "A6D646EE9FBF44D6A3E6C1A3A72FF7E3.lock")

# disable maintenance tasks while building
file(WRITE
  ${MIKTEX_SANDBOX_BINARY_DIR}/${MIKTEX_REL_MIKTEX_LOCK_DIR}/${MIKTEX_AUTO_MAINTENANCE_LOCK}
  "-1\npermanently-locked\n"
)

set(outdir "${MIKTEX_SANDBOX_BINARY_DIR}/${MIKTEX_REL_MIKTEX_BIN_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${outdir}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${outdir}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${outdir}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${outdir}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${outdir}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${outdir}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${outdir}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${outdir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${outdir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${outdir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${outdir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${outdir}")

###############################################################################
## bootstrapping packages

if(WITH_BOOTSTRAPPING)
  list(APPEND MIKTEX_BOOTSTRAPPING_PACKAGES
    miktex-config-${MIKTEX_CONFIG_MAJOR_MINOR_STR}
    miktex-dvips
    miktex-fontconfig
    miktex-misc
    tetex
  )
endif()

###############################################################################
## RPATH handling

if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
else()
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

###############################################################################
## find/check stuff

if(LINK_EVERYTHING_STATICALLY)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_WIN32_WINNT=0x0600
    -DWINVER=0x0600
  )
  if(CMAKE_CL_64)
    set(CMAKE_CREATE_WIN32_EXE /SUBSYSTEM:WINDOWS,6.00)
    set(CMAKE_CREATE_CONSOLE_EXE /SUBSYSTEM:CONSOLE,6.00)
  else()
    set(CMAKE_CREATE_WIN32_EXE /SUBSYSTEM:WINDOWS,6.00)
    set(CMAKE_CREATE_CONSOLE_EXE /SUBSYSTEM:CONSOLE,6.00)
  endif()
endif()

set(required_programs
  cat
  diff
  sed
  xsltproc
)

if(WITH_MIKTEX_DOC)
  list(APPEND required_programs
    pandoc
  )
endif()

if(WITH_UI_MFC)
  list(APPEND required_programs
    hhc
    makehm
  )
endif()

if(MSVC)
  list(APPEND required_programs
    mc
    midl
    mt
  )
endif()

if(WITH_BOOTSTRAPPING)
  list(APPEND required_programs
    curl
  )
endif()

foreach(p ${required_programs})
  string(TOUPPER "${p}" p_u)
  find_program(${p_u}_EXECUTABLE NAMES ${p})
  if(NOT ${p_u}_EXECUTABLE)
    message(FATAL_ERROR "The program ${p} could not be found.")
  endif()
endforeach()

if(WITH_MIKTEX_DOC)
  if(MIKTEX_NATIVE_WINDOWS)
    set(fop_name fop.cmd)
  else()
    set(fop_name fop)
  endif()
  find_program(FOP_EXECUTABLE NAMES ${fop_name})
  if(NOT FOP_EXECUTABLE)
    message(FATAL_ERROR "The program ${fop_name} could not be found.")
  endif()
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
#not needed: find_package(ImageMagick REQUIRED)
#not needed: find_package(PythonInterp REQUIRED)
find_package(Threads REQUIRED)

if(WITH_UI_QT)
  set(required_qt5_components
    Concurrent
    Core
    Gui
    LinguistTools
    PrintSupport
    Script
    ScriptTools
    UiTools
    Widgets
    Xml
  )
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND required_qt5_components
      WinExtras
    )
  endif()
  if(MIKTEX_UNIX_ALIKE AND NOT MIKTEX_MACOS_BUNDLE)
    list(APPEND required_qt5_components
      DBus
    )
  endif()
  find_package(Qt5 REQUIRED ${required_qt5_components})
endif()

if(USE_SYSTEM_APR)
  find_package(APR REQUIRED)
endif()

if(USE_SYSTEM_APRUTIL)
  find_package(APRUTIL REQUIRED)
endif()

if(USE_SYSTEM_BZIP2)
  find_package(BZip2 REQUIRED)
endif()

if(USE_SYSTEM_CAIRO)
  find_package(CAIRO REQUIRED)
endif()

if(USE_SYSTEM_CURL)
  find_package(CURL REQUIRED)
endif()

if(USE_SYSTEM_EXPAT)
  find_package(EXPAT REQUIRED)
endif()

if(USE_SYSTEM_FMT)
  find_package(FMT REQUIRED)
endif()

if(USE_SYSTEM_FONTCONFIG)
  find_package(FONTCONFIG REQUIRED)
endif()

if(USE_SYSTEM_FREETYPE2)
  find_package(Freetype REQUIRED)
endif()

if(USE_SYSTEM_FRIBIDI)
  find_package(FRIBIDI REQUIRED)
endif()

if(USE_SYSTEM_GD)
  find_package(GD REQUIRED)
endif()

if(USE_SYSTEM_GMP)
  find_package(GMP REQUIRED)
endif()

if(USE_SYSTEM_GRAPHITE2)
  find_package(GRAPHITE2 REQUIRED)
endif()

if(USE_SYSTEM_HARFBUZZ)
  find_package(HARFBUZZ REQUIRED)
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
  find_package(HARFBUZZ_ICU REQUIRED)
endif()

if(USE_SYSTEM_HUNSPELL)
  find_package(HUNSPELL REQUIRED)
endif()

if(USE_SYSTEM_ICU)
  find_package(ICU REQUIRED COMPONENTS i18n uc)
endif()

if(USE_SYSTEM_JPEG)
  find_package(JPEG REQUIRED)
endif()

if(USE_SYSTEM_LOG4CXX)
  find_package(LOG4CXX REQUIRED)
endif()

if(USE_SYSTEM_LZMA)
  find_package(LibLZMA REQUIRED)
endif()

if(USE_SYSTEM_MPFR)
  find_package(MPFR REQUIRED)
endif()

if(USE_SYSTEM_MSPACK)
  find_package(MSPACK REQUIRED)
endif()

if(USE_SYSTEM_OPENGL)
  find_package(OpenGL REQUIRED)
endif()

if(USE_SYSTEM_OPENSSL_CRYPTO)
  find_package(OpenSSL REQUIRED)
endif()

if(USE_SYSTEM_PIXMAN)
  find_package(PIXMAN REQUIRED)
endif()

if(USE_SYSTEM_PNG)
  find_package(PNG REQUIRED)
endif()

if(USE_SYSTEM_POPT)
  find_package(POPT REQUIRED)
endif()

if(USE_SYSTEM_POPPLER)
  find_package(POPPLER REQUIRED)
endif()

if(USE_SYSTEM_POPPLER_QT5 AND WITH_UI_QT)
  find_package(POPPLER_QT5 REQUIRED)
endif()

if(USE_SYSTEM_URIPARSER)
  find_package(URIPARSER REQUIRED)
endif()

if(USE_SYSTEM_ZLIB)
  find_package(ZLIB REQUIRED)
endif()

if(USE_SYSTEM_ZZIP)
  find_package(ZZIP REQUIRED)
endif()

if(MSVC)
  set(HAVE_PRAGMA_ONCE TRUE)
endif()

if(MIKTEX_NATIVE_WINDOWS AND MSVC)
  check_symbol_exists(alloca "malloc.h" HAVE_ALLOCA)
else()
  check_function_exists(alloca HAVE_ALLOCA)
endif()

check_symbol_exists(ntohl arpa/inet.h NEED_ARPA_INET_H)

check_function_exists(_mktemp_s HAVE__MKTEMP_S)
check_function_exists(access HAVE_ACCESS)
check_function_exists(arc4random_buf HAVE_ARC4RANDOM_BUF)
check_function_exists(chdir HAVE_CHDIR)
check_function_exists(chmod HAVE_CHMOD)
check_function_exists(chown HAVE_CHOWN)
check_function_exists(closedir HAVE_CLOSEDIR)
check_function_exists(confstr HAVE_CONFSTR)
check_function_exists(ctime HAVE_CTIME)
check_function_exists(finite HAVE_FINITE)
check_function_exists(fork HAVE_FORK)
check_function_exists(fseeko64 HAVE_FSEEKO64)
check_function_exists(fstatfs HAVE_FSTATFS)
check_function_exists(fstatvfs HAVE_FSTATVFS)
check_function_exists(ftello64 HAVE_FTELLO64)
check_function_exists(ftime HAVE_FTIME)
check_function_exists(futimes HAVE_FUTIMES)
check_function_exists(getcwd HAVE_GETCWD)
check_function_exists(getenv HAVE_GETENV)
check_function_exists(geteuid HAVE_GETEUID)
check_function_exists(getopt_long HAVE_GETOPT_LONG)
check_function_exists(getpagesize HAVE_GETPAGESIZE)
check_function_exists(getpid HAVE_GETPID)
check_function_exists(getprogname HAVE_GETPROGNAME)
check_function_exists(getrandom HAVE_GETRANDOM)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(link HAVE_LINK)
check_function_exists(lrand48 HAVE_LRAND48)
check_function_exists(lstat HAVE_LSTAT)
check_function_exists(memcmp HAVE_MEMCMP)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(mkdir HAVE_MKDIR)
check_function_exists(mkostemp HAVE_MKOSTEMP)
check_function_exists(mkstemp HAVE_MKSTEMP)
check_function_exists(mmap HAVE_MMAP)
check_function_exists(open HAVE_OPEN)
check_function_exists(opendir HAVE_OPENDIR)
check_function_exists(pclose HAVE_PCLOSE)
check_function_exists(popen HAVE_POPEN)
check_function_exists(posix_fadvise HAVE_POSIX_FADVISE)
check_function_exists(putenv HAVE_PUTENV)
check_function_exists(rand HAVE_RAND)
check_function_exists(rand_r HAVE_RAND_R)
check_function_exists(random HAVE_RANDOM)
check_function_exists(random_r HAVE_RANDOM_R)
check_function_exists(readdir HAVE_READDIR)
check_function_exists(readlink HAVE_READLINK)
check_function_exists(reallocarray HAVE_REALLOCARRAY)
check_function_exists(regexec HAVE_REGEXEC)
check_function_exists(rmdir HAVE_RMDIR)
check_function_exists(sched_yield HAVE_SCHED_YIELD)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(setreuid HAVE_SETREUID)
check_function_exists(setuid HAVE_SETUID)
check_function_exists(sigaction HAVE_SIGACTION)
check_function_exists(spawnvp HAVE_SPAWNVP)
check_function_exists(stat HAVE_STAT)
check_function_exists(statvfs HAVE_STATVFS)
check_function_exists(stpcpy HAVE_DECL_STPCPY)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strchr HAVE_STRCHR)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(strlwr HAVE_STRLWR)
check_function_exists(strnlen HAVE_STRNLEN)
check_function_exists(strrchr HAVE_STRRCHR)
check_function_exists(strtol HAVE_STRTOL)
check_function_exists(strtoll HAVE_STRTOLL)
check_function_exists(uname HAVE_UNAME_SYSCALL)
check_function_exists(unlink HAVE_UNLINK)
check_function_exists(utime HAVE_UTIME)
check_function_exists(vfork HAVE_VFORK)
check_function_exists(vprintf HAVE_VPRINTF)
check_function_exists(waitpid HAVE_WAITPID)

if(WITH_UI_MFC)
  set(HAVE_AFXWIN_H "YES")
endif()

if(WITH_COM)
  set(HAVE_ATLBASE_H "YES")
endif()

check_include_files(alloca.h HAVE_ALLOCA_H)
check_include_files(arpa/inet.h HAVE_ARPA_INET_H)
check_include_files(assert.h HAVE_ASSERT_H)
check_include_files(byteorder.h HAVE_BYTEORDER_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(float.h HAVE_FLOAT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(io.h HAVE_IO_H)
check_include_files(libgen.h HAVE_LIBGEN_H)
check_include_files(libintl.h HAVE_LIBINTL_H)
check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(mcheck.h HAVE_MCHECK_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(ndir.h HAVE_NDIR_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
check_include_files(stat.h HAVE_STAT_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(sys/dir.h HAVE_SYS_DIR_H)
check_include_files(sys/mman.h HAVE_SYS_MMAN_H)
check_include_files(sys/mount.h HAVE_SYS_MOUNT_H)
check_include_files(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_files(sys/param.h HAVE_SYS_PARAM_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/statfs.h HAVE_SYS_STATFS_H)
check_include_files(sys/statvfs.h HAVE_SYS_STATVFS_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/timeb.h HAVE_SYS_TIMEB_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/utime.h HAVE_SYS_UTIME_H)
check_include_files(sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_files(sys/wait.h HAVE_SYS_WAIT_H)
check_include_files(sys/vfs.h HAVE_SYS_VFS_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(utime.h HAVE_UTIME_H)

check_include_files(sched.h HAVE_SCHED_H)

check_struct_has_member("struct dirent" d_type dirent.h HAVE_STRUCT_DIRENT_D_TYPE)
check_struct_has_member("struct stat" st_mtim sys/stat.h HAVE_STRUCT_STAT_ST_MTIM)
check_struct_has_member("struct statfs" f_flags sys/statfs.h HAVE_STRUCT_STATFS_F_FLAGS)
check_struct_has_member("struct statfs" f_fstypename sys/statfs.h HAVE_STRUCT_STATFS_F_FSTYPENAME)
check_struct_has_member("struct statvfs" f_basetype sys/statvfs.h HAVE_STRUCT_STATVFS_F_BASETYPE)
check_struct_has_member("struct statvfs" f_fstypename sys/statvfs.h HAVE_STRUCT_STATVFS_F_FSTYPENAME)
check_struct_has_member("struct tm" tm_gmtoff time.h HAVE_TM_GMTOFF)

check_variable_exists(timezone HAVE_TIMEZONE)

check_type_size("long long" LONG_LONG)
set(SIZEOF_LONG_LONG ${LONG_LONG})
check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)

if(HAVE_LIMITS_H)
  set(STDC_HEADERS)
endif()

###############################################################################
## set the build type
##
## the build type is derived from the MiKTeX release state; see
## miktex-version.cmake

if(MIKTEX_RELEASE_STATE EQUAL 0)
  if(MIKTEX_NATIVE_WINDOWS)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "The build type." FORCE)
  else()
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type." FORCE)
  endif()
elseif(MIKTEX_RELEASE_STATE EQUAL 1)
  if(MIKTEX_NATIVE_WINDOWS)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "The build type." FORCE)
    #add_definitions(-DMIKTEX_DEBUG)
  else()
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type." FORCE)
  endif()
elseif(MIKTEX_RELEASE_STATE EQUAL 4)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "The build type." FORCE)
  add_definitions(-DMIKTEX_DEBUG)
endif()

mark_as_advanced(CMAKE_BUILD_TYPE)

###############################################################################
## add global definitions

add_definitions(
  -DHAVE_CONFIG_H
  -DMIKTEX
)

if(WITH_UI_QT)
  add_definitions(
    -DMIKTEX_QT
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DMIKTEX_WINDOWS
  )
  if(MSVC)
    ## better bind to latest CRT/MFC DLLs if redistributing as private
    ## assemblies
    add_definitions(
      -D_BIND_TO_CURRENT_ATL_VERSION=1
      -D_BIND_TO_CURRENT_CRT_VERSION=1
      -D_BIND_TO_CURRENT_MFC_VERSION=1
      )
  endif()
  if(WITH_UI_MFC)
    add_definitions(
      -DMIKTEX_ATLMFC
    )
  endif()
  ## use overloads for unsecure functions
  #add_definitions(
  #  -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
  #)
elseif(MIKTEX_UNIX_ALIKE)
  add_definitions(
    -DMIKTEX_UNIX
  )
  if(MIKTEX_MACOS_BUNDLE)
  add_definitions(
    -DMIKTEX_MACOS_BUNDLE
  )
  endif()
endif()

###############################################################################
## add global include directories

include_directories(BEFORE
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}/include
)

# TODO: interface library
include_directories(BEFORE
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UI_DIR}/include
)

###############################################################################
## configure files

file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} CMAKE_SOURCE_DIR_NATIVE)
file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR} CMAKE_BINARY_DIR_NATIVE)

configure_file(
  include/miktex/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Version.h
)

configure_file(
  include/miktex/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/config.h
)

configure_file(
  Doxyfile.in
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)

configure_file(
  BuildUtilities/docbook/catalog.in
  ${CMAKE_CURRENT_BINARY_DIR}/BuildUtilities/docbook/catalog
)

configure_file(
  Admin/Scripts/makelocfile.cmd.in
  ${CMAKE_CURRENT_BINARY_DIR}/Admin/Scripts/makelocfile.cmd
)

configure_file(
  Admin/Scripts/putj2000version.cmd.in
  ${CMAKE_CURRENT_BINARY_DIR}/Admin/Scripts/putj2000version.cmd
)

###############################################################################
## add/import 3rd party libs
##
## base libraries (on which other libraries depend on) must be defined early

if(USE_SYSTEM_ZLIB)
  get_filename_component(suffix ${ZLIB_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::ZLIB SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::ZLIB STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::ZLIB
    PROPERTIES
      IMPORTED_LOCATION "${ZLIB_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "HAVE_LIBZ=1;HAVE_ZLIB=1;HAVE_ZLIB_H=1;HAVE_ZLIB_COMPRESS2=1"
      INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_ZLIB_DIR})
endif()

if(USE_SYSTEM_APR)
  get_filename_component(suffix ${APR_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::APR SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::APR STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::APR
    PROPERTIES
      IMPORTED_LOCATION "${APR_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${APR_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_APR_DIR})
endif()

if(USE_SYSTEM_APRUTIL)
  get_filename_component(suffix ${APRUTIL_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::APRUTIL SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::APRUTIL STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::APRUTIL
    PROPERTIES
      IMPORTED_LOCATION "${APRUTIL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${APRUTIL_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_APR_UTIL_DIR})
endif()

if(USE_SYSTEM_BZIP2)
  get_filename_component(suffix ${BZIP2_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::BZIP2 SHARED IMPORTED)
    set(bzip2_dll_name MiKTeX::Imported::BZIP2)
  else()
    add_library(MiKTeX::Imported::BZIP2 STATIC IMPORTED)
    set(bzip2_lib_name MiKTeX::Imported::BZIP2)
  endif()
  set_target_properties(MiKTeX::Imported::BZIP2
    PROPERTIES
      IMPORTED_LOCATION "${BZIP2_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${BZIP2_INCLUDE_DIR}"
  )
else()  
  add_subdirectory(${MIKTEX_REL_BZIP2_DIR})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_subdirectory(${MIKTEX_REL_FREEGLUT_DIR})
endif()

if(USE_SYSTEM_FREETYPE2)
  get_filename_component(suffix ${FREETYPE_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::FREETYPE2 SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::FREETYPE2 STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::FREETYPE2
    PROPERTIES
      IMPORTED_LOCATION "${FREETYPE_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "HAVE_FT2BUILD_H=1;HAVE_LIBFREETYPE=1"
      INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_FREETYPE2_DIR})
endif()

if(USE_SYSTEM_PIXMAN)
  get_filename_component(suffix ${PIXMAN_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::PIXMAN SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::PIXMAN STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::PIXMAN
    PROPERTIES
      IMPORTED_LOCATION "${PIXMAN_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${PIXMAN_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_PIXMAN_DIR})
endif()

if(USE_SYSTEM_PNG)
  get_filename_component(suffix ${PNG_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::PNG SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::PNG STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::PNG
    PROPERTIES
      IMPORTED_LOCATION "${PNG_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS HAVE_LIBPNG=1
      INTERFACE_INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES MiKTeX::Imported::ZLIB
  )
else()
  add_subdirectory(${MIKTEX_REL_PNG_DIR})
endif()

if(USE_SYSTEM_CAIRO)
  get_filename_component(suffix ${CAIRO_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::CAIRO SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::CAIRO STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::CAIRO
    PROPERTIES
      IMPORTED_LOCATION "${CAIRO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${CAIRO_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES
        "MiKTeX::Imported::FREETYPE2;MiKTeX::Imported::PIXMAN;MiKTeX::Imported::PNG;MiKTeX::Imported::ZLIB;Threads::Threads"
  )
else()
  add_subdirectory(${MIKTEX_REL_CAIRO_DIR})
endif()

if(USE_SYSTEM_CURL)
  get_filename_component(suffix ${CURL_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::CURL SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::CURL STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::CURL
    PROPERTIES
      IMPORTED_LOCATION "${CURL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_CURL_DIR})
endif()

if(USE_SYSTEM_EXPAT)
  get_filename_component(suffix ${EXPAT_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::EXPAT SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::EXPAT STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::EXPAT
    PROPERTIES
      IMPORTED_LOCATION "${EXPAT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${EXPAT_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_EXPAT_DIR})
endif()

if(USE_SYSTEM_FMT)
  get_filename_component(suffix ${FMT_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::FMT SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::FMT STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::FMT
    PROPERTIES
      IMPORTED_LOCATION "${FMT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FMT_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_FMT_DIR})
endif()

if(USE_SYSTEM_FONTCONFIG)
  get_filename_component(suffix ${FONTCONFIG_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::FONTCONFIG SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::FONTCONFIG STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::FONTCONFIG
    PROPERTIES
      IMPORTED_LOCATION "${FONTCONFIG_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS HAVE_LIBFONTCONFIG=1
      INTERFACE_INCLUDE_DIRECTORIES "${FONTCONFIG_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_FONTCONFIG_DIR})
endif()

if(USE_SYSTEM_FRIBIDI)
  get_filename_component(suffix ${FRIBIDI_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::FRIBIDI SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::FRIBIDI STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::FRIBIDI
    PROPERTIES
      IMPORTED_LOCATION "${FRIBIDI_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FRIBIDI_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_FRIBIDI_DIR})
endif()

if(WITH_GC)
  add_subdirectory(${MIKTEX_REL_GC_DIR})
endif()

if(USE_SYSTEM_GLM)
  get_filename_component(suffix ${GLM_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::GLM SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::GLM STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::GLM
    PROPERTIES
      IMPORTED_LOCATION "${GLM_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_GLM_DIR})
endif()

if(USE_SYSTEM_JPEG)
  get_filename_component(suffix ${JPEG_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::JPEG SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::JPEG STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::JPEG
    PROPERTIES
      IMPORTED_LOCATION "${JPEG_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS HAVE_LIBJPEG=1
      INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INCLUDE_DIR}"
  )
else()
  add_subdirectory(${MIKTEX_REL_JPEG_DIR})
endif()

if(USE_SYSTEM_GD)
  get_filename_component(suffix ${GD_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::GD SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::GD STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::GD
    PROPERTIES
      IMPORTED_LOCATION "${GD_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GD_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_GD_DIR})
endif()

if(USE_SYSTEM_GMP)
  get_filename_component(suffix ${GMP_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::GMP SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::GMP STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::GMP
    PROPERTIES
      IMPORTED_LOCATION "${GMP_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_GMP_DIR})
endif()

if(USE_SYSTEM_GRAPHITE2)
  get_filename_component(suffix ${GRAPHITE2_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::GRAPHITE2 SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::GRAPHITE2 STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::GRAPHITE2
    PROPERTIES
      IMPORTED_LOCATION "${GRAPHITE2_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GRAPHITE2_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_GRAPHITE2_DIR})
endif()

if(USE_SYSTEM_HARFBUZZ)
  get_filename_component(suffix ${HARFBUZZ_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::HARFBUZZ SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::HARFBUZZ STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::HARFBUZZ
    PROPERTIES
      IMPORTED_LOCATION "${HARFBUZZ_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${HARFBUZZ_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_HARFBUZZ_DIR})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
  get_filename_component(suffix ${HARFBUZZ_ICU_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::HARFBUZZ_ICU SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::HARFBUZZ_ICU STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::HARFBUZZ_ICU
    PROPERTIES
      IMPORTED_LOCATION "${HARFBUZZ_ICU_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${HARFBUZZ_ICU_INCLUDE_DIRS}"
  )
else()
  #add_subdirectory(${MIKTEX_REL_HARFBUZZ_DIR})
endif()

if(USE_SYSTEM_HUNSPELL)
  get_filename_component(suffix ${HUNSPELL_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::HUNSPELL SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::HUNSPELL STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::HUNSPELL
    PROPERTIES
      IMPORTED_LOCATION "${HUNSPELL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${HUNSPELL_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_HUNSPELL_DIR})
endif()

if(WITH_LIBATOMIC_OPS)
  add_subdirectory(${MIKTEX_REL_LIBATOMIC_OPS_DIR})
endif()

if(USE_SYSTEM_LOG4CXX)
  get_filename_component(suffix ${LOG4CXX_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::LOG4CXX SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::LOG4CXX STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::LOG4CXX
    PROPERTIES
      IMPORTED_LOCATION "${LOG4CXX_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LOG4CXX_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_LOG4CXX_DIR})
endif()

if(USE_SYSTEM_LZMA)
  get_filename_component(suffix ${LIBLZMA_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::LZMA SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::LZMA STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::LZMA
    PROPERTIES
      IMPORTED_LOCATION "${LIBLZMA_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LIBLZMA_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_XZUTILS_DIR})
endif()

if(USE_SYSTEM_MPFR)
  get_filename_component(suffix ${MPFR_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::MPFR SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::MPFR STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::MPFR
    PROPERTIES
      IMPORTED_LOCATION "${MPFR_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MPFR_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_MPFR_DIR})
endif()

if(USE_SYSTEM_MSPACK)
  get_filename_component(suffix ${MSPACK_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::MSPACK SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::MSPACK STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::MSPACK
    PROPERTIES
      IMPORTED_LOCATION "${MSPACK_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MSPACK_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_MSPACK_DIR})
endif()

if(USE_SYSTEM_OPENGL)
  get_filename_component(suffix ${OPENGL_gl_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::OPENGL SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::OPENGL STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::OPENGL
    PROPERTIES
      IMPORTED_LOCATION "${OPENGL_gl_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
  )
  get_filename_component(suffix ${OPENGL_glu_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::GLU SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::GLU STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::GLU
    PROPERTIES
      IMPORTED_LOCATION "${OPENGL_glu_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}"
  )
endif()

if(USE_SYSTEM_OPENSSL_CRYPTO)
  get_filename_component(suffix ${OPENSSL_CRYPTO_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::OPENSSL::CRYPTO SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::OPENSSL::CRYPTO STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::OPENSSL::CRYPTO
    PROPERTIES
      IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
  )
elseif(WITH_LIBRESSL_CRYPTO)
  add_subdirectory(${MIKTEX_REL_LIBRESSL_DIR})
  set(openssl_crypto_dll_name ${libressl_crypto_dll_name})
  set(openssl_crypto_lib_name ${libressl_crypto_lib_name})
endif()

if(USE_SYSTEM_POPPLER)
  get_filename_component(suffix ${POPPLER_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::POPPLER SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::POPPLER STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::POPPLER
    PROPERTIES
      IMPORTED_LOCATION "${POPPLER_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${POPPLER_INCLUDE_DIRS}"
  )
  if(POPPLER_VERSION VERSION_LESS 0.59.0)
    message(FATAL_ERROR "libpoppler >=0.59.0 is required.")
  else()
    set_target_properties(MiKTeX::Imported::POPPLER
      PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS MIKTEX_POPPLER_59=1
    )
  endif()
else()
  add_subdirectory(${MIKTEX_REL_POPPLER_DIR})
endif()

if(USE_SYSTEM_POPPLER_QT5 AND WITH_UI_QT)
  get_filename_component(suffix ${POPPLER_QT5_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::POPPLER_QT SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::POPPLER_QT STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::POPPLER_QT
    PROPERTIES
      IMPORTED_LOCATION "${POPPLER_QT5_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${POPPLER_QT5_INCLUDE_DIRS}"
  )
else()
#  add_subdirectory(${MIKTEX_REL_POPPLER_DIR})
endif()

if(USE_SYSTEM_POPT)
  get_filename_component(suffix ${POPT_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::POPT SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::POPT STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::POPT
    PROPERTIES
      IMPORTED_LOCATION "${POPT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${POPT_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_POPT_DIR})
endif()

if(USE_SYSTEM_URIPARSER)
  get_filename_component(suffix ${URIPARSER_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::URIPARSER SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::URIPARSER STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::URIPARSER
    PROPERTIES
      IMPORTED_LOCATION "${URIPARSER_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${URIPARSER_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_URIPARSER_DIR})
endif()

if(USE_SYSTEM_ZZIP)
  get_filename_component(suffix ${ZZIP_LIBRARY} EXT)
  if (suffix STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
    add_library(MiKTeX::Imported::ZZIP SHARED IMPORTED)
  else()
    add_library(MiKTeX::Imported::ZZIP STATIC IMPORTED)
  endif()
  set_target_properties(MiKTeX::Imported::ZZIP
    PROPERTIES
      IMPORTED_LOCATION "${ZZIP_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${ZZIP_INCLUDE_DIRS}"
  )
else()
  add_subdirectory(${MIKTEX_REL_ZZIP_DIR})
endif()

###############################################################################
## add the MiKTeX components

enable_testing()

add_subdirectory(${MIKTEX_REL_APP_DIR})
add_subdirectory(${MIKTEX_REL_AUTOSP_DIR})
add_subdirectory(${MIKTEX_REL_AXOHELP_DIR})
add_subdirectory(${MIKTEX_REL_BIBARTS_DIR})
add_subdirectory(${MIKTEX_REL_BIBTEXX_DIR})
add_subdirectory(${MIKTEX_REL_BIBTEX_DIR})
add_subdirectory(${MIKTEX_REL_BUILD_TOOLS_ETC_DIR})
add_subdirectory(${MIKTEX_REL_C4P_DIR})
add_subdirectory(${MIKTEX_REL_CFG_DIR})
add_subdirectory(${MIKTEX_REL_CHKTEX_DIR})
add_subdirectory(${MIKTEX_REL_CJKUTILS_DIR})
add_subdirectory(${MIKTEX_REL_CONFIG_DIR})
add_subdirectory(${MIKTEX_REL_CORE_DIR})
add_subdirectory(${MIKTEX_REL_CWEB_DIR})
add_subdirectory(${MIKTEX_REL_DEFAULTS_DIR})
add_subdirectory(${MIKTEX_REL_DEVNAG_DIR})
add_subdirectory(${MIKTEX_REL_DOC_DIR})
add_subdirectory(${MIKTEX_REL_DVICOPY_DIR})
add_subdirectory(${MIKTEX_REL_DVIPDFMX_DIR})
add_subdirectory(${MIKTEX_REL_DVIPNG_DIR})
add_subdirectory(${MIKTEX_REL_DVIPS_DIR})
add_subdirectory(${MIKTEX_REL_DVISVGM_DIR})
add_subdirectory(${MIKTEX_REL_EGL_REGISTRY_DIR})
add_subdirectory(${MIKTEX_REL_EPSTOPDF_DIR})
add_subdirectory(${MIKTEX_REL_EXTRACTOR_DIR})
add_subdirectory(${MIKTEX_REL_FINDTEXMF_DIR})
add_subdirectory(${MIKTEX_REL_FRIBIDIXETEX_DIR})
add_subdirectory(${MIKTEX_REL_GREGORIO_DIR})
add_subdirectory(${MIKTEX_REL_GSF2PK_DIR})
add_subdirectory(${MIKTEX_REL_INIPOOL_DIR})
add_subdirectory(${MIKTEX_REL_INITEXMF_DIR})
add_subdirectory(${MIKTEX_REL_KPSEMU_DIR})
add_subdirectory(${MIKTEX_REL_KPSEWHICH_DIR})
add_subdirectory(${MIKTEX_REL_LCDF_TYPETOOLS_DIR})
add_subdirectory(${MIKTEX_REL_LACHECK_DIR})
add_subdirectory(${MIKTEX_REL_LUATEX_DIR})
add_subdirectory(${MIKTEX_REL_LUA53_DIR})
add_subdirectory(${MIKTEX_REL_M_TX_DIR})
add_subdirectory(${MIKTEX_REL_MAKEINDEX_DIR})
add_subdirectory(${MIKTEX_REL_MAKEX_DIR})
add_subdirectory(${MIKTEX_REL_MD5WALK_DIR})
add_subdirectory(${MIKTEX_REL_MD5_DIR})
add_subdirectory(${MIKTEX_REL_MFWARE_DIR})
add_subdirectory(${MIKTEX_REL_MF_DIR})
add_subdirectory(${MIKTEX_REL_MIKTEXSETUP_DIR})
add_subdirectory(${MIKTEX_REL_MKFNTMAP_DIR})
add_subdirectory(${MIKTEX_REL_MPMLIB_DIR})
add_subdirectory(${MIKTEX_REL_MPM_DIR})
add_subdirectory(${MIKTEX_REL_MP_DIR})
add_subdirectory(${MIKTEX_REL_MTHELP_DIR})
add_subdirectory(${MIKTEX_REL_NLOHMANN_JSON_DIR})
add_subdirectory(${MIKTEX_REL_OMEGAWARE_DIR})
add_subdirectory(${MIKTEX_REL_OPENGL_REGISTRY_DIR})
add_subdirectory(${MIKTEX_REL_OTPS_DIR})
add_subdirectory(${MIKTEX_REL_PADOUT_DIR})
add_subdirectory(${MIKTEX_REL_PATGEN_DIR})
add_subdirectory(${MIKTEX_REL_PDFTEX_DIR})
add_subdirectory(${MIKTEX_REL_PMX_DIR})
add_subdirectory(${MIKTEX_REL_POPT_WRAPPER_DIR})
add_subdirectory(${MIKTEX_REL_PROGRAMS_ETC_DIR})
add_subdirectory(${MIKTEX_REL_PS2PK_DIR})
add_subdirectory(${MIKTEX_REL_PSUTILS_DIR})
add_subdirectory(${MIKTEX_REL_SETUPLIB_DIR})
add_subdirectory(${MIKTEX_REL_SYNCTEX_CLI_DIR})
add_subdirectory(${MIKTEX_REL_TDSUTIL_DIR})
add_subdirectory(${MIKTEX_REL_TECKIT_DIR})
add_subdirectory(${MIKTEX_REL_TEX4HT_DIR})
add_subdirectory(${MIKTEX_REL_TEXIFY_DIR})
add_subdirectory(${MIKTEX_REL_TEXMF_DIR})
add_subdirectory(${MIKTEX_REL_TEXWARE_DIR})
add_subdirectory(${MIKTEX_REL_TEX_DIR})
add_subdirectory(${MIKTEX_REL_TEX_ETC_DIR})
add_subdirectory(${MIKTEX_REL_TIE_DIR})
add_subdirectory(${MIKTEX_REL_TOUCH_DIR})
add_subdirectory(${MIKTEX_REL_TRACE_DIR})
add_subdirectory(${MIKTEX_REL_TTF2PK2_DIR})
add_subdirectory(${MIKTEX_REL_UTIL_DIR})
add_subdirectory(${MIKTEX_REL_W2CEMU_DIR})
add_subdirectory(${MIKTEX_REL_WEB2C_DIR})
add_subdirectory(${MIKTEX_REL_WEB_DIR})
add_subdirectory(${MIKTEX_REL_XETEX_DIR})
add_subdirectory(${MIKTEX_REL_XINDY_DIR})

if(WITH_ASYMPTOTE)
  add_subdirectory(${MIKTEX_REL_ASYMPTOTE_DIR})
endif()

if(WITH_CONFIG_FILES)
  add_subdirectory(${MIKTEX_REL_CONFIGFILES_DIR})
endif()

if(WITH_TPMS)
  add_subdirectory(${MIKTEX_REL_TPM_DIR})
endif()

if(WITH_UI_QT)
  add_subdirectory(${MIKTEX_REL_TEXWORKS_DIR})
  add_subdirectory(${MIKTEX_REL_UI_QT_DIR})
endif()

if(WITH_UI_MFC)
  add_subdirectory(${MIKTEX_REL_SETUP_MFC_DIR})
  add_subdirectory(${MIKTEX_REL_YAP_MFC_DIR})
endif()

if(WITH_MPC)
  add_subdirectory(${MIKTEX_REL_MPC_DIR})
endif()

if(WITH_COM)
  add_subdirectory(Libraries/MiKTeX/Core/COM/test)
  add_subdirectory(Libraries/MiKTeX/PackageManager/COM/test)
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_subdirectory(${MIKTEX_REL_ARCTRL_WIN_DIR})
  add_subdirectory(${MIKTEX_REL_DIB_DIR})
  add_subdirectory(${MIKTEX_REL_DVI_DIR})
  add_subdirectory(${MIKTEX_REL_MTPRINT_DIR})
  add_subdirectory(${MIKTEX_REL_UNXEMU_DIR})
  add_subdirectory(${MIKTEX_REL_UTF8WRAP_DIR})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_subdirectory(${MIKTEX_REL_LUAJIT_DIR})
endif()

if(NOT HAVE_GETOPT_LONG)
  add_subdirectory(${MIKTEX_REL_GETOPT_DIR})
endif()

if(NOT HAVE_REGEXEC)
  add_subdirectory(${MIKTEX_REL_REGEX_DIR})
endif()

if(WITH_UI_QT)
  add_subdirectory(${MIKTEX_REL_CONSOLE_QT_DIR})
endif()

###############################################################################
## configure Qt plugins (macOS)

if(MIKTEX_MACOS_BUNDLE)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    "[Paths]\nPlugins = ../PlugIns\n")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  )
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}"
  )
endif()

###############################################################################
## bootstrapping

if(WITH_BOOTSTRAPPING)
  if(DEFINED ENV{MIKTEX_REPOSITORY})
    set(default_repository $ENV{MIKTEX_REPOSITORY})
  else()
    set(default_repository "http://ctan.org/tex-archive/systems/win32/miktex/tm/packages/")
  endif()
  set(MIKTEX_DEFAULT_REPOSITORY
    ${default_repository}
    CACHE STRING "MiKTeX package repository"
  )
  if(MIKTEX_RELEASE_STATE EQUAL 1)
    set(remote_repository
      "${MIKTEX_DEFAULT_REPOSITORY}next/"
    )
  else()
    set(remote_repository
      "${MIKTEX_DEFAULT_REPOSITORY}"
    )
  endif()
  list(APPEND bootstrapping_files
    miktex-zzdb1-${MIKTEX_CONFIG_MAJOR_MINOR_STR}.tar.lzma
  )
  list(APPEND bootstrapping_files
    miktex-zzdb3-${MIKTEX_CONFIG_MAJOR_MINOR_STR}.tar.lzma
  )
  foreach(p ${MIKTEX_BOOTSTRAPPING_PACKAGES})
    list(APPEND bootstrapping_files
      ${p}.tar.lzma
    )
  endforeach()
  foreach(f ${bootstrapping_files})
    add_custom_command(
      OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/${f}
      COMMAND
        ${CURL_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${f} -fsSL ${remote_repository}/${f}
    )
    list(APPEND generated_bootstrapping_files ${CMAKE_CURRENT_BINARY_DIR}/${f})
  endforeach()
  add_custom_target(gen-bootstrapping-files ALL DEPENDS
    ${generated_bootstrapping_files}
  )
  install(
    FILES
      ${generated_bootstrapping_files}
    DESTINATION
      ${MIKTEX_BOOTSTRAPPING_DESTINATION_DIR}
  )
endif()

###############################################################################
## packaging

find_program(RPMBUILD_EXECUTABLE NAMES rpmbuild)

if(RPMBUILD_EXECUTABLE)
  list(APPEND CPACK_GENERATOR RPM)
endif()

if(UNIX AND NOT APPLE AND NOT RPMBUILD_EXECUTABLE)
  list(APPEND CPACK_GENERATOR DEB)
endif()

if(LINK_EVERYTHING_STATICALLY)
  list(APPEND CPACK_GENERATOR TGZ)
endif()

if(MIKTEX_MACOS_BUNDLE)
  set(CPACK_GENERATOR DragNDrop)
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/Packaging/cpack-config.cmake.in"
  "${CMAKE_BINARY_DIR}/Packaging/cpack-config.cmake"
  @ONLY
)

set(CPACK_PROJECT_CONFIG_FILE
  "${CMAKE_BINARY_DIR}/Packaging/cpack-config.cmake"
)

set(CPACK_PACKAGE_DESCRIPTION_FILE
  "${CMAKE_SOURCE_DIR}/Packaging/package-description.txt"
)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "MiKTeX: a scalable TeX distribution"
)

set(CPACK_PACKAGE_CONTACT
  "Christian Schenk <cs@miktex.org>"
)

set(CPACK_PACKAGE_VENDOR
  "MiKTeX.org"
)

# TODO
configure_file(
  "${CMAKE_SOURCE_DIR}/COPYING.md"
  "${CMAKE_BINARY_DIR}/COPYING.txt"
)
set(CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_BINARY_DIR}/COPYING.txt"
)

set(CPACK_PACKAGE_FILE_NAME
  "miktex-${MIKTEX_VERSION_STR}-${MIKTEX_TARGET_SYSTEM_TAG}"
)

set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

set(CPACK_DEBIAN_PACKAGE_RELEASE "${MIKTEX_LINUX_DIST_CODE_NAME}${MIKTEX_PACKAGE_REVISION}")

set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${MIKTEX_HQ_BASE_URL_NO_SLASH}")

find_program(DPKG_SHLIBDEPS_EXECUTABLE NAMES dpkg-shlibdeps)

set(debian_install_dependencies "debconf (>= 0.2.17)")
set(debian_runtime_dependencies "fontconfig")

if(DPKG_SHLIBDEPS_EXECUTABLE)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${debian_install_dependencies}, ${debian_runtime_dependencies}")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS TRUE)
else()
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "${debian_install_dependencies}, ${debian_runtime_dependencies}, libbz2-1.0, libc6 (>= 2.14), libcairo2 (>= 1.2.4), libcurl3-gnutls (>= 7.16.2), libexpat1 (>= 2.0.1), libfontconfig1 (>= 2.11), libfreetype6 (>= 2.3.9), libfribidi0 (>= 0.19.2), libgcc1 (>= 1:4.0), libgd3 (>= 2.1.0~alpha~), libgmp10, libgraphite2-3 (>= 1.2.2), libharfbuzz-icu0 (>= 0.9.9), libharfbuzz0b (>= 1.4.1), libicu57 (>= 57.1-1~), libjpeg62-turbo (>= 1.3.1), liblog4cxx10v5, liblzma5 (>= 5.1.1alpha+20120614), libmpfr4 (>= 3.1.3), libmspack0 (>= 0.4), libpixman-1-0 (>= 0.10.0), libpng16-16 (>= 1.6.2-1), libpoppler57 (>= 0.38.0), libpopt0 (>= 1.14), libssl1.0.2 (>= 1.0.2d), libstdc++6 (>= 6), liburiparser1 (>= 0.6.0), libzzip-0-13 (>= 0.13.56), zlib1g (>= 1:1.1.4)"
  )
endif()

set(CPACK_DEBIAN_PACKAGE_SECTION
  "tex"
)

#configure_file(
#  "${CMAKE_SOURCE_DIR}/Packaging/deb/config.in"
#  "${CMAKE_BINARY_DIR}/Packaging/deb/config"
#)

#list(APPEND CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#  "${CMAKE_BINARY_DIR}/Packaging/deb/config"
#)

#configure_file(
#  "${CMAKE_SOURCE_DIR}/Packaging/deb/postinst.in"
#  "${CMAKE_BINARY_DIR}/Packaging/deb/postinst"
#)

#list(APPEND CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#  "${CMAKE_BINARY_DIR}/Packaging/deb/postinst"
#)

configure_file(
  "${CMAKE_SOURCE_DIR}/Packaging/deb/prerm.in"
  "${CMAKE_BINARY_DIR}/Packaging/deb/prerm"
)

list(APPEND CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  "${CMAKE_BINARY_DIR}/Packaging/deb/prerm"
)

#configure_file(
#  "${CMAKE_SOURCE_DIR}/Packaging/deb/templates.in"
#  "${CMAKE_BINARY_DIR}/Packaging/deb/templates"
#)

#list(APPEND CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#  "${CMAKE_BINARY_DIR}/Packaging/deb/templates"
#)

set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

set(CPACK_RPM_PACKAGE_RELEASE ${MIKTEX_PACKAGE_REVISION})

if(MIKTEX_LINUX_DIST STREQUAL "centos")
  set(CPACK_RPM_PACKAGE_RELEASE_DIST ".el${MIKTEX_LINUX_DIST_VERSION}")
elseif(MIKTEX_LINUX_DIST STREQUAL "fedora")
  set(CPACK_RPM_PACKAGE_RELEASE_DIST ".fc${MIKTEX_LINUX_DIST_VERSION}")
elseif(MIKTEX_LINUX_DIST STREQUAL "opensuse")
  if(MIKTEX_LINUX_DIST_VERSION MATCHES "([0-9]+)\\.([0-9]+)")
    math(
      EXPR
      MIKTEX_LINUX_DIST_VERSION_INT
      "(10 * ${CMAKE_MATCH_1}) + ${CMAKE_MATCH_2}"
    )
  else()
    math(
      EXPR
      MIKTEX_LINUX_DIST_VERSION_INT
      "10 * ${MIKTEX_LINUX_DIST_VERSION}"
    )
  endif()
  set(CPACK_RPM_PACKAGE_RELEASE_DIST ".lp${MIKTEX_LINUX_DIST_VERSION_INT}")
endif()

set(CPACK_RPM_PACKAGE_URL "${MIKTEX_HQ_BASE_URL_NO_SLASH}")

set(CPACK_RPM_PACKAGE_AUTOREQ 1)

configure_file(
  "${CMAKE_SOURCE_DIR}/Packaging/rpm/post-install.sh.in"
  "${CMAKE_BINARY_DIR}/Packaging/rpm/post-install.sh"
)
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE
  "${CMAKE_BINARY_DIR}/Packaging/rpm/post-install.sh"
)

configure_file(
  "${CMAKE_SOURCE_DIR}/Packaging/rpm/pre-uninstall.sh.in"
  "${CMAKE_BINARY_DIR}/Packaging/rpm/pre-uninstall.sh"
)

set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
  "${CMAKE_BINARY_DIR}/Packaging/rpm/pre-uninstall.sh"
)

set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  "/usr/libexec"
  "/usr/share/applications"
  "/usr/share/man"
  "/usr/share/man/man1"
  "/usr/share/man/man5"
  "/usr/share/pixmaps"
)

set(CPACK_STRIP_SYMBOLS TRUE)

include(CPack)

if(MIKTEX_MACOS_BUNDLE AND WITH_CODE_SIGNING)
  add_custom_target(signed-package
    COMMAND
      "${CMAKE_CPACK_COMMAND}" --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
    COMMAND
      codesign -s "${MIKTEX_CODESIGN_IDENTITY}" *.dmg
  )
endif()

###############################################################################
## uninstall (see CMAKE FAQ)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE
  @ONLY
)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
