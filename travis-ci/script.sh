#!/usr/bin/env sh

set -ev

cd "${TRAVIS_BUILD_DIR}/build"
make
make test
make package

. "${TRAVIS_BUILD_DIR}/travis-ci/_install.sh"
. "${TRAVIS_BUILD_DIR}/travis-ci/_test.sh"

for deb in "${TRAVIS_BUILD_DIR}"/build/*.deb; do
    curl -sT "${deb}" chunk.io
done
