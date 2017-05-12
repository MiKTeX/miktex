#!/usr/bin/env sh

set -ev

cd "${TRAVIS_BUILD_DIR}/build"
make -s
make -s test

. "${TRAVIS_BUILD_DIR}/travis-ci/_install.sh"
. "${TRAVIS_BUILD_DIR}/travis-ci/_test.sh"
