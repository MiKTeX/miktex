#!/usr/bin/env sh

set -ev

cd "${TRAVIS_BUILD_DIR}/build"
make
make test

. "${TRAVIS_BUILD_DIR}/travis-ci/_install.sh"
. "${TRAVIS_BUILD_DIR}/travis-ci/_test.sh"

cd "${TRAVIS_BUILD_DIR}/build"
make package
for deb in "${TRAVIS_BUILD_DIR}"/*.deb; do
    curl -sT "${deb}" chunk.io
done
