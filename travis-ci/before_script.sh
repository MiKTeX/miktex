#!/usr/bin/env sh

set -ev

mkdir "${TRAVIS_BUILD_DIR}/build"
cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    vardir=`brew --prefix`/var
    CMAKE_PREFIX_PATH="`brew --prefix`/opt/icu4c:${CMAKE_PREFIX_PATH}" \
		     cmake .. \
		     -DMIKTEX_MPM_AUTO_ADMIN=t \
		     -DMIKTEX_MPM_AUTO_INSTALL=t \
		     -DMIKTEX_SYSTEM_VAR_CACHE_DIR=\"${vardir}/cache\" \
		     -DMIKTEX_SYSTEM_VAR_LIB_DIR=\"${vardir}/lib\"
fi
