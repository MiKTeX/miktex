#!/usr/bin/env sh

set -e
set -v

runTests=$1

if [ -z $runTests ]; then
    runTests=false
    if [ ! -z $TRAVIS_TAG ]; then
	runTests=true
    fi
    if [ "$TRAVIS_EVENT_TYPE" == "cron" ]; then
	runTests=true
    fi
fi

cd "${TRAVIS_BUILD_DIR}/build"
make
make test

if [ "$runTests" == "true" ]; then
    . "${TRAVIS_BUILD_DIR}/travis-ci/osx/_install.sh"
    . "${TRAVIS_BUILD_DIR}/travis-ci/osx/_test.sh"
fi
