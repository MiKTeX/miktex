#!/usr/bin/env sh

set -ev

mkdir "${TRAVIS_BUILD_DIR}/build"
cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    if [ "$CXX" = "g++" ]; then export CXX="g++-5" CC="gcc-5"; fi
    mycmake="${TRAVIS_BUILD_DIR}/mycmake/bin/cmake"
    cmakeflags=-DMIKTEX_MPM_AUTO_INSTALL=t
    if [ -n $MIKTEX_BUILD_HARFBUZZ ]; then cmakeflags="-DUSE_SYSTEM_HARFBUZZ=FALSE -DUSE_SYSTEM_HARFBUZZ_ICU=FALSE $cmakeflags"; fi
    if [ -n $MIKTEX_BUILD_POPPLER ]; then cmakeflags="-DUSE_SYSTEM_POPPLER=FALSE $cmakeflags"; fi
    "${mycmake}" .. ${cmakeflags}	  
elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brewprefix="`brew --prefix`"
    etcdir="${brewprefix}/etc"
    vardir="${brewprefix}/var"
    CMAKE_PREFIX_PATH="${brewprefix}/opt/icu4c:${brewprefix}/opt/openssl:${CMAKE_PREFIX_PATH}" \
		     cmake .. \
		     -DUSE_SYSTEM_MSPACK=FALSE \
		     -DMIKTEX_MPM_AUTO_ADMIN=t \
		     -DMIKTEX_MPM_AUTO_INSTALL=t \
		     -DMIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR="${etcdir}/fonts/conf.d" \
		     -DMIKTEX_SYSTEM_VAR_CACHE_DIR="${vardir}/cache" \
		     -DMIKTEX_SYSTEM_VAR_LIB_DIR="${vardir}/lib"
fi
