#!/usr/bin/env sh

set -e
set -v

skip_tests=$MIKTEX_SKIP_TESTS

if [ -z $skip_tests ]; then
    skip_tests=true
    if [ ! -z $TRAVIS_TAG ]; then
	skip_tests=false
    fi
    if [ "$TRAVIS_EVENT_TYPE" = "cron" ]; then
	skip_tests=false
    fi
fi
   
cd "${TRAVIS_BUILD_DIR}/build"
make
CTEST_OUTPUT_ON_FAILURE=1 make test

if [ "$skip_tests" = "false" ]; then
    . "${TRAVIS_BUILD_DIR}/travis-ci/osx/_install.sh"
    . "${TRAVIS_BUILD_DIR}/travis-ci/osx/_test.sh"
fi
