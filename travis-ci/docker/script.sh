#!/usr/bin/env sh

set -e
set -v

dist=$1
tag=$2

mkdir -p ~/builds/$dist-$tag
docker run -t \
      -v "${TRAVIS_BUILD_DIR}:/miktex/source:ro" \
      -v ~/builds/$dist-$tag:/miktex/build:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      miktex/miktex-build-$dist:$tag
mkdir -p ~/tests/$dist-$tag/test
git clone https://github.com/MiKTeX/miktex-testing ~/tests/$dist-$tag/test-suite
docker run -t \
      -v ~/builds/$dist-$tag:/miktex/build:ro \
      -v ~/tests/$dist-$tag/test-suite:/miktex/test-suite:ro \
      -v ~/teste/$dist-$tag/test:/miktex/test:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      miktex/miktex-test-$dist:$tag
