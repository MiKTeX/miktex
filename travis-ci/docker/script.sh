#!/usr/bin/env sh

set -e
set -v

mkdir -p ~/builds/ubuntu-bionic
docker run -t \
      -v "${TRAVIS_BUILD_DIR}:/miktex/source:ro" \
      -v ~/builds/ubuntu-bionic:/miktex/build:rw \
      -e USER_ID=`id -u` \
      -e GROUP_ID=`id -g` \
      miktex/miktex-build-ubuntu:bionic
