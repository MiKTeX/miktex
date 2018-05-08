#!/usr/bin/env sh

set -e
set -v

cd "${TRAVIS_BUILD_DIR}/build"
make
make test

. "${TRAVIS_BUILD_DIR}/travis-ci/_install.sh"
. "${TRAVIS_BUILD_DIR}/travis-ci/_test.sh"
