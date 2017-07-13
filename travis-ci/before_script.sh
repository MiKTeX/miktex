#!/usr/bin/env sh

set -ev

mkdir "${TRAVIS_BUILD_DIR}/build"
cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_BRANCH}" = "next" ]; then
    relstate=1
else
    relstate=0
fi

cmakeflags=-DMIKTEX_RELEASE_STATE=${relstate}

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then
    if [ "$CXX" = "g++" ]; then export CXX="g++-5" CC="gcc-5"; fi
    mycmake="${TRAVIS_BUILD_DIR}/mycmake/bin/cmake"
    cmakeflags="-DMIKTEX_MPM_AUTO_INSTALL=t $cmakeflags"
    if [ -n $MIKTEX_BUILD_HARFBUZZ ]; then cmakeflags="-DUSE_SYSTEM_HARFBUZZ=FALSE -DUSE_SYSTEM_HARFBUZZ_ICU=FALSE $cmakeflags"; fi
    if [ -n $MIKTEX_BUILD_POPPLER ]; then cmakeflags="-DUSE_SYSTEM_POPPLER=FALSE $cmakeflags"; fi
    "${mycmake}" .. ${cmakeflags}	  
elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brewprefix="`brew --prefix`"
    etcdir="${brewprefix}/etc"
    vardir="${brewprefix}/var"
    CMAKE_PREFIX_PATH="${brewprefix}/opt/icu4c:${brewprefix}/opt/openssl:${CMAKE_PREFIX_PATH}" \
		     cmake .. $cmakeflags \
		     -DUSE_SYSTEM_MSPACK=FALSE \
		     -DUSE_SYSTEM_HARFBUZZ=FALSE \
		     -DUSE_SYSTEM_HARFBUZZ_ICU=FALSE \
		     -DMIKTEX_HOMEBREW=TRUE \
		     -DMIKTEX_MPM_AUTO_ADMIN=t \
		     -DMIKTEX_MPM_AUTO_INSTALL=t \
		     -DMIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR="${etcdir}/fonts/conf.d" \
		     -DMIKTEX_SYSTEM_VAR_CACHE_DIR="${vardir}/cache" \
		     -DMIKTEX_SYSTEM_VAR_LIB_DIR="${vardir}/lib" \
		     -DMIKTEX_SYSTEM_VAR_LOG_DIR="${vardir}/log"
fi
