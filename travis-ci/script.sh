#!/usr/bin/env sh

set -ev

cd "${TRAVIS_BUILD_DIR}/build"
(export MIKTEX_MPM_AUTOINSTALL=no; make && make test)

. "${TRAVIS_BUILD_DIR}/travis-ci/_install.sh"
. "${TRAVIS_BUILD_DIR}/travis-ci/_test.sh"
