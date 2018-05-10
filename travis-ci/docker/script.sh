#!/usr/bin/env sh

set -e
set -v

dist=$1
tag=$2
runTests=$3

if [ -z $runTests ]; then
    runTests=false
    if [ ! -z $TRAVIS_TAG ]; then
	runTests=true
    fi
    if [ "$TRAVIS_EVENT_TYPE" == "cron" ]; then
	runTests=true
    fi
fi
   
mkdir -p ~/builds/$dist-$tag
docker run -t \
      -v "${TRAVIS_BUILD_DIR}:/miktex/source:ro" \
      -v ~/builds/$dist-$tag:/miktex/build:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      miktex/miktex-build-$dist:$tag
if [ "$runTests" == "true" ]; then
    mkdir -p ~/tests/$dist-$tag/test
    git clone https://github.com/MiKTeX/miktex-testing ~/tests/$dist-$tag/test-suite
    docker run -t \
      -v ~/builds/$dist-$tag:/miktex/build:ro \
      -v ~/tests/$dist-$tag/test-suite:/miktex/test-suite:ro \
      -v ~/tests/$dist-$tag/test:/miktex/test:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      miktex/miktex-test-$dist:$tag
fi
