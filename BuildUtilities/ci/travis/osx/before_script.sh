#!/usr/bin/env sh

set -e
set -v

mkdir -p "${TRAVIS_BUILD_DIR}/build"
cd "${TRAVIS_BUILD_DIR}/build"

if [ "${TRAVIS_BRANCH}" = "next" ]; then
    relstate=1
else
    relstate=0
fi

cmakeflags=-DMIKTEX_RELEASE_STATE=${relstate}

brewprefix="`brew --prefix`"
CMAKE_PREFIX_PATH="${brewprefix}/opt/icu4c:${brewprefix}/opt/openssl:${brewprefix}/opt/icu4c:${brewprefix}/opt/qt:${CMAKE_PREFIX_PATH}" \
                cmake .. $cmakeflags \
                -DCMAKE_INSTALL_PREFIX="${TRAVIS_BUILD_DIR}/install" \
                -DMIKTEX_MPM_AUTO_INSTALL=t \
                -DWITH_UI_QT=TRUE
