#!/usr/bin/env sh

set -e
set -v

dist=$MIKTEX_LINUX_DIST
tag=${MIKTEX_LINUX_DIST_CODE_NAME-${MIKTEX_LINUX_DIST_VERSION}}
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
   
mkdir -p ~/builds/$dist-$tag
docker run -t \
      -v "${TRAVIS_BUILD_DIR}:/miktex/source:ro" \
      -v ~/builds/$dist-$tag:/miktex/build:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      -e CTEST_OUTPUT_ON_FAILURE=1 \
      -e MIKTEX_TRACE="$MIKTEX_TRACE" \
      miktex/miktex-build-$dist:$tag
if [ "$skip_tests" = "false" ]; then
    mkdir -p ~/tests/$dist-$tag/test
    git clone https://github.com/MiKTeX/miktex-testing ~/tests/$dist-$tag/test-suite
    docker run -t \
      -v ~/builds/$dist-$tag:/miktex/build:ro \
      -v ~/tests/$dist-$tag/test-suite:/miktex/test-suite:ro \
      -v ~/tests/$dist-$tag/test:/miktex/test:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      -e MIKTEX_CHECK_REPOSITORIES="$MIKTEX_CHECK_REPOSITORIES" \
      CTEST_OUTPUT_ON_FAILURE=1 \
      miktex/miktex-test-$dist:$tag
fi
